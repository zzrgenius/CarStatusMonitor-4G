/**
  ******************************************************************************
  * @file    bsp_serial.c
  * @author  LoryTech HARDWARE TEAM
  * @version V1.1.0
  * @date    2018-10-08
  * @brief   基于hal库的 串口空闲中断接收
  ******************************************************************************
  * @attention
  * Copyright (c) LoryTech. 
  * All rights reserved.
  *
 
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include "bsp_serial.h"
#include "bsp_rs485.h"
#include "msg.h"
#include "gps_cfg.h"
#if USE_OS
 #include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "queue.h"
#endif
#include "ipc_uart.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#ifndef	DEBUG
	#define DEBUG 	1
#endif
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
 

//MSG_TypeDef Msg_nb;
//MSG_TypeDef Msg_debug;
 uint8_t usart1_rx_buf[USART_BUF_SIZE];
 
 uint8_t usart4_rx_buf[USART_BUF_SIZE];

 extern UART_HandleTypeDef huart1;
 extern UART_HandleTypeDef huart2;
 extern UART_HandleTypeDef huart4;

extern int gpsProcessByte (unsigned char c, char *nmeaSentence);
extern char gGPSBuf[GPS_BUF_LEN];
extern uint8_t gps_recdata;
  
/* Private function prototypes -----------------------------------------------*/
 


/**
  * @brief  DeInitializes usart buf . 
  * @param  
  * @retval none/status
  */
void bsp_serial_config(void)
{
 
	//	__HAL_UART_CLEAR_IDLEFLAG(&huart1);
	 
 
	
//	__HAL_UART_CLEAR_IT(&hlpuart1,UART_CLEAR_IDLEF);
//	__HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_IDLE);
//	HAL_UART_Receive_DMA(&hlpuart1,lpuart_rx_buf,USART_BUF_SIZE);


}
#if 0
void HAL_LPUSART_RXT_IDLE_Handle(UART_HandleTypeDef *huart)  
{
	static	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
   	uint16_t wlength ;
	uint32_t temp; 	
	T_SIM_RECMSG tSimMsg = {0};
	if((__HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE) != RESET))  
	{   
		__HAL_UART_FLUSH_DRREGISTER(huart);
			__HAL_UART_CLEAR_IT(huart,UART_CLEAR_IDLEF);

 		__HAL_UART_CLEAR_IDLEFLAG(huart);
		huart->Instance->ICR |= UART_CLEAR_IDLEF;
		temp = huart->Instance->ISR; //USART2->SR;
		temp = huart->Instance->RDR;  //USART2->DR;	
		HAL_UART_DMAStop(huart); 
		//temp = huart->hdmarx->Instance->CNDTR;
		wlength = (uint16_t) (USART_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx));	

		tSimMsg.wLen = wlength;
		memcpy(tSimMsg.Data ,lpuart_rx_buf,wlength);
		HAL_UART_Receive_DMA(&hlpuart1,lpuart_rx_buf,USART_BUF_SIZE);
		
		#if USE_OS
 		xQueueSendFromISR( g_SIM_RecDataProcQueue, &tSimMsg, &xHigherPriorityTaskWoken );
		#endif	
	}   
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );

}
#endif

extern 	  xSemaphoreHandle gps_semaphore ;

 /**
   * @brief  uart idle irq handle . 
   * @param  huart  
   * @retval none
   */
void HAL_USART_RXT_IDLE_Handle(UART_HandleTypeDef *huart)  
{ 
	uint16_t wlength = 0;
	uint32_t temp; 
	SerialDataBuf_TypeDef tSerialBuf;
	#if USE_OS

	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	#endif
	
	__HAL_UART_FLUSH_DRREGISTER(huart);
//	__HAL_UART_CLEAR_IT(huart,UART_CLEAR_IDLEF);
	__HAL_UART_CLEAR_IDLEFLAG(huart);
	// huart->Instance->RQR |= 
 	
	HAL_UART_DMAStop(huart); 
	temp = huart->hdmarx->Instance->NDTR;  
	wlength = (uint16_t) (USART_BUF_SIZE - temp);	
	
 
	 
	if(huart->Instance == USART1)
	{
	 

	}
	if(huart->Instance == USART2)
	{
//				tMsg.MessageCom = COM2;
		rs485_rxidle_handle(wlength);
//		tMsg.data_len = wlength;
//		memcpy(tMsg.data_buf,usart1_rx_buf,wlength);
//		HAL_UART_Receive_DMA(&huart2,usart1_rx_buf,USART_BUF_SIZE);
// 			

	}
	
		if(huart->Instance == UART4)
	{
		
 
//		tMsg.data_len = wlength;
//		memcpy(tMsg.data_buf,usart1_rx_buf,wlength);
//		HAL_UART_Receive_DMA(&huart2,usart1_rx_buf,USART_BUF_SIZE);
		//xSemaphoreGiveFromISR( gps_semaphore, &xHigherPriorityTaskWoken );
// 			

	}
	 
	 

#if USE_OS
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
		#endif
	}
