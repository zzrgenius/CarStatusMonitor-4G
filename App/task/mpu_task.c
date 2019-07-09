

/**
  ******************************************************************************
  * @file    serial_task.c
  * @author  LoryTech HARDWARE TEAM
  * @version V1.1.0
  * @date    2018-10-08
  * @brief   This file provides serial task.
  ******************************************************************************
  * @attention
  * Copyright (c) LoryTech. 
  * All rights reserved.
  *
 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"


#include "bsp_serial.h"
#include "bsp_led.h"
#include "mpu9250_driver.h"
#include "mpu9250_app.h"

#include "inv_mpu.h" 
#include "inv_mpu_dmp_motion_driver.h"
#include "invensense.h"
#include "invensense_adv.h"
#include "eMPL_outputs.h" 
#include "mltypes.h"
#include "mpu.h"
#include "log.h"
#include "mpu9250_cfg.h"
#include "osprintf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
 /* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint32_t hal_timestamp = 0;
unsigned char *mpl_key = (unsigned char*)"eMPL 5.1";

static struct hal_s hal = {0};
/* USB RX binary semaphore. Actually, it's just a flag. Not included in struct
 * because it's declared extern elsewhere.
 */
volatile unsigned char rx_new;

 void alram_acc(uint8_t f_data);

/* Platform-specific information. Kinda like a boardfile. */
struct platform_data_s {
    signed char orientation[9];
};
/* The sensors can be mounted onto the board in any orientation. The mounting
 * matrix seen below tells the MPL how to rotate the raw data from the
 * driver(s).
 * TODO: The following matrices refer to the configuration on internal test
 * boards at Invensense. If needed, please modify the matrices to match the
 * chip-to-body matrix for your particular set up.
 */
static struct platform_data_s gyro_pdata = {
    .orientation = { 1, 0, 0,
                     0, 1, 0,
                     0, 0, 1}
};

#if defined MPU9150 || defined MPU9250
static struct platform_data_s compass_pdata = {
    .orientation = { 0, 1, 0,
                     1, 0, 0,
                     0, 0, -1}
};
#define COMPASS_ENABLED 1
#elif defined AK8975_SECONDARY
static struct platform_data_s compass_pdata = {
    .orientation = {-1, 0, 0,
                     0, 1, 0,
                     0, 0,-1}
};
#define COMPASS_ENABLED 1
#elif defined AK8963_SECONDARY
static struct platform_data_s compass_pdata = {
    .orientation = {-1, 0, 0,
                     0,-1, 0,
                     0, 0, 1}
};
#define COMPASS_ENABLED 1
#endif
 /* Private function prototypes -----------------------------------------------*/
SemaphoreHandle_t xSemaphoreMPU;
#ifdef COMPASS_ENABLED
    unsigned char new_compass = 0;
#endif
void gyro_data_ready_cb(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
 	hal.new_gyro = 1;
 	xSemaphoreGiveFromISR( xSemaphoreMPU, &xHigherPriorityTaskWoken );
     portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	
}
static void tap_cb(unsigned char direction, unsigned char count)
{
    switch (direction) {
    case TAP_X_UP:
        MPL_LOGI("Tap X+ ");
        break;
    case TAP_X_DOWN:
        MPL_LOGI("Tap X- ");
        break;
    case TAP_Y_UP:
        MPL_LOGI("Tap Y+ ");
        break;
    case TAP_Y_DOWN:
        MPL_LOGI("Tap Y- ");
        break;
    case TAP_Z_UP:
        MPL_LOGI("Tap Z+ ");
        break;
    case TAP_Z_DOWN:
        MPL_LOGI("Tap Z- ");
        break;
    default:
        return;
    }
    MPL_LOGI("x%d\n", count);
    return;
}
/* Handle sensor on/off combinations. */
static void setup_gyro(void)
{
    unsigned char mask = 0, lp_accel_was_on = 0;
    if (hal.sensors & ACCEL_ON)
        mask |= INV_XYZ_ACCEL;
    if (hal.sensors & GYRO_ON) {
        mask |= INV_XYZ_GYRO;
        lp_accel_was_on |= hal.lp_accel_mode;
    }
#ifdef COMPASS_ENABLED
    if (hal.sensors & COMPASS_ON) {
        mask |= INV_XYZ_COMPASS;
        lp_accel_was_on |= hal.lp_accel_mode;
    }
#endif
    /* If you need a power transition, this function should be called with a
     * mask of the sensors still enabled. The driver turns off any sensors
     * excluded from this mask.
     */
    mpu_set_sensors(mask);
    mpu_configure_fifo(mask);
    if (lp_accel_was_on) {
        unsigned short rate;
        hal.lp_accel_mode = 0;
        /* Switching out of LP accel, notify MPL of new accel sampling rate. */
        mpu_get_sample_rate(&rate);
        inv_set_accel_sample_rate(1000000L / rate);
    }
}

