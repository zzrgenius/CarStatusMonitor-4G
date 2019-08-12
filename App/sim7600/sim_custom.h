#ifndef SIM_CUSTOM_H
#define SIM_CUSTOM_H
#include "stm32f4xx_hal.h"
#include "sim_core.h"

#define MODEM_DEFAULT_TIMEOUT      ((uint32_t) 10000U)

/* Exported types ------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  INTERMEDIATE_CMD = 0,
  FINAL_CMD        = 1,
} atcustom_FinalCmd_t;
typedef enum
{
  /* standard commands */
  _AT = 0, /* empty command or empty answer */
  _AT_OK,
  _AT_CONNECT,
  _AT_RING,
  _AT_NO_CARRIER,
  _AT_ERROR,
  _AT_NO_DIALTONE,
  _AT_BUSY,
  _AT_NO_ANSWER,
  _AT_CME_ERROR,
  _AT_CMS_ERROR,

  /* 3GPP TS 27.007 and GSM 07.07 commands */
  _AT_CGMI, /* Request manufacturer identification */
  _AT_CGMM, /* Model identification */
  _AT_CGMR, /* Revision identification */
  _AT_CGSN, /* Product serial number identification */
  _AT_CIMI, /* IMSI */
  _AT_CEER, /* Extended error report */
  _AT_CMEE, /* Report mobile equipment error */
  _AT_CPIN, /* Enter PIN */
  _AT_CRESET, /*****reset module *********/

  _AT_CFUN, /* Set phone functionality */
  _AT_COPS, /* Operator selection */
  _AT_CNUM, /* Subscriber number */
  _AT_CGATT,   /* PS attach or detach */
  _AT_CREG,    /* Network registration: enable or disable +CREG urc */
  _AT_CGREG,   /* GPRS network registation status: enable or disable +CGREG urc */
  _AT_CEREG,   /* EPS network registration status: enable or disable +CEREG urc */
  _AT_CGEREP,  /* Packet domain event reporting: enable or disable +CGEV urc */
  _AT_CGEV,    /* EPS bearer indication status */
  _AT_CSQ,     /* Signal quality */
  _AT_CGDCONT, /* Define PDP context */
  _AT_CGACT,   /* PDP context activate or deactivate */
  _AT_CGDATA,  /* Enter Data state */
  _AT_CGPADDR, /* Show PDP Address */

  /* V.25TER commands */
  _ATD,       /* Dial */
  _ATE,       /* Command Echo */
  _ATH,       /* Hook control (disconnect existing connection) */
  _ATO,       /* Return to online data state (switch from COMMAND to DATA mode) */
  _ATV,       /* DCE response format */
  _AT_AND_W,  /* Store current Parameters to User defined profile */
  _AT_AND_D,  /* Set DTR function mode */
  _ATX,       /* CONNECT Result code and monitor call progress */
  _AT_GSN,    /* Request product serial number identification */
  _AT_IPR,    /* Fixed DTE rate */
  _AT_IFC,    /* set DTE-DCE local flow control */

  /* other */
  _AT_ESC_CMD, /* escape command for switching from DATA to COMMAND mode */

  _AT_LAST_GENERIC, /* keep it at last position */

} atcustom_modem_standard_cmd_t;

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  /* modem specific commands */
  _AT_CPOF = (_AT_LAST_GENERIC + 1U), /* */
  _AT_QCFG,                           /* Extended configuration settings */
  _AT_QINDCFG,                        /* URC indication configuration settings */
  _AT_QIND,                           /* URC indication */
  _AT_QUSIM,                          /* URC indication SIM card typpe (SIM or USIM) */
  _AT_CPSMS,                          /* Power Saving Mode Setting */
  _AT_CEDRXS,                         /* e-I-DRX Setting */
  _AT_QNWINFO,                        /* Query Network Information */
  _AT_NETOPEN,                           /* Open a packet network */
  _AT_NETCLOSE,                           /* CLOSE a packet network */

 _AT_CIPHEAD,								/**Add an IP head when receiving data**/
  /* BG96 specific TCP/IP commands */
  _AT_QICSGP,                         /* Configure parameters of a TCP/IP context */
  _AT_QIACT,                          /* Activate a PDP context */
  _AT_QIDEACT,                        /* Deactivate a PDP context */
  _AT_QIOPEN,                         /* Open a TCP service */
  _AT_QICLOSE,                        /* Close a TCP service */
  _AT_QISEND,                         /* Send Data - waiting for prompt */
  _AT_QISEND_WRITE_DATA,              /* Send Data - writing data */
  _AT_QIRD,                           /* Retrieve the received TCP/IP Data */
  _AT_QICFG,                          /* Configure optionnal parameters */
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
  _AT_POWERED_DOWN_EVENT,

} ATCustom_SIM7600_cmdlist_t;

typedef struct atcustom_LUT_struct
{
  uint32_t     cmd_id;
  char         cmd_str[ATCMD_MAX_NAME_SIZE];
  uint32_t     cmd_timeout;
} atcustom_LUT_t;
const char *atcm_get_CmdStr( uint32_t cmd_id);
uint32_t atcm_get_CmdTimeout( uint32_t cmd_id);
at_status_t ATCustom_getCmd(atparser_context_t *p_atp_ctxt, uint32_t *p_ATcmdTimeout);
uint16_t atcm_check_text_line_cmd( at_context_t *p_at_ctxt,RxMessage_t *p_msg_in, at_element_info_t *element_infos);
#endif