#include  "sim_core.h"
#include "main.h"
#include "stdio.h"
#include "string.h"
#include "sim_custom.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sim_service.h"
#include "sim_service_os.h"
#define USE_TRACE_ATCORE 1
/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_ATCORE == 1U)
#if (USE_PRINTF  == 0U)
#include "trace_interface.h"
#define PrintINFO(format, args...) TracePrint(DBG_CHAN_ATCMD, DBL_LVL_P0, "ATCore:" format "\n\r", ## args)
#define PrintDBG(format, args...)  TracePrint(DBG_CHAN_ATCMD, DBL_LVL_P1, "ATCore:" format "\n\r", ## args)
#define PrintAPI(format, args...)  TracePrint(DBG_CHAN_ATCMD, DBL_LVL_P2, "ATCore API:" format "\n\r", ## args)
#define PrintErr(format, args...)  TracePrint(DBG_CHAN_ATCMD, DBL_LVL_ERR, "ATCore ERROR:" format "\n\r", ## args)
#else
#define PrintINFO(format, args...)  printf("ATCore:" format "\n\r", ## args);
#define PrintDBG(format, args...)   printf("ATCore:" format "\n\r", ## args);
#define PrintAPI(format, args...)   printf("ATCore API:" format "\n\r", ## args);
#define PrintErr(format, args...)   printf("ATCore ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#define PrintINFO(format, args...)  do {} while(0);
#define PrintDBG(format, args...)   do {} while(0);
#define PrintAPI(format, args...)   do {} while(0);
#define PrintErr(format, args...)   do {} while(0);
#endif /* USE_TRACE_ATCORE */

 
/* Private defines -----------------------------------------------------------*/
#define ATCORE_SEM_WAIT_ANSWER_COUNT     (1)
#define ATCORE_SEM_SEND_COUNT            (1)
#define MSG_IPC_RECEIVED_SIZE (uint32_t) (16)
#define SIG_IPC_MSG                      (1U) /* signals definition for IPC message queue */

/* Global variables ----------------------------------------------------------*/
/* ATCore task handler */
osThreadId atcoreTaskId = NULL;

/* Private variables ---------------------------------------------------------*/
/* this semaphore is used for waiting for an answer from Modem */
static osSemaphoreId s_WaitAnswer_SemaphoreId = NULL;
/* this semaphore is used to confirm that a msg has been sent (1 semaphore per device) */
#define mySemaphoreDef(name,index)  \
const osSemaphoreDef_t os_semaphore_def_##name##index = { 0 }
#define mySemaphore(name,index)  \
&os_semaphore_def_##name##index
/* Queues definition */
/* this queue is used by IPC to inform that messages are ready to be retrieved */
osMessageQId q_msg_received_Id;

/* Private function prototypes -----------------------------------------------*/
static void ATCoreTaskBody(void const *argument);
static at_status_t sendToIPC( uint8_t *cmdBuf, uint16_t cmdSize);
static at_status_t waitFromIPC(uint32_t tickstart, uint32_t cmdTimeout, RxMessage_t *p_msg);

at_context_t at_context_sim;
static uint8_t       build_atcmd[ATCMD_MAX_CMD_SIZE] = {0};
static RxMessage_t  msgFromIPC;        /* array of IPC msg (1 per ATCore handler) */
static RxMessage_t  msgFromSIM;         

