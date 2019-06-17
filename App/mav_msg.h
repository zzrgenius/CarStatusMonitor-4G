#ifndef __MAV_MSG_H
#define __MAV_MSG_H

#include "mavlink.h"
#include "packet.h"
#include "mavlink_msg_command_ack.h"
//typedef enum {
//    PACKET_DATA_ACCEL = 0,
//    PACKET_DATA_GYRO,
//    PACKET_DATA_COMPASS,
//    PACKET_DATA_QUAT,
//    PACKET_DATA_EULER,
//    PACKET_DATA_ROT,
//    PACKET_DATA_HEADING,
//    PACKET_DATA_LINEAR_ACCEL,
//    NUM_DATA_PACKETS
//} eMPL_packet_e;

#define MAV_SYSTEM_ID  				0x31


 
 
//#define MAV_COMP_ID_IMU 					200 
 

void mav_sendmsg(uint8_t compid ,uint32_t msgid,uint8_t databuf[],uint8_t len);
void mav_revmsg(uint8_t byte );
void mav_send_imu_msg(int16_t xacc ,int16_t yacc ,int16_t zacc, int16_t xgyro ,int16_t ygyro,int16_t zgyro,int16_t xmag  ,int16_t ymag ,int16_t zmag );

//void read_alarm_am_fromflash(float *data);
#endif
