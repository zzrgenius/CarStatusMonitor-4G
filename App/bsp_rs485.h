#ifndef  __BSP_RS485_H__
#define  __BSP_RS485_H__

#include "main.h"
void rs485_config(void);
void rs485_rx_enable(void); 
void rs485_rx_disable(void); 
void rs485_senddata(uint8_t data_len,uint8_t *databuf); 
void rs485_sendstring(char *strbuf); 
void rs485_rxidle_handle(uint16_t wlen);
#endif