static uint8_t         AT_Core_initialized = 0U;
/* Functions Definition ------------------------------------------------------*/
at_status_t  AT_init(void)
{
uint16_t affectedHandle;
  PrintAPI("enter AT_init()")

  /* should be called once */
  if (AT_Core_initialized == 1U)
  {
//    LogError(1, ERROR_WARNING);
//    return (ATSTATUS_ERROR);
  }

  /* Initialize at_context */
 
 
    at_context_sim.in_data_mode = AT_FALSE;
    at_context_sim.processing_cmd = 0U;
    at_context_sim.dataSent = AT_FALSE;
 
//    at_context_sim.action_flags = ATACTION_RSP_NO_ACTION;
    at_context_sim.p_rsp_buf = NULL;
    at_context_sim.s_SendConfirm_SemaphoreId = NULL;
 

    memset((void *)&at_context_sim.parser, 0, sizeof(atparser_context_t));
	  mySemaphoreDef(ATCORE_SEM_SEND, affectedHandle);
  at_context_sim.s_SendConfirm_SemaphoreId = osSemaphoreCreate(mySemaphore(ATCORE_SEM_SEND, affectedHandle), ATCORE_SEM_SEND_COUNT);
  if (at_context_sim.s_SendConfirm_SemaphoreId == NULL)
  {
    PrintErr("SendSemaphoreId creation error for handle = %d", affectedHandle)
   // LogError(5, ERROR_WARNING);
    return (ATSTATUS_ERROR);
  }
  /* init semaphore */
  osSemaphoreWait(at_context_sim .s_SendConfirm_SemaphoreId, osWaitForever);

  AT_Core_initialized = 1U;
    if (atcore_task_start(osPriorityNormal, 256) != ATSTATUS_OK)
  {
   // ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 2, ERROR_WARNING);
	  __nop();
  }
  return (ATSTATUS_OK);
}

at_status_t atcore_task_start(osPriority taskPrio, uint32_t stackSize)
{
  /* check if AT_init has been called before */
  if (AT_Core_initialized != 1U)
  {
    PrintErr("error, ATCore is not initialized")
   // LogError(22, ERROR_WARNING);
    return (ATSTATUS_ERROR);
  }

  /* semaphores creation */
  osSemaphoreDef(ATCORE_SEM_WAIT_ANSWER);
  s_WaitAnswer_SemaphoreId = osSemaphoreCreate(osSemaphore(ATCORE_SEM_WAIT_ANSWER), ATCORE_SEM_WAIT_ANSWER_COUNT);
  if (s_WaitAnswer_SemaphoreId == NULL)
  {
    PrintErr("s_WaitAnswer_SemaphoreId creation error")
   // LogError(23, ERROR_WARNING);
    return (ATSTATUS_ERROR);
  }
  /* init semaphore */
  osSemaphoreWait(s_WaitAnswer_SemaphoreId, osWaitForever);

  /* queues creation */
  osMessageQDef(IPC_MSG_RCV, MSG_IPC_RECEIVED_SIZE, uint16_t); /* Define message queue */
  q_msg_received_Id = osMessageCreate(osMessageQ(IPC_MSG_RCV), NULL); /* create message queue */

  /* start driver thread */
  osThreadDef(atcoreTask, ATCoreTaskBody, taskPrio, 0, stackSize);
  atcoreTaskId = osThreadCreate(osThread(atcoreTask), NULL);
  if (atcoreTaskId == NULL)
  {
    PrintErr("atcoreTaskId creation error")
   // LogError(24, ERROR_WARNING);
    return (ATSTATUS_ERROR);
  }
  else
  {
#if (STACK_ANALYSIS_TRACE == 1)
    stackAnalysis_addStackSizeByHandle(atcoreTaskId, stackSize);
#endif /* STACK_ANALYSIS_TRACE */
  }

  return (ATSTATUS_OK);
}



  at_status_t waitOnMsgUntilTimeout(  uint32_t Tickstart, uint32_t Timeout)
{
 
 
  if (osSemaphoreWait(s_WaitAnswer_SemaphoreId, Timeout) != osOK)
  {
    PrintDBG("**** Sema Timeout (=%d) !!! *****", Timeout)
    return (ATSTATUS_TIMEOUT);
  }
 
  PrintDBG("**** Sema Freed *****")

 
  return (ATSTATUS_OK);
}
static void reset_current_command(atparser_context_t *p_atp_ctxt)
{
  p_atp_ctxt->current_atcmd.id = _AT_INVALID;
  p_atp_ctxt->current_atcmd.type = ATTYPE_UNKNOWN_CMD;
  memset((void *)&p_atp_ctxt->current_atcmd.name[0], 0, sizeof(uint8_t) * (ATCMD_MAX_NAME_SIZE));
  memset((void *)&p_atp_ctxt->current_atcmd.params[0], 0, sizeof(uint8_t) * (ATCMD_MAX_NAME_SIZE));
  p_atp_ctxt->current_atcmd.raw_cmd_size = 0U;
}

