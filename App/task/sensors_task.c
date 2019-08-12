#include "main.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "osprintf.h"
#include "bmp280.h"
#include "sensirion_common.h"
#include "sht3x.h"
#include "bsp_led.h"
#include "eMPL_outputs.h"
#include "time.h"
#include "nmea.h"

extern uint16_t opt3001_manufacturer_id(void);
extern uint16_t opt3001_device_id(void);
extern uint8_t opt3001_get_lux(void);
extern void opt3001_config(void);
extern int8_t i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
extern int8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);

typedef struct _gpsINFO
{
    nmeaTIME utc;       /**< UTC of position */
    int     sig;        /**< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive) */
	double  lat;        /**< Latitude in NDEG - +/-[degree][min].[sec/60] */
    double  lon;        /**< Longitude in NDEG - +/-[degree][min].[sec/60] */
    double  elv;        /**< Antenna altitude above/below mean sea level (geoid) in meters */
    double  speed;      /**< Speed over the ground in kilometers/hour */
    double  direction;  /**< Track angle in degrees True */
    double  declination; /**< Magnetic variation degrees (Easterly var. subtracts from true course) */
} gpsINFO;
typedef struct    {
	int32_t temperature;
	int32_t humidity;
	double pres;
	long euler[3];
	long accel[3];
    long quat[4]; 
	gpsINFO gpsinf;
    //int nine_axis_status;
 }Sensors_output_t;
Sensors_output_t sensors_upload;
 
extern nmeaINFO info;
///*********int bmp280_test(void)
//{

 
 
//    while (1)
   
    /* printf("SHT sensor probing successful\n"); */
 
//}
  struct bmp280_dev bmp;
 long euler_data[3];
uint16_t opt3001_id;
 void GetGpsInf(void)
 {
	 taskENTER_CRITICAL();
	memcpy((uint8_t*)&sensors_upload.gpsinf.utc,(uint8_t*)&info.utc,sizeof(nmeaTIME));       /**< UTC of position */
    sensors_upload.gpsinf.sig 			= info.sig;        /**< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive) */
	sensors_upload.gpsinf.lat 			= info.lat;        /**< Latitude in NDEG - +/-[degree][min].[sec/60] */
    sensors_upload.gpsinf.lon			= info.lon	;        /**< Longitude in NDEG - +/-[degree][min].[sec/60] */
    sensors_upload.gpsinf.elv			= info.elv;        /**< Antenna altitude above/below mean sea level (geoid) in meters */
    sensors_upload.gpsinf.speed			= info.speed;      /**< Speed over the ground in kilometers/hour */
    sensors_upload.gpsinf.direction		= info.direction ;  /**< Track angle in degrees True */
    sensors_upload.gpsinf.declination	= info.declination; //
	  taskEXIT_CRITICAL();         //ÍË³öÁÙ½çÇø
 
 }
