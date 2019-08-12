#include "main.h"
#include "stdio.h"
#include "sim_drv.h"
#include "at_custom_modem.h"
#include "ringbuffer.h"
#include  "at_main.h"
#include "usart.h"
#include "ipc_uart.h"

#define MODEM_UART_IRQn		USART3_IRQn
#define  SIM_RINGBUF_SIZE   512
struct rt_ringbuffer gSim_ringbuf;
uint8_t sim_usart_buf[SIM_RINGBUF_SIZE];
uint8_t gSimRxchar;
extern at_context_t    at_context_sim;
extern osMessageQId q_msg_IPC_received_Id;

	//SysCtrl_SIM_power_on(0);
uint8_t SIM_power_on(void)
{
  uint8_t retval = HAL_OK;

  /* Reference: 
  *  PWRKEY   connected to MODEM_PWR_EN (inverse pulse)
  *  RESET_N  connected to MODEM_RST    (inverse)
  *
  * Turn ON module sequence (cf paragraph 4.2)
  *
  *          PWRKEY  RESET_N  modem_state
  * init       0       0        OFF
  * T=0        1       1        OFF
  * T1=100     0       1        BOOTING
  * T1+100     1       1        BOOTING
  * T1+13000   1       1        RUNNING
  */

  /* Re-enale the UART IRQn */
  rt_ringbuffer_init(&gSim_ringbuf,sim_usart_buf,SIM_RINGBUF_SIZE);
  HAL_NVIC_EnableIRQ(MODEM_UART_IRQn);

  /* POWER DOWN */
  HAL_GPIO_WritePin(SIM_POWER_EN_GPIO_Port, SIM_POWER_EN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SIM_RST_GPIO_Port_GPIO_Port, SIM_RST_GPIO_Port_Pin, GPIO_PIN_RESET);
  HAL_Delay(150U);
  /* POWER UP */
 
  /* Waits for Modem complete its booting procedure */
  HAL_Delay(1000);
  printf("...done");
  return (retval);
}
static void msgSentCallback(void)
{
  /* Warning ! this function is called under IT */
  at_context_sim.dataSent = AT_TRUE;

#if (RTOS_USED == 1)
  osSemaphoreRelease(at_context_sim.s_SendConfirm_SemaphoreId);
#endif

}
static void msgReceivedCallback(void)
{
  /* Warning ! this function is called under IT */
   
 
  /* disable irq not required, we are under IT */
 // MsgReceived++;

  if (osMessagePut(q_msg_IPC_received_Id, 1, 0U) != osOK)
  {
    //PrintErr("q_msg_IPC_received_Id error for SIG_IPC_MSG")
  }

 
}

/* Callback functions ----------------------------------------------------------*/
/**
* @brief  IPC uart RX callback (called under IT !).
* @param  UartHandle Ptr to the HAL UART handle.
* @retval none
*/
void IPC_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Warning ! this function is called under IT */
  static uint8_t lastchar = 0;
  {
    // RxFifoWrite(g_IPC_Devices_List[device_id].h_current_channel, g_IPC_Devices_List[device_id].RxChar[0]);
	    rt_ringbuffer_putchar_force(&gSim_ringbuf, gSimRxchar);
	  
		if(gSimRxchar == 0x0A)
		{
			if(lastchar == 0x0D)
			{
				msgReceivedCallback();				
			}
		}
		else
		{
			lastchar = gSimRxchar;
		}

  }
  HAL_UART_Receive_IT(&huart3, (uint8_t *)&gSimRxchar, 1U);

}
/**
* @brief  IPC uart TX callback (called under IT !).
* @param  UartHandle Ptr to the HAL UART handle.
* @retval none
*/
void IPC_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Warning ! this function is called under IT */
 
    /* Set transmission flag: transfer complete */
   msgSentCallback();
  
}


/**
* @brief  Send data over an UART channel.
* @param  hipc IPC handle.
* @param  p_TxBuffer Pointer to the data buffer to transfer.
* @param  bufsize Length of the data buffer.
* @retval status
*/
HAL_StatusTypeDef SIM_send_uart( uint8_t *p_TxBuffer, uint16_t bufsize)
{
  /* PrintDBG(">IPC_send: buf@=%p size=%d", aTxBuffer, bufsize) */

  /* check the handles */
   /* send string in one block */
  while (1)
  {
    HAL_StatusTypeDef err;
    err = HAL_UART_Transmit_IT(&huart3, (uint8_t *)p_TxBuffer, bufsize);
	      if (err !=  HAL_BUSY)
		  {
			  break;
		  }
//    if (err !=  HAL_BUSY)
//    {             
//        while (HAL_UART_Receive_IT(&huart3, (uint8_t *)&gSimRxchar, 1U) != HAL_OK)
//        {
//        }
//        printf("DTO : Receive rearmed...");
//      
//      break;
//    }


#if (RTOS_USED == 1)
    osDelay(10U);
#endif /* RTOS_USED */
  }

  return (HAL_OK);
}

/**
* @brief  Receive a message from an UART channel.
* @param  hipc IPC handle.
* @param  p_msg Pointer to the IPC message structure to fill with received message.
* @retval status
*/
HAL_StatusTypeDef SIM_receive_uart( IPC_RxMessage_t *p_msg)
{
  int16_t unread_msg = 0;
	uint16_t data_len;
#if (DBG_IPC_RX_FIFO != 0U)
  int16_t free_bytes;
#endif /* DBG_IPC_RX_FIFO */

    if (p_msg == NULL)
    {
      printf("IPC_receive err - p_msg NULL");
      return (1);
    }


    /* read the first unread message */
 //   unread_msg = RXFIFO_read(hipc, p_msg);
	data_len = rt_ringbuffer_data_len(&gSim_ringbuf);
	unread_msg = rt_ringbuffer_get(&gSim_ringbuf,p_msg->buffer,data_len);
    if (unread_msg == 0)
    {
      //PrintErr("IPC_receive err - no unread msg")
      return (1);
    }
 
   //   HAL_UART_Receive_IT(&huart3, (uint8_t *)&gSimRxchar, 1U);
	return 0;
   
}

uint8_t checkEndOfMsgCallback(uint8_t rxChar)
{
  uint8_t last_char = 0U;
 static uint8_t  state_SyntaxAutomaton = WAITING_FOR_FIRST_CHAR;
  switch (state_SyntaxAutomaton)
  {
    case WAITING_FOR_INIT_CR:
    {
      /* waiting for first valid <CR>, char received before are considered as trash */
      if ('\r' == rxChar)
      {
        /* current     : xxxxx<CR>
        *  command format : <CR><LF>xxxxxxxx<CR><LF>
        *  waiting for : <LF>
        */
        state_SyntaxAutomaton = WAITING_FOR_LF;
      }
      break;
    }

    case WAITING_FOR_CR:
    {
      if ('\r' == rxChar)
      {
        /* current     : xxxxx<CR>
        *  command format : <CR><LF>xxxxxxxx<CR><LF>
        *  waiting for : <LF>
        */
        state_SyntaxAutomaton = WAITING_FOR_LF;
      }
      break;
    }

    case WAITING_FOR_LF:
    {
      /* waiting for <LF> */
      if ('\n' == rxChar)
      {
         
      }
      break;
    }

    case WAITING_FOR_FIRST_CHAR:
		break;

    case WAITING_FOR_SOCKET_DATA:
      break;

    default:
      /* should not happen */
      break;
  }

 
 
  return (last_char);
}
