#include "mav_msg.h"
#include "bsp_rs485.h"
#include "sfud.h"
#include "rtc_time.h"
#include "easyflash.h"
#include "stm32f4xx_hal.h"
#include "storage_imudata.h"

//#include "mavlink_types.h"

#define mav_debug  0
extern 	 float acc_zthreshold ; 
static mavlink_message_t mav_message;
static uint8_t mav_sendbuf[MAVLINK_MAX_PAYLOAD_LEN];
mavlink_raw_imu_t g_raw_imu,test_raw_imu;

//extern RTC_TimeTypeDef TimeToUpdate;
//extern RTC_DateTypeDef DateToUpdate;
 

void mav_send_paramset_test(void)
{
			uint16_t send_len;
	mavlink_param_ext_set_t packet_in = {
        MAV_SYSTEM_ID,MAV_COMP_ID_ALL,"ACC_ZThreshold","5.11",MAV_PARAM_EXT_TYPE_REAL32
    };
	mavlink_param_ext_set_t packet_in1 = {
        MAV_SYSTEM_ID,MAV_COMP_ID_ALL,"Date","20190527",MAV_PARAM_EXT_TYPE_UINT32
    };
	mavlink_param_ext_set_t packet_in2 = {
        MAV_SYSTEM_ID,MAV_COMP_ID_ALL,"Time","172454", MAV_PARAM_EXT_TYPE_UINT32
    };
	mavlink_msg_param_ext_set_encode(0,0,&mav_message,&packet_in);
	send_len = mavlink_msg_to_send_buffer(mav_sendbuf,&mav_message);
	rs485_senddata(send_len,mav_sendbuf); 
	mavlink_msg_param_ext_set_encode(0,0,&mav_message,&packet_in1);
	send_len = mavlink_msg_to_send_buffer(mav_sendbuf,&mav_message);
	rs485_senddata(send_len,mav_sendbuf); 
	mavlink_msg_param_ext_set_encode(0,0,&mav_message,&packet_in2);
	send_len = mavlink_msg_to_send_buffer(mav_sendbuf,&mav_message);  
	rs485_senddata(send_len,mav_sendbuf); 
}

void mav_send_cmdack_test(void)
{
			uint16_t send_len;
	mavlink_command_ack_t packet_in = {
      MAV_CMD_REQUEST_MESSAGE,0,0,0, MAV_SYSTEM_ID,MAV_COMP_ID_ALL 
    };
		mavlink_msg_command_ack_encode(0,0,&mav_message,&packet_in);
	send_len = mavlink_msg_to_send_buffer(mav_sendbuf,&mav_message);
	rs485_senddata(send_len,mav_sendbuf); 
	 
}
void mav_send_imu_msg(int16_t xacc ,int16_t yacc ,int16_t zacc,
					 int16_t xgyro ,int16_t ygyro,int16_t zgyro,
					 int16_t xmag  ,int16_t ymag ,int16_t zmag )
{
	
		uint16_t send_len;
	nmeaTIME time;
	unsigned int utc_time;
 
	TM_GetLocaltime(&time );
	utc_time = xDate2Seconds(&time);	
 
    

	g_raw_imu.time_usec = (uint64_t)(utc_time*1000);
	g_raw_imu.xacc = xacc;
	g_raw_imu.yacc = yacc;
	g_raw_imu.zacc = zacc;
	g_raw_imu.xgyro = xgyro;
	g_raw_imu.ygyro = ygyro;
	g_raw_imu.zgyro = zgyro;
	g_raw_imu.xmag = xmag;
	g_raw_imu.ymag = ymag;
	g_raw_imu.xacc = zmag;
	printf("zacc is %d",zacc);
	mavlink_msg_raw_imu_encode(MAV_SYSTEM_ID,MAV_COMP_ID_IMU,&mav_message,&g_raw_imu);
	//save_imubuf_toflash(&g_raw_imu);
	//load_imubuf_toflash(&test_raw_imu);
	//printf("\r\nsize of raw struct is %d\r\n",sizeof(g_raw_imu));
	//mavlink_msg_raw_imu_pack(MAV_SYSTEM_ID,MAV_COMP_ID_IMU,&mav_message,(uint64_t)(utc_time*1000),  xacc ,  yacc ,  zacc,xgyro ,  ygyro,  zgyro,xmag  ,  ymag ,  zmag);
	send_len = mavlink_msg_to_send_buffer(mav_sendbuf,&mav_message);
	rs485_senddata(send_len,mav_sendbuf); 
	//HAL_Delay(100);
	//mav_send_paramset_test();
	//mav_send_cmdack_test();

}
 
 
void mav_sendmsg(uint8_t compid ,uint32_t msgid,uint8_t databuf[],uint8_t len)
{
	mavlink_message_t message;
	uint8_t sendbuf[MAVLINK_MAX_PAYLOAD_LEN];
	uint16_t send_len;
	//(mavlink_message_t* msg, uint8_t system_id, uint8_t component_id,mavlink_status_t* status, uint8_t min_length, uint8_t length, uint8_t crc_extra)
	memcpy(_MAV_PAYLOAD_NON_CONST(&message), databuf, len);  
	message.msgid = msgid;
	mavlink_finalize_message(&message, MAV_SYSTEM_ID, compid, len, len, MAVLINK_MSG_ID_MESSAGE_INTERVAL_CRC);
	
	send_len = mavlink_msg_to_send_buffer(sendbuf,&message);
	rs485_senddata(send_len,sendbuf); 

}

