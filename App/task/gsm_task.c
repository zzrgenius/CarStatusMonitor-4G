/**
 * \file            main.c
 * \brief           Main file
 */

/*
 * Copyright (c) 2018 Tilen Majerle
 *  
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 */
#include "main.h"
#include "cmsis_os.h"
#include "osprintf.h"
//#include "at_custom_modem.h"
#include <string.h> 
#include "sim_drv.h"
#include "sim_core.h"
#include "sim_service.h"
#include "sim_service_os.h"


#define USE_TRACE_CELLULAR_SERVICE		1
/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_CELLULAR_SERVICE == 1U)
#if (USE_PRINTF == 0U)
#define PrintCellularService(format, args...)      TracePrint(DBG_CHAN_CELLULAR_SERVICE, DBL_LVL_P0, format, ## args)
#define PrintCellularServiceErr(format, args...)   TracePrint(DBG_CHAN_CELLULAR_SERVICE, DBL_LVL_ERR, "ERROR " format, ## args)
#else
#define PrintCellularService(format, args...)      printf(format , ## args);
#define PrintCellularServiceErr(format, args...)   printf(format , ## args);
#endif /* USE_PRINTF */
#else
#define PrintCellularService(format, args...)   do {} while(0);
#define PrintCellularServiceErr(format, args...)  do {} while(0);
#endif /* USE_TRACE_CELLULAR_SERVICE */

	
 static uint8_t cmd_buf[ATCMD_MAX_BUF_SIZE];
 static uint8_t rsp_buf[ATCMD_MAX_BUF_SIZE];
#define CST_MODEM_POLLING_PERIOD          (10000U)  /* Polling period = 10s */
#define CST_MODEM_POLLING_PERIOD_DEFAULT 	5000U
#define CELLULAR_SERVICE_THREAD_STACK_SIZE  (512U)
#define CST_BAD_SIG_RSSI 99U
#define CST_COUNT_FAIL_MAX (5U)




extern uint8_t gCS_Send_steps;

static CST_nfmc_context_t CST_nfmc_context;
static CS_OperatorSelector_t    ctxt_operator =
{
  .mode = CS_NRM_AUTO,
  .format = CS_ONF_NOT_PRESENT,
  .operator_name = "00101",
};


  CST_context_t     cst_context = {    CST_MODEM_INIT_STATE, CST_NO_FAIL, CS_PDN_EVENT_NW_DETACH,    /* Automaton State, FAIL Cause,  */
  { 0, 0},                              /* signal quality */
  CS_NRS_NOT_REGISTERED_NOT_SEARCHING, CS_NRS_NOT_REGISTERED_NOT_SEARCHING, CS_NRS_NOT_REGISTERED_NOT_SEARCHING,
  CELLULAR_OK,                           /* sim_mode */
  1,                                     /* set_pdn_mode */
  0,                                     /* activate_pdn_nfmc_tempo_count */
  0,                                     /* register_retry_tempo_count */
  0, 0, 0, 0, 0, 0, 0, 0, 0              /* Reset counters */
};
static osTimerId         cst_polling_timer_handle;

static uint8_t  CST_polling_timer_flag = 0U;
static uint8_t CST_csq_count_fail      = 0U;

osThreadId CST_cellularServiceThreadId = NULL;

static void  CST_network_event_mngt(void);
CS_Status_t CST_cellular_service_start(void);
CS_Status_t CST_cellular_service_init(void);

/* Global variables ----------------------------------------------------------*/
extern osMutexId CellularServiceMutexHandle ;

CS_SignalQuality_t g_sim_sig_qual;
uint8_t local_ip_addr_value[MAX_IP_ADDR_SIZE];
char remote_ip_addr_value[MAX_IP_ADDR_SIZE] = {"47.101.48.153"};
char testsendbuf[] = {"send from sim7600\r\n"};
uint8_t socket_rec_buf[DEFAULT_IP_MAX_PACKET_SIZE];

