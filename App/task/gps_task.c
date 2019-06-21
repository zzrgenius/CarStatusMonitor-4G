
/**
  ******************************************************************************
  * @file    gps_task.c
  * @author  LoryTech HARDWARE TEAM
  * @version V1.1.0
  * @date    2018-10-12
  * @brief   This file provides all the RCC firmware functions.
  ******************************************************************************
  * @attention
  * Copyright (c) LoryTech. 
  * All rights reserved.
  *
 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#include "FreeRTOS.h"
//#include "task.h"
#include "cmsis_os.h"

#include "string.h"
#include "stdio.h"

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>
//#include <fcntl.h>
#include <ctype.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "nmea.h"
#include "bsp_serial.h"
#include "bsp_led.h"
#include "gps_cfg.h"
//#include "gps.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

//#define USE_GPS_POWER_SWITCH
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern UART_HandleTypeDef huart3;

#define GPS_UART_Handle   huart3
 
 
nmeaINFO info;
nmeaPARSER parser;
 

//static gpsData_t gpsData;
static xSemaphoreHandle gps_semaphore = NULL;
TaskHandle_t taskHandle_GPS;//TASKHANDLE_GPS;
char gGPSBuf[GPS_MAX_NMEA_SENTENCE];
uint8_t gps_recdata;
  
 
 
/* Private function prototypes -----------------------------------------------*/

#ifdef USE_GPS_POWER_SWITCH
void enable_gps_power(void)
{

	HAL_GPIO_WritePin( GPS_POWER_GPIO_Port,GPS_POWER_Pin, GPIO_PIN_RESET );

}
#endif
void HAL_GpsInit( void )
{
   //gps_init(&g_hgps);                             /* Init GPS */
    /* Create buffer for received data */
   // gps_buff_init(&hgps_buff, hgps_buff_data, sizeof(hgps_buff_data));
  	
//	g_GPSDataProcQueue = xQueueCreate(1,GPS_BUF_LEN); 
//	if(g_GPSDataProcQueue == NULL)
//	{
//		__nop();
//	}
	#ifdef USE_GPS_POWER_SWITCH
	enable_gps_power();
	#endif
//	__HAL_DMA_ENABLE_IT(huart3.hdmarx, (DMA_IT_TC | DMA_IT_TE));

//	HAL_UART_Receive_DMA(&GPS_UART_Handle,gps_buf,GPS_BUF_LEN);
	 
}


void vStartTaskGPS( void *pvParameters )
{
	HAL_GpsInit();
	nmea_zero_INFO(&info);
    nmea_parser_init(&parser);
	 if (!gps_semaphore)
		vSemaphoreCreateBinary (gps_semaphore);
	 HAL_UART_Receive_IT(&GPS_UART_Handle,&gps_recdata,1);
  
	while(1)
	{
			/* See if we can obtain the semaphore.  If the semaphore is not
			available wait 10 ticks to see if it becomes free. */
			//if( xSemaphoreTake( xBinarySemGPS, ( TickType_t ) portMAX_DELAY ) == pdTRUE )
		if(	osSemaphoreWait(gps_semaphore,osWaitForever))
		     //if( xQueueReceive( g_GPSDataProcQueue, ( gGPSBuf ), ( TickType_t ) 100 ) )
		{
			/* We were able to obtain the semaphore and can now access the
			shared resource. */
			/* ... */
			LED_Toggle(LED1);
			 
		//	printf("Latitude: %f degrees\r\n", g_hgps.latitude);
			nmea_parse(&parser,(const char*)gGPSBuf,GPS_MAX_NMEA_SENTENCE, &info); 

			//__HAL_DMA_ENABLE_IT(huart3.hdmarx, (DMA_IT_TC | DMA_IT_TE));
			//HAL_UART_Receive_DMA(&GPS_UART_Handle,gps_buf,GPS_BUF_LEN);
		
			osSemaphoreRelease (gps_semaphore);

			/* We have finished accessing the shared resource.  Release the semaphore. */
			
		}
		else
		{
			
		}
		 
		
	}
}


 
 
 
//    if (xSemaphoreTake (semaphore, portMAX_DELAY) == pdTRUE)
//    {
//      gpsData.valid = (valid - '0');
//      gpsData.height = height;
//      gpsData.latitude  = latitude * (latitudeSign == 'N' ? 1.0 : -1.0);
//      gpsData.longitude = longitude * (longitudeSign == 'E' ? 1.0 : -1.0);

//      xSemaphoreGive (semaphore);
//    }
 
//   static char nmeaSentence [GPS_MAX_NMEA_SENTENCE];
static unsigned char gpsChecksumNMEA (char *sz)
{
  short i;
  unsigned char cs;

  for (cs = 0, i = 1; sz [i] && sz [i] != '*'; i++)
    cs ^= ((unsigned char) sz [i]);

  return cs;
}
static int hatoi (const char *s)
{
  const unsigned char hexToDec [] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255, 255, 255, 10, 11, 12, 13, 14, 15};
  int value = 0;

  while (*s && isxdigit (*s))
    value = (value << 4) | hexToDec [toupper (*s++) - '0'];

  return value;
}
 
  int gpsProcessByte (unsigned char c, char *nmeaSentence)
{
  short complete = 0;
  static short state = 0;
  static short pos = 0;

  switch (state)
  {
    case 0 :
      {
        if (c == '$')
        {
          pos = 0;
          nmeaSentence [pos++] = '$';
          nmeaSentence [pos] = '\0';
          state = 1;
        }
        else
          state = 0;
      }
      break;

    case 1 :
      {
        if (pos < GPS_MAX_NMEA_SENTENCE)
        {
          if (c == 0x0a)
          {
            char *s;

            state = 0;

            if ((s = strchr (nmeaSentence, '*')))
            {
              int cksum;
              if (gpsChecksumNMEA (nmeaSentence) == (cksum = hatoi (s + 1)))
			  {
				  //complete = 1;
				  osSemaphoreRelease (gps_semaphore);

			  }
			  else
			  {
//                printf ("NMEA checksum error: got 0x%02x, want %s", cksum, s);
				  
			  }
            }
            else
			{
				
              //printf ("NMEA checksum not found: \"%s\"", nmeaSentence);
			}
          }
          else if (c != 0x0d)
          {
            nmeaSentence [pos++] = c;
            nmeaSentence [pos] = '\0';
          }
        }
        else
          state = 0;
      }
      break;
  }

  return (complete);
}
 
 BaseType_t gpsTaskStart (void)
{
	BaseType_t xReturned;

   xReturned = xTaskCreate (vStartTaskGPS, "GPSTask", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1), &taskHandle_GPS);  
	if( xReturned == pdPASS )
    {
        /* The task was created.  Use the task's handle to delete the task. */
        vTaskDelete( taskHandle_GPS );
    }
}

BaseType_t gpsTaskStop (void)
{
 
  
  vTaskDelete (taskHandle_GPS);
  taskHandle_GPS  = NULL;
  return 1;
}