void mavlink_param_ext_set_handle(mavlink_param_ext_set_t *param_ext)
{
	uint8_t result = 0;
	uint16_t send_len;
	uint32_t datetime;
	uint8_t month,day,hour,	minute,	second;
	uint16_t year;
 			/**********
0		PARAM_ACK_ACCEPTED	Parameter value ACCEPTED and SET
1	PARAM_ACK_VALUE_UNSUPPORTED	Parameter value UNKNOWN/UNSUPPORTED
2	PARAM_ACK_FAILED	 
3	PARAM_ACK_IN_PROGRESS 
 
*/
	mavlink_param_ext_ack_t t_param_ext_ack;
	memcpy(t_param_ext_ack.param_id,param_ext->param_id,16);
	memcpy(t_param_ext_ack.param_value,param_ext->param_value,128);
	t_param_ext_ack.param_type = param_ext->param_type;
	if(param_ext->target_system == MAV_SYSTEM_ID)
	{
		switch(param_ext->param_id[0])
		{
			case 'A':
				if(strcmp(param_ext->param_id,"ACC_ZThreshold") == 0)
				{
					if(param_ext->param_type == MAV_PARAM_EXT_TYPE_REAL32)
					{
						acc_zthreshold = atof(param_ext->param_value);
						if((acc_zthreshold < 1.0f) || (acc_zthreshold > 100.0f))
							result = PARAM_ACK_VALUE_UNSUPPORTED;
						else
						{
							ef_set_env("ACC_ZThreshold", param_ext->param_value);  //  保存到外部Flash中
							result = PARAM_ACK_ACCEPTED;
						}
					}
					else result = PARAM_ACK_VALUE_UNSUPPORTED;
				}
				break;
			case 'D':
				if(strcmp(param_ext->param_id,"Date") == 0)
				{
					if(param_ext->param_type == MAV_PARAM_EXT_TYPE_UINT32)
					{
						//20190527
						datetime =(uint32_t) atol(param_ext->param_value);
						year = (uint16_t) (datetime/10000);
						month =(uint8_t) ((datetime/100)%100);
						day = (uint8_t)(datetime%100);
						if(( month < 1) || (month > 12))
							result = PARAM_ACK_VALUE_UNSUPPORTED;
						else if((day < 1) || (day > 31))
							result = PARAM_ACK_VALUE_UNSUPPORTED;
						else 
						{
							TM_SetDate(year ,month,day);
							result = PARAM_ACK_ACCEPTED;
						 }
					 }
					else 
						result = PARAM_ACK_VALUE_UNSUPPORTED;
				 }
					else result = PARAM_ACK_VALUE_UNSUPPORTED;
					break;
			case 'T':
				if(strcmp(param_ext->param_id,"Time") == 0)
				{
					if(param_ext->param_type == MAV_PARAM_EXT_TYPE_UINT32)
					{					
						//171210
						datetime =(uint32_t) atol(param_ext->param_value);
						hour = (uint8_t)(datetime/10000);
						minute = (uint8_t)((datetime/100)%100); 
						second = (uint8_t)(datetime%100);
						if( hour > 24)
							result = PARAM_ACK_VALUE_UNSUPPORTED;
						else if(minute > 60)
							result = PARAM_ACK_VALUE_UNSUPPORTED;
						else 
						{
							TM_SetTime(hour ,minute,second);
							result = PARAM_ACK_ACCEPTED;
						 }
					 }
					else 
						result = PARAM_ACK_VALUE_UNSUPPORTED;
				 }
				else result = PARAM_ACK_VALUE_UNSUPPORTED;
			default:
				result = PARAM_ACK_VALUE_UNSUPPORTED;
			break;
			}
		}
		else
		{
			result = PARAM_ACK_FAILED;
		}
	 
	t_param_ext_ack.param_result = result;
//	static inline uint16_t mavlink_msg_param_ext_ack_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_param_ext_ack_t* param_ext_ack)

	mavlink_msg_param_ext_ack_encode(MAV_SYSTEM_ID,MAV_COMP_ID_ALL,&mav_message,&t_param_ext_ack);
	send_len = mavlink_msg_to_send_buffer(mav_sendbuf,&mav_message);
	rs485_senddata(send_len,mav_sendbuf); 
}
void mavlink_cmdack_handle(mavlink_command_ack_t *cmd_ack)
{
	switch(cmd_ack->command)
	{
		case MAV_CMD_REQUEST_MESSAGE:
			//upload_imubuf_toflash(1);
			break;
		case MAV_CMD_SET_MESSAGE_INTERVAL :
			//Set to -1 to disable and 0 to request default rate
			//MAVLINK_MSG_ID_RAW_IMU
			if(cmd_ack->result_param2 == MAVLINK_MSG_ID_RAW_IMU)
			{
				
			}
			break;
		case MAV_CMD_GET_MESSAGE_INTERVAL :
			
			break;
		default:
			break;
	}
}

