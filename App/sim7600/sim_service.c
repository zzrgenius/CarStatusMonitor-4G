#include <stdio.h>
#include <string.h>

#include "sim_service.h"
#include "sim_drv.h"
#include "sim_core.h"
#include "sim_service.h"
#include "main.h"
#include "cmsis_os.h"
#include "osprintf.h"
#include "ringbuffer.h"
 
/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define USE_TRACE_CELLULAR_SERVICE 1

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_CELLULAR_SERVICE == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PrintINFO(format, args...) TracePrint(DBG_CHAN_MAIN, DBL_LVL_P0, "CS:" format "\n\r", ## args)
#define PrintDBG(format, args...)  TracePrint(DBG_CHAN_MAIN, DBL_LVL_P1, "CS:" format "\n\r", ## args)
#define PrintAPI(format, args...)  TracePrint(DBG_CHAN_MAIN, DBL_LVL_P2, "CS API:" format "\n\r", ## args)
#define PrintErr(format, args...)  TracePrint(DBG_CHAN_MAIN, DBL_LVL_ERR, "CS ERROR:" format "\n\r", ## args)
#else
#define PrintINFO(format, args...)  printf("CS:" format "\n\r", ## args);
#define PrintDBG(format, args...)   printf("CS:" format "\n\r", ## args);
#define PrintAPI(format, args...)   printf("CS API:" format "\n\r", ## args);
#define PrintErr(format, args...)   printf("CS ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#define PrintINFO(format, args...)  do {} while(0);
#define PrintDBG(format, args...)   do {} while(0);
#define PrintAPI(format, args...)   do {} while(0);
#define PrintErr(format, args...)   do {} while(0);
#endif /* USE_TRACE_CELLULAR_SERVICE */
  uint8_t cmd_buf[ATCMD_MAX_BUF_SIZE];
  uint8_t rsp_buf[ATCMD_MAX_BUF_SIZE];
 csint_socket_infos_t cs_ctxt_sockets_info; /* socket infos (array index = socket handle) */
  csint_pdn_infos_t   cs_pdn_infos;
uint8_t gCS_Send_steps;
  void socket_init(void)
{ 
  cs_ctxt_sockets_info.state = SOCKETSTATE_NOT_ALLOC;
  cs_ctxt_sockets_info.config = CS_SON_NO_OPTION;

  //cs_ctxt_sockets_info.addr_type = CS_IPAT_IPV4;
  cs_ctxt_sockets_info.protocol = CS_TCP_PROTOCOL;
  cs_ctxt_sockets_info.local_port = 0U;
 // cs_ctxt_sockets_info.conf_id = CS_PDN_NOT_DEFINED;

  cs_ctxt_sockets_info.ip_max_packet_size = DEFAULT_IP_MAX_PACKET_SIZE;
  cs_ctxt_sockets_info.trp_max_timeout = DEFAULT_TRP_MAX_TIMEOUT;
  cs_ctxt_sockets_info.trp_conn_setup_timeout = DEFAULT_TRP_CONN_SETUP_TIMEOUT;
  cs_ctxt_sockets_info.trp_transfer_timeout = DEFAULT_TRP_TRANSFER_TIMEOUT;
 // cs_ctxt_sockets_info.trp_connect_mode = CS_CM_COMMAND_MODE;
  cs_ctxt_sockets_info.trp_suspend_timeout = DEFAULT_TRP_SUSPEND_TIMEOUT;
  cs_ctxt_sockets_info.trp_rx_timeout = DEFAULT_TRP_RX_TIMEOUT;

  /* socket callback functions pointers */
  cs_ctxt_sockets_info.socket_data_ready_callback = NULL;
  cs_ctxt_sockets_info.socket_data_sent_callback = NULL;
  cs_ctxt_sockets_info.socket_remote_close_callback = NULL;
}
/**
  * @brief  Power ON the modem
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_power_on(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
  char *pstr;
  PrintAPI("CS_power_on")
  /* init of Cellular module */
  if (	SIM_power_on() == CELLULAR_OK)
  {
     gCS_Send_steps = 2;
	  do{
		  err = AT_sendcmd( SID_CS_POWER_ON, &cmd_buf[0], &rsp_buf[0]);
		 if( (pstr = strstr((char*)rsp_buf,"OK")) != NULL)
		 {
			  PrintDBG("Cellular started and ready")
			  retval = CELLULAR_OK;
		  }
		  else
		  {
			  retval = CELLULAR_ERROR;
			  break;
		  }
		  
	  }while(gCS_Send_steps);
     
  }

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error when power on process")
  }
  return (retval);
}

