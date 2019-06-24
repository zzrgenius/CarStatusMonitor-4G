/**
  ******************************************************************************
  * @file    at_custom_modem_specific.h
  * @author  MCD Application Team
  * @brief   Header for at_custom_modem_specific.c module
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
#ifndef AT_CUSTOM_MODEM_UG96_H
#define AT_CUSTOM_MODEM_UG96_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "at_core.h"
#include "at_parser.h"
#include "at_custom_common.h"
#include "at_custom_modem.h"
#include "cellular_service.h"
#include "cellular_service_int.h"
#include "ipc_common.h"

/* Exported constants --------------------------------------------------------*/
#define MODEM_MAX_SOCKET_TX_DATA_SIZE ((uint32_t)1460U) /* cf AT+SQNSSEND */
#define MODEM_MAX_SOCKET_RX_DATA_SIZE ((uint32_t)1500U) /* cf AT+SQNSRECV */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  /* modem specific commands */
  _AT_QPOWD = (_AT_LAST_GENERIC + 1U), /* */
  _AT_QCFG,                           /* Extended configuration settings */ /*Enable/Disable State URC indication (+CPIN,+CFUN,+QUSIM,+QIND) */
  _AT_QUSIM,                          /* URC indication SIM card typpe (SIM or USIM) */
  _AT_QIND,                           /* URC indication */

  /* UG96 specific TCP/IP commands */
  _AT_QICSGP,                         /* Configure parameters of a TCP/IP context */
  _AT_QIACT,                          /* Activate a PDP context */
  _AT_QIDEACT,                        /* Deactivate a PDP context */
  _AT_QIOPEN,                         /* Open a socket service */
  _AT_QICLOSE,                        /* Close a socket service */
  _AT_QISEND,                         /* Send Data - waiting for prompt */
  _AT_QISEND_WRITE_DATA,              /* Send Data - writing data */
  _AT_QIRD,                           /* Retrieve the received TCP/IP Data */
  _AT_QISTATE,                        /* Query socket service status */
  _AT_QIURC,                          /* TCP/IP URC */
  _AT_SOCKET_PROMPT,                  /* when sending socket data : prompt = "> " */
  _AT_SEND_OK,                        /* socket send data OK */
  _AT_SEND_FAIL,                      /* socket send data problem */
  _AT_QIDNSCFG,                       /* configure address of DNS server */
  _AT_QIDNSGIP,                       /* get IP address by Domain Name */
  _AT_QPING,                          /* ping a remote server */

  /* modem specific events (URC, BOOT, ...) */
  _AT_WAIT_EVENT,
  _AT_BOOT_EVENT,
  _AT_SIMREADY_EVENT,
  _AT_RDY_EVENT,
} ATCustom_UG96_cmdlist_t;

/* device specific parameters */
typedef enum
{
  _QCFG_unknown,
  _QCFG_stateurc_check,
  _QCFG_stateurc_enable,
  _QCFG_stateurc_disable,
} ATCustom_UG96_QCFG_function_t;

typedef enum
{
  _QIURC_unknown,
  _QIURC_closed,
  _QIURC_recv,
  _QIURC_incoming_full,
  _QIURC_incoming,
  _QIURC_pdpdeact,
  _QIURC_dnsgip,
} ATCustom_UG96_QIURC_function_t;

typedef enum
{
  /* QIOPEN servic type parameter */
  _QIOPENservicetype_TCP_Client  = 0x0, /* get 2G or 3G serving cell information */
  _QIOPENservicetype_UDP_Client  = 0x1, /* get 2G or 3G neighbour cell information */
  _QIOPENservicetype_TCP_Server  = 0x2, /* get 2G or 3G cell information during packet switch connected */
  _QIOPENservicetype_UDP_Server  = 0x3, /* get 2G or 3G cell information during packet switch connected */
} ATCustom_UG96_QIOPENservicetype_t;

typedef struct
{
  at_bool_t finished;
  at_bool_t wait_header; /* indicate we are waiting for +QIURC: "dnsgip",<err>,<IP_count>,<DNS_ttl> */
  uint32_t  error;
  uint32_t  ip_count;
  uint32_t  ttl;
  AT_CHAR_t hostIPaddr[MAX_SIZE_IPADDR]; /* = host_name parameter from CS_DnsReq_t */
} ug96_qiurc_dnsgip_t;

/* External variables --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void        ATCustom_UG96_init(atparser_context_t *p_atp_ctxt);
uint8_t     ATCustom_UG96_checkEndOfMsgCallback(uint8_t rxChar);
at_status_t ATCustom_UG96_getCmd(atparser_context_t *p_atp_ctxt, uint32_t *p_ATcmdTimeout);
at_endmsg_t ATCustom_UG96_extractElement(atparser_context_t *p_atp_ctxt,
                                         IPC_RxMessage_t *p_msg_in,
                                         at_element_info_t *element_infos);
at_action_rsp_t ATCustom_UG96_analyzeCmd(at_context_t *p_at_ctxt,
                                         IPC_RxMessage_t *p_msg_in,
                                         at_element_info_t *element_infos);
at_action_rsp_t ATCustom_UG96_analyzeParam(at_context_t *p_at_ctxt,
                                           IPC_RxMessage_t *p_msg_in,
                                           at_element_info_t *element_infos);
at_action_rsp_t ATCustom_UG96_terminateCmd(atparser_context_t *p_atp_ctxt, at_element_info_t *element_infos);

at_status_t ATCustom_UG96_get_rsp(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf);
at_status_t ATCustom_UG96_get_urc(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf);
at_status_t ATCustom_UG96_get_error(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf);

#ifdef __cplusplus
}
#endif

#endif /* AT_CUSTOM_MODEM_UG96_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
