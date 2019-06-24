/**
  ******************************************************************************
  * @file    board_interrupts.c
  * @author  MCD Application Team
  * @brief   Implements HAL weak functions for Interrupts
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      http://www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ipc_uart.h"
#include "board_buttons.h"
#include "plf_config.h"

/* NOTE : this code is designed for FreeRTOS */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == JOY_UP_Pin)
  {
    BB_JoyUP_Pressed();
  }
  else if (GPIO_Pin == JOY_DOWN_Pin)
  {
    BB_JoyDOWN_Pressed();
  }
  else if (GPIO_Pin == JOY_RIGHT_Pin)
  {
    BB_JoyRIGHT_Pressed();
  }
  else if (GPIO_Pin == JOY_LEFT_Pin)
  {
    BB_JoyLEFT_Pressed();
  }
  else
  {
    /* nothing to do */
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == MODEM_UART_INSTANCE)
  {
    IPC_UART_RxCpltCallback(huart);
  }
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
  if (huart->Instance == MODEM_UART_INSTANCE)
  {
    IPC_UART_ErrorCallback(huart);
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
