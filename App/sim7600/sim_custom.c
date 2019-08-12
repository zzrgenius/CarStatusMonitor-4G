
#include "main.h"
#include "sim_custom.h"
#include "sim_service.h"


#if (USE_TRACE_ATCUSTOM == 1U)
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

#define SIM7600_DEFAULT_TIMEOUT  ((uint32_t)15000)
#define SIM7600_RDY_TIMEOUT      ((uint32_t)30000)
#define SIM7600_SIMREADY_TIMEOUT ((uint32_t)5000)
#define SIM7600_ESCAPE_TIMEOUT   ((uint32_t)1000)  /* maximum time allowed to receive a response to an Escape command */
#define SIM7600_COPS_TIMEOUT     ((uint32_t)180000) /* 180 sec */
#define SIM7600_CGATT_TIMEOUT    ((uint32_t)140000) /* 140 sec */
#define SIM7600_CGACT_TIMEOUT    ((uint32_t)150000) /* 150 sec */
#define SIM7600_ATH_TIMEOUT      ((uint32_t)90000) /* 90 sec */
#define SIM7600_AT_TIMEOUT       ((uint32_t)3000)  /* timeout for AT */
#define SIM7600_SOCKET_PROMPT_TIMEOUT ((uint32_t)10000)
#define SIM7600_QIOPEN_TIMEOUT   ((uint32_t)150000) /* 150 sec */
#define SIM7600_QICLOSE_TIMEOUT  ((uint32_t)150000) /* 150 sec */
#define SIM7600_QIACT_TIMEOUT    ((uint32_t)150000) /* 150 sec */
#define SIM7600_QIDEACT_TIMEOUT  ((uint32_t)40000) /* 40 sec */
#define SIM7600_QNWINFO_TIMEOUT  ((uint32_t)1000) /* 1000ms */
#define SIM7600_QIDNSGIP_TIMEOUT ((uint32_t)60000) /* 60 sec */
#define SIM7600_QPING_TIMEOUT    ((uint32_t)150000) /* 150 sec */

	
/* Commands Look-up table */
const atcustom_LUT_t ATCMD_SIM7600_LUT[] =
{
  /* cmd enum - cmd string - cmd timeout (in ms) - build cmd ftion - analyze cmd ftion */
  {_AT,             "",             SIM7600_AT_TIMEOUT},
  {_AT_OK,          "OK",           SIM7600_DEFAULT_TIMEOUT },
  {_AT_CONNECT,     "CONNECT",      SIM7600_DEFAULT_TIMEOUT },
  {_AT_RING,        "RING",         SIM7600_DEFAULT_TIMEOUT },
  {_AT_NO_CARRIER,  "NO CARRIER",   SIM7600_DEFAULT_TIMEOUT },
  {_AT_ERROR,       "ERROR",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_NO_DIALTONE, "NO DIALTONE",  SIM7600_DEFAULT_TIMEOUT },
  {_AT_BUSY,        "BUSY",         SIM7600_DEFAULT_TIMEOUT },
  {_AT_NO_ANSWER,   "NO ANSWER",    SIM7600_DEFAULT_TIMEOUT },
  {_AT_CME_ERROR,   "+CME ERROR",   SIM7600_DEFAULT_TIMEOUT },
  {_AT_CMS_ERROR,   "+CMS ERROR",   SIM7600_DEFAULT_TIMEOUT },

  /* GENERIC MODEM commands */
  {_AT_CGMI,        "+CGMI",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_CGMM,        "+CGMM",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_CGMR,        "+CGMR",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_CGSN,        "+CGSN",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_GSN,         "+GSN",         SIM7600_DEFAULT_TIMEOUT },
  {_AT_CIMI,        "+CIMI",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_CEER,        "+CEER",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_CMEE,        "+CMEE",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_CPIN,        "+CPIN",        SIM7600_DEFAULT_TIMEOUT },
  
  {_AT_CRESET,        "+CRESET",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_CFUN,        "+CFUN",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_COPS,        "+COPS",        SIM7600_COPS_TIMEOUT	},
  {_AT_CNUM,        "+CNUM",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_CGATT,       "+CGATT",       SIM7600_CGATT_TIMEOUT    },
  {_AT_CGPADDR,     "+CGPADDR",     SIM7600_DEFAULT_TIMEOUT  },
  {_AT_CEREG,       "+CEREG",       SIM7600_DEFAULT_TIMEOUT  },
  {_AT_CREG,        "+CREG",        SIM7600_DEFAULT_TIMEOUT  },
  {_AT_CGREG,       "+CGREG",       SIM7600_DEFAULT_TIMEOUT  },
  {_AT_CSQ,         "+CSQ",         SIM7600_DEFAULT_TIMEOUT  },
  {_AT_CGDCONT,     "+CGDCONT",     SIM7600_DEFAULT_TIMEOUT  },
  {_AT_CGACT,       "+CGACT",       SIM7600_CGACT_TIMEOUT    },
  {_AT_CGDATA,      "+CGDATA",      SIM7600_DEFAULT_TIMEOUT  },
  {_AT_CGEREP,      "+CGEREP",      SIM7600_DEFAULT_TIMEOUT  },
  {_AT_CGEV,        "+CGEV",        SIM7600_DEFAULT_TIMEOUT  },
  {_ATD,            "D",            SIM7600_DEFAULT_TIMEOUT  },
  {_ATE,            "E",            SIM7600_DEFAULT_TIMEOUT  },
  {_ATH,            "H",            SIM7600_ATH_TIMEOUT     },
  {_ATO,            "O",            SIM7600_DEFAULT_TIMEOUT },
  {_ATV,            "V",            SIM7600_DEFAULT_TIMEOUT  },
  {_ATX,            "X",            SIM7600_DEFAULT_TIMEOUT  },
  {_AT_ESC_CMD,     "+++",          SIM7600_ESCAPE_TIMEOUT   },
  {_AT_IPR,         "+IPR",         SIM7600_DEFAULT_TIMEOUT  },
  {_AT_IFC,         "+IFC",         SIM7600_DEFAULT_TIMEOUT  },
  {_AT_AND_W,       "&W",           SIM7600_DEFAULT_TIMEOUT },
  {_AT_AND_D,       "&D",           SIM7600_DEFAULT_TIMEOUT },

  /* MODEM SPECIFIC COMMANDS */
  {_AT_CPOF,       "+CPOF",       SIM7600_DEFAULT_TIMEOUT },
  {_AT_QCFG,        "+CIPMODE",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_QINDCFG,    "+QINDCFG",     SIM7600_DEFAULT_TIMEOUT  },
  {_AT_QIND,        "+QIND",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_QUSIM,       "+QUSIM",       SIM7600_DEFAULT_TIMEOUT },
  {_AT_CPSMS,       "+CPSMS",       SIM7600_DEFAULT_TIMEOUT },
  {_AT_CEDRXS,      "+CEDRXS",      SIM7600_DEFAULT_TIMEOUT },
  {_AT_QNWINFO,     "+QNWINFO",     SIM7600_QNWINFO_TIMEOUT },
  {_AT_NETOPEN,     "+NETOPEN",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_NETCLOSE,     "+NETCLOSE",        SIM7600_DEFAULT_TIMEOUT },

  {_AT_CIPHEAD,     "+CIPHEAD",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_QICSGP,      "+CSOCKSETPN",      SIM7600_DEFAULT_TIMEOUT },
  {_AT_QIACT,       "+IPADDR",       SIM7600_QIACT_TIMEOUT   },
  {_AT_QIOPEN,      "+CIPOPEN",      SIM7600_QIOPEN_TIMEOUT  },
  {_AT_QICLOSE,     "+CIPCLOSE",     SIM7600_QICLOSE_TIMEOUT },
  {_AT_QISEND,      "+CIPSEND",      SIM7600_RDY_TIMEOUT },
  {_AT_QISEND_WRITE_DATA,  "",      SIM7600_DEFAULT_TIMEOUT },
  {_AT_QIRD,        "+QIRD",        SIM7600_DEFAULT_TIMEOUT },
  {_AT_QICFG,       "+QICFG",       SIM7600_DEFAULT_TIMEOUT },
  {_AT_QISTATE,     "+QISTATE",     SIM7600_DEFAULT_TIMEOUT },
  {_AT_QIURC,       "+QIURC",       SIM7600_DEFAULT_TIMEOUT },
  {_AT_SOCKET_PROMPT, "> ",         SIM7600_SOCKET_PROMPT_TIMEOUT },
  {_AT_SEND_OK,      "SEND OK",     SIM7600_DEFAULT_TIMEOUT },
  {_AT_SEND_FAIL,    "SEND FAIL",   SIM7600_DEFAULT_TIMEOUT },
  {_AT_QIDNSCFG,     "+QIDNSCFG",   SIM7600_DEFAULT_TIMEOUT  }, 
  {_AT_QIDNSGIP,     "+IPADDR",   SIM7600_QIDNSGIP_TIMEOUT },
  {_AT_QPING,        "+QPING",      SIM7600_QPING_TIMEOUT  },

  /* MODEM SPECIFIC EVENTS */
  {_AT_WAIT_EVENT,     "",          SIM7600_DEFAULT_TIMEOUT  },
  {_AT_BOOT_EVENT,     "",          SIM7600_RDY_TIMEOUT    },
  {_AT_RDY_EVENT,      "RDY",       SIM7600_RDY_TIMEOUT    },
  {_AT_POWERED_DOWN_EVENT, "POWERED DOWN",       SIM7600_RDY_TIMEOUT    },
};
#define size_ATCMD_SIM7600_LUT ((uint16_t) (sizeof (ATCMD_SIM7600_LUT) / sizeof (atcustom_LUT_t)))
/**
  * @brief  Search command string corresponding to a command Id
  *
  * @param  p_modem_ctxt modem context
  * @param  cmd_id Id of the command to find
  * @retval string of the command name
  */
