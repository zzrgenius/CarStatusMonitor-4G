/**
  ******************************************************************************
  * @file    board_leds.c
  * @author  MCD Application Team
  * @brief   Implements functions for leds actions
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
#include "plf_config.h"


/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

void BL_LED_Init(BL_Led_t led)
{
  BSP_LED_Init((Led_TypeDef)led);
}

void BL_LED_Off(BL_Led_t led)
{
  BSP_LED_Off((Led_TypeDef)led);
}

void BL_LED_On(BL_Led_t led)
{
  BSP_LED_On((Led_TypeDef)led);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
