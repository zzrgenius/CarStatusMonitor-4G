
#include <stdio.h>
#include <string.h>

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "sd_diskio.h"

#include "ff_gen_drv.h"
#include "fatfs.h"
#include "bsp_led.h"
#include "osprintf.h"
#include "msg.h"


#define SD_STORAGE_BUF_SIZE  512

FIL MyFile;     /* File object */
extern char SDPath[4]; /* SD card logical drive path */
static uint8_t buffer[_MAX_SS]; /* a work buffer for the f_mkfs() */
QueueHandle_t g_SDDataStoreQueue; 

/**
  * @brief  Start task
  * @param  pvParameters not used
  * @retval None
  */
  void StartTaskSD(void const *argument)
{
  FRESULT res;                                          /* FatFs function common result code */
  uint32_t byteswritten, bytesread;                     /* File write/read counts */
  uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
  uint8_t rtext[100];                                   /* File read buffer */
	sd_t sd_buf;
  	 g_SDDataStoreQueue = xQueueCreate(1,sizeof(sd_t)); 
	if(g_SDDataStoreQueue == NULL)
	{
		__nop();
	}
  /*##-1- Link the micro SD disk I/O driver ##################################*/
//	MX_FATFS_Init();
  if(FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
  { 
    /*##-2- Register the file system object to the FatFs module ##############*/
    if(f_mount(&SDFatFS, (TCHAR const*)SDPath, 0) != FR_OK)
    {
      /* FatFs Initialization Error */
      Error_Handler();
    }
    else
    {
		__nop();
	}
 }
	#if 0
  if(FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
  { 
    /*##-2- Register the file system object to the FatFs module ##############*/
    if(f_mount(&SDFatFS, (TCHAR const*)SDPath, 0) != FR_OK)
    {
      /* FatFs Initialization Error */
      Error_Handler();
    }
    else
    {
      /*##-3- Create a FAT file system (format) on the logical drive #########*/
      /* WARNING: Formatting the uSD card will delete all content on the device */
      if(f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, buffer, sizeof(buffer)) != FR_OK)
      {
        /* FatFs Format Error */
        Error_Handler();
      }
      else
      { 
        /*##-4- Create and Open a new text file object with write access #####*/
        if(f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
        {
          /* 'STM32.TXT' file Open for write Error */
          Error_Handler();
        }
        else
        {
          /*##-5- Write data to the text file ################################*/
          res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);
          
          if((byteswritten == 0) || (res != FR_OK))
          {
            /* 'STM32.TXT' file Write or EOF Error */
            Error_Handler();
          }
          else
          {
            /*##-6- Close the open text file #################################*/
            f_close(&MyFile);
            
            /*##-7- Open the text file object with read access ###############*/
            if(f_open(&MyFile, "STM32.TXT", FA_READ) != FR_OK)
            {
              /* 'STM32.TXT' file Open for read Error */
              Error_Handler();
            }
            else
            {
              /*##-8- Read data from the text file ###########################*/
              res = f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread);
              
              if((bytesread == 0) || (res != FR_OK))
              {
                /* 'STM32.TXT' file Read or EOF Error */
                Error_Handler();
              }
              else
              {
                /*##-9- Close the open text file #############################*/
                f_close(&MyFile);
                
                /*##-10- Compare read data with the expected data ############*/
                if((bytesread != byteswritten))
                {                
                  /* Read data is different from the expected data */
                  Error_Handler();
                }
                else
                {
                  /* Success of the demo: no error occurrence */
                  LED_On(LED1);
                }
              }
            }
          }
        }
      }
    }
  } 
  #endif
  
  /*##-11- Unlink the RAM disk I/O driver ####################################*/
  //FATFS_UnLinkDriver(SDPath);
 
  
  /* Infinite Loop */
  for( ;; )
  {

		if( xQueueReceive( g_SDDataStoreQueue, (& sd_buf ), ( TickType_t ) 100 ) )
		{
			if((sd_buf.length != 0) && (sd_buf.length < _MAX_SS))
			{
				if(f_open(&MyFile,sd_buf.filename ,    FA_OPEN_APPEND | FA_WRITE | FA_READ) != FR_OK)  //sd_buf.filename
				{
					/* 'STM32.TXT' file Open for write Error */
					//Error_Handler();
					__nop();
				}
				else
				{
					/*##-5- Write data to the text file ################################*/
					res = f_write(&MyFile, sd_buf.filebuf, sd_buf.length, (void *)&byteswritten);
					if((byteswritten == 0) || (res != FR_OK))
					{
						/* 'STM32.TXT' file Write or EOF Error */
						__nop();
						Error_Handler();
					}
					else
					{
						f_close(&MyFile);
						osprintf("write bytes is %d\r\n",byteswritten);
					}
				}
          
			}
		}
	  vTaskDelay(1000);
  }
}