CS_Status_t CS_get_net_status(CS_RegistrationStatus_t *p_reg_status);

 
CS_Status_t CST_cellular_service_init(void)
{
  cst_context.current_state = CST_MODEM_INIT_STATE;
 // CST_modem_init(); 

  osCDS_cellular_service_init();
  CST_csq_count_fail = 0U;

 
 
  return CELLULAR_OK;
}

 
/* start modem processing */
static void CST_modem_start(void)
{
 
	AT_init();
}

 

/* CST_polling_timer_callback function */
static void CST_polling_timer_callback(void const *argument)
{
  if (cst_context.current_state != CST_MODEM_INIT_STATE)
  {
    
  }
}
 


 
/* Functions Definition ------------------------------------------------------*/
 

uint8_t CST_signal_quality = 0U;

/* init modem processing */
static CS_Status_t CST_set_signal_quality(void)
{
  CS_Status_t cs_status = CELLULAR_ERROR;
  CS_SignalQuality_t sig_quality;
  if (osCS_get_signal_quality(&sig_quality) == CELLULAR_OK)
  {
    CST_csq_count_fail = 0U;
    if ((sig_quality.rssi != cst_context.signal_quality.rssi) || (sig_quality.ber != cst_context.signal_quality.ber))
    {
      cst_context.signal_quality.rssi = sig_quality.rssi;
      cst_context.signal_quality.ber  = sig_quality.ber;

     // dc_com_read(&dc_com_db, DC_COM_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));
      /* if((sig_quality.rssi == 0) || (sig_quality.rssi == CST_BAD_SIG_RSSI)) */
      if (sig_quality.rssi == CST_BAD_SIG_RSSI)
      {
         
      }
      else
      {
        cs_status = CELLULAR_OK;
        CST_signal_quality = sig_quality.rssi;
//        cst_cellular_info.cs_signal_level     = sig_quality.rssi;             /*  range 0..99 */
//        cst_cellular_info.cs_signal_level_db  = (-113 + 2 * (int32_t)sig_quality.rssi); /* dBm */
      }
      
    }

  //  PrintCellularService(" -Sig quality rssi : %d\n\r", sig_quality.rssi)
  //  PrintCellularService(" -Sig quality ber  : %d\n\r", sig_quality.ber)
  }
  else
  {
    CST_csq_count_fail++;
 //   PrintCellularService("Modem signal quality error\n\r")
    if (CST_csq_count_fail >= CST_COUNT_FAIL_MAX)
    {
 //     PrintCellularService("Modem signal quality error max\n\r")
      CST_csq_count_fail = 0U;
      //CST_config_fail_mngt("CS_get_signal_quality",       CST_MODEM_CSQ_FAIL,      &cst_context.csq_reset_count,   CST_CSQ_MODEM_RESET_MAX);
    }
  }
  return cs_status;
}


 
/* Timer handler
     During configuration :  Signal quality  and network status Polling
     After configuration  :  Modem monitoring (Signal quality checking)
 */
static void CST_timer_handler(void)
{
  CS_Status_t cs_status ;
  CS_RegistrationStatus_t reg_status;

  /*    PrintCellularService("-----> CST_timer_handler  <-----\n\r") */
  CST_polling_timer_flag = 0U;
  if (cst_context.current_state == CST_WAITING_FOR_NETWORK_STATUS_STATE)
  {
    memset((void *)&reg_status, 0, sizeof(CS_RegistrationStatus_t));
    cs_status = osCDS_get_net_status(&reg_status);
    if (cs_status == CELLULAR_OK)
    {
      cst_context.current_EPS_NetworkRegState  = reg_status.EPS_NetworkRegState;
      cst_context.current_GPRS_NetworkRegState = reg_status.GPRS_NetworkRegState;
      cst_context.current_CS_NetworkRegState   = reg_status.CS_NetworkRegState;
      PrintCellularService("-----> CST_timer_handler - CST_NETWORK_STATUS_EVENT <-----\n\r")
 
     
    }
    else
    {
       //fail
    }
  }

  
 

  if ((cst_context.current_state == CST_MODEM_DATA_READY_STATE) && (CST_MODEM_POLLING_PERIOD != 0U))
  {
 
    CST_set_signal_quality();
 
  }
}
 
