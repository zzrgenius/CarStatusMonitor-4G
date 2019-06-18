#ifndef __filter_h
#define __filter_h
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "arm_math.h"
float32_t filter_average(float32_t sam)     ;        //均值滤波

float32_t fir_pro(float32_t data);
uint8_t filter_limit(float32_t data);
#endif