const char *atcm_get_CmdStr( uint32_t cmd_id)
{
  uint16_t i;

  /* check if this is the invalid cmd id*/
  if (cmd_id != _AT_INVALID)
  {
    /* search in LUT the cmd ID */
    for (i = 0U; i < size_ATCMD_SIM7600_LUT; i++)
    {
      if (ATCMD_SIM7600_LUT[i].cmd_id == cmd_id)
      {
        return (const char *)(&ATCMD_SIM7600_LUT[i].cmd_str);
      }
    }
  }

  /* return default value */
  PrintDBG("exit atcm_get_CmdStr() with default value ")
  return (((const char *)"\0"));
}

/**
  * @brief  Search timeout corresponding to a command Id
  *
  * @param  p_modem_ctxt modem context
  * @param  cmd_id Id of the command to find
  * @retval timeout for this command
  */
uint32_t atcm_get_CmdTimeout( uint32_t cmd_id)
{
  uint16_t i;

  /* check if this is the invalid cmd id */
  if (cmd_id != _AT_INVALID)
  {
    /* search in LUT the cmd ID */
    for (i = 0U; i <  size_ATCMD_SIM7600_LUT; i++)
    {
      if (ATCMD_SIM7600_LUT[i].cmd_id == cmd_id)
      {
        return (ATCMD_SIM7600_LUT[i].cmd_timeout);
      }
    }
  }

  /* return default value */
  PrintDBG("exit atcm_get_CmdTimeout() with default value ")
  return (MODEM_DEFAULT_TIMEOUT);
} 

