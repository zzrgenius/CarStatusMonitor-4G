
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
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "string.h"
#include "stdio.h"
#include "nmea.h"
#include "bsp_serial.h"
#include "bsp_led.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define GPS_BUF_LEN	256
 
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
QueueHandle_t g_GPSDataProcQueue;  
extern UART_HandleTypeDef huart3;
extern uint8_t usart3_rx_buf[USART_BUF_SIZE];

extern  DMA_HandleTypeDef hdma_usart3_rx; 

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
	HAL_GPIO_WritePin( GPS_POWER_GPIO_Port,GPS_POWER_Pin, GPIO_PIN_RESET );
	__HAL_DMA_ENABLE_IT(huart3.hdmarx, (DMA_IT_TC | DMA_IT_TE));

	HAL_UART_Receive_DMA(&huart3,usart3_rx_buf,USART_BUF_SIZE);
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
		     if( xQueueReceive( g_GPSDataProcQueue, ( gGPSBuf ), ( TickType_t ) 100 ) )
			{
				/* We were able to obtain the semaphore and can now access the
				shared resource. */
				/* ... */
				LED_Toggle(LED1);
				 
			//	printf("Latitude: %f degrees\r\n", g_hgps.latitude);
				nmea_parse(&parser,(const char*)gGPSBuf,USART_BUF_SIZE, &info); 

				//__HAL_DMA_ENABLE_IT(huart3.hdmarx, (DMA_IT_TC | DMA_IT_TE));
				HAL_UART_Receive_DMA(&huart3,usart3_rx_buf,USART_BUF_SIZE);
			
				/* We have finished accessing the shared resource.  Release the semaphore. */
				
			}
		 
		
	}
}