static void reset_parser_context(atparser_context_t *p_atp_ctxt)
{
  p_atp_ctxt->current_SID = SID_INVALID;

  p_atp_ctxt->step = 0U;
  p_atp_ctxt->answer_expected = CMD_MANDATORY_ANSWER_EXPECTED;
  p_atp_ctxt->cmd_timeout = 0U;

  reset_current_command(p_atp_ctxt);

  p_atp_ctxt->p_cmd_input = NULL;
}

at_status_t  ATParser_process_request(at_context_t *p_at_ctxt,  uint16_t msg_id, uint8_t *p_cmd_buf)
{
  at_status_t retval = ATSTATUS_OK;
  PrintAPI("enter ATParser_process_request()")

  /* reset the context */
  reset_parser_context(&p_at_ctxt->parser);

  /* save ptr on input cmd buffer */
  p_at_ctxt->parser.p_cmd_input = p_cmd_buf;

  /* set the SID */
  p_at_ctxt->parser.current_SID = msg_id;

  return (retval);
}


uint16_t  ATParser_get_ATcmd(at_context_t *p_at_ctxt,uint8_t *p_ATcmdBuf, uint16_t *p_ATcmdSize, uint32_t *p_ATcmdTimeout)
{
  uint16_t action = ATACTION_SEND_NO_ACTION;
  PrintAPI("enter ATParser_get_ATcmd()")

  /* init command size to 0 */
  *p_ATcmdSize = 0U;
  reset_current_command(&p_at_ctxt->parser);
	  /* get the next command to send and set timeout value */
  if (ATCustom_getCmd(&p_at_ctxt->parser, p_ATcmdTimeout) != ATSTATUS_OK)
  {
    PrintDBG("parser f_getCmd error")
    action = ATACTION_SEND_ERROR;
  }
	  sprintf((char *) p_at_ctxt->parser.endstr, "\r\n");
    /* Build the command string */
    if (ATTYPE_WRITE_CMD == p_at_ctxt->parser.current_atcmd.type)
    {
      /* get additional parameters for the command if any */
      sprintf((char *)p_ATcmdBuf, "AT%s=%s%s",
              p_at_ctxt->parser.current_atcmd.name, p_at_ctxt->parser.current_atcmd.params, p_at_ctxt->parser.endstr);
      *p_ATcmdSize = (uint16_t)strlen((char *)p_ATcmdBuf);
    }
    else if (ATTYPE_EXECUTION_CMD == p_at_ctxt->parser.current_atcmd.type)
    {
      /* get additional parameters for the command if any */
      sprintf((char *)p_ATcmdBuf, "AT%s%s%s",
              p_at_ctxt->parser.current_atcmd.name, p_at_ctxt->parser.current_atcmd.params, p_at_ctxt->parser.endstr);
      *p_ATcmdSize = (uint16_t)strlen((char *)p_ATcmdBuf);
    }
    else if (ATTYPE_READ_CMD == p_at_ctxt->parser.current_atcmd.type)
    {
      sprintf((char *)p_ATcmdBuf, "AT%s?%s", p_at_ctxt->parser.current_atcmd.name, p_at_ctxt->parser.endstr);
      *p_ATcmdSize = (uint16_t)strlen((char *)p_ATcmdBuf);
    }
    else if (ATTYPE_TEST_CMD == p_at_ctxt->parser.current_atcmd.type)
    {
      sprintf((char *)p_ATcmdBuf, "AT%s=?%s", p_at_ctxt->parser.current_atcmd.name, p_at_ctxt->parser.endstr);
      *p_ATcmdSize = (uint16_t)strlen((char *)p_ATcmdBuf);
    }
    else if (ATTYPE_RAW_CMD == p_at_ctxt->parser.current_atcmd.type)
    {
      /* RAW command: command with NON-AT format
      * send it as provided without header and without end string
      * raw cmd content has been copied into parser.current_atcmd.params
      * its size is in parser.current_atcmd.raw_cmd_size
      *
      */
      if ((p_at_ctxt->parser.current_atcmd.raw_cmd_size != 0U)
          && (p_at_ctxt->parser.current_atcmd.raw_cmd_size <= ATCMD_MAX_CMD_SIZE))
      {
        memcpy((void *)p_ATcmdBuf, (void *)p_at_ctxt->parser.current_atcmd.params, p_at_ctxt->parser.current_atcmd.raw_cmd_size);
        *p_ATcmdSize = (uint16_t)p_at_ctxt->parser.current_atcmd.raw_cmd_size;
      }
      else
      {
        PrintErr("Error with RAW command size = %d", p_at_ctxt->parser.current_atcmd.raw_cmd_size)
      }
    }
    else if (ATTYPE_NO_CMD == p_at_ctxt->parser.current_atcmd.type)
    {
      /* no command to send */
      PrintDBG("no command to send")
      *p_ATcmdSize = 0U;
    }
    else
    {
      PrintErr("Error, invalid command type")
      action = ATACTION_SEND_ERROR;
    }
  

  if (action != ATACTION_SEND_ERROR)
  {
    /* test if cmd is invalid */
    if (p_at_ctxt->parser.current_atcmd.id == _AT_INVALID)
    {
      *p_ATcmdSize = 0U;
    }

    /* Prepare returned code (if no error) */
    if (p_at_ctxt->parser.answer_expected == CMD_MANDATORY_ANSWER_EXPECTED)
    {
      action |= ATACTION_SEND_WAIT_MANDATORY_RSP;
    }
    else if (p_at_ctxt->parser.answer_expected == CMD_OPTIONAL_ANSWER_EXPECTED)
    {
      action |= ATACTION_SEND_TEMPO;
    }
    else /* NO ANSWER EXPECTED */
    {
      PrintErr("Invalid answer_expected value")
      action = ATACTION_SEND_ERROR;
    }
  }

 
   

  PrintDBG("ATParser_get_ATcmd returned action = 0x%x", action)
  return (action);
}


