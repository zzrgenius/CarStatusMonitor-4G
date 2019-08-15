/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define Camera_RST_Pin GPIO_PIN_2
#define Camera_RST_GPIO_Port GPIOE
#define Camera_PWDN_Pin GPIO_PIN_3
#define Camera_PWDN_GPIO_Port GPIOE
#define SD_DETECT_Pin GPIO_PIN_13
#define SD_DETECT_GPIO_Port GPIOC
#define USER_BUTTON_Pin GPIO_PIN_0
#define USER_BUTTON_GPIO_Port GPIOB
#define USER_BUTTON_EXTI_IRQn EXTI0_IRQn
#define RS485_DE_Pin GPIO_PIN_1
#define RS485_DE_GPIO_Port GPIOB
#define SIM_POWER_EN_Pin GPIO_PIN_9
#define SIM_POWER_EN_GPIO_Port GPIOE
#define GPS_POWER_EN_Pin GPIO_PIN_10
#define GPS_POWER_EN_GPIO_Port GPIOE
#define LED1_Pin GPIO_PIN_12
#define LED1_GPIO_Port GPIOE
#define LED2_Pin GPIO_PIN_13
#define LED2_GPIO_Port GPIOE
#define LED3_Pin GPIO_PIN_14
#define LED3_GPIO_Port GPIOE
#define LED4_Pin GPIO_PIN_15
#define LED4_GPIO_Port GPIOE
#define INVEN_INT_Pin GPIO_PIN_15
#define INVEN_INT_GPIO_Port GPIOB
#define INVEN_INT_EXTI_IRQn EXTI15_10_IRQn
#define SIM_RST_GPIO_Port_Pin GPIO_PIN_11
#define SIM_RST_GPIO_Port_GPIO_Port GPIOD
#define SPI1_CS_Pin GPIO_PIN_5
#define SPI1_CS_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

#define USE_OS 				1
#define RTOS_USED			1
#define USE_PRINTF 				1
#define LONG_TIME			0xffff 
#define LIMITED_AM				4.81f
#define USE_FULL_ASSERT

#define BCD2BIN(n)		(((((n) >> 4) & 0x0F) * 10) + ((n) & 0x0F))
#define BIN2BCD(n)		((((n) / 10) << 4) | ((n) % 10))
#define RTM_EXPORT(symbol)
#define RT_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))
#define RT_ALIGN_SIZE 4

#define RT_ASSERT(EX)                                                         \
if (!(EX))                                                                    \
{                                                                             \
	_Error_Handler(__FILE__, __LINE__);										  \
}
    //rt_assert_handler(#EX, __FUNCTION__, __LINE__);                           

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
