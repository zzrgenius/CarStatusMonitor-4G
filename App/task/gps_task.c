
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
 
//#include "gps.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define GPS_BUF_LEN	256
//#define USE_GPS_POWER_SWITCH
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
QueueHandle_t g_GPSDataProcQueue;  
extern UART_HandleTypeDef huart3;

#define GPS_UART_Handle   huart3
uint8_t gps_buf[GPS_BUF_LEN];

extern uint8_t usart3_rx_buf[USART_BUF_SIZE];

extern  DMA_HandleTypeDef hdma_usart3_rx; 

nmeaINFO info;
nmeaPARSER parser;
osSemaphoreId  gpsSemaphoreHandle;
osSemaphoreDef_t gpsSemaphore;
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
	osSemaphoreDef(gpsSemaphore);
	gpsSemaphoreHandle =  osSemaphoreCreate (osSemaphore(gpsSemaphore),  0);
	
//	g_GPSDataProcQueue = xQueueCreate(1,GPS_BUF_LEN); 
//	if(g_GPSDataProcQueue == NULL)
//	{
//		__nop();
//	}
	#ifdef USE_GPS_POWER_SWITCH
	enable_gps_power();
	#endif
	__HAL_DMA_ENABLE_IT(huart3.hdmarx, (DMA_IT_TC | DMA_IT_TE));

	HAL_UART_Receive_DMA(&GPS_UART_Handle,gps_buf,GPS_BUF_LEN);
}

char* gGPSBuf[USART_BUF_SIZE];

  
void vStartTaskGPS( void *pvParameters )
{
	HAL_GpsInit();
	nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

	while(1)
	{
			/* See if we can obtain the semaphore.  If the semaphore is not
			available wait 10 ticks to see if it becomes free. */
			//if( xSemaphoreTake( xBinarySemGPS, ( TickType_t ) portMAX_DELAY ) == pdTRUE )
		if(	osSemaphoreWait(gpsSemaphoreHandle,osWaitForever))
		     //if( xQueueReceive( g_GPSDataProcQueue, ( gGPSBuf ), ( TickType_t ) 100 ) )
			{
				/* We were able to obtain the semaphore and can now access the
				shared resource. */
				/* ... */
				LED_Toggle(LED1);
				 
			//	printf("Latitude: %f degrees\r\n", g_hgps.latitude);
				nmea_parse(&parser,(const char*)gGPSBuf,GPS_BUF_LEN, &info); 

				//__HAL_DMA_ENABLE_IT(huart3.hdmarx, (DMA_IT_TC | DMA_IT_TE));
				HAL_UART_Receive_DMA(&GPS_UART_Handle,gps_buf,GPS_BUF_LEN);
			
				osSemaphoreRelease (gpsSemaphoreHandle);

				/* We have finished accessing the shared resource.  Release the semaphore. */
				
			}
		 
		
	}
}

#define GPS_MAX_NMEA_SENTENCE 128

 
 

 
//
//
//
//static gpsData_t gpsData;
static xSemaphoreHandle semaphore = NULL;
TaskHandle_t taskHandle_GPS;//TASKHANDLE_GPS;
 
 
 
 
 
//    if (xSemaphoreTake (semaphore, portMAX_DELAY) == pdTRUE)
//    {
//      gpsData.valid = (valid - '0');
//      gpsData.height = height;
//      gpsData.latitude  = latitude * (latitudeSign == 'N' ? 1.0 : -1.0);
//      gpsData.longitude = longitude * (longitudeSign == 'E' ? 1.0 : -1.0);

//      xSemaphoreGive (semaphore);
//    }
 
 
 
static int gpsProcessByte (unsigned char c, char *nmeaSentence)
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

//              if (gpsChecksumNMEA (nmeaSentence) == (cksum = hatoi (s + 1)))
//                complete = 1;
//              else
//                printf ("NMEA checksum error: got 0x%02x, want %s", cksum, s);
            }
            else
              printf ("NMEA checksum not found: \"%s\"", nmeaSentence);
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

//
//  Return 1 if got a copy, 0 if not.
//
//int gpsCopyData (gpsData_t *dst)
//{
//  if (semaphore && xSemaphoreTake (semaphore, 100 / portTICK_RATE_MS) == pdTRUE)
//  {
//    memcpy (dst, &gpsData, sizeof (gpsData_t));
//    xSemaphoreGive (semaphore);
//    return 1;
//  }

//  memset (dst, 0, sizeof (gpsData_t));
//  return 0;
//}

//
//
//
static portTASK_FUNCTION (vGPSTask, pvParameters __attribute__ ((unused)))
{
  int fd;
  static char nmeaSentence [GPS_MAX_NMEA_SENTENCE];

 // memset (&gpsData, 0, sizeof (gpsData));

  if (!semaphore)
    vSemaphoreCreateBinary (semaphore);
  
  

  for (;;)
  {
//    portCHAR c;

   // if (read (fd, &c, sizeof (c)) == sizeof (c))
  //    if (gpsProcessByte (c, nmeaSentence))
    //    gpsDispatchMessages (nmeaSentence);
	  
  }
}

//
//
//
signed portBASE_TYPE gpsTaskStart (void)
{
  return xTaskCreate (vGPSTask, "GPSTask", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1), &taskHandle_GPS);  
}

signed portBASE_TYPE gpsTaskStop (void)
{
 
  
  vTaskDelete (taskHandle_GPS);
  taskHandle_GPS  = NULL;
  return 1;
}

