
#include "stm32f4xx_hal.h"
#include "bsp.h"

 
#define EVAL_I2Cx       I2C1
#define  heval_I2c		hi2c1
#define IIC2_SCLH  	 	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET)	 
#define IIC2_SCLL	 	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET)	 
#define IIC2_SDAH  	 	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET)	 
#define IIC2_SDAL	 	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET)	 
 
 
 
static void I2Cx_Init(void)
{
//  if(HAL_I2C_GetState(&heval_I2c) == HAL_I2C_STATE_RESET)
//  {
//    heval_I2c.Instance = EVAL_I2Cx;
//    heval_I2c.Init.ClockSpeed      = BSP_I2C_SPEED;
//    heval_I2c.Init.DutyCycle       = I2C_DUTYCYCLE_2;
//    heval_I2c.Init.OwnAddress1     = 0;
//    heval_I2c.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
//    heval_I2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
//    heval_I2c.Init.OwnAddress2     = 0;
//    heval_I2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
//    heval_I2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLED;  

//    /* Init the I2C */
//    I2Cx_MspInit();
//    HAL_I2C_Init(&heval_I2c);
//  }
	MX_I2C1_Init();
}
/**********
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
    SDA_OUT();         
    IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1;       
        delay_us(2);   //对TEA5767这三个延时都是必须的
        IIC_SCL=1;
        delay_us(2); 
        IIC_SCL=0;    
        delay_us(2);
    }     
} 

*******/
  void I2Cx_Reset(I2C_HandleTypeDef* i2cHandle)
{
	
  GPIO_InitTypeDef GPIO_InitStruct;
	uint8_t i;
 
  /*Configure GPIO pin :   */
 if( i2cHandle->Instance==I2C2)
 {
  
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	 HAL_Delay(100);
	 IIC2_SDAH;
	 IIC2_SCLH;
	 for(i = 0; i < 9; i++)
	 {
		  IIC2_SCLL;
		 HAL_Delay(1);
		IIC2_SCLH;
	 }
	 IIC2_SCLH;
	 
 }
}

/**
  * @brief  Manages error callback by re-initializing I2C.
  * @param  Addr: I2C Address
  */
static void I2Cx_Error(uint8_t Addr)
{
  /* De-initialize the IOE comunication BUS */
  HAL_I2C_DeInit(&heval_I2c);
  
  /* Re-Initiaize the IOE comunication BUS */
  I2Cx_Init();  
}

/**
  * @brief  Reads a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @retval Data to be read
  */
static uint8_t I2Cx_Read(uint8_t Addr, uint8_t Reg)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t Value = 0;
  
  status = HAL_I2C_Mem_Read(&heval_I2c, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, I2C_TIMEOUT);
  
  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Execute user timeout callback */
    I2Cx_Error(Addr);
  }
  
  return Value;   
}

/**
  * @brief  Writes a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @param  Value: Data to be written
  */
static void I2Cx_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Write(&heval_I2c, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, I2C_TIMEOUT); 
  
  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* I2C error occured */
    I2Cx_Error(Addr);
  }
}

/**
  * @brief  Reads multiple data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @param  MemAddress Internal memory address
  * @param  Buffer: Pointer to data buffer
  * @param  Length: Length of the data
  * @retval Number of read data
  */
static HAL_StatusTypeDef I2Cx_ReadMultiple(uint8_t Addr, uint16_t Reg, uint16_t MemAddress, uint8_t *Buffer, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Read(&heval_I2c, Addr, (uint16_t)Reg, MemAddress, Buffer, Length, I2C_TIMEOUT);
  
  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* I2C error occured */
    I2Cx_Error(Addr);
  }
  return status;    
}

/**
  * @brief  Write a value in a register of the device through BUS in using DMA mode
  * @param  Addr: Device address on BUS Bus.  
  * @param  Reg: The target register address to write
  * @param  MemAddress Internal memory address
  * @param  Buffer: The target register value to be written 
  * @param  Length: buffer size to be written
  * @retval HAL status
  */
static HAL_StatusTypeDef I2Cx_WriteMultiple(uint8_t Addr, uint16_t Reg, uint16_t MemAddress, uint8_t *Buffer, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Write(&heval_I2c, Addr, (uint16_t)Reg, MemAddress, Buffer, Length, I2C_TIMEOUT);
  
  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initiaize the I2C Bus */
    I2Cx_Error(Addr);
  }
  return status;
}

/**
  * @brief  Checks if target device is ready for communication. 
  * @note   This function is used with Memory devices
  * @param  DevAddress: Target device address
  * @param  Trials: Number of trials
  * @retval HAL status
  */
static HAL_StatusTypeDef I2Cx_IsDeviceReady(uint16_t DevAddress, uint32_t Trials)
{ 
  return (HAL_I2C_IsDeviceReady(&heval_I2c, DevAddress, Trials, I2C_TIMEOUT));
}

 
/**
  * @brief  Initializes Camera low level.
  */
void CAMERA_IO_Init(void) 
{
  I2Cx_Init();
}

/**
  * @brief  Camera writes single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @param  Value: Data to be written
  */
void CAMERA_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
  I2Cx_Write(Addr, Reg, Value);
}

/**
  * @brief  Camera reads single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @retval Read data
  */
uint8_t CAMERA_IO_Read(uint8_t Addr, uint8_t Reg)
{
  return I2Cx_Read(Addr, Reg);
}
/**
  * @brief  Camera delay. 
  * @param  Delay: Delay in ms
  */
void CAMERA_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}