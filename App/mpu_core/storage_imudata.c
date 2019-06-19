
 #include "bsp_rs485.h"

#include "rtc_time.h"
#include "stm32f4xx_hal.h"
#include "storage_imudata.h"
#define NUM_IMU_BUF_SIZE   16
#define IMU_START_ADDR	   ENV_AREA_SIZE  
#define IMU_AREA_SIZE		((256*16-1) * EF_ERASE_MIN_SIZE)
#define IMU_END_ADDR		(256*16* EF_ERASE_MIN_SIZE)

//#define ENV_AREA_SIZE          (5 * EF_ERASE_MIN_SIZE)
extern mavlink_raw_imu_t g_raw_imu;
uint32_t g_write_addr = IMU_START_ADDR;
mavlink_raw_imu_t storage_imu_buf[NUM_IMU_BUF_SIZE];

//void save_addr_writed_env(uint32_t addr)
//{
//	
//}
void save_imubuf_toflash(mavlink_raw_imu_t *imu_data)
{
	static uint8_t i = 0;
	EfErrCode res = EF_NO_ERR;
	mavlink_raw_imu_t t_imudata;
	char addr_str[32];
	memcpy((uint8_t*)&storage_imu_buf[i++],(uint8_t*)imu_data,sizeof(mavlink_raw_imu_t));
	if(i == NUM_IMU_BUF_SIZE)
	{
		i = 0;
//save 
		if((g_write_addr%EF_ERASE_MIN_SIZE) == 0)
			ef_port_erase(g_write_addr,EF_ERASE_MIN_SIZE);

		ef_port_read(g_write_addr,( uint32_t *)&t_imudata,sizeof(mavlink_raw_imu_t));
		if((uint32_t)t_imudata.time_usec == 0xffffffff)
		{
			res =  ef_port_write(g_write_addr,   (uint32_t *)&storage_imu_buf ,   sizeof(mavlink_raw_imu_t)*NUM_IMU_BUF_SIZE) ;
			if(res == EF_NO_ERR)
			{
				g_write_addr +=  sizeof(mavlink_raw_imu_t)*NUM_IMU_BUF_SIZE;
				if(g_write_addr > (IMU_END_ADDR - sizeof(mavlink_raw_imu_t)*NUM_IMU_BUF_SIZE) )
					g_write_addr = IMU_START_ADDR;
				sprintf(addr_str,"%d",g_write_addr);
				ef_set_env("Write_Addr", addr_str);  //  保存到外部Flash中
			}
		}
		else
		{
			ef_port_erase(g_write_addr,EF_ERASE_MIN_SIZE);
		}
	}
	
	return;
	 
}
void load_imubuf_toflash(uint32_t addr,mavlink_raw_imu_t *imu_data )
{
	uint8_t i;
	mavlink_message_t mav_message;
	uint8_t mav_sendbuf[MAVLINK_MAX_PAYLOAD_LEN];

	ef_port_read(addr,(uint32_t *)imu_data,sizeof(mavlink_raw_imu_t));
//	for(i = 0;i<sizeof(mavlink_raw_imu_t);i++)
//		printf("%02x",(uint8_t*)(imu_data + i));
	 
	
	
}
uint16_t upload_imubuf_toflash( uint16_t num)
{
	uint8_t i;
	uint16_t send_len;
	uint32_t addr;
	mavlink_message_t mav_message;
	uint8_t mav_sendbuf[MAVLINK_MAX_PAYLOAD_LEN];
	mavlink_raw_imu_t imu_data;

	// 计算地址
	for(i=0;i<num;i++)
	{
		addr = g_write_addr - sizeof(mavlink_raw_imu_t);
		if(addr < IMU_START_ADDR)
			break;
		load_imubuf_toflash(addr,&imu_data);
		mavlink_msg_raw_imu_encode(MAV_SYSTEM_ID,MAV_COMP_ID_IMU,&mav_message,&g_raw_imu);
		send_len = mavlink_msg_to_send_buffer(mav_sendbuf,&mav_message);
		rs485_senddata(send_len,mav_sendbuf); 
	}
	
	return i+1;
	
	
}