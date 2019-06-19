#ifndef __MPU9250_DRIVER_H
#define __MPU9250_DRIVER_H

 
int get_tick_count(unsigned long *count);
void mdelay(unsigned long nTime);
int Sensors_I2C_ReadRegister(unsigned char slave_addr,unsigned char reg_addr,unsigned short len,unsigned char *data_ptr);
int Sensors_I2C_WriteRegister(unsigned char slave_addr,unsigned char reg_addr,unsigned short len, unsigned char *data_ptr);
#endif	/*   */
