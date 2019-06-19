#ifndef  __BSP_SERIAL_H__
#define  __BSP_SERIAL_H__



#include "main.h"
#include "stm32f4xx_hal.h"

#define USART_BUF_SIZE   256
typedef struct 
{
	uint16_t MessageCom;
	uint16_t data_len;
	uint8_t data_buf[USART_BUF_SIZE];
}SerialDataBuf_TypeDef;

enum SER_PROT
{
	COM0 = 0,
	COM1,
	COM2,
	COM3,
	COM4,
	COM5
};
void bsp_serial_config(void);
#endif