/**
  * @brief  Power OFF the modem
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_power_off(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
  PrintAPI("CS_power_off")

  
    err = AT_sendcmd( SID_CS_POWER_OFF, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PrintDBG("<Cellular_Service> Stopped")
      retval = CELLULAR_OK;
    }
  

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error during power off process")
  }
  return (retval);
}

/**
  * @brief  Power reset the modem
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_reset(CS_Reset_t rst_type)
{
  CS_Status_t retval = CELLULAR_OK;
  at_status_t err;
	char *pstr;
  PrintAPI("CS_power_reset")

 if(rst_type == CS_RESET_HW )
	 retval = SIM_power_reset();
 else
 {
	 //CRESET
	  err = AT_sendcmd( SID_CS_RESET, &cmd_buf[0], &rsp_buf[0]);
		 if( (pstr = strstr((char*)rsp_buf,"OK")) != NULL)
		 {
			  PrintDBG("Cellular started and ready")
			  retval = CELLULAR_OK;
		  }
	 
 }
  return (retval);
}

/**
  * @brief  Check that modem connection is successfully established
  * @note   Usually, the command AT is sent and OK is expected as response
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_check_connection(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
  PrintAPI("CS_check_connection")

  
    err = AT_sendcmd( SID_CS_CHECK_CNX, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
	  if(strstr((char*)rsp_buf,"OK\r\n") != NULL)
	  {
		  PrintDBG("<Cellular_Service> Modem connection OK")
		  retval = CELLULAR_OK;
	  }
    }
  

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error with modem connection")
  }
  return (retval);
}
/**
  * @brief  Read the actual signal quality seen by Modem .
  * @param  p_sig_qual Handle to signal quality structure.
  * @retval CS_Status_t
  */
CS_Status_t CS_get_signal_quality(CS_SignalQuality_t *p_sig_qual)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
 char *pstr;
	uint8_t rssi;
  PrintAPI("CS_get_signal_quality")

  CS_SignalQuality_t local_sig_qual = {0};
  memset((void *)&local_sig_qual, 0, sizeof(CS_SignalQuality_t));

 
    err = AT_sendcmd( SID_CS_GET_SIGNAL_QUALITY, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
		if( (pstr = strstr((char*)rsp_buf,"+CSQ:")) != NULL)
	  {
		local_sig_qual.rssi = ATutil_convertStringToInt((uint8_t*)(pstr+6),2);
		 
		if( (pstr = strstr((char*)rsp_buf,",")) != NULL)
			local_sig_qual.ber = ATutil_convertStringToInt((uint8_t*)(pstr+1),2);
		 PrintDBG("rssi is %d\r\n",local_sig_qual.rssi ); 
		 PrintDBG("ber is %d\r\n",local_sig_qual.ber );
	  }
      PrintDBG("<Cellular_Service> Signal quality informations received")
		 
      /* recopy info to user */
      memcpy((void *)p_sig_qual, (void *)&local_sig_qual, sizeof(CS_SignalQuality_t));
	  
      retval = CELLULAR_OK;
    }
  

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error when getting signal quality")
  }
  return (retval);
}
//atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, _AT_CREG, 

/**
  * @brief  Request the Modem to register to the Cellular Network.
  * @note   This function is used to select the operator. It returns a detailled
  *         network registration status.
  * @param  p_devinfo Handle on operator information structure.
  * @param  p_reg_status Handle on registration information structure.
  *         This information is valid only if return code is CELLULAR_OK
  * @retval CS_Status_t
  */