static at_status_t process_AT_transaction( uint16_t msg_in_id, uint8_t *p_rsp_buf)
{
  /*UNUSED(p_rsp_buf);*/

  at_status_t retval;
  uint32_t tickstart;
  uint32_t at_cmd_timeout = 0U;
  uint16_t action_send;
  uint16_t action_rsp;
  uint16_t build_atcmd_size = 0U;
  uint8_t another_cmd_to_send;
  uint8_t send_err_ticks = 3;

  /* reset at cmd buffer */
  memset((void *) build_atcmd, 0, ATCMD_MAX_CMD_SIZE);
  
  do
  {
    memset((void *)&build_atcmd[0], 0, sizeof(char) * ATCMD_MAX_CMD_SIZE);
    build_atcmd_size = 0U;

    /* Get command to send */
    action_send = ATParser_get_ATcmd(&at_context_sim,(uint8_t *)&build_atcmd[0], &build_atcmd_size, &at_cmd_timeout);
    if ((action_send & ATACTION_SEND_ERROR) != 0U)
    {
      PrintDBG("AT_sendcmd error: get at command")
    //  LogError(12, ERROR_WARNING);
      return (ATSTATUS_ERROR);
    }

    /* Send AT command through IPC if a valid command is available */
    if (build_atcmd_size > 0U)
    {
      /* Before to send a command, check if current mode is DATA mode
      *  (exception if request is to suspend data mode)
      */  

      retval = sendToIPC((uint8_t *)&build_atcmd[0], build_atcmd_size);
      if (retval != ATSTATUS_OK)
      {
        PrintErr("AT_sendcmd error: send to ipc")
       // LogError(14, ERROR_WARNING);
        return (ATSTATUS_ERROR);
      }
    }

    /* Wait for a response or a temporation (which could be = 0)*/
  
/* Wait for a response or a temporation (which could be = 0)*/
    if (((action_send & ATACTION_SEND_WAIT_MANDATORY_RSP) != 0U) ||
        ((action_send & ATACTION_SEND_TEMPO) != 0U))
    {
      /* init tickstart for a full AT transaction */
      tickstart = HAL_GetTick();

      do
      {
        /* Wait for response from IPC */
        retval = waitFromIPC( tickstart, at_cmd_timeout, &msgFromIPC);
        if (retval == ATSTATUS_OK)
        {
			action_rsp = ATACTION_RSP_IGNORED;
			//printf("%s\r\n",(char*)msgFromIPC.buffer);
			
          
        }
		else if(retval == ATSTATUS_TIMEOUT)
		{
			return  ATSTATUS_ERROR;
		}
			

 
        /* Retrieve the action which has been set on IPC msg reception in ATCoreTaskBody
        *  More than one action could has been set
        */
//        if ((at_context_sim.action_flags & ATACTION_RSP_FRC_END) != 0U)
//        {
//          action_rsp = ATACTION_RSP_FRC_END;
//          /* clean flag */
//          at_context_sim.action_flags &= ~((uint16_t) ATACTION_RSP_FRC_END);
//        }
//        else if ((at_context_sim.action_flags & ATACTION_RSP_FRC_CONTINUE) != 0U)
//        {
//          action_rsp = ATACTION_RSP_FRC_CONTINUE;
//          /* clean flag */
//          at_context_sim.action_flags &= ~((uint16_t) ATACTION_RSP_FRC_CONTINUE);
//        }
//        else if ((at_context_sim.action_flags & ATACTION_RSP_ERROR) != 0U)
//        {
//          action_rsp = ATACTION_RSP_ERROR;
//          /* clean flag */
//          at_context_sim.action_flags &= ~((uint16_t) ATACTION_RSP_ERROR);
//          PrintDBG("AT_sendcmd error: parse from rsp")
//         // LogError(16, ERROR_WARNING);
//          return (ATSTATUS_ERROR);
//        }
//        else
//        {
//          /* all other actions are ignored */
//          action_rsp = ATACTION_RSP_IGNORED;
//        }
 
        /* continue if this is an intermediate response */
 
      }  while (  action_rsp == ATACTION_RSP_NO_ACTION );
         

      if (action_rsp == ATACTION_RSP_FRC_CONTINUE)
      {
        another_cmd_to_send = 1U;
      }
      else
      {
        /* FRC_END, ERRORS,... */
        another_cmd_to_send = 0U;
      }
    }
    else
    {
      PrintErr("Invalid action code")
     // LogError(18, ERROR_WARNING);
      return (ATSTATUS_ERROR);
    }

  }  while (another_cmd_to_send == 1U);

  /* clear all flags*/
  at_context_sim.action_flags = ATACTION_RSP_NO_ACTION;

  PrintDBG("action_rsp value = %d", action_rsp)
  if (action_rsp == ATACTION_RSP_ERROR)
  {
    //LogError(19, ERROR_WARNING);
    return (ATSTATUS_ERROR);
  }
 return (ATSTATUS_OK);
  }