void mav_revmsg(uint8_t byte  )
{
 
	mavlink_message_t msg;
	mavlink_status_t status;
 
	mavlink_param_ext_set_t t_param_ext;
    mavlink_command_ack_t	t_cmd_ack;

	int chan = MAVLINK_COMM_1;
	// uint8_t *payload8 = (uint8_t *)msg.payload64;
    if (mavlink_frame_char(chan, byte, &msg,&status) != MAVLINK_FRAMING_INCOMPLETE)
      {
		  #if mav_debug
		  printf("Received message with ID %d, sequence: %d from component %d of system %d", msg.msgid, msg.seq, msg.compid, msg.sysid);
		  #endif
		  if(msg.sysid == 0)
		  {
			  
			  //static inline void mavlink_msg_param_ext_set_decode(const mavlink_message_t* msg, mavlink_param_ext_set_t* param_ext_set)
				 switch(msg.msgid)
				 {
					 case MAVLINK_MSG_ID_PARAM_EXT_ACK:
	//					 if(msg.compid == MAV_COMP_ID_ALL)
						 {
							 mavlink_msg_param_ext_set_decode(&msg,&t_param_ext);
							 mavlink_param_ext_set_handle(&t_param_ext);
						 }
						 break;
					 case MAVLINK_MSG_ID_COMMAND_ACK:
						 mavlink_msg_command_ack_decode(&msg,&t_cmd_ack);
						 mavlink_cmdack_handle(&t_cmd_ack);
						break;
					 default:
						 break;
				 }			  
		  }
 
		   
      }

}
#if 0
#define FLASH_ADDR_ALARM_LIMITED  0X02
static void save_alarm_limited_sfud( int size, uint8_t *data) {
    sfud_err result = SFUD_SUCCESS;
	uint32_t addr = FLASH_ADDR_ALARM_LIMITED;
    const sfud_flash *flash = sfud_get_device_table() + 0;
    int i;
    /* prepare write data */
  
    /* erase test */
    result = sfud_erase(flash, addr, size);
    if (result == SFUD_SUCCESS) {
        printf("Erase the %s flash data finish. Start from 0x%08X, size is %ld.\r\n", flash->name, addr, size);
    } else {
        printf("Erase the %s flash data failed.\r\n", flash->name);
        return;
    }
    /* write test */
    result = sfud_write(flash, addr, size, data);
    if (result == SFUD_SUCCESS) {
        printf("Write the %s flash data finish. Start from 0x%08X, size is %ld.\r\n", flash->name, addr,  size);
    } else {
        printf("Write the %s flash data failed.\r\n", flash->name);
        return;
    }
   
 
}
void read_alarm_am_fromflash(float *data)
{
	uint8_t read_data[2];
	uint16_t temp;
	int i;
	sfud_err result = SFUD_SUCCESS;
	const sfud_flash *flash = sfud_get_device_table() + 0;
	result = sfud_read(flash, FLASH_ADDR_ALARM_LIMITED, 2, read_data);
    if (result == SFUD_SUCCESS) 
	{   
        for (i = 0; i < 2; i++) 
		{         
            printf("%02X ", read_data[i]);
            
        
        }
        printf("\r\n");
		
		temp = read_data[1]<<8 | read_data[0]; 
		*data =(float) (temp/100.0f);   
		if( (*data < 1.0f) || (*data > 10.0f))
			*data =  LIMITED_AM;
		 
    } else 
	{
        printf("Read the %s flash data failed.\r\n", flash->name);
    }
}
#endif