/*************

  if ((cst_context.current_EPS_NetworkRegState  != CS_NRS_REGISTERED_HOME_NETWORK)
        && (cst_context.current_EPS_NetworkRegState  != CS_NRS_REGISTERED_ROAMING)
        && (cst_context.current_GPRS_NetworkRegState != CS_NRS_REGISTERED_HOME_NETWORK)
        && (cst_context.current_GPRS_NetworkRegState != CS_NRS_REGISTERED_ROAMING))
    {
      
************/
CS_Status_t CS_register_net(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
	char *pstr;
	uint8_t reg_stat;
	static uint8_t step = 0;
  PrintAPI("CS_register_net")

  /* init returned fields */
 
	gCS_Send_steps = 4;
	do{
		
		err = AT_sendcmd(SID_CS_REGISTER_NET, &cmd_buf[0], &rsp_buf[0]);
		if (err == ATSTATUS_OK)
		{
			
			if( (pstr = strstr((char*)rsp_buf,"+CEREG:")) != NULL)
			{
				if( (pstr = strstr((char*)rsp_buf,",")) != NULL)
					reg_stat = ATutil_convertStringToInt((uint8_t*)(pstr+1),1);
				if(reg_stat == 1 )
				{
					//cst_context.current_EPS_NetworkRegState  = CS_NRS_REGISTERED_HOME_NETWORK;
					retval = CELLULAR_NOT_IMPLEMENTED;
					//PrintDBG("<Cellular_Service> Network registration done")
				}
				else if (reg_stat == 5)
				{
					//cst_context.current_EPS_NetworkRegState  = CS_NRS_REGISTERED_ROAMING;

				}
				else
					PrintDBG(" Cellular_Service creg stat is %d\r\n",reg_stat)
					
			}
			else if( (pstr = strstr((char*)rsp_buf,"+CGREG:")) != NULL)
			{
				__nop();
//				if( (pstr = strstr((char*)rsp_buf,",")) != NULL)
//					reg_stat = ATutil_convertStringToInt((uint8_t*)(pstr+1),1);
//				if((reg_stat == 1 )||(reg_stat == 5))
//				{
//					retval = CELLULAR_OK;
//					PrintDBG("<Cellular_Service> Network registration done")
//				}
//				else
//					PrintDBG(" Cellular_Service cgreg stat is %d\r\n",reg_stat)
				if( (pstr = strstr((char*)rsp_buf,"OK")) != NULL)
				{
						retval = CELLULAR_OK;    
				}

			   
			}
			else
			{
				retval =	CELLULAR_ERROR;
				break;
			}
		 
		}
		
	}while(gCS_Send_steps); 
  

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error during network registration ")
  }
  return (retval);
}
/**
  * @brief  Initialize the service and configures the Modem FW functionalities
  * @note   Used to provide PIN code (if any) and modem function level.
  * @param  init Function level (MINI, FULL, SIM only).
  * @param  reset Indicates if modem reset will be applied or not.
  * @param  pin_code PIN code string.
  *
  * @retval CS_Status_t
  */
CS_Status_t CS_init_modem(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
	char *pstr;
  PrintAPI("CS_init_modem")

 
    err = AT_sendcmd( SID_CS_INIT_MODEM, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
		if( (pstr = strstr((char*)rsp_buf,"+CPIN:")) != NULL)
		{
			 PrintDBG("SID_CS_INIT_MODEM %s",pstr)
		}
      PrintDBG("<Cellular_Service> Init done succesfully")
      retval = CELLULAR_OK;
    }
    else
    {
       retval = CELLULAR_ERROR;
    }
  

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error during init")
  }
  return (retval);
}

/**
  * @brief  Request of packet attach status.
  * @param  p_attach Handle to PS attach status.
  * @retval CS_Status_t
  */
CS_Status_t CS_get_attach_status(CS_PSattach_t *p_attach)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
		char *pstr;
  PrintAPI("CS_get_attachstatus")

 
    err = AT_sendcmd(  SID_CS_GET_ATTACHSTATUS, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
		if( (pstr = strstr((char*)rsp_buf,",1")) != NULL)
		{
			*p_attach = CS_PS_ATTACHED;
			retval = CELLULAR_OK;
		}
    }
  

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error when getting attachment status")
  }
  return (retval);
}

/**
  * @brief  Register to event notification related to internet connection.
  * @note   This function is used to register to an event related to a PDN
  *         Only explicit config id (CS_PDN_USER_CONFIG_1 to CS_PDN_USER_CONFIG_5) are
  *         suppported and CS_PDN_PREDEF_CONFIG
  * @param  cid Configuration identifier number.
  * @param  pdn_event_callback client callback to call when an event occured.
  * @retval CS_Status_t
  */