static inline void run_self_test(void)
{
    int result;
    long gyro[3], accel[3];

#if defined (MPU6500) || defined (MPU9250)
    result = mpu_run_6500_self_test(gyro, accel, 0);
#elif defined (MPU6050) || defined (MPU9150)
    result = mpu_run_self_test(gyro, accel);
#endif
    if (result == 0x7) {
	MPL_LOGI("Passed!\n");
        MPL_LOGI("accel: %7.4f %7.4f %7.4f\n",
                    accel[0]/65536.f,
                    accel[1]/65536.f,
                    accel[2]/65536.f);
        MPL_LOGI("gyro: %7.4f %7.4f %7.4f\n",
                    gyro[0]/65536.f,
                    gyro[1]/65536.f,
                    gyro[2]/65536.f);
        /* Test passed. We can trust the gyro data here, so now we need to update calibrated data*/

#ifdef USE_CAL_HW_REGISTERS
        /*
         * This portion of the code uses the HW offset registers that are in the MPUxxxx devices
         * instead of pushing the cal data to the MPL software library
         */
        unsigned char i = 0;

        for(i = 0; i<3; i++) {
        	gyro[i] = (long)(gyro[i] * 32.8f); //convert to +-1000dps
        	accel[i] *= 2048.f; //convert to +-16G
        	accel[i] = accel[i] >> 16;
        	gyro[i] = (long)(gyro[i] >> 16);
        }

        mpu_set_gyro_bias_reg(gyro);

#if defined (MPU6500) || defined (MPU9250)
        mpu_set_accel_bias_6500_reg(accel);
#elif defined (MPU6050) || defined (MPU9150)
        mpu_set_accel_bias_6050_reg(accel);
#endif
#else
        /* Push the calibrated data to the MPL library.
         *
         * MPL expects biases in hardware units << 16, but self test returns
		 * biases in g's << 16.
		 */
    	unsigned short accel_sens;
    	float gyro_sens;

		mpu_get_accel_sens(&accel_sens);
		accel[0] *= accel_sens;
		accel[1] *= accel_sens;
		accel[2] *= accel_sens;
		inv_set_accel_bias(accel, 3);
		mpu_get_gyro_sens(&gyro_sens);
		gyro[0] = (long) (gyro[0] * gyro_sens);
		gyro[1] = (long) (gyro[1] * gyro_sens);
		gyro[2] = (long) (gyro[2] * gyro_sens);
		inv_set_gyro_bias(gyro, 3);
#endif
    }
    else {
            if (!(result & 0x1))
                MPL_LOGE("Gyro failed.\n");
            if (!(result & 0x2))
                MPL_LOGE("Accel failed.\n");
            if (!(result & 0x4))
                MPL_LOGE("Compass failed.\n");
     }

}

void MPU_Config(void)
{
	
}
 