/*
*  Program an AT command: answer is mandatory, an error will be raised if no answer received before timeout
*/
void atcm_program_AT_CMD( atparser_context_t *p_atp_ctxt, at_type_t cmd_type, uint32_t cmd_id)
{
  /* command type */
  p_atp_ctxt->current_atcmd.type = cmd_type;
  /* command id */
  p_atp_ctxt->current_atcmd.id = cmd_id;
  /* an answer is expected */
  p_atp_ctxt->answer_expected = CMD_MANDATORY_ANSWER_EXPECTED;

  /* set command timeout according to LUT */
  p_atp_ctxt->cmd_timeout =  atcm_get_CmdTimeout(p_atp_ctxt->current_atcmd.id);
}

at_status_t atcm_modem_build_cmd( atparser_context_t *p_atp_ctxt, uint32_t *p_ATcmdTimeout)
{
  at_status_t retval = ATSTATUS_OK;

  /* 1- set the commande name (get it from LUT) */
  sprintf((char *)p_atp_ctxt->current_atcmd.name,(const char *)atcm_get_CmdStr(p_atp_ctxt->current_atcmd.id)); 

  PrintDBG("<modem custom> build the cmd %s (type=%d)",    p_atp_ctxt->current_atcmd.name, p_atp_ctxt->current_atcmd.type)

  /* 2- set the command parameters (only for write or execution commands or for data) */
  if ((p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD) ||
      (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD) ||
      (p_atp_ctxt->current_atcmd.type == ATTYPE_RAW_CMD))
  {
    //retval = (atcm_get_CmdBuildFunc(p_modem_ctxt, p_atp_ctxt->current_atcmd.id))(p_atp_ctxt, p_modem_ctxt);
  }
	
  if(p_atp_ctxt->current_atcmd.id == _AT)
	  p_atp_ctxt->answer_wait = 1;
  /* 3- set command timeout (has been sset in command programmation) */
  *p_ATcmdTimeout = p_atp_ctxt->cmd_timeout;
  PrintDBG("==== CMD TIMEOUT = %ld ====", *p_ATcmdTimeout)

 
  PrintDBG("atcm_modem_build_cmd returned status = %d", retval)
  return (retval);
}