/* power on modem processing */
static void CST_power_on_only_modem_mngt(void)
{
  PrintCellularService("*********** CST_power_on_only_modem_mngt ********\n\r")
  osCDS_power_on();
  PrintCellularService("*********** MODEM ON ********\n\r")
}

static void CST_init_state(void)
{
 
      cst_context.current_state = CST_MODEM_ON_STATE;
      CST_power_on_only_modem_mngt();
 
  /* subscribe modem events after power ON */
 // CST_subscribe_modem_events();
}
 

static CS_Status_t CST_reset_modem(void)
{
  return osCDS_reset(CS_RESET_HW);
}

/* power on modem processing */
static void CST_reset_modem_mngt(void)
{
  CS_Status_t cs_status;
 
  PrintCellularService("*********** CST_power_on_modem_mngt ********\n\r")
  cs_status = CST_reset_modem();
 
}

static void CST_reset_state(void)
{
  
      cst_context.current_state = CST_MODEM_ON_STATE;
      
}
/* init modem management */
static void  CST_init_modem_mngt(void)
{
  CS_Status_t cs_status;

  PrintCellularService("*********** CST_init_modem_mngt ********\n\r")

  cs_status = osCDS_init_modem( CELLULAR_FALSE, "");
 
  if (cs_status == CELLULAR_ERROR)
  {
   // CST_config_fail_mngt("CST_init_modem_mngt", CST_MODEM_REGISTER_FAIL, &cst_context.init_modem_reset_count, CST_INIT_MODEM_RESET_MAX);
  }
  else if (cs_status == CELLULAR_SIM_NOT_INSERTED)
  {
    cst_context.sim_mode = CELLULAR_SIM_NOT_INSERTED;
  }
  else
  {
	//	cst_context.current_state = CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE;
  
  }

}

static void CST_modem_on_state( void)
{
  
      cst_context.current_state = CST_MODEM_POWERED_ON_STATE;
      CST_init_modem_mngt();
  
}
/* pdn definition modem management */
static void CST_modem_define_pdn(void)
{
  
  CS_Status_t cs_status;
  /*
  #if (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM)
      cs_status = CELLULAR_OK;
  #else
  */
  /* define user PDN configurations */

  			//	if(CS_define_pdn(1,(uint8_t*)"CMNET",(uint8_t*)"IP") == CELLULAR_OK)

  /* exemple for CS_PDN_USER_CONFIG_1 with access point name =  "PDN CONFIG 1" */
  cs_status = osCDS_define_pdn( 1, (uint8_t*)"CMNET",(uint8_t*)"IP");
   if (cs_status != CELLULAR_OK)
  {
   
  }
  /*
      else
      {
          CST_send_message(CST_MESSAGE_CS_EVENT, CST_PDP_ACTIVATED_EVENT);
      }
  */
}
/* registration modem management */
static void  CST_net_register_mngt(void)
{
  CS_Status_t cs_status;
  CS_RegistrationStatus_t  cst_ctxt_reg_status;

  PrintCellularService("=== CST_net_register_mngt ===\n\r")
  cs_status = osCDS_register_net(&ctxt_operator, &cst_ctxt_reg_status);
  if (cs_status == CELLULAR_OK)
  {
    cst_context.current_EPS_NetworkRegState  = cst_ctxt_reg_status.EPS_NetworkRegState;
    cst_context.current_GPRS_NetworkRegState = cst_ctxt_reg_status.GPRS_NetworkRegState;
    cst_context.current_CS_NetworkRegState   = cst_ctxt_reg_status.CS_NetworkRegState;

  //  CST_send_message(CST_MESSAGE_CS_EVENT, CST_SIGNAL_QUALITY_EVENT);
  }
  else
  {
    PrintCellularService("===CST_net_register_mngt - FAIL !!! ===\n\r")
    //ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 3, ERROR_WARNING);
  }
}

static void CST_modem_powered_on_state(CST_autom_event_t autom_event)
{
	PrintCellularService("CST_modem_on_state : CST_modem_define_pdn\n\r")
	CST_modem_define_pdn();
	cst_context.current_state = CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE;
	//CST_net_register_mngt();
  
}
 