CS_Status_t  CS_register_pdn_event(uint8_t cid, cellular_pdn_event_callback_t pdn_event_callback)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
  PrintAPI("CS_register_pdn_event")

  /* check parameters validity */
//  if (cid > CS_PDN_USER_CONFIG_5)
//  {
//    PrintErr("<Cellular_Service> only explicit PDN user config is supported (cid=%d)", cid)
//    return (CELLULAR_ERROR);
//  }

   err = AT_sendcmd(SID_CS_REGISTER_PDN_EVENT, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PrintDBG("<Cellular_Service> PDN events registered successfully")
      /* register callback */
      //urc_packet_domain_event_callback[cid] = pdn_event_callback;
    //  cs_ctxt_urc_subscription.packet_domain_event = CELLULAR_TRUE;
      retval = CELLULAR_OK;
    }
   
  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service>error when registering PDN events")
  }
  return (retval);
}

CS_Status_t CDS_socket_open( uint8_t force)
{
  
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;

  PrintAPI("CDS_socket_open")

 
    
      /* Send socket informations to ATcustom */
     
        err = AT_sendcmd( SID_CS_SOCKET_OPEN, &cmd_buf[0], &rsp_buf[0]);
        if (err == ATSTATUS_OK)
        {
			if(strstr((char*)rsp_buf,"OK") != NULL)
			{
				__nop();
				retval = CELLULAR_OK;
			}
			else if(strstr((char*)rsp_buf,"already opened") != NULL)
			{
				retval = CELLULAR_ERROR;
			}
        }
		return retval;

 }
   
 
/**
  * @brief  Free a socket handle.
  * @note   If a PDN is activated at socket creation, the socket will not be deactivated at socket closure.
  * @param  sockHandle Handle of the socket
  * @param  force Force to free the socket.
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_close( uint8_t force)
{
  
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;

  PrintAPI("CDS_socket_close")

 
      /* Send socket informations to ATcustom */
      
        err = AT_sendcmd( SID_CS_SOCKET_CLOSE, &cmd_buf[0], &rsp_buf[0]);
        if (err == ATSTATUS_OK)
        {
			if(strstr((char*)rsp_buf,"OK") != NULL)
			{
				/* deallocate socket handle and reinit socket parameters */
				//  socket_deallocateHandle(sockHandle);
				socket_init();
				retval = CELLULAR_OK;
			}
        }
    
  

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error when closing socket")
  }
  return (retval);
}
/**
  * @brief  Free a socket handle.
  * @note   If a PDN is activated at socket creation, the socket will not be deactivated at socket closure.
  * @param  sockHandle Handle of the socket
  * @param  force Force to free the socket.
  * @retval CS_Status_t
  */
CS_Status_t CDS_tcp_close( uint8_t force)
{
  
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;

  PrintAPI("CDS_tcp_close")

 
    err = AT_sendcmd(  SID_CS_TCP_CLOSE, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
			if(strstr((char*)rsp_buf,"OK") != NULL)
			{
				__nop();
				retval = CELLULAR_OK;
			}
			else
				retval = CELLULAR_ERROR;
    }
 

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error when closing socket")
  }
  return (retval);
}
/**
  * @brief  Activates a PDN (Packet Data Network Gateway) allowing communication with internet.
  * @note   This function triggers the allocation of IP public WAN to the device.
  * @note   Only one PDN can be activated at a time.
  * @param  cid Configuration identifier
  *         This parameter can be one of the following values:
  *         CS_PDN_PREDEF_CONFIG To use default PDN configuration.
  *         CS_PDN_USER_CONFIG_1-5 To use a dedicated PDN configuration.
  *         CS_PDN_CONFIG_DEFAULT To use default PDN config set by CS_set_default_pdn().
  * @retval CS_Status_t
  */
CS_Status_t CS_activate_pdn(uint8_t cid)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
  PrintAPI("CS_activate_pdn for cid=%d", cid)

 
    err = AT_sendcmd( SID_CS_ACTIVATE_PDN, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PrintDBG("<Cellular_Service> PDN %d connected", cid)
      retval = CELLULAR_OK;
    }
 

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error when PDN %cid activation", cid)
  }
  return (retval);
}

