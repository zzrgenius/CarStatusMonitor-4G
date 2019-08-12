#ifndef _SIM_CORE_H
#define _SIM_CORE_H

#include "main.h"
#include "sim_drv.h"
#include "cmsis_os.h"
/* Exported types ------------------------------------------------------------*/
#define ATCMD_MAX_NAME_SIZE  ((uint16_t) 32U)
#define ATCMD_MAX_CMD_SIZE   ((uint16_t) 128U)
#define ATCMD_MAX_BUF_SIZE   ((uint16_t) 128U) 

#define _AT_INVALID ((uint32_t) 0xFFFFFFFFU)
#define SID_INVALID               (0U)
#define CELLULAR_SERVICE_START_ID (100U)
/* at_action_send_t
 * code returned when preparing a command to send
 */
#define ATACTION_SEND_NO_ACTION          ((uint16_t) 0x0000) /* No action defined yet (used to reset flags) */
#define ATACTION_SEND_WAIT_MANDATORY_RSP ((uint16_t) 0x0001) /* Wait a response (mandatory: if not received, this is an error) */
#define ATACTION_SEND_TEMPO              ((uint16_t) 0x0002) /* Temporisation for waiting an eventual response or event */
#define ATACTION_SEND_ERROR              ((uint16_t) 0x0004) /* Error: unknown msg, etc... */
#define ATACTION_SEND_FLAG_LAST_CMD      ((uint16_t) 0x8000)  /* when this flag is set, this is the final command */

/* at_action_rsp_t
 * code returned when analyzing a response
 */
#define ATACTION_RSP_NO_ACTION          ((uint16_t) 0x0000) /* No action defined yet (used to reset flags) */
#define ATACTION_RSP_FRC_END            ((uint16_t) 0x0001) /* Received Final Result Code (OK, CONNECT...), no more AT command to send */
#define ATACTION_RSP_FRC_CONTINUE       ((uint16_t) 0x0002) /* Received Final Result Code (OK, CONNECT...), still AT command to send */
#define ATACTION_RSP_ERROR              ((uint16_t) 0x0004) /* Error: unknown msg, etc... */
#define ATACTION_RSP_INTERMEDIATE       ((uint16_t) 0x0008) /* FRC not received for this AT transaction */
#define ATACTION_RSP_IGNORED            ((uint16_t) 0x0010) /* Message ignored */
#define ATACTION_RSP_URC_IGNORED        ((uint16_t) 0x0020) /* Result Code received, to ignore (default) */
#define ATACTION_RSP_URC_FORWARDED      ((uint16_t) 0x0040) /* Unsolicited Result Code received, to forward to client */
#define ATACTION_RSP_FLAG_DATA_MODE     ((uint16_t) 0x8000) /* when this flag is set, DATA mode activated in modem */



typedef enum
{
  AT_FALSE = 0,
  AT_TRUE  = 1,
} at_bool_t;
typedef enum
{
  ATENDMSG_YES = 0,
  ATENDMSG_NO,
  ATENDMSG_ERROR,
} at_endmsg_t;

typedef enum
{
  ATSTATUS_OK = 0,
  ATSTATUS_ERROR,
  ATSTATUS_TIMEOUT,
  ATSTATUS_OK_PENDING_URC,
} at_status_t;

typedef enum
{
  CMD_MANDATORY_ANSWER_EXPECTED   = 0,
  CMD_OPTIONAL_ANSWER_EXPECTED    = 1,
} atparser_AnswerExpect_t;
typedef enum
{
  ATTYPE_UNKNOWN_CMD = 0, /* unknown AT cmd type                 */
  ATTYPE_TEST_CMD,        /* AT test cmd type:      AT+<x>=?     */
  ATTYPE_READ_CMD,        /* AT read cmd type:      AT+<x>?      */
  ATTYPE_WRITE_CMD,       /* AT write cmd type:     AT+<x>=<...> */
  ATTYPE_EXECUTION_CMD,   /* AT execution cmd type: AT+<x>       */
  ATTYPE_NO_CMD,          /* No command to send */
  ATTYPE_RAW_CMD,         /* RAW command to send (Non AT cmd type) */
} at_type_t;

typedef struct
{
  at_type_t    type;
  uint32_t     id;
  uint8_t      name[ATCMD_MAX_NAME_SIZE];
  uint8_t      params[ATCMD_MAX_CMD_SIZE];
  uint32_t     raw_cmd_size;                   /* raw_cmd_size is used only for raw commands */
} atcmd_desc_t;

typedef struct
{
  /* parameters set in AT Parser */
  uint16_t             current_SID;     /* Current Service ID */

  /* parameters set in AT Custom */
  uint8_t                 answer_wait;
  uint8_t                  step;            /* indicates which step in current SID treatment */
  atparser_AnswerExpect_t  answer_expected; /* expected answer type for this command */
  atcmd_desc_t             current_atcmd;   /* current AT command to send parameters */
  uint8_t                  endstr[3];       /* termination string for AT cmd */
  uint32_t                 cmd_timeout;     /* command timeout value */

  /* save ptr on input buffer */
  uint8_t              *p_cmd_input;

} atparser_context_t;

typedef struct
{
  /* Core context */
 
  uint8_t             in_data_mode;   /* current mode is DATA mode or COMMAND mode */
  uint8_t             processing_cmd; /* indicate if a command is currently processed or if idle mode */
  uint8_t             dataSent;       /* receive the confirmation that data has been sent by the IPC */

  /* Parser context */
  atparser_context_t   parser; 
  /* RTOS parameters */
  uint16_t     		action_flags;
  uint8_t            *p_rsp_buf;
  osSemaphoreId       s_SendConfirm_SemaphoreId;
 } at_context_t;
  typedef struct
{
  uint16_t    current_parse_idx; /* current parse index in the input buffer */
  uint32_t    cmd_id_received;   /* cmd id received */
  uint16_t    param_rank;        /* current param number/rank */
  uint16_t    str_start_idx;     /* current param start index in the message */
  uint16_t    str_end_idx;       /* current param end index in the message */
  uint16_t    str_size;          /* current param size */
} at_element_info_t;


at_status_t  AT_init(void);
at_status_t atcore_task_start(osPriority taskPrio, uint32_t stackSize);
at_status_t AT_sendcmd(   uint16_t msg_in_id, uint8_t *p_cmd_in_buf, uint8_t *p_rsp_buf);
uint32_t ATutil_convertStringToInt(uint8_t *p_string, uint16_t size);
#endif
