#ifndef __SIM_DRV_H__
#define __SIM_DRV_H__
#include "stm32f4xx_hal.h"
#include "ipc_uart.h"
uint8_t SIM_power_on(void);
//typedef enum
//{
//  /*  WAITING FOR  /  COMMAND FORMAT */
//  WAITING_FOR_INIT_CR,       /*   <CR>        /  xxxxx<CR> */
//  WAITING_FOR_FIRST_CHAR,    /*   <CR> or x   /                     */
//  WAITING_FOR_CR,            /*   <CR>        /                     */
//  WAITING_FOR_LF,            /*   <LF>        /                     */
//  WAITING_FOR_SOCKET_DATA,   /* count received characters, do not analyze them */
//} atcustom_modem_SyntaxAutomatonState_t;
void IPC_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle);
void IPC_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle);
HAL_StatusTypeDef SIM_send_uart( uint8_t *p_TxBuffer, uint16_t bufsize);
HAL_StatusTypeDef SIM_receive_uart( IPC_RxMessage_t *p_msg);

#endif