static CS_Status_t socket_configure_remote(uint8_t *p_ip_addr_value, uint16_t remote_port)
{
  CS_Status_t retval = CELLULAR_OK;

  /* check that socket has been allocated */
//  if (cs_ctxt_sockets_info.state == SOCKETSTATE_NOT_ALLOC)
//  {
//    PrintErr("<Cellular_Service> invalid socket handle ")
//    return (CELLULAR_ERROR);
//  }

  if (p_ip_addr_value == NULL)
  {
    PrintErr("<Cellular_Service> NULL ptr")
    return (CELLULAR_ERROR);
  }

  cs_ctxt_sockets_info.remote_port = remote_port;
  memset((void *) &cs_ctxt_sockets_info.remote_ip_addr_value, 0, MAX_IP_ADDR_SIZE);
  memcpy((void *) &cs_ctxt_sockets_info.remote_ip_addr_value, (void *)p_ip_addr_value, strlen((const char *)p_ip_addr_value));

  PrintDBG("DBG: remote_port=%d", cs_ctxt_sockets_info.remote_port)
  PrintDBG("DBG: ip_addr cb=%s", cs_ctxt_sockets_info.remote_ip_addr_value)

  return (retval);
}

CS_Status_t CDS_socket_set_option(void)
{
 
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
char *pstr;
  PrintAPI("CDS_socket_set_option")

 
	gCS_Send_steps = 2;


	do{
			err = AT_sendcmd( SID_CS_SOCKET_CREATE, &cmd_buf[0], &rsp_buf[0]);
		if (err == ATSTATUS_OK)
		{
			PrintDBG("<Cellular_Service> CDS_socket_set_option")
			retval = CELLULAR_OK;

			if( (pstr = strstr((char*)rsp_buf,"OK")) != NULL)
			{
			 __nop();
					
			}
			else if( (pstr = strstr((char*)rsp_buf,"OK")) != NULL)
			{
			      retval= CELLULAR_OK;
			}
			else
			{
				retval =	CELLULAR_ERROR;
				break;
			}
		 
		}
		
	}while(gCS_Send_steps); 
		
  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error when CDS_socket_set_option")
  }
  return (retval);
 
}

CS_Status_t CDS_ipmode_set(uint8_t par)
{
 
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
  PrintAPI("CDS_socket_set_option")

 
    err = AT_sendcmd( SID_CS_SECLECT_IPMODE, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PrintDBG("<Cellular_Service> CDS_socket_set_option")
      retval = CELLULAR_OK;
    }
 

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error when CDS_socket_set_option")
  }
  return (retval);
  }

/**
  * @brief  Connect to a remote server (for socket client mode).
  * @note   This function is blocking until the connection is setup or when the timeout to wait
  *         for socket connection expires.
  * @param  sockHandle Handle of the socket.
  * @param  ip_addr_type Specifies the type of IP address of the remote server.
  *         This parameter can be one of the following values:
  *         IPAT_IPV4 for IPV4 (default)
  *         IPAT_IPV6  for IPV6
  * @param  p_ip_addr_value Specifies the IP address of the remote server.
  * @param  remote_port Specifies the port of the remote server.
  * @retval CS_Status_t
  */
  
CS_Status_t CDS_socket_connect(   uint8_t *p_ip_addr_value, uint16_t remote_port)
{
  at_status_t err;
  CS_Status_t retval;

  PrintAPI("CDS_socket_connect")

 retval = socket_configure_remote( p_ip_addr_value, remote_port);
  if (retval == CELLULAR_OK)
  {
    /* Send socket informations to ATcustom
    * no need to test sockHandle validity, it has been tested in socket_configure_remote()
    */
//    csint_socket_infos_t *socket_infos = &cs_ctxt_sockets_info;
   
		
        err = AT_sendcmd( SID_CS_DIAL_COMMAND, &cmd_buf[0], &rsp_buf[0]);
   
      if (err == ATSTATUS_OK)
      {
		  if(strstr((char*)rsp_buf,"OK") != NULL)
		  {
			  /* update socket state */
			  cs_ctxt_sockets_info.state = SOCKETSTATE_CONNECTED;
			  retval = CELLULAR_OK;
		  }
      }
      else
      {
        PrintErr("<Cellular_Service> error when socket connection")
        retval = CELLULAR_ERROR;
      }
    }
    else
    {
      retval = CELLULAR_ERROR;
	}

  return (retval);
}

