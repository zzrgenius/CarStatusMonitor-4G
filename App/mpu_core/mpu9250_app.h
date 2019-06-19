#ifndef __MPU9250_APP_H
#define __MPU9250_APP_H
#include "inv_mpu.h" 
#include "main.h"
int mpu_main_process( unsigned long *timestamp);
uint8_t mpu_dmp_init(void);
uint8_t mpu_mpl_get_data(float *pitch,float *roll,float *yaw);
int mpu_config( struct int_param_s *int_param,unsigned char *accel_fsr,unsigned short *gyro_rate,unsigned short *gyro_fsr,unsigned short *compass_fsr);
void gyro_data_ready_cb(void);
#endif
