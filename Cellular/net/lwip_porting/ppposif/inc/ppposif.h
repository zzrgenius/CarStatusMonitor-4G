/**
  ******************************************************************************
  * @file    ppposif.h
  * @author  MCD Application Team
  * @brief   Header for ppposif.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      http://www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef PPPOSIF_H
#define PPPOSIF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "ppp.h"
#include "netif/ppp/pppos.h"
#include "ipc_uart.h"
#include "plf_config.h"


#if (USE_TRACE_PPPOSIF == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PrintPPPoSIf(format, args...) TracePrint(DBG_CHAN_PPPOSIF, DBL_LVL_P0, "UTILS:" format "\n\r", ## args)
#else
#define PrintPPPoSIf(format, args...)  printf("" format , ## args);
#endif /* USE_PRINTF */
#else
#define PrintPPPoSIf(format, args...)  do {} while(0);
#endif /* USE_TRACE_PPPOSIF */

/* Exported types ------------------------------------------------------------*/

typedef enum
{
  PPPOSIF_OK = 0x00,
  PPPOSIF_ERROR
} ppposif_status_t;

/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
/**
  * @brief  read data from serial and send it to PPP
  * @param  ppp_netif      (in) netif reference
  * @param  p_ppp_pcb      (in) pcb reference
  * @param  pDevice        (in) serial device id
  * @retval none
  */
extern void  ppposif_input(struct netif *ppp_netif, ppp_pcb  *p_ppp_pcb, IPC_Device_t pDevice);

/**
  * @brief      Closing PPP connection
  * @note       Initiate the end of the PPP session, without carrier lost signal
  *             (nocarrier=0), meaning a clean shutdown of PPP protocols.
  *             You can call this function at anytime.
  * @param  ppp_pcb_struct            (in) pcb reference
  * @retval ppposif_status_t    return status
  */
extern ppposif_status_t ppposif_close(ppp_pcb *ppp_pcb_struct);


/**
  * @brief  PPP status callback is called on PPP status change (up, down, …) from lwIP
  * @param  pcb        pcb reference
  * @param  err_code   error
  * @param  pcb        user context
  * @retval ppposif_status_cb    return status
  */

extern void  ppposif_status_cb(ppp_pcb *pcb, int32_t err_code, void *ctx);

/**
  * @brief  PPPoS serial output callback
  * @param  pcb            (in) PPP control block
  * @param  data           (in) data, buffer to write to serial port
  * @param  len            (in) length of the data buffer
  * @param  ctx            (in) user context : contains serial device id
  * @retval number of char sent if write succeed - 0 otherwise
  */
extern u32_t ppposif_output_cb(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx);

/**
  * @brief  sys_now
  * @param  None
  * @retval GetTick
  */
extern uint32_t sys_now(void);


#ifdef __cplusplus
}
#endif

#endif /* PPPOSIF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