static void CST_waiting_for_signal_quality_ok_state(uint8_t event)
{
	 CS_get_signal_quality(&cst_context.signal_quality);
	if(event == 1)
		cst_context.current_state = CST_WAITING_FOR_NETWORK_STATUS_STATE;
	else
		cst_context.current_state = CST_MODEM_DATA_READY_STATE;

  
}
 
static void CST_waiting_for_network_status_state(CST_autom_event_t autom_event)
{
 
 
     // ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 9, ERROR_WARNING);
		cst_context.current_state =CST_NETWORK_STATUS_OK_STATE;
  
}

/* attach modem management */
static void  CST_attach_modem_mngt(void)
{
  CS_Status_t              cs_status;
  CS_PSattach_t            cst_ctxt_attach_status;
  CS_RegistrationStatus_t  reg_status;

  PrintCellularService("*********** CST_attach_modem_mngt ********\n\r")

  memset((void *)&reg_status, 0, sizeof(CS_RegistrationStatus_t));
  cs_status = osCDS_get_net_status(&reg_status);

  if (cs_status == CELLULAR_OK)
  {
    if ((reg_status.optional_fields_presence & CS_RSF_FORMAT_PRESENT) != 0)
    {
   
      PrintCellularService(" ->operator_name = %s\n\r", reg_status.operator_name)
    }
  }

  cs_status = osCDS_get_attach_status(&cst_ctxt_attach_status);
#if (CST_RESET_DEBUG == 1)
  if (cst_context.attach_reset_count == 0)
  {
    cs_status = CELLULAR_ERROR;
  }
#endif
  if (cs_status != CELLULAR_OK)
  {
    PrintCellularService("*********** CST_attach_modem_mngt fail ********\n\r")
    
  }
  else
  {
    if (cst_ctxt_attach_status == CS_PS_ATTACHED)
    {
      PrintCellularService("*********** CST_attach_modem_mngt OK ********\n\r")
      cst_context.current_state = CST_MODEM_REGISTERED_STATE;
     // CST_send_message(CST_MESSAGE_CS_EVENT, CST_MODEM_ATTACHED_EVENT);
    }

    else
    {
      PrintCellularService("===CST_attach_modem_mngt - NOT ATTACHED !!! ===\n\r")
      /* Workaround waiting for Modem behaviour clarification */
      //fail
 
    }
  }
}
static void CST_network_status_ok_state(void)
{
   
      CST_attach_modem_mngt();
  
}
/* PDN event callback */
static void CST_pdn_event_callback( CS_PDN_event_t pdn_event)
{
  PrintCellularService("====================================CST_pdn_event_callback (cid= 0 / event=%d)\n\r",
                        pdn_event)
  cst_context.pdn_status = pdn_event;
 // CST_send_message(CST_MESSAGE_CS_EVENT, CST_PDN_STATUS_EVENT);

}
/* pdn activation modem management */
static CS_Status_t CST_modem_activate_pdn(void)
{
  CS_Status_t cs_status;
  cs_status = osCDS_set_default_pdn(0);

  /* register to PDN events for this CID*/
  osCDS_register_pdn_event(0, CST_pdn_event_callback);

  cs_status = osCDS_activate_pdn(0);
 
 

  if (cs_status == CELLULAR_OK)	  
  {
        cst_context.current_state =  CST_MODEM_SOCKET_SET_STATE;

  }
    
  return cs_status;
}

static void CST_modem_registered_state(void)
{
  
//      cst_context.current_state = CST_MODEM_PDN_ACTIVATE_STATE;
//      CST_modem_activate_pdn();
 
      CST_network_event_mngt();
  
}
static void CST_reset_fail_count(void)
{
  cst_context.power_on_reset_count       = 0U;
  cst_context.reset_reset_count          = 0U;
  cst_context.init_modem_reset_count     = 0U;
  cst_context.csq_reset_count            = 0U;
  cst_context.gns_reset_count            = 0U;
  cst_context.attach_reset_count         = 0U;
  cst_context.activate_pdn_reset_count   = 0U;
  cst_context.cellular_data_retry_count  = 0U;
  cst_context.global_retry_count         = 0U;
}
 

