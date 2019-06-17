 /**
   ******************************************************************************
   * @file    xxx.c
   * @author  LoryTech HARDWARE TEAM
   * @version V1.1.0
   * @date    2018-10-08
   * @brief  xxx
   ******************************************************************************
   * @attention
   * Copyright (c) LoryTech. 
   * All rights reserved.
   *
  
   ******************************************************************************
   */
   
 /* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>
#if USE_RTOS

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#endif
 /* Private typedef -----------------------------------------------------------*/
 
 /* Private define ------------------------------------------------------------*/
 
 /* Private macro -------------------------------------------------------------*/
 /* Private variables ---------------------------------------------------------*/
extern  ADC_HandleTypeDef hadc1;
extern  DMA_HandleTypeDef hdma_adc1;

uint32_t DMA_Transfer_Complete_Count=0;  
__IO uint8_t adc_flag;
 //void StartTaskADC( void *pvParameters )
//{
//	uint16_t gAdcConvertedValue[348];

//	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)gAdcConvertedValue,sizeof(gAdcConvertedValue));
//	
//	while(1)
//	{
//		
////			if(dmaflage==1)
////	 {
////		 dmaflage=0;
////		 for(int a=0;a<CHN;a++)
////		 {vcc[a]=uhADCxConvertedValue[a]*3.3/4095;
////			printf("Vcc%d=%0.2f \r\n",a,vcc[a]);
////			vcc[a]=0; 
////		 }
////		 HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&uhADCxConvertedValue,CHN);	
////	 }
////	 
//		vTaskDelay(1000);
//		
//	}
//	
//}

void  HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	DMA_Transfer_Complete_Count++;
	adc_flag = 1;
	
//		dmaflage=1;
//	HAL_ADC_Stop_DMA(&hadc1);
}

void filter_adc(uint8_t* adcData,uint16_t wlen)
{
	
}
