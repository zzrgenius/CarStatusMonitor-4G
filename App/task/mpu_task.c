

/**
  ******************************************************************************
  * @file    serial_task.c
  * @author  LoryTech HARDWARE TEAM
  * @version V1.1.0
  * @date    2018-10-08
  * @brief   This file provides serial task.
  ******************************************************************************
  * @attention
  * Copyright (c) LoryTech. 
  * All rights reserved.
  *
 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "main.h"
 
#include "bsp_serial.h"
#include "bsp_led.h"
#include "mpu9250_driver.h"
#include "mpu9250_app.h"

#include "inv_mpu.h" 
#include "inv_mpu_dmp_motion_driver.h"
#include "invensense.h"
#include "invensense_adv.h"
#include "eMPL_outputs.h" 
#include "mltypes.h"
#include "mpu.h"
#include "log.h"
#include "packet.h"
 
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
 /* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
 /* Private function prototypes -----------------------------------------------*/




void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{	
 	if(GPIO_Pin== INVEN_INT_Pin)
		{
//			LED_Toggle(LED1);
	//		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
			gyro_data_ready_cb();
 
			  
		}
 }
 
 