void StartTaskSensorsGet( void *pvParameters )
{
	int8_t rslt;  
	int8_t accuracy;
    unsigned long timestamp;
	  long msg, data[9];
    
    float float_data[3] = {0};

    struct bmp280_config conf;
    struct bmp280_uncomp_data ucomp_data;
//    uint32_t pres32, pres64;
    
	 
		TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1000*10;

     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
	
    /* Map the delay function pointer with the function responsible for implementing the delay */
    bmp.delay_ms = HAL_Delay;

    /* Assign device I2C address based on the status of SDO pin (GND for PRIMARY(0x76) & VDD for SECONDARY(0x77)) */
    bmp.dev_id = BMP280_I2C_ADDR_SEC;

    /* Select the interface mode as I2C */
    bmp.intf = BMP280_I2C_INTF;

    /* Map the I2C read & write function pointer with the functions responsible for I2C bus transfer */
    bmp.read = i2c_reg_read;
    bmp.write = i2c_reg_write;

    /* To enable SPI interface: comment the above 4 lines and uncomment the below 4 lines */

    /*
     * bmp.dev_id = 0;
     * bmp.read = spi_reg_read;
     * bmp.write = spi_reg_write;
     * bmp.intf = BMP280_SPI_INTF;
     */
    rslt = bmp280_init(&bmp);
   // print_rslt(" bmp280_init status", rslt);

    /* Always read the current settings before writing, especially when
     * all the configuration is not modified
     */
    rslt = bmp280_get_config(&conf, &bmp);
    //print_rslt(" bmp280_get_config status", rslt);

    /* configuring the temperature oversampling, filter coefficient and output data rate */
    /* Overwrite the desired settings */
    conf.filter = BMP280_FILTER_COEFF_2;

    /* Pressure oversampling set at 4x */
    conf.os_pres = BMP280_OS_4X;

    /* Setting the output data rate as 1HZ(1000ms) */
    conf.odr = BMP280_ODR_1000_MS;
    rslt = bmp280_set_config(&conf, &bmp);
  //  print_rslt(" bmp280_set_config status", rslt);

    /* Always set the power mode after setting the configuration */
    rslt = bmp280_set_power_mode(BMP280_NORMAL_MODE, &bmp);
  //  print_rslt(" bmp280_set_power_mode status", rslt);


//	//opt3001_config();
//	HAL_Delay(1000);
////	
////	while(1)
////	{
////		if( opt3001_manufacturer_id()== 0x5449)
////			break;
////	}

//	opt3001_id = opt3001_manufacturer_id();
//	osprintf("manufacturer id is %X\r\n",opt3001_id);

//	opt3001_id = opt3001_device_id();

//	osprintf("opt dev id is %X\r\n",opt3001_id);
 
	 while (sht3x_probe() != STATUS_OK) 
	{
         printf("SHT sensor probing failed\n"); 
    }
	while(1)
	{
		
        /* Measure temperature and relative humidity and store into variables
         * temperature, humidity (each output multiplied by 1000).
         */
		
        int8_t ret = sht3x_measure_blocking_read(&sensors_upload.temperature, &sensors_upload.humidity);
        if (ret == STATUS_OK) 
		{
             osprintf("measured temperature: %0.2f degreeCelsius, "
                      "measured humidity: %0.2f percentRH\n",
                      sensors_upload.temperature / 1000.0f,
                      sensors_upload.humidity / 1000.0f); 
        } else 
		{
            /* printf("error reading measurement\n"); */
        }
		rslt = bmp280_get_uncomp_data(&ucomp_data, &bmp);

//        /* Getting the compensated pressure using 32 bit precision */
//        rslt = bmp280_get_comp_pres_32bit(&pres32, ucomp_data.uncomp_press, &bmp);

//        /* Getting the compensated pressure using 64 bit precision */
//        rslt = bmp280_get_comp_pres_64bit(&pres64, ucomp_data.uncomp_press, &bmp);

        /* Getting the compensated pressure as floating point value */
        rslt = bmp280_get_comp_pres_double(&sensors_upload.pres, ucomp_data.uncomp_press, &bmp);
        osprintf("P: %f\r\n",sensors_upload.pres);
		
		if (inv_get_sensor_type_euler(data, &accuracy,&timestamp))
		{
			memcpy((uint8_t*)sensors_upload.euler,(uint8_t*)data,sizeof(long)*3);
			osprintf("Pitch %ld, Roll: %ld, Yaw: %ld\r\n", sensors_upload.euler[0],sensors_upload.euler[1],sensors_upload.euler[2]);

			
		}
		if (inv_get_sensor_type_quat(data, &accuracy, (inv_time_t*)&timestamp)) 
		{
			memcpy(sensors_upload.quat,(uint8_t*)data,sizeof(long)*4);
			osprintf(" W, %ld X, %ld Y, %ld Z  %ld\r\n", sensors_upload.quat[0],sensors_upload.quat[1],sensors_upload.quat[2],sensors_upload.quat[3]);

		}
      if (inv_get_sensor_type_accel(data, &accuracy,        (inv_time_t*)&timestamp))
	  {
		  memcpy(sensors_upload.accel,(uint8_t*)data,sizeof(long)*3);
		  osprintf("Acceleration (g's) x %ld y %ld z %ld\r\n", sensors_upload.accel[0],sensors_upload.accel[1],sensors_upload.accel[2]);

//		  eMPL_send_data(PACKET_DATA_ACCEL, data);
	  }
	   GetGpsInf();
	  osprintf("utc time is %d:%d:%d\r\n", sensors_upload.gpsinf.utc.hour,sensors_upload.gpsinf.utc.min,sensors_upload.gpsinf.utc.sec );


		//opt_res = opt3001_get_lux();
	//	printf("opt_res is %d",opt_res);
		LED_Toggle(LED4); 
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}
	
 