static at_status_t sendToIPC( uint8_t *cmdBuf, uint16_t cmdSize)
{
  PrintAPI("enter sendToIPC()")

  /* Send AT command */
  SIM_send_uart(cmdBuf, cmdSize);
 

#if (RTOS_USED == 1)
  osSemaphoreWait(at_context_sim.s_SendConfirm_SemaphoreId, osWaitForever);
#else
  /* waiting TX confirmation, done by callback from IPC */
  while (at_context_sim.dataSent == AT_FALSE)
  {
  }
  at_context_sim.dataSent = AT_FALSE;
#endif /* RTOS_USED */

  return (ATSTATUS_OK);
}

static at_status_t waitFromIPC(uint32_t tickstart, uint32_t cmdTimeout, RxMessage_t *p_msg)
{
  
  at_status_t retval;
  PrintAPI("enter waitFromIPC()")

  /* wait for complete message */
  retval = waitOnMsgUntilTimeout( tickstart, cmdTimeout);
  if (retval != ATSTATUS_OK)
  {
    if (cmdTimeout != 0U)
    {
      PrintINFO("TIMEOUT EVENT(%d ms)", cmdTimeout)
    }
    else
    {
      /* PrintINFO("No timeout"); */
    }
    return (retval);
  }

 
  return (ATSTATUS_OK);
}