extern uint8_t gCS_Send_steps;
extern csint_pdn_infos_t   	cs_pdn_infos;
extern csint_socket_infos_t cs_ctxt_sockets_info; /* socket infos (array index = socket handle) */
extern 	   csint_socket_data_buffer_t send_data_struct;

at_status_t ATCustom_getCmd(atparser_context_t *p_atp_ctxt, uint32_t *p_ATcmdTimeout)
{
  at_status_t retval = ATSTATUS_OK;
  uint16_t curSID = p_atp_ctxt->current_SID;
  static uint8_t sid_cmdid_ticks = 0;
	char strtemp[8];
  PrintAPI("enter ATCustom_BG96_getCmd() for SID %d", curSID)

  /* retrieve parameters from SID command (will update SID_ctxt) */
 
 
  /* For each SID, athe sequence of AT commands to send id defined (it can be dynamic)
  * Determine and prepare the next command to send for this SID
  */

  /* ###########################  START CUSTOMIZATION PART  ########################### */
 //sid_cmdid_ticks = gCS_Send_steps;
  switch(curSID)
  {
	  case SID_CS_POWER_ON:
		    switch(gCS_Send_steps)
		   {
			   case 2:
					atcm_program_AT_CMD(p_atp_ctxt, ATTYPE_EXECUTION_CMD, _AT);
					gCS_Send_steps --;
					break;
			   case 1:
				    sprintf((char *)p_atp_ctxt->current_atcmd.params, "0") ;
					atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_EXECUTION_CMD, _ATE);
					gCS_Send_steps --;
					break;
			 
			   default:
				   gCS_Send_steps = 0;
			   break;
		   }
		   break;
	  case	SID_CS_RESET:
		  atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_EXECUTION_CMD, _AT_CRESET);
		  break;
	  case SID_CS_POWER_OFF:
			atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_EXECUTION_CMD, _AT_CPOF  );

		  break;
	  case SID_CS_CHECK_CNX:
			atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_EXECUTION_CMD, _AT);
			break;
	  case SID_CS_INIT_MODEM:
			atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_READ_CMD, _AT_CPIN);
			break;
	  case SID_CS_GET_SIGNAL_QUALITY:
			atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_EXECUTION_CMD, _AT_CSQ);
			break;
	  case SID_CS_REGISTER_NET:
		   switch(gCS_Send_steps)
		   {
			   case 4:
					atcm_program_AT_CMD(p_atp_ctxt, ATTYPE_READ_CMD, _AT_CEREG);
					gCS_Send_steps --;
					break;
			   case 3:
					atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_READ_CMD, _AT_CREG);
					gCS_Send_steps --;
					break;
			   case 2:
					atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_READ_CMD, _AT_CGREG);
					gCS_Send_steps --;
					break;
			   case 1:
			        atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_READ_CMD, _AT_COPS);
			   		gCS_Send_steps --;
					break;
			   default:
				   gCS_Send_steps = 0;
			   break;
		   }
			break;
	   case   SID_CS_GET_ATTACHSTATUS:
			atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_READ_CMD, _AT_CGACT);
			break;
	   case SID_CS_ACTIVATE_PDN:
			atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_READ_CMD, _AT_CGACT);
 			break;
	   case SID_CS_SOCKET_CREATE:
		      switch(gCS_Send_steps)
		   {
			   case 2:
				    sprintf((char *)p_atp_ctxt->current_atcmd.params, "1") ;
					atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_WRITE_CMD,_AT_QICSGP  );
					gCS_Send_steps --;
			   break;
			   case 1:
				    sprintf((char *)p_atp_ctxt->current_atcmd.params, "1") ;
					atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_WRITE_CMD,_AT_CIPHEAD  );
					gCS_Send_steps --;
			   break;
			   
			   default:
				   sid_cmdid_ticks = 0;
			   break;
		   }  		  

		   break;
	   case SID_CS_SECLECT_IPMODE:
			sprintf((char *)p_atp_ctxt->current_atcmd.params, "0") ;
			atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_WRITE_CMD, _AT_QCFG);
		 	break;
	   case SID_CS_GET_IP_ADDRESS:
		   atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_EXECUTION_CMD, _AT_QIDNSGIP );
		   break;
	   
	   case SID_CS_TCP_OPEN:
		   sprintf((char *)p_atp_ctxt->current_atcmd.params, "0,\"TCP\",\"%s\",%d",cs_ctxt_sockets_info.remote_ip_addr_value,cs_ctxt_sockets_info.remote_port ) ;
		   atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_WRITE_CMD, _AT_QIOPEN);
		   break;
	   case SID_CS_TCP_CLOSE:
		      sprintf((char *)p_atp_ctxt->current_atcmd.params, "1") ;
		   atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_WRITE_CMD, _AT_QICLOSE);
		   break;
	   case SID_CS_SOCKET_CLOSE:
		   atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_EXECUTION_CMD, _AT_NETCLOSE);
		   break;
	   case SID_CS_DEFINE_PDN:
		   sprintf((char *)p_atp_ctxt->current_atcmd.params, "%d,\"%s\",\"%s\"",  cs_pdn_infos.conf_id,cs_pdn_infos.PDP_type,cs_pdn_infos.apn);
			atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_WRITE_CMD, _AT_CGDCONT);
		   break;
	  
	   case SID_CS_SOCKET_OPEN:
		   atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_EXECUTION_CMD, _AT_NETOPEN);
		   break;
	   case SID_CS_SEND_DATA:
		   //sprintf((char *)p_atp_ctxt->current_atcmd.params, "0") ;
			sprintf((char *)p_atp_ctxt->current_atcmd.params, "0,%d",send_data_struct.buffer_size) ;

		   atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_WRITE_CMD, _AT_QISEND);
		   break;
	   case SID_CS_DIAL_COMMAND:
		 // sscanf(strtemp,"%d",&cs_ctxt_sockets_info.remote_port);
		   sprintf((char *)p_atp_ctxt->current_atcmd.params, "0,\"TCP\",\"%s\",%d",cs_ctxt_sockets_info.remote_ip_addr_value,cs_ctxt_sockets_info.remote_port ) ;
		   atcm_program_AT_CMD( p_atp_ctxt, ATTYPE_WRITE_CMD, _AT_QIOPEN);
		   break;
	  default:
		  break;

  }
 
  /* if no error, build the command to send */
  if (retval == ATSTATUS_OK)
  {
    retval = atcm_modem_build_cmd( p_atp_ctxt, p_ATcmdTimeout);
  }

  return (retval);
}