static void CST_modem_pdn_activate_state(void)
{
 
    {
      CST_modem_activate_pdn();
	//  cst_context.current_state = CST_MODEM_SOCKET_SET_STATE;

    }
  
}

static void  CST_network_event_mngt(void)
{
  CS_Status_t cs_status;
  CS_RegistrationStatus_t reg_status;

  PrintCellularService("*********** CST_network_event_mngt ********\n\r")
  memset((void *)&reg_status, 0, sizeof(reg_status));
  cs_status = osCDS_get_net_status(&reg_status);
  if (cs_status == CELLULAR_OK)
  {
    cst_context.current_EPS_NetworkRegState  = reg_status.EPS_NetworkRegState;
    cst_context.current_GPRS_NetworkRegState = reg_status.GPRS_NetworkRegState;
    cst_context.current_CS_NetworkRegState   = reg_status.CS_NetworkRegState;

    if ((cst_context.current_EPS_NetworkRegState  != CS_NRS_REGISTERED_HOME_NETWORK)
        && (cst_context.current_EPS_NetworkRegState  != CS_NRS_REGISTERED_ROAMING)
        && (cst_context.current_GPRS_NetworkRegState != CS_NRS_REGISTERED_HOME_NETWORK)
        && (cst_context.current_GPRS_NetworkRegState != CS_NRS_REGISTERED_ROAMING))
    {
      
      cst_context.current_state = CST_WAITING_FOR_NETWORK_STATUS_STATE;
     // CST_send_message(CST_MESSAGE_CS_EVENT, CST_NETWORK_CALLBACK_EVENT);
    }
	else
	{
		cst_context.current_state = CST_MODEM_PDN_ACTIVATE_STATE;
	}

    if ((reg_status.optional_fields_presence & CS_RSF_FORMAT_PRESENT) != 0)
    {
      
      PrintCellularService(" ->operator_name = %s", reg_status.operator_name)
    }

  }
  else
  {
  }

}

 

static void CST_data_ready_state(void)
{
  osCDS_socket_listen();
   
}

static void CST_fail_state(void)
{
 
}

 

CS_Status_t CST_cellular_service_start(void)
{
 // dc_nfmc_info_t nfmc_info;
  uint32_t cst_polling_period = 0U;

 // CST_modem_start();

 
 
  osTimerDef(cs_polling_timer, CST_polling_timer_callback);
  cst_polling_timer_handle = osTimerCreate(osTimer(cs_polling_timer), osTimerPeriodic, NULL);
  if (CST_MODEM_POLLING_PERIOD == 0U)
  {
    cst_polling_period = CST_MODEM_POLLING_PERIOD_DEFAULT;
  }
  else
  {
    cst_polling_period = CST_MODEM_POLLING_PERIOD;
  }

 
 
  if (CST_cellularServiceThreadId == NULL)
  {
     // ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 19, ERROR_FATAL);
  }
  else
  {
#if (STACK_ANALYSIS_TRACE == 1)
    stackAnalysis_addStackSizeByHandle(CST_cellularServiceThreadId, CELLULAR_SERVICE_THREAD_STACK_SIZE);
#endif /* STACK_ANALYSIS_TRACE */
  }

  return CELLULAR_OK;
}


