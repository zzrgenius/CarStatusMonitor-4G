

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

void gyro_data_ready_cb(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
 	hal.new_gyro = 1;
 	xSemaphoreGiveFromISR( xSemaphoreMPU, &xHigherPriorityTaskWoken );
     portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	
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
#if USE_OS
	TickType_t xLastWakeTime;
 const TickType_t xFrequency = COMPASS_READ_MS;

     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
	
#endif
#ifdef COMPASS_ENABLED
    unsigned char new_compass = 0;
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
  /* Compass reads are handled by scheduler. */
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
	 HAL_NVIC_SetPriority(EXTI15_10_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	while(1)
	{
		
		 if( xSemaphoreTake( xSemaphoreMPU, LONG_TIME ) == pdTRUE )
        {
				/* It is time to execute. */
				printf("running in mpu\r\n");
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
				}
				new_data = 1;
			}
	#endif
			if (new_data) 
			{
 				//int8_t accuracy;
 				//  unsigned long data_timestamp;

				inv_execute_on_data();
				/* This function reads bias-compensated sensor data and sensor
				 * fusion outputs from the MPL. The outputs are formatted as seen
				 * in eMPL_outputs.c. This function only needs to be called at the
				 * rate requested by the host.
				 */
				hal.report =   0; 
				 
	 
			}
		
 
        }
		
//		#ifdef COMPASS_ENABLED
//        /* We're not using a data ready interrupt for the compass, so we'll
//         * make our compass reads timer-based instead.
//         */
//        if (  !hal.lp_accel_mode &&  hal.new_gyro && (hal.sensors & COMPASS_ON)) 
//		{             
//            new_compass = 1;
//        }
//		#endif
//		
//		         vTaskDelayUntil( &xLastWakeTime, xFrequency );

	}
}
 
