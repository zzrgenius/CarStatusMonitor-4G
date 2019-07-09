/*
 * raw_imu.h
 *
 *  Created on: Dec 6, 2015
 *      Author: thebh_000
 */

#ifndef RAW_IMU_H_
#define RAW_IMU_H_

#include <stdlib.h>
#include <stdint.h>
 
 
 
 typedef struct _3axis_s
{
    int16_t x;
    int16_t y;
    int16_t z;
    uint32_t timestamp;
}_3axis_s;

typedef union _3axis
{
    _3axis_s val;
    int16_t array[5];
}_3axis;

// structs ///////////////////
typedef struct
{
    _3axis accel;
    _3axis gyro;
    _3axis mag;
} mpu_value_struct;

typedef union
{
    mpu_value_struct values;
    uint8_t bytes [30];
}mpu_values;

struct platform_data_s
{
    signed char orientation[9];
};

typedef struct quaternion_vals
{
    int32_t w;
    int32_t x;
    int32_t y;
    int32_t z;
    uint32_t timestamp;
} quaternion_vals;

typedef union quaternion
{
    quaternion_vals quat;
    long array[5];
} quaternion;

extern uint8_t data_ready;

extern uint8_t Start_MPU9250(void);
extern void tap_callback(void);
extern void Imu_Get_Data(void);
extern const quaternion * get_quaternion_history(uint8_t * count, uint8_t * head);
extern const mpu_values * get_raw_history(uint8_t * count, uint8_t * head);

#endif /* RAW_IMU_H_ */