/* function called to finalize a SID */
at_status_t ATCustom_SIM_get_rsp(atparser_context_t *p_atp_ctxt, uint8_t *p_rsp_buf)
{
  at_status_t retval = ATSTATUS_OK;
  PrintAPI("enter ATCustom_BG96_get_rsp()")

  /* prepare response for a SID - common part */
 // retval = atcm_modem_get_rsp( p_atp_ctxt, p_rsp_buf);

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  /* prepare response for a SID
  *  all specific behaviors for SID which are returning datas in rsp_buf have to be implemented here
  */
  switch (p_atp_ctxt->current_SID)
  {
    case SID_CS_DNS_REQ:
      /* PACK data to response buffer */
//      if (DATAPACK_writeStruct(p_rsp_buf, CSMT_DNS_REQ, sizeof(bg96_qiurc_dnsgip.hostIPaddr), (void *)bg96_qiurc_dnsgip.hostIPaddr) != DATAPACK_OK)
//      {
//        retval = ATSTATUS_OK;
//      }
      break;

    case SID_CS_POWER_ON:
    case SID_CS_RESET:
     // display_user_friendly_mode_and_bands_config();
      break;

    default:
      break;
  }
  /* ###########################  END CUSTOMIZATION PART  ########################### */

  /* reset SID context */
 // atcm_reset_SID_context(&BG96_ctxt.SID_ctxt);

  /* reset socket context */
  //atcm_reset_SOCKET_context(&BG96_ctxt);

  return (retval);
}
uint16_t ATCustom_SIM_analyzeCmd(at_context_t *p_at_ctxt,RxMessage_t *p_msg_in,        at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  uint16_t retval = ATACTION_RSP_ERROR;

  PrintAPI("enter ATCustom_UG96_analyzeCmd()")

  /* search in LUT the ID corresponding to command received */
  //if (ATSTATUS_OK != atcm_searchCmdInLUT( p_atp_ctxt, p_msg_in, element_infos))
  {
    /* if cmd_id not found in the LUT, may be it's a text line to analyze */

    /* 1st STEP: search in common modems commands

      NOTE: if this modem has a specific behaviour for one of the common commands, bypass this
      step and manage all command in the 2nd step
    */
    retval = atcm_check_text_line_cmd( p_at_ctxt, p_msg_in, element_infos);

    /* 2nd STEP: search in specific modems commands if not found at 1st STEP */
    if (retval == ATACTION_RSP_NO_ACTION)
    {
      switch (p_atp_ctxt->current_atcmd.id)
      {
        /* ###########################  START CUSTOMIZED PART  ########################### */
        case _AT_QIRD:
          //if (fRspAnalyze_QIRD_data_UG96(p_at_ctxt, &UG96_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
          {
            /* received a valid intermediate answer */
            retval = ATACTION_RSP_INTERMEDIATE;
          }
          break;

        case _AT_QISTATE:
         // if (fRspAnalyze_QISTATE_UG96(p_at_ctxt, &UG96_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
          {
            /* received a valid intermediate answer */
            retval = ATACTION_RSP_INTERMEDIATE;
          }
          break;

        /* ###########################  END CUSTOMIZED PART  ########################### */
        default:
          /* this is not one of modem common command, need to check if this is an answer to a modem's specific cmd */
          PrintINFO("receive an un-expected line... is it a text line ?")
          retval = ATACTION_RSP_IGNORED;
          break;
      }
    }

    /* we fall here when cmd_id not found in the LUT
    * 2 cases are possible:
    *  - this is a valid line: ATACTION_RSP_INTERMEDIATE
    *  - this is an invalid line: ATACTION_RSP_ERROR
    */
    return (retval);
  }

  /* cmd_id has been found in the LUT: determine next action */
  switch (element_infos->cmd_id_received)
  {
    case _AT_OK:
      
      retval = ATACTION_RSP_FRC_END;
      break;

    case _AT_RING:
    case _AT_NO_CARRIER:
    case _AT_NO_DIALTONE:
    case _AT_BUSY:
    case _AT_NO_ANSWER:
      /* VALUES NOT MANAGED IN CURRENT IMPLEMENTATION BECAUSE NOT EXPECTED */
      retval = ATACTION_RSP_ERROR;
      break;

    case _AT_CONNECT:
      PrintINFO("MODEM SWITCHES TO DATA MODE")
     // atcm_set_modem_data_mode(&UG96_ctxt, AT_TRUE);
      //retval = (at_action_rsp_t)(ATACTION_RSP_FLAG_DATA_MODE | ATACTION_RSP_FRC_END);
      break;

    /* ###########################  START CUSTOMIZATION PART  ########################### */
    case _AT_CREG:
      /* check if response received corresponds to the command we have send
      *  if not => this is an URC
      */
      if (element_infos->cmd_id_received == p_atp_ctxt->current_atcmd.id)
      {
        retval = ATACTION_RSP_INTERMEDIATE;
      }
      else
      {
        retval = ATACTION_RSP_URC_FORWARDED;
      }
      break;

    case _AT_CGREG:
      /* check if response received corresponds to the command we have send
      *  if not => this is an URC
      */
      if (element_infos->cmd_id_received == p_atp_ctxt->current_atcmd.id)
      {
        retval = ATACTION_RSP_INTERMEDIATE;
      }
      else
      {
        retval = ATACTION_RSP_URC_FORWARDED;
      }
      break;

    case _AT_RDY_EVENT:
      PrintDBG(" MODEM READY TO RECEIVE AT COMMANDS")
      /* modem is ready */
      //UG96_ctxt.persist.modem_at_ready = AT_TRUE;

      /* if we were waiting for this event, we can continue the sequence */
      if ((p_atp_ctxt->current_SID == SID_CS_POWER_ON) || (p_atp_ctxt->current_SID == SID_CS_RESET))
      {
        /* UNLOCK the WAIT EVENT : as there are still some commands to send after, use CONTINUE */
       // UG96_ctxt.persist.modem_at_ready = AT_FALSE;
        retval = ATACTION_RSP_FRC_CONTINUE;
      }
      else
      {
        retval = ATACTION_RSP_URC_IGNORED;
      }
      break;

    case _AT_SOCKET_PROMPT:
      PrintINFO(" SOCKET PROMPT RECEIVED")
      /* if we were waiting for this event, we can continue the sequence */
      if (p_atp_ctxt->current_SID == SID_CS_SEND_DATA)
      {
        /* UNLOCK the WAIT EVENT */
        retval = ATACTION_RSP_FRC_END;
      }
      else
      {
        retval = ATACTION_RSP_URC_IGNORED;
      }
      break;

    case _AT_SEND_OK:
      if (p_atp_ctxt->current_SID == SID_CS_SEND_DATA)
      {
        retval = ATACTION_RSP_FRC_END;
      }
      else
      {
        retval = ATACTION_RSP_ERROR;
      }
      break;

    case _AT_SEND_FAIL:
      retval = ATACTION_RSP_ERROR;
      break;

    case _AT_QIURC:
      /* retval will be override in analyze of +QUIRC content
      *  indeed, QIURC can be considered as an URC or a normal msg (for DNS request)
      */
      retval = ATACTION_RSP_INTERMEDIATE;
      break;

    case _AT_QIOPEN:
      /* now waiting for an URC  */
      retval = ATACTION_RSP_INTERMEDIATE;
      break;

    case _AT_QUSIM:
      retval = ATACTION_RSP_URC_IGNORED;
      break;

    case _AT_QIND:
      retval = ATACTION_RSP_URC_IGNORED;
      break;

    case _AT_CFUN:
      retval = ATACTION_RSP_URC_IGNORED;
      break;

    case _AT_CPIN:
      PrintDBG(" SIM STATE RECEIVED")
      /* retval will be override in analyze of +CPIN content if needed
      */
      retval = ATACTION_RSP_URC_IGNORED;
      break;

    case _AT_QCFG:
      retval = ATACTION_RSP_INTERMEDIATE;
      break;

    case _AT_CGEV:
      retval = ATACTION_RSP_URC_FORWARDED;
      break;

    case _AT_QPING:
      retval = ATACTION_RSP_URC_FORWARDED;
      break;

    /* ###########################  END CUSTOMIZATION PART  ########################### */

    case _AT:
      retval = ATACTION_RSP_IGNORED;
      break;

    case _AT_INVALID:
      retval = ATACTION_RSP_ERROR;
      break;

    case _AT_ERROR:
      /* ERROR does not contains parameters, call the analyze function explicity */
     // retval = fRspAnalyze_Error_UG96(p_at_ctxt, &UG96_ctxt, p_msg_in, element_infos);
      break;

    case _AT_CME_ERROR:
    case _AT_CMS_ERROR:
      /* do the analyze here because will not be called by parser */
    //  retval = fRspAnalyze_Error_UG96(p_at_ctxt, &UG96_ctxt, p_msg_in, element_infos);
      break;

    default:
      /* check if response received corresponds to the command we have send
      *  if not => this is an ERROR
      */
      if (element_infos->cmd_id_received == p_atp_ctxt->current_atcmd.id)
      {
        retval = ATACTION_RSP_INTERMEDIATE;
      }
      else
      {
        PrintINFO("UNEXPECTED MESSAGE RECEIVED")
        retval = ATACTION_RSP_IGNORED;
      }
      break;
  }

  return (retval);
}

