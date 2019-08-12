#include "main.h"
#include "cmsis_os.h"
#include "osprintf.h"
#include "mpu9250_driver.h"
#include "os_i2c.h"
osMutexId I2C_MutexHandle ;

void os_iic(void)
{
	osMutexDef(osI2C_Mutex);
    I2C_MutexHandle = osMutexCreate(osMutex(osI2C_Mutex));
    if (I2C_MutexHandle == NULL)
    {
 //     result = CELLULAR_FALSE;
     __nop();
    }
   
  
  osMutexRelease(I2C_MutexHandle);
}

int os_I2C_WriteReg(unsigned char slave_addr, unsigned char reg_addr,unsigned short len, unsigned char *data_ptr)
{
	int result;
	osMutexWait(I2C_MutexHandle, osWaitForever);
	result=	Sensors_I2C_WriteRegister( slave_addr,  reg_addr, len,data_ptr);

  osMutexRelease(I2C_MutexHandle);
	return result;
}
int os_I2C_ReadReg(unsigned char slave_addr,unsigned char reg_addr,unsigned short len,unsigned char *data_ptr)
{
	int result;
	osMutexWait(I2C_MutexHandle, osWaitForever);
	result	=	Sensors_I2C_ReadRegister( slave_addr,  reg_addr, len,data_ptr);
	osMutexRelease(I2C_MutexHandle);
	return result;
}