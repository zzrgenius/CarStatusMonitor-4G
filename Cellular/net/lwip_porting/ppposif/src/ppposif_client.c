/**
  ******************************************************************************
  * @file    ppposif_client.c
  * @author  MCD Application Team
  * @brief   This file contains pppos client adatation layer
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

#include "ppposif_client.h"
#include "plf_config.h"

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
#include "ppp.h"
#include "ppposif.h"
#include "ipc_uart.h"
#include "ppposif_ipc.h"
#include "main.h"
#include "error_handler.h"
#include "trace_interface.h"
#include "main.h"
#include "dc_common.h"
#include "cellular_init.h"
#include "plf_config.h"
#include "cmsis_os.h"

/* Private defines -----------------------------------------------------------*/
#define IPC_DEVICE IPC_DEVICE_0
#define PPPOSIF_CONFIG_TIMEOUT_VALUE 5000U
#define PPPOSIF_CONFIG_FAIL_MAX 3

/* Private typedef -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static struct netif  gnetif_ppp_client;
static ppp_pcb      *ppp_pcb_client;
static osTimerId     ppposif_config_timeout_timer_handle;
static osSemaphoreId sem_ppp_init_client = NULL;
static uint32_t ppposif_create_done = 0U;

/* Global variables ----------------------------------------------------------*/

osThreadId pppClientThreadId = NULL;

/* Private function prototypes -----------------------------------------------*/
static void ppp_notify_phase_client_cb(ppp_pcb *pcb, u8_t phase, void *ctx);
static void ppposif_client_running(ppp_pcb *pcb);
static void ppposif_reconf(void);
static void ppposif_client_thread(const void *argument);
static void ppposif_config_timeout_timer_callback(void const *argument);
ppposif_status_t ppposif_client_close(void);

/* Functions Definition ------------------------------------------------------*/

/* Private Functions Definition ------------------------------------------------------*/

/*
 Notify phase callback (PPP_NOTIFY_PHASE)
==========================================

Notify phase callback, enabled using the PPP_NOTIFY_PHASE config option, let
you configure a callback that is called on each PPP internal state change.
This is different from the status callback which only warns you about
up(running) and down(dead) events.

Notify phase callback can be used, for example, to set a LED pattern depending
on the current phase of the PPP session. Here is a callback example which
tries to mimic what we usually see on xDSL modems while they are negotiating
the link, which should be self-explanatory:
*/

static void ppp_notify_phase_client_cb(ppp_pcb *pcb, u8_t phase, void *ctx)
{

  osDelay(500U); /* hack, add a delay to avoid race condition with Modem which sends an LCP Request earlier.
                   To be improved by syncing the input reading and PPP state machine */
  switch (phase)
  {

    /* Session is down (either permanently or briefly) */
    case PPP_PHASE_DEAD:
      PrintPPPoSIf("client ppp_notify_phase_cb: PPP_PHASE_DEAD")
      break;

    /* We are between two sessions */
    case PPP_PHASE_HOLDOFF:
      PrintPPPoSIf("client ppp_notify_phase_cb: PPP_PHASE_HOLDOFF")
      ppposif_reconf();

      break;

    /* Session just started */
    case PPP_PHASE_INITIALIZE:
      PrintPPPoSIf("client ppp_notify_phase_cb: PPP_PHASE_INITIALIZE")
      break;

    /* Session is running */
    case PPP_PHASE_RUNNING:
      PrintPPPoSIf("client ppp_notify_phase_cb: PPP_PHASE_RUNNING")
      ppposif_client_running(pcb);
      break;

    default:
      break;
  }
}

/* ppposif thread */
static void ppposif_client_thread(const void *argument)
{
  osSemaphoreWait(sem_ppp_init_client, osWaitForever);
  while (1)
  {
    ppposif_input(&gnetif_ppp_client, ppp_pcb_client, IPC_DEVICE);
  }
}