void StartTaskGSM(void const * argument)
{
	uint8_t err,result;
	  CST_autom_event_t autom_event;

//	static at_handle_t _Adapter_Handle;
	//static sysctrl_info_t modem_device_infos;  /* LTE Modem informations */
	SIM_power_on();
	AT_init();
	static uint16_t current_state_test = 0;
	  CS_RegistrationStatus_t reg_status;
		CST_cellular_service_init();
		CST_cellular_service_start();
//	AT_init();
//    AT_open( &modem_device_infos ,  NULL  ,NULL);
	//AT_main_config(&at_context_sim);
	//SIM_power_on();
	
	/* Terminate  
 typedef enum
{
  CST_MODEM_INIT_STATE,
  CST_MODEM_RESET_STATE,
  CST_MODEM_ON_STATE,
  CST_MODEM_ON_ONLY_STATE,
  CST_MODEM_POWERED_ON_STATE,
  CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE,
  CST_WAITING_FOR_NETWORK_STATUS_STATE,
  CST_NETWORK_STATUS_OK_STATE,
  CST_MODEM_REGISTERED_STATE,
  CST_MODEM_PDN_ACTIVATE_STATE,
  CST_MODEM_PDN_ACTIVATED_STATE,
  CST_MODEM_DATA_READY_STATE,
  CST_MODEM_REPROG_STATE,
  CST_MODEM_FAIL_STATE,
  CST_MODEM_NETWORK_STATUS_FAIL_STATE,
} CST_state_t;

//	SysCtrl_power_on(0);

  statical init of components */
 	while(1)
	{
		
	  switch (cst_context.current_state)
      {
        case CST_MODEM_INIT_STATE:
        {
          PrintCellularService("-----> State : CST_MODEM_INIT_STATE <-----\n\r")
			autom_event = CST_MODEM_POWER_ON_EVENT;
          CST_init_state();
          break;
        }
        case CST_MODEM_RESET_STATE:
        {
          PrintCellularService("-----> State : CST_MODEM_RESET_STATE <-----\n\r")
          CST_reset_state();
          break;
        }
        case CST_MODEM_ON_STATE:
        {
          PrintCellularService("-----> State : CST_MODEM_ON_STATE <-----\n\r")
			autom_event = CST_MODEM_POWERED_ON_EVENT; 
          CST_modem_on_state();
          break;
        }
        case CST_MODEM_POWERED_ON_STATE:
        {
          PrintCellularService("-----> State : CST_MODEM_POWERED_ON_STATE <-----\n\r")
			
          CST_modem_powered_on_state(CST_MODEM_INITIALIZED_EVENT);
          break;
        }
        case CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE:
        {
          PrintCellularService("-----> State : CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE <-----\n\r")
          CST_waiting_for_signal_quality_ok_state(1);
          break;
        }
        case CST_WAITING_FOR_NETWORK_STATUS_STATE:
        {
          PrintCellularService("-----> State : CST_WAITING_FOR_NETWORK_STATUS_STATE <-----\n\r")
          CST_waiting_for_network_status_state(CST_NO_EVENT);
          break;
        }

        case CST_NETWORK_STATUS_OK_STATE:
        {
          PrintCellularService("-----> State : CST_NETWORK_STATUS_OK_STATE <-----\n\r")
          CST_network_status_ok_state();
          break;
        }

        case CST_MODEM_REGISTERED_STATE:
        {
          PrintCellularService("-----> State : CST_MODEM_REGISTERED_STATE <-----\n\r")
          CST_modem_registered_state();
          break;
        }
        case CST_MODEM_PDN_ACTIVATE_STATE:
        {
          PrintCellularService("-----> State : CST_MODEM_PDN_ACTIVATE_STATE <-----\n\r")
         // CST_modem_pdn_activate_state();
			cst_context.current_state = CST_MODEM_SOCKET_SET_STATE;
          break;
        }
		case CST_MODEM_SOCKET_SET_STATE:
		{
// 			if(CDS_ipmode_set(0) == CELLULAR_OK)
			{		 
				if(CDS_socket_set_option() == CELLULAR_OK)
				{
					cst_context.current_state = CST_MODEM_SOCKET_OPENNET_STATE;
 
				}
			}
		}
		case  CST_MODEM_SOCKET_OPENNET_STATE:
			if(CDS_socket_open(0)== CELLULAR_OK)
			{
				cst_context.current_state = CST_MODEM_SOCKET_OPENTCP_STATE;
			}
			else
			{
				cst_context.current_state =CST_MODEM_SOCKET_CLOSENET_STATE;
			}
			break;
		case  CST_MODEM_SOCKET_OPENTCP_STATE:
			if(osCDS_socket_connect( (uint8_t*)remote_ip_addr_value,7903)== CELLULAR_OK)
			{
				cst_context.current_state = CST_MODEM_SOCKET_TCPOK_STATE;
			}
			break;
		case   CST_MODEM_SOCKET_TCPOK_STATE:
			cst_context.current_state = CST_MODEM_DATA_READY_STATE;
			if( osCDS_socket_send(( const uint8_t *)testsendbuf,strlen(testsendbuf))== CELLULAR_OK)
			{
				__nop();
				cst_context.cellular_data_retry_count = 0;
			}
			else
			{
				cst_context.cellular_data_retry_count++;
				
			}
			if(cst_context.cellular_data_retry_count > CST_GLOBAL_RETRY_MAX)
				cst_context.current_state = CST_MODEM_REGISTERED_STATE;

			break;
		case CST_MODEM_SOCKET_CLOSETCP_STATE:
			
			break;
		case CST_MODEM_SOCKET_CLOSENET_STATE:
			CDS_socket_close(1);
			cst_context.current_state = CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE;
			break;
        case CST_MODEM_DATA_READY_STATE:
        {
//          PrintCellularService("-----> State : CST_MODEM_DATA_READY_STATE <-----\n\r")
//			if( osCDS_socket_send(( const uint8_t *)testsendbuf,strlen(testsendbuf))== CELLULAR_OK)
//			{
//				__nop();
//				cst_context.cellular_data_retry_count = 0;
//			}
//			else
//			{
//				cst_context.cellular_data_retry_count++;
//				
//			}
//			if(cst_context.cellular_data_retry_count > CST_GLOBAL_RETRY_MAX)
//			{
//				cst_context.current_state = CST_MODEM_REGISTERED_STATE;
//				cst_context.cellular_data_retry_count = 0;
//			}

          CST_data_ready_state();
          break;
        }
        case CST_MODEM_FAIL_STATE:
        {
          PrintCellularService("-----> State : CST_MODEM_FAIL_STATE <-----\n\r")
          CST_fail_state();
          break;
        }
        default:
        {
          PrintCellularService("-----> State : Not State <-----\n\r")
          break;
        }
      }
	 	#if 0
		switch(current_state_test)
		{
			case  0:
				CS_reset(1);
				current_state_test++;
				break;
			case  1:
				if(CS_check_connection() == CELLULAR_OK)
					current_state_test++;
				break;
			case  2:
				if(CS_init_modem() == CELLULAR_OK)
					current_state_test++;
				break;
			case 3:
				if(CS_get_signal_quality(&g_sim_sig_qual) == CELLULAR_OK)
					current_state_test++;
				break;
			case  4:
//				if(CS_register_net() == CELLULAR_OK)  //CS_get_net_status
//					current_state_test++;
				if(CS_get_net_status(&reg_status) == CELLULAR_OK)  //
					current_state_test++;			
				break;
			case  5:
				socket_init();	
				current_state_test++;	

 				break;
			case 6:
				CDS_socket_close(1);
				current_state_test++;	
				break;
			case 7: 
				if(CS_define_pdn(1,(uint8_t*)"CMNET",(uint8_t*)"IP") == CELLULAR_OK)
					current_state_test++;	

				break;
			case 8:
				if(CDS_ipmode_set(0) == CELLULAR_OK)
				{
					current_state_test++;	
				}
			break;
			case 9:
				if(CDS_socket_set_option() == CELLULAR_OK)
				{
					current_state_test++;	
				}
			break;
			case 10:
				if(CDS_socket_open(0)== CELLULAR_OK)
				{
							current_state_test++;	
				}
				else
				{
					
				}
			
				break;
			case 11:
				if(CS_get_dev_IP_address(0, local_ip_addr_value)== CELLULAR_OK)
							current_state_test++;	
			case 12:
				if(CDS_socket_connect( (uint8_t*)remote_ip_addr_value,7903)== CELLULAR_OK)
							current_state_test++;	
			case 13:
				if( CDS_socket_send(( const uint8_t *)testsendbuf,strlen(testsendbuf))== CELLULAR_OK)
				{
					__nop();
				}
			default:
				break;
		}
	//	CDS_socket_receive(socket_rec_buf);
	//	CS_get_signal_quality(&g_sim_sig_qual);
		/****************** 
		ATParser_get_urc(&at_context, at_context.p_rsp_buf);
		***/
		osprintf("current_state_test is %d\r\n",current_state_test);
		#endif
		osDelay(5000);
		
	}
}
 