extern QueueHandle_t g_GPSDataProcQueue;  
extern DMA_HandleTypeDef hdma_uart4_rx;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	#if USE_OS
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	#endif
	
	if(huart->hdmarx == &hdma_uart4_rx)
	{
		__HAL_DMA_CLEAR_FLAG(huart->hdmarx,DMA_FLAG_TCIF2_6); 

		HAL_UART_DMAStop(huart);
		#if USE_OS
		xQueueSendFromISR(g_GPSDataProcQueue,( void * )usart4_rx_buf , &xHigherPriorityTaskWoken );
		#endif
		//__HAL_DMA_DISABLE_IT(huart3.hdmarx, (DMA_IT_TC | DMA_IT_TE));		
  	}
	if (huart->Instance == MODEM_UART_INSTANCE)
  {
    IPC_UART_RxCpltCallback(huart);
  }
//	if(huart->hdmarx == &hdma_usart3_rx)
//	{
//	//	__HAL_DMA_CLEAR_FLAG(huart->hdmarx,DMA_FLAG_TCIF3);  

//		HAL_UART_DMAStop(huart); 
//		//xQueueSendFromISR(g_GPSDataProcQueue,( void * )usart3_rx_buf , &xHigherPriorityTaskWoken );
//		//__HAL_DMA_DISABLE_IT(huart3.hdmarx, (DMA_IT_TC | DMA_IT_TE));		
//  	}
//#if USE_OS
//	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
//	#endif
		
		 
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == MODEM_UART_INSTANCE)
  {
    IPC_UART_TxCpltCallback(huart);
  }
}
 
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	    uint8_t i = 0;
	
	
	 if (huart->Instance == MODEM_UART_INSTANCE)
  {
    IPC_UART_ErrorCallback(huart);
  }
//	      __HAL_UART_CLEAR_NEFLAG(huart);
//        __HAL_UART_CLEAR_FEFLAG(huart);
//        __HAL_UART_CLEAR_OREFLAG(huart);
//	if((__HAL_UART_GET_FLAG(huart,UART_FLAG_ORE) != RESET))  
//	{
////		__HAL_UART_CLEAR_IT(huart,UART_CLEAR_OREF);
//		HAL_UART_Receive(huart,(uint8_t *)&i,1,0xff);
//		
//	}
	
}
//#if (defined(__GNUC__) && !defined(__CC_ARM))
///* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
//   set to 'Yes') calls __io_putchar() */
//#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
//#define GETCHAR_PROTOTYPE int __io_getchar(void)
//#else
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
//#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
//#endif /* __GNUC__ */

///**
//  * @brief  Retargets the C library printf function to the USART.
//  * @param  None
//  * @retval None
//  */
//PUTCHAR_PROTOTYPE
//{
//  /* Place your implementation of fputc here */
//  /* e.g. write a character to the USART2 and Loop until the end of transmission */
//  while (HAL_OK != HAL_UART_Transmit(&xConsoleUart, (uint8_t *) &ch, 1, 30000))
//  {
//    ;
//  }
//  return ch;
//}

///**
//  * @brief  Retargets the C library scanf function to the USART.
//  * @param  None
//  * @retval None
//  */
//GETCHAR_PROTOTYPE
//{
//  /* Place your implementation of fgetc here */
//  /* e.g. read a character on USART and loop until the end of read */
//  uint8_t ch = 0;
//  while (HAL_OK != HAL_UART_Receive(&xConsoleUart, (uint8_t *)&ch, 1, 30000))
//  {
//    ;
//  }
//  return ch;
//}
//void vMainUARTPrintString( char * pcString )
//{
//    const uint32_t ulTimeout = 3000UL;

//    HAL_UART_Transmit( &xConsoleUart,
//                       ( uint8_t * ) pcString,
//                       strlen( pcString ),
//                       ulTimeout );
//}

// 