static void ppposif_client_running(ppp_pcb *pcb)
{
  dc_ppp_client_info_t ppp_client_info;
  struct netif *pppif = ppp_netif(pcb);

  osTimerStop(ppposif_config_timeout_timer_handle);


  dc_com_read(&dc_com_db, DC_COM_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
  ppp_client_info.rt_state = DC_SERVICE_ON;
  ppp_client_info.ip_addr = pppif->ip_addr;
  ppp_client_info.gw      = pppif->gw;
  ppp_client_info.netmask = pppif->netmask;

  dc_com_write(&dc_com_db, DC_COM_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
}

static void ppposif_reconf(void)
{
  dc_ppp_client_info_t ppp_client_info;
  ppp_close(ppp_pcb_client, 0U);
  PrintPPPoSIf("ppposif_config_timeout_timer_callback")
  dc_com_read(&dc_com_db, DC_COM_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
  ppp_client_info.rt_state = DC_SERVICE_FAIL;
  dc_com_write(&dc_com_db, DC_COM_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
}

static void ppposif_config_timeout_timer_callback(void const *argument)
{
  ppposif_reconf();
}

/*  Exported functions Definition ------------------------------------------------------*/

/**
  * @brief  component init
  * @param  none
  * @retval ppposif_status_t    return status
  */
ppposif_status_t ppposif_client_init(void)
{
  ppposif_ipc_init(IPC_DEVICE);
  return PPPOSIF_OK;
}

/**
  * @brief  component start
  * @param  none
  * @retval ppposif_status_t    return status
  */
ppposif_status_t ppposif_client_start(void)
{
  ppposif_status_t ret = PPPOSIF_OK;

  PrintPPPoSIf("ppposif_client_config")

  osSemaphoreDef(SEM_PPP_CLIENT_INIT);
  sem_ppp_init_client = osSemaphoreCreate(osSemaphore(SEM_PPP_CLIENT_INIT), 1);
  osSemaphoreWait(sem_ppp_init_client, osWaitForever);

  osTimerDef(PPPOSIF_CONFIG_TIMEOUT_timer, ppposif_config_timeout_timer_callback);
  ppposif_config_timeout_timer_handle = osTimerCreate(osTimer(PPPOSIF_CONFIG_TIMEOUT_timer), osTimerOnce, NULL);

  osThreadDef(PPPOS_CLIENT, ppposif_client_thread, PPPOSIF_CLIENT_THREAD_PRIO, 0, PPPOSIF_CLIENT_THREAD_STACK_SIZE);
  pppClientThreadId = osThreadCreate(osThread(PPPOS_CLIENT), NULL);
  if (pppClientThreadId == NULL)
  {
    ERROR_Handler(DBG_CHAN_PPPOSIF, 1, ERROR_FATAL);
  }
  else
  {
#if (STACK_ANALYSIS_TRACE == 1)
    stackAnalysis_addStackSizeByHandle(pppClientThreadId, PPPOSIF_CLIENT_THREAD_STACK_SIZE);
#endif /* STACK_ANALYSIS_TRACE */
  }


  return ret;
}

/**
  * @brief  Create a new PPPoS client interface
  * @param  none
  * @retval ppposif_status_t    return status
  */
ppposif_status_t ppposif_client_config(void)
{
  ppposif_status_t ret = PPPOSIF_OK;

  PrintPPPoSIf("ppposif_client_config")
  ppposif_ipc_select(IPC_DEVICE);

  if (ppposif_create_done == 0U)
  {
    ppp_pcb_client = pppos_create(&gnetif_ppp_client, ppposif_output_cb, (ppp_link_status_cb_fn)ppposif_status_cb, (void *)IPC_DEVICE);
    if (ppp_pcb_client == NULL)
    {
      ERROR_Handler(DBG_CHAN_PPPOSIF, 3, ERROR_FATAL);
      ret =  PPPOSIF_ERROR;
    }
    else
    {
      netif_set_default(&gnetif_ppp_client);
      ppposif_create_done = 1U;
    }
  }

  if (ret ==  PPPOSIF_OK)
  {
    ppp_set_default(ppp_pcb_client);
    ppp_set_notify_phase_callback(ppp_pcb_client,  ppp_notify_phase_client_cb);
    ppp_set_usepeerdns(ppp_pcb_client, 1);

    /*  ppp_set_auth(ppp_pcb_client, PPPAUTHTYPE_PAP, "USER", "PASS"); */

    osTimerStart(ppposif_config_timeout_timer_handle, PPPOSIF_CONFIG_TIMEOUT_VALUE);
    ret = (ppposif_status_t)ppp_connect(ppp_pcb_client, 0U);

    osSemaphoreRelease(sem_ppp_init_client);
  }
  return ret;
}


/**
  * @brief  close a PPPoS client interface
  * @param  none
  * @retval ppposif_status_t    return status
  */
ppposif_status_t ppposif_client_close(void)
{
  return PPPOSIF_OK;
}


#endif /* USE_SOCKETS_TYPE == USE_SOCKETS_LWIP */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
