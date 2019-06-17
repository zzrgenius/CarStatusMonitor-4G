/**
  ******************************************************************************
  * @file    xxx.c
  * @author  LoryTech HARDWARE TEAM
  * @version V1.1.0
  * @date    2018-12-18
  * @brief  xxx
  ******************************************************************************
  * @attention
  * Copyright (c) LoryTech. 
  * All rights reserved.
  *
 
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "bsp_serial.h"
#include "gpio.h"
#include "bsp_rs485.h"
#include "bsp_led.h"
#include "r_message.h"
#include "mav_msg.h"
#if USE_RTOS
 #include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "queue.h"
#endif
/* Private typedef -----------------------------------------------------------*/
extern UART_HandleTypeDef huart2;
extern message_t g_rx_message;

/* Private define ------------------------------------------------------------*/
#define RS485_UART_HANDLE  huart2
#define USE_DMA_RS485  1
uint8_t rs485_rx_buf[USART_BUF_SIZE];

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

void rs485_rx_enable(void)
{
  HAL_GPIO_WritePin(  RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET );
}
void rs485_rx_disable(void)
{
  HAL_GPIO_WritePin(  RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET );
} 
void rs485_config(void)
{
	__HAL_UART_CLEAR_IDLEFLAG(&RS485_UART_HANDLE);
	__HAL_UART_ENABLE_IT(&RS485_UART_HANDLE, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&RS485_UART_HANDLE,rs485_rx_buf,USART_BUF_SIZE);
		rs485_rx_enable();

	
}

void rs485_senddata(uint8_t data_len,uint8_t *databuf)
{
	 
	 rs485_rx_disable();
	HAL_Delay(3);	
	HAL_UART_Transmit(&RS485_UART_HANDLE, databuf, data_len, 0xFFFF);
	rs485_rx_enable();
}
void rs485_sendstring(char *strbuf)
{
	 rs485_rx_disable();
	HAL_Delay(3);
	 HAL_UART_Transmit(&RS485_UART_HANDLE, (uint8_t*)strbuf, strlen(strbuf), 0xFFFF);
	rs485_rx_enable();
}

uint8_t rs485_rx_msg_handle(uint16_t wlen,uint8_t *data_buf)
{
	uint8_t *pData = data_buf;
	message_t *p_msg;
	uint16_t t_wlength;
	 
	while(t_wlength--)
	{
		mav_revmsg(*data_buf++  );

//		if(*pData++ == 0xE0)
//		{
//			if(*pData == 0xF0)
//			{
//				p_msg = (message_t *)(pData-3);
//				memcpy((uint8_t*)&g_rx_message,(uint8_t*)p_msg,MESSAGE_HEADER_SIZE);
//				if(message_check_headersum(&g_rx_message) != 0)
//					return 	MESSAGE_HEADER_ERR;
//				if(g_rx_message.msg_header.length  > 2)
//				{
//					memcpy((uint8_t*)&g_rx_message.buf,p_msg->buf,g_rx_message.msg_header.length);
//					g_rx_message.crc16 = *(uint16_t*)&p_msg->buf[g_rx_message.msg_header.length];
//					HandleMSG(&g_rx_message);
//					LED_AllOff();
//					return MESSAGE_OK;
//				}
//			}
//		}
		
	}
	return MESSAGE_OK;
}

void rs485_rxidle_handle(uint16_t wlen)
{		 	
//		memcpy(tMsg.data_buf,usart1_rx_buf,wlength);
		rs485_rx_msg_handle(wlen,rs485_rx_buf);
		HAL_UART_Receive_DMA(&RS485_UART_HANDLE,rs485_rx_buf,USART_BUF_SIZE);
// 			
}
