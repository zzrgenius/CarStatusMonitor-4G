#include "i2c.h"
#include "bsp_led.h"
#include "bsp.h"

#include "mpu9250_driver.h"
extern I2C_HandleTypeDef hi2c2;
#define    I2C_EXPBD_Handle  hi2c2
#define 	I2C_EXPBD                            I2C2

 static volatile uint32_t TimingDelay=0;
static uint32_t I2C_EXPBD_Timeout = 0x1000;    /*<! Value of Timeout when I2C communication fails */

static void I2C_EXPBD_Error( uint8_t Addr );

 void mdelay(unsigned long nTime)
{
//	TimingDelay = nTime;
//	while(TimingDelay != 0);
	HAL_Delay(nTime);
}

int get_tick_count(unsigned long *count)
{
      //  count[0] = g_ul_ms_ticks;
	count[0] = HAL_GetTick();
	return 0;
}

void TimingDelay_Decrement(void)
{
	if (TimingDelay != 0x00)
		TimingDelay--;
}
  
 
 
/**
 * @brief  Write data to the register of the device through BUS
 * @param  Addr Device address on BUS
 * @param  Reg The target register address to be written
 * @param  pBuffer The data to be written
 * @param  Size Number of bytes to be written
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
static uint8_t I2C_EXPBD_WriteData( uint8_t Addr, uint8_t Reg,  uint16_t Size, uint8_t* pBuffer)
{

  HAL_StatusTypeDef status = HAL_OK;

 //HAL_I2C_Mem_Write( &I2C_EXPBD_Handle, Addr, ( uint16_t )Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Size,                              I2C_EXPBD_Timeout );	
  status = HAL_I2C_Mem_Write( &I2C_EXPBD_Handle, Addr, ( uint16_t )Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Size,
                              I2C_EXPBD_Timeout );
  /* Check the communication status */
  if( status != HAL_OK )
  {

    /* Execute user timeout callback */
    I2C_EXPBD_Error( Addr );
    return 1;
  }
  else
  {
    return 0;
  }
}



/**
 * @brief  Read a register of the device through BUS
 * @param  Addr Device address on BUS
 * @param  Reg The target register address to read
 * @param  pBuffer The data to be read
 * @param  Size Number of bytes to be read
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
static uint8_t I2C_EXPBD_ReadData( uint8_t Addr, uint16_t Reg,  uint16_t Size,uint8_t* pBuffer )
{

  HAL_StatusTypeDef status = HAL_OK;

 status = HAL_I2C_Mem_Read( &I2C_EXPBD_Handle, Addr, ( uint16_t )Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Size,  I2C_EXPBD_Timeout );


  /* Check the communication status */
  if( status != HAL_OK )
  {

    /* Execute user timeout callback */
    I2C_EXPBD_Error( Addr );
    return 1;
  }
  else
  {
    return 0;
  }
}


/**
 * @brief  Configures I2C interface.
 * @param  None
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
static uint8_t I2C_EXPBD_Init( void )
{
  if(HAL_I2C_GetState( &I2C_EXPBD_Handle) == HAL_I2C_STATE_RESET )
  {

    /* I2C_EXPBD peripheral configuration */

//    I2C_EXPBD_Handle.Init.ClockSpeed = 400000;
//    I2C_EXPBD_Handle.Init.DutyCycle = I2C_DUTYCYCLE_2;
// 

// 
//    I2C_EXPBD_Handle.Init.OwnAddress1    = 0x33;
//    I2C_EXPBD_Handle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
//    I2C_EXPBD_Handle.Instance            = I2C_EXPBD;

    /* Init the I2C */
    MX_I2C2_Init();
    
  }

  if( HAL_I2C_GetState( &I2C_EXPBD_Handle) == HAL_I2C_STATE_READY )
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
/**
 * @brief  Manages error callback by re-initializing I2C
 * @param  Addr I2C Address
 * @retval None
 */
static void I2C_EXPBD_Error( uint8_t Addr )
{

  /* De-initialize the I2C comunication bus */
	    I2Cx_Reset( &I2C_EXPBD_Handle);
 
	
  HAL_I2C_DeInit( &I2C_EXPBD_Handle );

  /* Re-Initiaize the I2C comunication bus */
  I2C_EXPBD_Init();
}
static unsigned short RETRY_IN_MLSEC  = 55;

void Set_I2C_Retry(unsigned short ml_sec)
{
  RETRY_IN_MLSEC = ml_sec;
}

unsigned short Get_I2C_Retry()
{
  return RETRY_IN_MLSEC;
}


 int Sensors_I2C_WriteRegister(unsigned char slave_addr, unsigned char reg_addr,unsigned short len, unsigned char *data_ptr)
{
  char retries=0;
  int ret = 0;
  unsigned short retry_in_mlsec = Get_I2C_Retry();
                              
tryWriteAgain:  
  ret = 0;
  ret = I2C_EXPBD_WriteData( slave_addr, reg_addr, len, data_ptr); 

  if(ret && retry_in_mlsec)
  {
    if( retries++ > 4 )
        return ret;
    
    mdelay(retry_in_mlsec);
    goto tryWriteAgain;
  }
  return ret;  
}

int Sensors_I2C_ReadRegister(unsigned char slave_addr,
                                       unsigned char reg_addr,
                                       unsigned short len, 
                                       unsigned char *data_ptr)
{
  char retries=0;
  int ret = 0;
  unsigned short retry_in_mlsec = Get_I2C_Retry();
  
tryReadAgain:  
  ret = 0;
  ret = I2C_EXPBD_ReadData( slave_addr, reg_addr, len, data_ptr);

  if(ret && retry_in_mlsec)
  {
    if( retries++ > 4 )
        return ret;
    
    mdelay(retry_in_mlsec);
    goto tryReadAgain;
  } 
  return ret;
}


/**
  * @brief  I2C error callbacks.
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
 void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
  /* Turn LED5 on: Transfer error in reception/transmission process */
  LED_On(LED4); 
}