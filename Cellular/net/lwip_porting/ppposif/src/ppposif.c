/**
  ******************************************************************************
  * @file    ppposif.c
  * @author  MCD Application Team
  * @brief   This file contains pppos adatation layer
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

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)

#include "ppp.h"
#include "ppposif.h"
#include "ppposif_ipc.h"
#include "error_handler.h"
#include "netif/ppp/pppos.h"
#include "lwip/sys.h"
#include "lwip/dns.h"

/* Private defines -----------------------------------------------------------*/
#define RCV_SIZE_MAX 100

/* Private typedef -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/

/* sys_now needed by pppos.c module */
u32_t sys_now(void)
{
  return HAL_GetTick();
}

/**
  * @brief  PPP status callback is called on PPP status change (up, down, …) from lwIP
  * @param  pcb        pcb reference
  * @param  err_code   error
  * @param  pcb        user context
  * @retval ppposif_status_cb    return status
  */
void ppposif_status_cb(ppp_pcb *pcb, int32_t err_code, void *ctx)
{
#if (USE_TRACE_PPPOSIF == 1) || (USE_PRINTF == 1)
  struct netif *pppif = ppp_netif(pcb);
#endif
  switch (err_code)
  {
    case PPPERR_NONE:
    {
#if LWIP_DNS
#if (USE_TRACE_PPPOSIF == 1) || (USE_PRINTF == 1)
      const ip_addr_t *ns;
#endif
#endif /* LWIP_DNS */
      /* PrintPPPoSIf("status_cb: Connected\n") */
#if PPP_IPV4_SUPPORT
      PrintPPPoSIf("\n\r")
      PrintPPPoSIf("   our_ipaddr  = %s", ipaddr_ntoa(&pppif->ip_addr))
      PrintPPPoSIf("   his_ipaddr  = %s", ipaddr_ntoa(&pppif->gw))
      PrintPPPoSIf("   netmask     = %s", ipaddr_ntoa(&pppif->netmask))
#if LWIP_DNS
#if (USE_TRACE_PPPOSIF == 1) || (USE_PRINTF == 1)
      ns = dns_getserver(0U);
      PrintPPPoSIf("   dns1        = %s", ipaddr_ntoa(ns))
      ns = dns_getserver(1U);
      PrintPPPoSIf("   dns2        = %s", ipaddr_ntoa(ns))
#endif
#endif /* LWIP_DNS */
#endif /* PPP_IPV4_SUPPORT */
#if PPP_IPV6_SUPPORT
      PrintPPPoSIf("   our6_ipaddr = %s", ip6addr_ntoa(netif_ip6_addr(pppif, 0)))
#endif /* PPP_IPV6_SUPPORT */
      break;
    }
    case PPPERR_PARAM:
    {
      PrintPPPoSIf("status_cb: Invalid parameter\n\r")
      break;
    }
    case PPPERR_OPEN:
    {
      PrintPPPoSIf("status_cb: Unable to open PPP session\n\r")
      break;
    }
    case PPPERR_DEVICE:
    {
      PrintPPPoSIf("status_cb: Invalid I/O device for PPP\n\r")
      break;
    }
    case PPPERR_ALLOC:
    {
      PrintPPPoSIf("status_cb: Unable to allocate resources\n\r")
      break;
    }
    case PPPERR_USER:
    {
      PrintPPPoSIf("status_cb: User interrupt\n\r")
      break;
    }
    case PPPERR_CONNECT:
    {
      PrintPPPoSIf("status_cb: Connection lost\n\r")
      break;
    }
    case PPPERR_AUTHFAIL:
    {
      PrintPPPoSIf("status_cb: Failed authentication challenge\n\r")
      break;
    }
    case PPPERR_PROTOCOL:
    {
      PrintPPPoSIf("status_cb: Failed to meet protocol\n\r")
      break;
    }
    case PPPERR_PEERDEAD:
    {
      PrintPPPoSIf("status_cb: Connection timeout\n\r")
      break;
    }
    case PPPERR_IDLETIMEOUT:
    {
      PrintPPPoSIf("status_cb: Idle Timeout\n\r")
      break;
    }
    case PPPERR_CONNECTTIME:
    {
      PrintPPPoSIf("status_cb: Max connect time reached\n\r")
      break;
    }
    case PPPERR_LOOPBACK:
    {
      PrintPPPoSIf("status_cb: Loopback detected\n\r")
      break;
    }
    default:
    {
      PrintPPPoSIf("status_cb: Unknown error code %d\n\r", err_code)
      break;
    }
  }

  /*
   * This should be in the switch case, this is put outside of the switch
   * case for example readability.
   */

  if ((err_code != PPPERR_NONE) && (err_code != PPPERR_USER))
  {
    /*
     * Try to reconnect in 30 seconds, if you need a modem chatscript you have
     * to do a much better signaling here ;-)
     */
    ppp_connect(pcb, 30U);
  }
}


/**
  * @brief  read data from serial and send it to PPP
  * @param  ppp_netif      (in) netif reference
  * @param  p_ppp_pcb      (in) pcb reference
  * @param  pDevice        (in) serial device id
  * @retval none
  */

void ppposif_input(struct netif *ppp_netif, ppp_pcb  *p_ppp_pcb, IPC_Device_t pDevice)
{
  int32_t rcv_size;
  uint8_t rcvChar[RCV_SIZE_MAX];
  rcv_size  = 0;

  rcv_size = ppposif_ipc_read(pDevice, rcvChar, RCV_SIZE_MAX);
  if (rcv_size != 0)
  {
    /* traceIF_hexPrint(DBG_CHAN_PPPOSIF, DBL_LVL_P0, rcvChar, rcv_size) */
    pppos_input(p_ppp_pcb, rcvChar, rcv_size);
  }
}

/**
  * @brief  PPPoS serial output callback
  * @param  pcb            (in) PPP control block
  * @param  data           (in) data, buffer to write to serial port
  * @param  len            (in) length of the data buffer
  * @param  ctx            (in) user context : contains serial device id
  * @retval number of char sent if write succeed - 0 otherwise
  */

u32_t ppposif_output_cb(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx)
{
  return (u32_t)ppposif_ipc_write((IPC_Device_t)(int32_t)ctx, data, (int16_t)len);
}


/**
  * @brief      Closing PPP connection
  * @note       Initiate the end of the PPP session, without carrier lost signal
  *             (nocarrier=0), meaning a clean shutdown of PPP protocols.
  *             You can call this function at anytime.
  * @param  ppp_pcb_struct            (in) pcb reference
  * @retval ppposif_status_t    return status
  */
ppposif_status_t ppposif_close(ppp_pcb *ppp_pcb_struct)
{
  u8_t nocarrier = 0U;
  ppp_close(ppp_pcb_struct, nocarrier);

  /*
   * Then you must wait your status_cb() to be called, it may takes from a few
   * seconds to several tens of seconds depending on the current PPP state.
   */

  /*
   * Freeing a PPP connection
   * ========================
   */

  /*
   * Free the PPP control block, can only be called if PPP session is in the
   * dead state (i.e. disconnected). You need to call ppp_close() before.
   */
  ppp_free(ppp_pcb_struct);
  return PPPOSIF_OK;
}
#else
/* Private typedef -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
uint32_t sys_now(void);

uint32_t sys_now(void)
{
  return HAL_GetTick();
}

#endif /* USE_SOCKETS_TYPE == USE_SOCKETS_LWIP  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