at_status_t ATParser_get_rsp(at_context_t *p_at_ctxt, uint8_t *p_rsp_buf)
{
  at_status_t retval = ATSTATUS_OK;
  PrintAPI("enter ATParser_get_rsp()")

  //retval = atcc_get_rsp(p_at_ctxt, p_rsp_buf);

  /* current SID treament is finished, reset parser context */
  reset_parser_context(&p_at_ctxt->parser);

  return (retval);
}
at_status_t AT_sendcmd(   uint16_t msg_in_id, uint8_t *p_cmd_in_buf, uint8_t *p_rsp_buf)
{
 
  at_status_t retval;

  PrintAPI("enter AT_sendcmd()")

  /* Check if a command is already ongoing for this handle */
  if (at_context_sim.processing_cmd == 1U)
  {
    PrintErr("!!!!!!!!!!!!!!!!!! WARNING COMMAND IS UNDER PROCESS !!!!!!!!!!!!!!!!!!")
    retval = ATSTATUS_ERROR;
  //  LogError(7, ERROR_WARNING);
    goto exit_func;
  }

  /* initialize response buffer */
  memset((void *)p_rsp_buf, 0, ATCMD_MAX_BUF_SIZE);
//  memset(msgFromIPC.buffer, 0,RXBUF_MAXSIZE);
  /* start to process this command */
  at_context_sim.processing_cmd = 1U;

 
  /* save ptr on response buffer */
  at_context_sim.p_rsp_buf = p_rsp_buf;
  /* RTOS_USED */

  /* Check if current mode is DATA mode */
  if ( at_context_sim.in_data_mode == AT_TRUE)
  {
    /* Check if user command is DATA suspend */
//    if (msg_in_id ==  )
//    {
//      /* restore IPC Command channel to send ESCAPE COMMAND */
//      PrintDBG("<<< restore IPC COMMAND channel >>>")
//      
//    }
  }
 
  /* Process the user request */
  retval = ATParser_process_request(&at_context_sim, msg_in_id, p_cmd_in_buf);
  if (retval != ATSTATUS_OK)
  {
    PrintDBG("AT_sendcmd error: process request")
   // ATParser_abort_request(&at_context_sim);
    goto exit_func;
  }

  /* Start an AT command transaction */
  retval = process_AT_transaction( msg_in_id, p_rsp_buf);
  if (retval != ATSTATUS_OK)
  {
    PrintDBG("AT_sendcmd error: process AT transaction")
    /* retrieve and send error report if exist */
   // ATParser_get_error(&at_context_sim, p_rsp_buf);
   // ATParser_abort_request(&at_context_sim);
    goto exit_func;
  }
	
  /* get command response buffer */
//  ATParser_get_rsp(&at_context_sim, p_rsp_buf);
	memcpy(p_rsp_buf, msgFromIPC.buffer,ATCMD_MAX_BUF_SIZE);     
exit_func:
  /* finished to process this command */
  at_context_sim.processing_cmd = 0U;

  return (retval);
}

 extern CST_context_t     cst_context;

 uint16_t ATParser_parse_rsp(at_context_t *p_at_ctxt, RxMessage_t *p_message)
{
  uint16_t cmd_retval = ATACTION_RSP_ERROR, param_retval = ATACTION_RSP_IGNORED, final_retval, clean_retval;
  at_endmsg_t msg_end = ATENDMSG_NO;
	uint16_t socket_len = 0;
	uint8_t rec_buf[RXBUF_MAXSIZE];
	char *pstr;
  //at_element_info_t element_infos = { .current_parse_idx = 0, .cmd_id_received = _AT_INVALID, .param_rank = 0U, .str_start_idx = 0, .str_end_idx = 0, .str_size = 0        };
  uint16_t data_mode;

  PrintAPI("enter ATParser_parse_rsp()")
	//PrintDBG("%s\r\n",(char *)p_message->buffer)
	
	// data_len = rt_ringbuffer_data_len(&gSim_ringbuf);
  if(p_message->size > 4)
  {
	  memcpy(rec_buf,(uint8_t*)p_message->buffer, p_message->size);
	  p_message->size = 0;
	 
	  pstr = strstr((char *)rec_buf,"+IPD");
	  if(pstr != NULL)
	{
		socket_len = atoi((char*)(pstr+4)); 
		osCDS_socket_receive( (char*)(pstr+4), socket_len);

		PrintAPI("socket rec len is %d\r\n",socket_len)
	}
	  if(strstr((char *)rec_buf,"++CIPCLOSE: ") != NULL)
	  {
		  cst_context.current_state = CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE;
	  }
}
  /* DUMP RECEIVE BUFFER */
  //display_buffer(p_at_ctxt, (uint8_t *)&p_message->buffer[0], (uint16_t)p_message->size, 0U);

  /* extract next element to analyze */
//  msg_end = atcc_extractElement(p_at_ctxt, p_message, &element_infos);

//  /* Search for command name */
//  cmd_retval = atcc_analyzeCmd(p_at_ctxt, p_message, &element_infos);

//  /* extract return code only (remove data mode flag) */
//  clean_retval = (at_action_rsp_t)(cmd_retval & ~(at_action_rsp_t)ATACTION_RSP_FLAG_DATA_MODE);
//  data_mode = (((uint16_t)cmd_retval & (uint16_t)ATACTION_RSP_FLAG_DATA_MODE) != 0U) ? (uint16_t)1U : (uint16_t)0U;
//  PrintDBG("analyzeCmd retval = %d (DATA mode=%d) msg_end = %d", clean_retval, data_mode, (msg_end == ATENDMSG_YES))

//  if ((msg_end != ATENDMSG_YES) && (cmd_retval != ATACTION_RSP_ERROR))
//  {
//    PrintDBG("proceed to params")

//    /* Search for command parameters */
//    param_retval = atcc_analyzeParam(p_at_ctxt, p_message, &element_infos);
//    if (param_retval != ATACTION_RSP_IGNORED)
//    {
//      /* action has been modified by analysis of parameters */
//      PrintDBG("action modified by analysis of params: %d to %d", cmd_retval, param_retval)
//      clean_retval = param_retval;
//    }
//  }

  /* if AT cmd treatment is finished, check if another AT cmd should be sent after */
//  if (clean_retval == ATACTION_RSP_FRC_END)
//  {
//    /* final result code for current AT cmd: clean related context params */
//    final_retval = atcc_terminateCmd(p_at_ctxt, &element_infos);
//    if (final_retval == ATACTION_RSP_ERROR)
//    {
//      clean_retval = ATACTION_RSP_ERROR;
//    }
//    /* do we have another command to send for this SID ? */
//    else if (p_at_ctxt->parser.is_final_cmd == 0U)
//    {
//      clean_retval = ATACTION_RSP_FRC_CONTINUE;
//    }
//    else
//    {
//      /* ignore */
//    }

//    /* current CMD treament is finished: reset command context */
//    reset_current_command(&p_at_ctxt->parser);
//  }

  /* reintegrate data mode flag if needed */
  if (data_mode)
  {
    //cmd_retval = (at_action_rsp_t)(clean_retval | ATACTION_RSP_FLAG_DATA_MODE);
  }
  else
  {
    //cmd_retval = clean_retval;
  }

  PrintDBG("ATParser_parse_rsp returned action = 0x%x", cmd_retval)
  return (cmd_retval);
}