/**
  * @brief  Listen to clients (for socket server mode).
  * @note   Function not implemeted yet
  * @param  sockHandle Handle of the socket
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_listen(void)
{
  /*UNUSED(sockHandle);*/

  /* for socket server mode */
  return (CELLULAR_NOT_IMPLEMENTED);
}
/**
  * @brief  Define internet data profile for a configuration identifier
  * @param  cid Configuration identifier
  * @param  apn A string of the access point name (must be non NULL)
  * @param  pdn_conf Structure which contains additional configurations parameters (if non NULL)
  * @retval CS_Status_t
  */
CS_Status_t CS_define_pdn(uint8_t cid, const uint8_t *apn,const uint8_t *pdptype)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
 
  PrintAPI("CS_define_pdn for cid=%d", cid)

 

  /* prepare and send PDN infos */
   memset((void *)&cs_pdn_infos, 0, sizeof(csint_pdn_infos_t));
//  pdn_infos.conf_id = cid;
  memcpy((void *)&cs_pdn_infos.apn[0],   (const uint8_t *)apn,      strlen((const char *)apn) );
  memcpy((void *)&cs_pdn_infos.PDP_type, (const uint8_t *)pdptype, strlen((const char *)pdptype) );

  cs_pdn_infos.conf_id = cid;
    err = AT_sendcmd( SID_CS_DEFINE_PDN, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
		if(strstr((char*)rsp_buf,"OK") != NULL)
		{
			PrintDBG("<Cellular_Service> PDN %d defined", cid)
			retval = CELLULAR_OK;
		}
    }
 

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error when defining PDN %d", cid)
  }
  return (retval);
}
/**
  * @brief  Get the IP address allocated to the device for a given PDN.
  * @param  cid Configuration identifier number.
  * @param  ip_addr_type IP address type and format.
  * @param  p_ip_addr_value Specifies the IP address of the given PDN.
  * @retval CS_Status_t
  */
CS_Status_t CS_get_dev_IP_address(uint8_t cid,  uint8_t *p_ip_addr_value)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
  PrintAPI("CS_get_dev_IP_address (for conf_id=%d)", cid)

 
    err = AT_sendcmd( SID_CS_GET_IP_ADDRESS, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      char ip_addr_value[MAX_IP_ADDR_SIZE];
    
        PrintDBG("<Cellular_Service> IP address informations received")
        /* recopy info to user */
       // *ip_addr_type = ip_addr_info.ip_addr_type;
        memcpy((void *)p_ip_addr_value, (void *)&ip_addr_value, strlen((char *)ip_addr_value));
        PrintDBG("<Cellular_Service> IP address = %s ", (char*)ip_addr_value)
        retval = CELLULAR_OK;
       
	}

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error when getting IP address informations")
  }
  return (retval);
}
 
/**
  * @brief  Send data over a socket to a remote server.
  * @note   This function is blocking until the data is transfered or when the
  *         timeout to wait for transmission expires.
  * @param  sockHandle Handle of the socket
  * @param  p_buf Pointer to the data buffer to transfer.
  * @param  length Length of the data buffer.
  * @retval CS_Status_t
  */
csint_socket_data_buffer_t send_data_struct;
  csint_socket_data_buffer_t receive_data_struct = {0};
CS_Status_t CDS_socket_send( const uint8_t *p_buf, uint32_t length)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;

  PrintAPI("CDS_socket_send (buf@=%p - buflength = %d)", p_buf, length)

  /* check that size does not exceed maximum buffers size */
  if (length > DEFAULT_IP_MAX_PACKET_SIZE)
  {
    PrintErr("<Cellular_Service> buffer size %d exceed maximum value %d",   length,    DEFAULT_IP_MAX_PACKET_SIZE)
    return (CELLULAR_ERROR);
  }

  /* check that socket has been allocated */
  if (cs_ctxt_sockets_info.state != SOCKETSTATE_CONNECTED)
  {
    PrintErr("<Cellular_Service> socket not connected (state=%d)", cs_ctxt_sockets_info.state)
    return (CELLULAR_ERROR);
  }

   send_data_struct.p_buffer_addr_send = p_buf;
  send_data_struct.p_buffer_addr_rcv = NULL;
  send_data_struct.buffer_size = length;
  
    err = AT_sendcmd( SID_CS_SEND_DATA, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
		if(strstr((char*)rsp_buf,">") != NULL)
		{
			    SIM_send_uart((uint8_t *)&send_data_struct.p_buffer_addr_send[0], length);
				PrintDBG("<Cellular_Service> socket data sent")
				retval = CELLULAR_OK;
		}
     
    }
 

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error when sending data to socket")
  }
  return (retval);
}

