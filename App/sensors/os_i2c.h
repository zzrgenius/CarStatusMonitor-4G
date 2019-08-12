#ifndef __OS_I2C_H__
#define __OS_I2C_H__

void os_iic(void);
int os_I2C_WriteReg(unsigned char slave_addr, unsigned char reg_addr,unsigned short len, unsigned char *data_ptr);
int os_I2C_ReadReg(unsigned char slave_addr,unsigned char reg_addr,unsigned short len,unsigned char *data_ptr);
#endif