void StartTaskMPU(void const * argument)
{
			inv_error_t result;
	    struct int_param_s int_param;
	unsigned char accel_fsr,  new_temp = 0;
    unsigned short gyro_rate, gyro_fsr;
	    unsigned long timestamp;
	uint16_t count_ticks = 0;

#ifdef COMPASS_ENABLED
     
    unsigned short compass_fsr;
#endif

	
	  result = mpu_init(&int_param);
  if (result) {
      MPL_LOGE("Could not initialize gyro.\n");
  }
   result = inv_init_mpl();
  if (result) {
      MPL_LOGE("Could not initialize MPL.\n");
  }
 /* Compute 6-axis and 9-axis quaternions. */
    inv_enable_quaternion();
    inv_enable_9x_sensor_fusion();
    /* Update gyro biases when not in motion.
     * WARNING: These algorithms are mutually exclusive.
     */
    inv_enable_fast_nomot();
     /* Update gyro biases when temperature changes. */
    inv_enable_gyro_tc();
#ifdef COMPASS_ENABLED
    /* Compass calibration algorithms. */
    inv_enable_vector_compass_cal();
    inv_enable_magnetic_disturbance();
#endif
  // inv_enable_eMPL_outputs();
//    result = inv_start_mpl();
//  if (result == INV_ERROR_NOT_AUTHORIZED) {
//      while (1) {
//          MPL_LOGE("Not authorized.\n");
//      }
//  }
//  if (result) {
//      MPL_LOGE("Could not start the MPL.\n");
//  }

#ifdef COMPASS_ENABLED
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
#else
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
#endif
      /* Get/set hardware configuration. Start gyro. */
    /* Wake up all sensors. */
#ifdef COMPASS_ENABLED
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
#else
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
#endif
    /* Push both gyro and accel data into the FIFO. */
    mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    mpu_set_sample_rate(DEFAULT_MPU_HZ);
#ifdef COMPASS_ENABLED
    /* The compass sampling rate can be less than the gyro/accel sampling rate.
     * Use this function for proper power management.
     */
    mpu_set_compass_sample_rate(1000 / COMPASS_READ_MS);
#endif
    /* Read back configuration in case it was set improperly. */
    mpu_get_sample_rate(&gyro_rate);
    mpu_get_gyro_fsr(&gyro_fsr);
    mpu_get_accel_fsr(&accel_fsr);
#ifdef COMPASS_ENABLED
    mpu_get_compass_fsr(&compass_fsr);
#endif

 /* Sync driver configuration with MPL. */
    /* Sample rate expected in microseconds. */
    inv_set_gyro_sample_rate(1000000L / gyro_rate);
    inv_set_accel_sample_rate(1000000L / gyro_rate);
#ifdef COMPASS_ENABLED
    /* The compass rate is independent of the gyro and accel rates. As long as
     * inv_set_compass_sample_rate is called with the correct value, the 9-axis
     * fusion algorithm's compass correction gain will work properly.
     */
    inv_set_compass_sample_rate(COMPASS_READ_MS * 1000L);
#endif
    /* Set chip-to-body orientation matrix.
     * Set hardware units to dps/g's/degrees scaling factor.
     */
    inv_set_gyro_orientation_and_scale(
            inv_orientation_matrix_to_scalar(gyro_pdata.orientation),
            (long)gyro_fsr<<15);
    inv_set_accel_orientation_and_scale(
            inv_orientation_matrix_to_scalar(gyro_pdata.orientation),
            (long)accel_fsr<<15);
#ifdef COMPASS_ENABLED
    inv_set_compass_orientation_and_scale(
            inv_orientation_matrix_to_scalar(compass_pdata.orientation),
            (long)compass_fsr<<15);
#endif
    /* Initialize HAL state variables. */
#ifdef COMPASS_ENABLED
    hal.sensors = ACCEL_ON | GYRO_ON | COMPASS_ON;
#else
    hal.sensors = ACCEL_ON | GYRO_ON;
#endif
    hal.dmp_on = 0;
    hal.report = 0;
    hal.rx.cmd = 0;
    hal.next_pedo_ms = 0;
    hal.next_compass_ms = 0;
    hal.next_temp_ms = 0;
	  hal.report |=  PRINT_EULER;
  /* To initialize the DMP:
     * 1. Call dmp_load_motion_driver_firmware(). This pushes the DMP image in
     *    inv_mpu_dmp_motion_driver.h into the MPU memory.
     * 2. Push the gyro and accel orientation matrix to the DMP.
     * 3. Register gesture callbacks. Don't worry, these callbacks won't be
     *    executed unless the corresponding feature is enabled.
     * 4. Call dmp_enable_feature(mask) to enable different features.
     * 5. Call dmp_set_fifo_rate(freq) to select a DMP output rate.
     * 6. Call any feature-specific control functions.
     *
     * To enable the DMP, just call mpu_set_dmp_state(1). This function can
     * be called repeatedly to enable and disable the DMP at runtime.
     *
     * The following is a short summary of the features supported in the DMP
     * image provided in inv_mpu_dmp_motion_driver.c:
     * DMP_FEATURE_LP_QUAT: Generate a gyro-only quaternion on the DMP at
     * 200Hz. Integrating the gyro data at higher rates reduces numerical
     * errors (compared to integration on the MCU at a lower sampling rate).
     * DMP_FEATURE_6X_LP_QUAT: Generate a gyro/accel quaternion on the DMP at
     * 200Hz. Cannot be used in combination with DMP_FEATURE_LP_QUAT.
     * DMP_FEATURE_TAP: Detect taps along the X, Y, and Z axes.
     * DMP_FEATURE_ANDROID_ORIENT: Google's screen rotation algorithm. Triggers
     * an event at the four orientations where the screen should rotate.
     * DMP_FEATURE_GYRO_CAL: Calibrates the gyro data after eight seconds of
     * no motion.
     * DMP_FEATURE_SEND_RAW_ACCEL: Add raw accelerometer data to the FIFO.
     * DMP_FEATURE_SEND_RAW_GYRO: Add raw gyro data to the FIFO.
     * DMP_FEATURE_SEND_CAL_GYRO: Add calibrated gyro data to the FIFO. Cannot
     * be used in combination with DMP_FEATURE_SEND_RAW_GYRO.
     */
    dmp_load_motion_driver_firmware();
    dmp_set_orientation(
        inv_orientation_matrix_to_scalar(gyro_pdata.orientation));
    dmp_register_tap_cb(tap_cb);
  //  dmp_register_android_orient_cb(android_orient_cb);
    /*
     * Known Bug -
     * DMP when enabled will sample sensor data at 200Hz and output to FIFO at the rate
     * specified in the dmp_set_fifo_rate API. The DMP will then sent an interrupt once
     * a sample has been put into the FIFO. Therefore if the dmp_set_fifo_rate is at 25Hz
     * there will be a 25Hz interrupt from the MPU device.
     *
     * There is a known issue in which if you do not enable DMP_FEATURE_TAP
     * then the interrupts will be at 200Hz even if fifo rate
     * is set at a different rate. To avoid this issue include the DMP_FEATURE_TAP
     *
     * DMP sensor fusion works only with gyro at +-2000dps and accel +-2G
     */
    hal.dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
        DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
        DMP_FEATURE_GYRO_CAL;
    dmp_enable_feature(hal.dmp_features);
    dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    mpu_set_dmp_state(1);
    hal.dmp_on = 1;
	
	  
            MPL_LOGI("DMP enabled.\n");
			
   get_tick_count(&timestamp); 
    int new_data = 0;
    unsigned long sensor_timestamp;
	#if USE_OS 
	xSemaphoreMPU =   xSemaphoreCreateBinary();
	 if( xSemaphoreMPU == NULL )
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to
        be created successfully. */
    }
    else
    {
        /* The semaphore can now be used. Its handle is stored in the
        xSemahore variable.  Calling xSemaphoreTake() on the semaphore here
        will fail until the semaphore has first been given. */
    }
	#endif
	//	run_self_test();

	        setup_gyro();
	 //HAL_NVIC_SetPriority(EXTI15_10_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
 				int8_t accuracy;
				  unsigned long data_timestamp;
				float float_data[3];
	while(1)
	{
		 get_tick_count(&timestamp);		
		 if( xSemaphoreTake( xSemaphoreMPU, LONG_TIME ) == pdTRUE )
        {
				/* It is time to execute. */
				//osprintf("running in mpu\r\n");
				count_ticks++;
				if(count_ticks == 100)
				{
					#ifdef COMPASS_ENABLED
					if (  !hal.lp_accel_mode &&  hal.new_gyro && (hal.sensors & COMPASS_ON)) 
					{             
						new_compass = 1;
						new_temp = 1;
						osprintf("timestamp is %ld new_compass is %d",timestamp,new_compass);
//						if (inv_get_sensor_type_linear_acceleration(float_data, &accuracy, (inv_time_t*)&data_timestamp)) 
//						{
//							osprintf("linear acceleration:\r\nx is %f\r\ny is %f\r\nz is %f\r\n",float_data[0],float_data[1],float_data[2]);
//						}
					}
					#endif
					count_ticks = 0;
//					if (inv_get_sensor_type_linear_acceleration(float_data, &accuracy, (inv_time_t*)&data_timestamp)) 
//					{
//						osprintf("linear acceleration:\r\nx is %f\r\ny is %f\r\nz is %f\r\n",float_data[0],float_data[1],float_data[2]);
//					}

					
				}
				if (hal.new_gyro && hal.lp_accel_mode) 
				{
					short accel_short[3];
					long accel[3];
					mpu_get_accel_reg(accel_short, &sensor_timestamp);
					accel[0] = (long)accel_short[0];
					accel[1] = (long)accel_short[1];
					accel[2] = (long)accel_short[2];
					inv_build_accel(accel, 0, sensor_timestamp);
					new_data = 1;
					hal.new_gyro = 0;
				} else if (hal.new_gyro && hal.dmp_on) 
				{
					short gyro[3], accel_short[3], sensors;
					unsigned char more;
					long accel[3], quat[4], temperature;
					/* This function gets new data from the FIFO when the DMP is in
					 * use. The FIFO can contain any combination of gyro, accel,
					 * quaternion, and gesture data. The sensors parameter tells the
					 * caller which data fields were actually populated with new data.
					 * For example, if sensors == (INV_XYZ_GYRO | INV_WXYZ_QUAT), then
					 * the FIFO isn't being filled with accel data.
					 * The driver parses the gesture data to determine if a gesture
					 * event has occurred; on an event, the application will be notified
					 * via a callback (assuming that a callback function was properly
					 * registered). The more parameter is non-zero if there are
					 * leftover packets in the FIFO.
					 */
					dmp_read_fifo(gyro, accel_short, quat, &sensor_timestamp, &sensors, &more);
					if (!more)
						hal.new_gyro = 0;
					if (sensors & INV_XYZ_GYRO) {
						/* Push the new data to the MPL. */
						inv_build_gyro(gyro, sensor_timestamp);
						new_data = 1;
						if (new_temp) {
							new_temp = 0;
							/* Temperature only used for gyro temp comp. */
							mpu_get_temperature(&temperature, &sensor_timestamp);
							inv_build_temp(temperature, sensor_timestamp);
						}
					}
					if (sensors & INV_XYZ_ACCEL) {
						accel[0] = (long)accel_short[0];
						accel[1] = (long)accel_short[1];
						accel[2] = (long)accel_short[2];
						inv_build_accel(accel, 0, sensor_timestamp);
						new_data = 1;
					}
					if (sensors & INV_WXYZ_QUAT) {
						inv_build_quat(quat, 0, sensor_timestamp);
						new_data = 1;
					}
				} else if (hal.new_gyro) 
				{
					short gyro[3], accel_short[3];
					unsigned char sensors, more;
					long accel[3], temperature;
					/* This function gets new data from the FIFO. The FIFO can contain
					 * gyro, accel, both, or neither. The sensors parameter tells the
					 * caller which data fields were actually populated with new data.
					 * For example, if sensors == INV_XYZ_GYRO, then the FIFO isn't
					 * being filled with accel data. The more parameter is non-zero if
					 * there are leftover packets in the FIFO. The HAL can use this
					 * information to increase the frequency at which this function is
					 * called.
					 */
					hal.new_gyro = 0;
					mpu_read_fifo(gyro, accel_short, &sensor_timestamp,
						&sensors, &more);
					if (more)
						hal.new_gyro = 1;
					if (sensors & INV_XYZ_GYRO) {
						/* Push the new data to the MPL. */
						inv_build_gyro(gyro, sensor_timestamp);
						new_data = 1;
						if (new_temp) {
							new_temp = 0;
							/* Temperature only used for gyro temp comp. */
							mpu_get_temperature(&temperature, &sensor_timestamp);
							inv_build_temp(temperature, sensor_timestamp);
						}
					}
					if (sensors & INV_XYZ_ACCEL) {
						accel[0] = (long)accel_short[0];
						accel[1] = (long)accel_short[1];
						accel[2] = (long)accel_short[2];
						inv_build_accel(accel, 0, sensor_timestamp);
						new_data = 1;
					}
				}
					#ifdef COMPASS_ENABLED
			if (new_compass) 
			{
				short compass_short[3];
				long compass[3];
				new_compass = 0;
				/* For any MPU device with an AKM on the auxiliary I2C bus, the raw
				 * magnetometer registers are copied to special gyro registers.
				 */
				if (!mpu_get_compass_reg(compass_short, &sensor_timestamp)) {
					compass[0] = (long)compass_short[0];
					compass[1] = (long)compass_short[1];
					compass[2] = (long)compass_short[2];
					/* NOTE: If using a third-party compass calibration library,
					 * pass in the compass data in uT * 2^16 and set the second
					 * parameter to INV_CALIBRATED | acc, where acc is the
					 * accuracy from 0 to 3.
					 */
					inv_build_compass(compass, 0, sensor_timestamp);
					osprintf("compass is %d,%d,%d",compass_short[0],compass_short[1],compass_short[2]);
				}
				new_data = 1;
			}
			#endif
			
			if (new_data) 
			{

				inv_execute_on_data();
				new_data = 0;
				/* This function reads bias-compensated sensor data and sensor
				 * fusion outputs from the MPL. The outputs are formatted as seen
				 * in eMPL_outputs.c. This function only needs to be called at the
				 * rate requested by the host.
				 */
				hal.report =   0; 
				
	 
			}
		
 
        }
		
//		
//        /* We're not using a data ready interrupt for the compass, so we'll
//         * make our compass reads timer-based instead.
//         */


	}
}
  void StartTaskSensorsGet(void const * argument)
  {
	      int new_data = 0;
    unsigned long sensor_timestamp;
#if USE_OS
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = COMPASS_READ_MS*10;

     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
	
#endif
	  
	  while(1)
	  {
		LED_Toggle(LED4); 
		  vTaskDelayUntil( &xLastWakeTime, xFrequency );
	  }        
	  
  }