uint16_t atcm_check_text_line_cmd( at_context_t *p_at_ctxt,RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  uint16_t retval = ATACTION_RSP_ERROR;

  /* in this section, we treat all commands which can return text lines */
  switch (p_atp_ctxt->current_atcmd.id)
  {
    case _AT_CGMI:
//      if (fRspAnalyze_CGMI(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
//      {
//        retval = ATACTION_RSP_INTERMEDIATE;
//      }
      break;

    case _AT_CGMM:
//      if (fRspAnalyze_CGMM(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
//      {
//        /* received a valid intermediate answer */
//        retval = ATACTION_RSP_INTERMEDIATE;
//      }
      break;

    case _AT_CGMR:
//      if (fRspAnalyze_CGMR(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
//      {
//        /* received a valid intermediate answer */
//        retval = ATACTION_RSP_INTERMEDIATE;
//      }
      break;

    case _AT_CGSN:
//      if (fRspAnalyze_CGSN(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
//      {
//        /* received a valid intermediate answer */
//        retval = ATACTION_RSP_INTERMEDIATE;
//      }
      break;

    case _AT_GSN:
//      if (fRspAnalyze_GSN(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
//      {
//        /* received a valid intermediate answer */
//        retval = ATACTION_RSP_INTERMEDIATE;
//      }
      break;

    case _AT_IPR:
//      if (fRspAnalyze_IPR(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
//      {
//        /* received a valid intermediate answer */
//        retval = ATACTION_RSP_INTERMEDIATE;
//      }
      break;

    case _AT_CIMI:
//      if (fRspAnalyze_CIMI(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
//      {
//        /* received a valid intermediate answer */
//        retval = ATACTION_RSP_INTERMEDIATE;
//      }
      break;

    case _AT_CGPADDR:
//      if (fRspAnalyze_CGPADDR(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
//      {
//        /* received a valid intermediate answer */
//        retval = ATACTION_RSP_INTERMEDIATE;
//      }
      break;

    default:
      /* this is not one of modem common command, need to check if this is an answer to a modem's specific cmd */
      retval = ATACTION_RSP_NO_ACTION;
      break;
  }

  return (retval);
}