/**
  * @brief  Get connection status for a given socket.
  * @note   If a PDN is activated at socket creation, the socket will not be deactivated at socket closure.
  * @param  sockHandle Handle of the socket
  * @param  p_infos Pointer of infos structure.
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_cnx_status( CS_SocketCnxInfos_t *p_infos)
{
  CS_Status_t retval = CELLULAR_OK;
  at_status_t err;
  PrintAPI("CDS_socket_cnx_status")

//  /* check that socket has been allocated */
//  if (cs_ctxt_sockets_info[sockHandle].state != SOCKETSTATE_CONNECTED)
//  {
//    PrintErr("<Cellular_Service> socket not connected (state=%d) for handle %d (status)",
//             cs_ctxt_sockets_info[sockHandle].state,
//             sockHandle)
//    return (CELLULAR_ERROR);
//  }

//  /* Send socket informations to ATcustom */
//  csint_socket_cnx_infos_t socket_cnx_infos;
//  socket_cnx_infos.socket_handle = sockHandle;
//  socket_cnx_infos.infos = p_infos;
//  memset((void *)p_infos, 0, sizeof(CS_SocketCnxInfos_t));
//  if (DATAPACK_writePtr(&cmd_buf[0], CSMT_SOCKET_CNX_STATUS, (void *)&socket_cnx_infos) == DATAPACK_OK)
//  {
//    err = AT_sendcmd(_Adapter_Handle, SID_CS_SOCKET_CNX_STATUS, &cmd_buf[0], &rsp_buf[0]);
//    if (err == ATSTATUS_OK)
//    {
//      PrintDBG("<Cellular_Service> socket cnx status received")
//      retval = CELLULAR_OK;
//    }
//  }

//  if (retval == CELLULAR_ERROR)
//  {
//    PrintErr("<Cellular_Service> error when requesting socket cnx status")
//  }
  return (retval);
}
/**
  * @brief  Read the latest registration state to the Cellular Network.
  * @param  p_reg_status Handle to registration status structure.
  *         This information is valid only if return code is CELLULAR_OK
  * @retval CS_Status_t
  */
CS_Status_t CS_get_net_status(CS_RegistrationStatus_t *p_reg_status)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
	char *pstr;
 // PrintAPI("CS_get_netstatus")

  /* init returned fields */
  p_reg_status->optional_fields_presence = CS_RSF_NONE;
  p_reg_status->CS_NetworkRegState = CS_NRS_UNKNOWN;
  p_reg_status->GPRS_NetworkRegState = CS_NRS_UNKNOWN;
  p_reg_status->EPS_NetworkRegState = CS_NRS_UNKNOWN;

  gCS_Send_steps = 4;
	do{
		
		err = AT_sendcmd(SID_CS_REGISTER_NET, &cmd_buf[0], &rsp_buf[0]);
		if (err == ATSTATUS_OK)
		{
			switch(gCS_Send_steps)
			{
				case 3:
					if( (pstr = strstr((char*)rsp_buf,"+CEREG:")) != NULL)
					{
						if( (pstr = strstr((char*)rsp_buf,",")) != NULL)
						{
							if(*(pstr+1) == '1' )
								p_reg_status->EPS_NetworkRegState = CS_NRS_REGISTERED_HOME_NETWORK;
							else if (*(pstr+1) == '5' )
								p_reg_status->EPS_NetworkRegState = CS_NRS_REGISTERED_ROAMING;
						}
					}
					break;
				case 2:
					if( (pstr = strstr((char*)rsp_buf,"+CREG:")) != NULL)
					{
						if( (pstr = strstr((char*)rsp_buf,",")) != NULL)
						{
							if(*(pstr+1) == '1' )
								p_reg_status->CS_NetworkRegState = CS_NRS_REGISTERED_HOME_NETWORK;
							else if (*(pstr+1) == '5' )
								p_reg_status->CS_NetworkRegState = CS_NRS_REGISTERED_ROAMING;
						}
						
					}
					break;
				case 1:
					if( (pstr = strstr((char*)rsp_buf,"+CGREG:")) != NULL)
					{
							if( (pstr = strstr((char*)rsp_buf,",")) != NULL)
						{
							if(*(pstr+1) == '1' )
								p_reg_status->GPRS_NetworkRegState = CS_NRS_REGISTERED_HOME_NETWORK;
							else if (*(pstr+1) == '5' )
								p_reg_status->GPRS_NetworkRegState = CS_NRS_REGISTERED_ROAMING;
						}
						
						
					}
					break;
				case 0:
					if( (pstr = strstr((char*)rsp_buf,"+COPS:")) != NULL)
					{
						if( (pstr = strstr((char*)rsp_buf,",")) != NULL)
						{
							if(*(pstr+1) == '0' )
								p_reg_status->optional_fields_presence = CS_RSF_FORMAT_PRESENT ;
							 
						}
						retval = CELLULAR_OK;  
					}
					break;
				default:
					gCS_Send_steps = 0;
					break;
			}
 
		 
		}
		
	}while(gCS_Send_steps); 
  

  if (retval == CELLULAR_ERROR)
  {
    //PrintErr("<Cellular_Service> error when getting net status")
  }
  return (retval);
}
/**
  * @brief  Return information related to modem status.
  * @param  p_devinfo Handle on modem information structure.
  * @retval CS_Status_t
  */
