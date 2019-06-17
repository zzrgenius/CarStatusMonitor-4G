#ifndef _STORAGE_IMUDATA_H_
#define _STORAGE_IMUDATA_H_
#include "sfud.h"
#include "mav_msg.h"
#include "easyflash.h"

void save_imubuf_toflash(mavlink_raw_imu_t *imu_data);
void load_imubuf_toflash(uint32_t addr,mavlink_raw_imu_t *imu_data );

uint16_t upload_imubuf_toflash( uint16_t num);
#endif