#ifndef _BSP_H
#define _BSP_H

#include "i2c.h"

#define I2C_TIMEOUT  100 /*<! Value of Timeout when I2C communication fails */

#define IO_I2C_ADDRESS                        0x88
#define TS_I2C_ADDRESS                        0x82
#define CAMERA_I2C_ADDRESS                    0x60
#define AUDIO_I2C_ADDRESS                     0x94
/* For M24C64 devices, E0, E1 and E2 pins are all used for device 
  address selection (no need for additional address lines). According to the 
  Hardware connection on the board (on STM324xG-EVAL board E0 = E1 = E2 = 0) */
#define EEPROM_I2C_ADDRESS                    0xA0
#ifndef BSP_I2C_SPEED
 #define BSP_I2C_SPEED                            100000
#endif /* BSP_I2C_SPEED */
  void I2Cx_Reset(I2C_HandleTypeDef* i2cHandle);


#endif