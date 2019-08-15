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
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "string.h"
#include "stdio.h"
#include "nmea.h"
#include "bsp_serial.h"
#include "osprintf.h"
#include "bsp_led.h"
#include "gpio.h"
#include "time.h"
#include "ov2640.h"
#include "ov2640_camera.h"   
#include "fatfs.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
 //#define CAMERA_FRAME_BUFFER               0x64000000
uint8_t  CAMERA_FRAME_BUFFER[ 100 ];  //0x9600  38400
/* Private macro -------------------------------------------------------------*/
static void PicturePrepare(void) ;
/* Private variables ---------------------------------------------------------*/
FATFS MSC_FatFs;  /* File system object for USB disk logical drive */
extern FIL MyFile;       /* File object */
char MSC_Path[4]; /* USB Host logical drive path */


/* Image header */  
const uint32_t aBMPHeader[14]=
{0xB0364D42, 0x00000004, 0x00360000, 0x00280000, 0x01400000, 0x00F00000, 0x00010000, 
 0x00000020, 0xF5400000, 0x00000006, 0x00000000, 0x00000000, 0x00000000, 0x0000};

/**
  * @brief  Frame Event callback.
  * @param  None
  * @retval None
*/
void BSP_CAMERA_FrameEventCallback(void)
{
  /* Display on LCD */
  //BSP_LCD_DrawRGBImage(0, 0, 320, 240, (uint8_t *)CAMERA_FRAME_BUFFER);
	LED_Toggle(LED4);
	LED_Toggle(LED1);
	LED_Toggle(LED2);
	LED_Toggle(LED3);
}


/**
  * @brief  Main routine for Mass Storage Class
  * @param  None
  * @retval None
  */
static void SavePicture(void)
{
  FRESULT res1, res2;     /* FatFs function common result code */
  uint32_t byteswritten;  /* File write count */
  
  static uint32_t counter = 0;
  uint8_t str[30];
  
  /* Suspend the camera capture */
  BSP_CAMERA_Suspend();
  
//  /* Prepare the image to be saved */
//  PicturePrepare();
//   
  /* Format the string */
  sprintf((char *)str, "image_%lu.bmp", counter);
  
  /* Create and Open a new file object with write access */
  if(f_open(&MyFile, (const char*)str, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) 
  {
    Error_Handler();
  }
  else
  {
    /* Write data to the BMP file */
    res1 = f_write(&MyFile, (uint32_t *)aBMPHeader, 54, (void *)&byteswritten);
  //  res2 = f_write(&MyFile, (uint16_t *)SRAM_DEVICE_ADDR, (BSP_LCD_GetYSize()*320()*sizeof(uint32_t)), (void *)&byteswritten);
    
    if((res1 != FR_OK) || (byteswritten == 0))
    {
      Error_Handler();
    }
    else
    {
      /* Close the open BMP file */
      f_close(&MyFile);
      
      /* Success of the demo: no error occurrence */
      LED_On(LED1);

      /* Wait for 2s */
      HAL_Delay(2000);

      counter++;
      LED_Off(LED1);
    }
  }
}

///**
//  * @brief  Prepares the picture to be Saved in USB.
//  * @param  None
//  * @retval None
//  */
//static void PicturePrepare(void) 
//{
//  uint32_t address = SRAM_DEVICE_ADDR;
//  uint16_t x = 0;
//  uint16_t y = 0;
//  uint16_t tmp = 0;
//  uint8_t aRGB[4];
//  
//  /* Go to the address of the last line of BMP file */
//  address += ((320 * 240 - 1)) * 4);

//  /* Read data from GRAM and swap it into SRAM */
//  for(y = 0; y < (240); y++)
//  { 
//    for(x = 0; x < (320); x++)
//    {      
//      /* Write data to the SRAM memory */
//    //  tmp  = BSP_LCD_ReadPixel(x, y); 
//      
//      aRGB[0] =  RGB565_TO_R(tmp);
//      aRGB[1] =  RGB565_TO_G(tmp);
//      aRGB[2] =  RGB565_TO_B(tmp);
//      aRGB[3] =  0xFF;
//      
////      if(BSP_SRAM_WriteData(address, (uint16_t *)aRGB, 2) != SRAM_OK)
////      {
////        Error_Handler();
////      }
////      else
////      {
////        address += 4;
////      }
//    }
//    address -= 8*320;
//  }    
//}
uint32_t BSP_PB_GetState(  void)
{
  return HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port,USER_BUTTON_Pin);
}

/**
  * @brief  Main routine for Camera capture
  * @param  None
  * @retval None
  */
static void CAMERA_Capture(void)
{
  
    if(BSP_PB_GetState( ) != GPIO_PIN_RESET) 
    {
      if(BSP_PB_GetState( ) != GPIO_PIN_SET) 
      {
        SavePicture();
        BSP_CAMERA_Resume();
      }
    }
  
} 

void StartTaskCamera( void *pvParameters )
{
	int8_t rslt;  
 uint32_t pres32, pres64;
    
	 
		TickType_t xLastWakeTime;
	const TickType_t xFrequency = 30*1000;

     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
  /* Initialize the Camera */
  BSP_CAMERA_Init(RESOLUTION_R320x240);
  
  /* Start the Camera Capture */
//	CAMERA_FRAME_BUFFER =malloc(  GetSize(RESOLUTION_R320x240) );
  BSP_CAMERA_ContinuousStart((uint8_t *)CAMERA_FRAME_BUFFER);
   
	while(1)
	{
		
		CAMERA_Capture();
		LED_Toggle(LED3); 
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}
	
 