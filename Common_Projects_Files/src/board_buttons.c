/**
  ******************************************************************************
  * @file    board_buttons.c
  * @author  MCD Application Team
  * @brief   Implements functions for user buttons actions
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
#include "board_leds.h"
#include "dc_common.h"
#include "dc_control.h"
#include "plf_config.h"
#include "board_buttons.h"


/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#if (BB_LEFT_POLLING == 1)
static void BB_JoyLEFT_Polling(void);
#endif
/* Functions Definition ------------------------------------------------------*/

void BB_JoyUP_Pressed(void)
{
  /* USER can implement button action here
  * WARNING ! this function is called under an IT, treatment has to be short !
  * (like posting an event into a queue for example)
  */
  dc_ctrl_post_event_debounce(DC_COM_BUTTON_UP);
}

void BB_JoyDOWN_Pressed(void)
{
  /* USER can implement button action here
  * WARNING ! this function is called under an IT, treatment has to be short !
  * (like posting an event into a queue for example)
  */
  dc_ctrl_post_event_debounce(DC_COM_BUTTON_DN);
}

void BB_JoyRIGHT_Pressed(void)
{
  /* USER can implement button action here
  * WARNING ! this function is called under an IT, treatment has to be short !
  * (like posting an event into a queue for example)
  */
  dc_ctrl_post_event_debounce(DC_COM_BUTTON_RIGHT);
}

void BB_JoyLEFT_Pressed(void)
{
  /* USER can implement button action here
  * WARNING ! this function is called under an IT, treatment has to be short !
  * (like posting an event into a queue for example)
  */
  dc_ctrl_post_event_debounce(DC_COM_BUTTON_LEFT);
}


#if (BB_LEFT_POLLING == 1)
static void BB_JoyLEFT_Polling(void)
{
  GPIO_PinState newGpioState;
  newGpioState =  HAL_GPIO_ReadPin(JOY_LEFT_GPIO_Port, JOY_LEFT_Pin);
  if (GPIO_PIN_RESET != newGpioState)
  {
    BB_JoyLEFT_Pressed();
  }
}
#endif

#if (BB_RIGHT_POLLING == 1)
static void BB_JoyRIGHT_Polling(void)
{
  GPIO_PinState newGpioState;
  newGpioState =  HAL_GPIO_ReadPin(JOY_RIGHT_GPIO_Port, JOY_RIGHT_Pin);
  if (GPIO_PIN_RESET != newGpioState)
  {
    BB_JoyRIGHT_Pressed();
  }
}
#endif

#if (BB_DOWN_POLLING == 1)
static void BB_JoyDOWN_Polling(void)
{
  GPIO_PinState newGpioState;
  newGpioState =  HAL_GPIO_ReadPin(JOY_DOWN_GPIO_Port, JOY_DOWN_Pin);
  if (GPIO_PIN_RESET != newGpioState)
  {
    BB_JoyDOWN_Pressed();
  }
}
#endif

#if (BB_UP_POLLING == 1)
static void BB_JoyUP_Polling(void)
{
  GPIO_PinState newGpioState;
  newGpioState =  HAL_GPIO_ReadPin(JOY_UP_GPIO_Port, JOY_UP_Pin);
  if (GPIO_PIN_RESET != newGpioState)
  {
    BB_JoyUP_Pressed();
  }
}
#endif

void BB_ButtonPolling(void)
{
#if (BB_LEFT_POLLING == 1)
  BB_JoyLEFT_Polling();
#endif

#if (BB_RIGHT_POLLING == 1)
  BB_JoyRIGHT_Polling();
#endif

#if (BB_UP_POLLING == 1)
  BB_JoyUP_Polling();
#endif

#if (BB_DOWN_POLLING == 1)
  BB_JoyDOWN_Polling();
#endif
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
