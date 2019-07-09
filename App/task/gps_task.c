
/**
  ******************************************************************************
  * @file    gps_task.c
  * @author  LoryTech HARDWARE TEAM
  * @version V1.1.0
  * @date    2019-01-12
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
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "string.h"
#include "stdio.h"
#include "nmea.h"
#include "bsp_serial.h"
#include "bsp_led.h"
#include "gpio.h"
#include "time.h"
 /* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define GPS_BUF_LEN	256
  
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
QueueHandle_t g_GPSDataProcQueue;  
extern UART_HandleTypeDef huart4;
 
extern uint8_t usart4_rx_buf[USART_BUF_SIZE];

extern  DMA_HandleTypeDef hdma_usart4_rx; 
 
char gGPSBuf[USART_BUF_SIZE];

nmeaINFO info;
nmeaPARSER parser;
 /* Private function prototypes -----------------------------------------------*/
 
void HAL_GpsInit( void )
{
   //gps_init(&g_hgps);                             /* Init GPS */
    /* Create buffer for received data */
   // gps_buff_init(&hgps_buff, hgps_buff_data, sizeof(hgps_buff_data));
	g_GPSDataProcQueue = xQueueCreate(1,GPS_BUF_LEN); 
	if(g_GPSDataProcQueue == NULL)
	{
		__nop();
	}
	//HAL_GPIO_WritePin( GPS_POWER_EN_GPIO_Port,GPS_POWER_EN_Pin, GPIO_PIN_SET );
//	__HAL_DMA_ENABLE_IT(huart4.hdmarx, (DMA_IT_TC | DMA_IT_TE));

//	HAL_UART_Receive_DMA(&huart4,usart4_rx_buf,USART_BUF_SIZE);
	//gpsTaskStart();
}

  
void StartTaskGPS( void *pvParameters )
{
//	nmeaTIME bj_time;
	HAL_GpsInit();
	nmea_zero_INFO(&info);
    nmea_parser_init(&parser);
	__HAL_DMA_ENABLE_IT(huart4.hdmarx, (DMA_IT_TC | DMA_IT_TE));

	HAL_UART_Receive_DMA(&huart4,usart4_rx_buf,USART_BUF_SIZE);
	
	HAL_GPIO_WritePin( GPS_POWER_EN_GPIO_Port,GPS_POWER_EN_Pin, GPIO_PIN_RESET );

	while(1)
	{
			/* See if we can obtain the semaphore.  If the semaphore is not
			available wait 10 ticks to see if it becomes free. */
			//if( xSemaphoreTake( xBinarySemGPS, ( TickType_t ) portMAX_DELAY ) == pdTRUE )
		     if( xQueueReceive( g_GPSDataProcQueue, ( gGPSBuf ), ( TickType_t ) 100 ) )
			{
				/* We were able to obtain the semaphore and can now access the
				shared resource. */
				/* ... */
 
						 
				LED_Toggle(LED1); 

				
				nmea_parse(&parser,(const char*)gGPSBuf,USART_BUF_SIZE, &info); 
			//	printf("Latitude: %f degrees\r\n", info.lat);
//				memcpy((uint8_t *)&bj_time ,(uint8_t*)&(info.utc),sizeof(nmeaTIME));
//				DateTime_GMT2BJTime(& bj_time);

				HAL_UART_Receive_DMA(&huart4,usart4_rx_buf,USART_BUF_SIZE);
			
				/* We have finished accessing the shared resource.  Release the semaphore. */
				
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
#if 0 
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
 		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

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
				//  __HAL_UART_DISABLE_IT(&GPS_UART_Handle,UART_IT_RXNE);
				  nmeaSentence[pos] = 0x0d;
				  nmeaSentence[pos+1] = 0x0a;
				  nmeaSentence[pos+2] = '\0';
				  xSemaphoreGiveFromISR( gps_semaphore, &xHigherPriorityTaskWoken );
					    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

 					  return (complete);

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
  HAL_UART_Receive_IT(&GPS_UART_Handle,&gps_recdata,1);

  return (complete);
}
 #endif