static void ATCoreTaskBody(void const *argument)
{
  /*UNUSED(argument);*/

  at_status_t retUrc;
   at_status_t ret;
  uint16_t action;
  osEvent event;

  PrintAPI("<start ATCore TASK>")

  /* Infinite loop */
  for (;;)
  {
    /* waiting IPC message received event (message) */
    event = osMessageGet(q_msg_received_Id, osWaitForever);
    if (event.status == osEventMessage)
    {
      PrintDBG("<ATCore TASK> - received msg event= 0x%x", event.value.v)

      if (event.value.v == (SIG_IPC_MSG))
      {
        /* a message has been received, retrieve its handle */
      
			//SIM_GetBuffer( &msgFromIPC) ;
		  
		  if(at_context_sim.parser.answer_wait)
		  {
			  at_context_sim.parser.answer_wait--;
		  }
		  else
		  {
			  SIM_GetBuffer( &msgFromIPC) ;
			  if(msgFromIPC.size > 0)
			  {
				  
			   action = ATParser_parse_rsp(&at_context_sim, &msgFromIPC);
				  
			  }
			   osSemaphoreRelease(s_WaitAnswer_SemaphoreId);
		  }
        /* retrieve message from IPC */
//        if (IPC_receive( &msgFromIPC) == IPC_ERROR)
//        {
//           
//          PrintDBG("**** Sema Realesed on error 1 *****")
//          osSemaphoreRelease(s_WaitAnswer_SemaphoreId);
//          /* skip this loop iteration */
//          continue;
//        }
	 
        /* one message has been read */
      
//        /* Parse the response */
//        action = ATParser_parse_rsp(&at_context_sim, &msgFromIPC[athandle]);

//        /* analyze the response (check data mode flag) */
//        action = analyze_action_result(athandle, action);

      
       //   osSemaphoreRelease(s_WaitAnswer_SemaphoreId);
		
//        /* check if this is an URC to forward */
//        if (action == ATACTION_RSP_URC_FORWARDED)
//        {
//           
//        }
//        else if ((action == ATACTION_RSP_FRC_CONTINUE) ||
//                 (action == ATACTION_RSP_FRC_END) ||
//                 (action == ATACTION_RSP_ERROR))
//        {
//          PrintDBG("**** Sema released *****")
//          osSemaphoreRelease(s_WaitAnswer_SemaphoreId);
//        }
//        else
//        {
//          /* nothing to do */
//        }

      }
    }
  }
}
uint32_t ATutil_ipow(uint32_t base, uint16_t exp)
{
  /* implementation of power function */
  uint32_t result = 1U;
  while (exp)
  {
    if ((exp & 1U) != 0U)
    {
      result *= base;
    }
    exp >>= 1;
    base *= base;
  }

  return result;
}
uint32_t ATutil_convertStringToInt(uint8_t *p_string, uint16_t size)
{
  uint16_t idx, nb_digit_ignored = 0U, loop = 0U;
  uint32_t conv_nbr = 0U;

  /* auto-detect if this is an hexa value (format: 0x....) */
  if (p_string[1] == 120U)
  {
    /* check if the value exceed maximum size */
    if (size > 10U)
    {
      /* PrintErr("String to convert exceed maximum size"); */
      return (0u);
    }

    /* hexa value */
    nb_digit_ignored = 2U;
    for (idx = 2U; idx < size; idx++)
    {
      /* consider only the numbers */
      if ((p_string[idx] >= 48U) && (p_string[idx] <= 57U))
      {
        /* 0 to 9 */
        loop++;
        conv_nbr = conv_nbr + ((p_string[idx] - 48U) * ATutil_ipow(16U, (size - loop - nb_digit_ignored)));
      }
      else if ((p_string[idx] >= 97U) && (p_string[idx] <= 102U))
      {
        /* a to f */
        loop++;
        conv_nbr = conv_nbr + ((p_string[idx] - 97U + 10U) * ATutil_ipow(16U, (size - loop - nb_digit_ignored)));
      }
      else if ((p_string[idx] >= 65U) && (p_string[idx] <= 70U))
      {
        /* A to F */
        loop++;
        conv_nbr = conv_nbr + ((p_string[idx] - 65U + 10U) * ATutil_ipow(16U, (size - loop - nb_digit_ignored)));
      }
      else
      {
        nb_digit_ignored++;
      }
    }
  }
  else
  {
    /* decimal value */
    for (idx = 0U; idx < size; idx++)
    {
      /* consider only the numbers */
      if ((p_string[idx] >= 48U) && (p_string[idx] <= 57U))
      {
        loop++;
        conv_nbr = conv_nbr + ((p_string[idx] - 48U) * ATutil_ipow(10U, (size - loop - nb_digit_ignored)));
      }
      else
      {
        nb_digit_ignored++;
      }
    }
  }

  return (conv_nbr);
}