CS_Status_t CS_get_device_info(CS_DeviceInfo_t *p_devinfo)
{
  CS_Status_t retval = CELLULAR_OK;
  at_status_t err;
  PrintAPI("CS_get_device_info")

//  /* reset our local copy */
//  memset((void *)&cs_ctxt_device_info, 0, sizeof(cs_ctxt_device_info));
// // cs_ctxt_device_info. field_requested = p_devinfo->field_requested;
// 
//    err = AT_sendcmd(_Adapter_Handle, SID_CS_GET_DEVICE_INFO, &cmd_buf[0], &rsp_buf[0]);
//    if (err == ATSTATUS_OK)
//    {
//      PrintDBG("<Cellular_Service> Device infos received")
//      /* send info to user */
//      memcpy((void *)p_devinfo, (void *)&cs_ctxt_device_info, sizeof(CS_DeviceInfo_t));
//      retval = CELLULAR_OK;
//    }
//    else
//    {
//      /retval = CELLULAR_analyze_error_report(&rsp_buf[0]);
//    }
// 

  if (retval == CELLULAR_ERROR)
  {
    PrintErr("<Cellular_Service> error when getting device infos")
  }
  return (retval);
}


/**
  * @brief  Receive data from the connected remote server.
  * @note   This function is blocking until expected data length is received or a receive timeout has expired.
  * @param  sockHandle Handle of the socket
  * @param  p_buf Pointer to the data buffer to received data.
  * @param  max_buf_length Maximum size of receive data buffer.
  * @retval Size of received data (in bytes).
  */
extern struct rt_ringbuffer gSim_ringbuf;

int32_t CDS_socket_receive(uint8_t *socket_rec_buf,uint16_t length)
{
  CS_Status_t status = CELLULAR_ERROR;
  at_status_t err;
  uint32_t bytes_received = 0U;

uint16_t  data_len;

  /* check that socket has been allocated */
  if (cs_ctxt_sockets_info.state != SOCKETSTATE_CONNECTED)
  {
    //PrintErr("<Cellular_Service> socket not connected (state=%d)  ", cs_ctxt_sockets_info.state)
    return (-1);
  }


 
 // receive_data_struct.p_buffer_addr_send = NULL;
 // receive_data_struct.p_buffer_addr_rcv = p_buf;
//  receive_data_struct.buffer_size = 0U;
 
 data_len = rt_ringbuffer_data_len(&gSim_ringbuf);
  if(data_len > 4)
  {
	  
	  rt_ringbuffer_get(&gSim_ringbuf,socket_rec_buf,data_len);
	  if(strstr((char*)socket_rec_buf,"+IPD:") != NULL)
	  {
		  __nop();
		 bytes_received = data_len;
		  status = CELLULAR_OK;
	  }
  }
  
  if (status == CELLULAR_OK)
  {
 
    PrintINFO("Size of data received on the socket= %d bytes", bytes_received)
    return ((int32_t)bytes_received);
  }
  return 0;
}
