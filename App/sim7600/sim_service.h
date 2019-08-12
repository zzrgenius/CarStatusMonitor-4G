#ifndef _SIM_SERVICE_H
#define _SIM_SERVICE_H

#include "main.h"
#include "sim_drv.h"
#include "cmsis_os.h"
#include "sim_core.h"
#include "sim_service.h"

/* Exported constants --------------------------------------------------------*/
#define CELLULAR_MAX_SOCKETS     (6U)
#define MAX_SIZE_IMEI            ((uint16_t) 64U)  /* MAX = 32 characters */
#define MAX_SIZE_MANUFACT_NAME   ((uint16_t) 256U) /* theorical MAX = 2048 characters !!! */
#define MAX_SIZE_MODEL           ((uint16_t) 256U) /* theorical MAX = 2048 characters !!! */
#define MAX_SIZE_REV             ((uint16_t) 64U)  /* theorical MAX = 2048 characters !!! */
#define MAX_SIZE_SN              ((uint16_t) 64U)  /* theorical MAX = 2048 characters !!! */
#define MAX_SIZE_IMSI            ((uint16_t) 64U)  /* MAX = 32 characters */
#define MAX_SIZE_PHONE_NBR       ((uint16_t) 64U)  /* MAX = 32 characters */
#define MAX_SIZE_OPERATOR_NAME   ((uint16_t) 64U)  /* MAX = 64 characters */
#define MAX_SIZE_USERNAME        ((uint16_t) 32U)  /* MAX = 32 characters */
#define MAX_SIZE_PASSWORD        ((uint16_t) 32U)  /* MAX = 32 characters */
#define MAX_SIZE_IPADDR          ((uint16_t) 64U)  /* MAX = 64 characters */
#define CS_INVALID_SOCKET_HANDLE ((socket_handle_t)-1)


#define MAX_IP_ADDR_SIZE               (64U)
#define MAX_APN_SIZE                   (64U)
#define MAX_SIZE_OPERATOR_NAME   ((uint16_t) 64U)  /* MAX = 64 characters */

#define DEFAULT_IP_MAX_PACKET_SIZE     (512U) /* TODO: Hard-Coded but should use real modem limit */
#define DEFAULT_TRP_MAX_TIMEOUT        (90U)
#define DEFAULT_TRP_CONN_SETUP_TIMEOUT (600U)
#define DEFAULT_TRP_TRANSFER_TIMEOUT   (50U)
#define DEFAULT_TRP_SUSPEND_TIMEOUT    (1000U)
#define DEFAULT_TRP_RX_TIMEOUT         (50U)
/* enum */
typedef enum
{
  CELLULAR_FALSE = 0,
  CELLULAR_TRUE  = 1,
} CS_Bool_t;
typedef int socket_handle_t;
typedef enum
{
  SID_CS_CHECK_CNX = CELLULAR_SERVICE_START_ID,
  /* Control */
  SID_CS_POWER_ON,
  SID_CS_POWER_OFF,
  SID_CS_INIT_MODEM,
  SID_CS_GET_DEVICE_INFO,
  SID_CS_REGISTER_NET,
  SID_CS_SUSBCRIBE_NET_EVENT,
  SID_CS_UNSUSBCRIBE_NET_EVENT,
  SID_CS_GET_NETSTATUS,
  SID_CS_GET_ATTACHSTATUS,
  SID_CS_GET_SIGNAL_QUALITY,
  SID_CS_ACTIVATE_PDN,
  SID_ATTACH_PS_DOMAIN,
  SID_DETACH_PS_DOMAIN,
  SID_CS_DEACTIVATE_PDN,
  SID_CS_REGISTER_PDN_EVENT,
  SID_CS_DEREGISTER_PDN_EVENT,
  SID_CS_GET_IP_ADDRESS,
  SID_CS_DEFINE_PDN,
  SID_CS_SET_DEFAULT_PDN,
  /* SID_CS_AUTOACTIVATE_PDN, */
  /* SID_CS_CONNECT, */
  /* SID_CS_DISCONNECT, */
  SID_CS_DIAL_ONLINE, /* NOT SUPPORTED */
  SID_CS_DIAL_COMMAND,
  SID_CS_SEND_DATA,
  SID_CS_RECEIVE_DATA,
  SID_CS_SECLECT_IPMODE,
  SID_CS_SOCKET_CREATE, 
  SID_CS_SOCKET_NETOPEN,
  SID_CS_SOCKET_SET_OPTION,     /* not needed, config is done at CS level */
  /* SID_CS_SOCKET_GET_OPTION, */ /* not needed, config is done at CS level */
  /* SID_CS_SOCKET_RECEIVE_DATA, */
  SID_CS_SOCKET_OPEN,
  SID_CS_SOCKET_CLOSE,
  SID_CS_TCP_OPEN,
  SID_CS_TCP_CLOSE,
  /* DATA mode and COMMAND mode swiches */
  SID_CS_DATA_SUSPEND,
  SID_CS_DATA_RESUME,
  SID_CS_RESET,
  SID_CS_SOCKET_CNX_STATUS,
  SID_CS_MODEM_CONFIG,
  SID_CS_DNS_REQ,
  SID_CS_PING_IP_ADDRESS,
  SID_CS_SUSBCRIBE_MODEM_EVENT,

} CS_msg_t;



/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  CST_MESSAGE_CS_EVENT    = 0,
  CST_MESSAGE_DC_EVENT    = 1,
  CST_MESSAGE_URC_EVENT   = 2,
  CST_MESSAGE_TIMER_EVENT = 3,
  CST_MESSAGE_CMD         = 4
} CST_message_type_t;

typedef enum
{
  CST_OFF_EVENT,
  CST_INIT_EVENT,
  CST_MODEM_POWER_ON_EVENT,
  CST_MODEM_POWERED_ON_EVENT,
  CST_MODEM_INITIALIZED_EVENT,
  CST_NETWORK_CALLBACK_EVENT,
  CST_SIGNAL_QUALITY_EVENT,
  CST_NW_REG_TIMEOUT_TIMER_EVENT,
  CST_NETWORK_STATUS_EVENT,
  CST_NETWORK_STATUS_OK_EVENT,
  CST_MODEM_ATTACHED_EVENT,
  CST_PDP_ACTIVATED_EVENT,
  CST_MODEM_PDN_ACTIVATE_RETRY_TIMER_EVENT,
  CST_PDN_STATUS_EVENT,
  CST_CELLULAR_DATA_FAIL_EVENT,
  CST_FAIL_EVENT,
  CST_POLLING_TIMER,
  CST_MODEM_URC,
  CST_NO_EVENT,
  CST_CMD_UNKWONW_EVENT
} CST_autom_event_t;
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
  CST_MODEM_SOCKET_SET_STATE, 
  CST_MODEM_SOCKET_OPENNET_STATE,
  CST_MODEM_SOCKET_OPENTCP_STATE,
  CST_MODEM_SOCKET_TCPOK_STATE,

  CST_MODEM_SOCKET_CLOSETCP_STATE,
  CST_MODEM_SOCKET_CLOSENET_STATE,
  CST_MODEM_REPROG_STATE,
  CST_MODEM_FAIL_STATE,
  CST_MODEM_NETWORK_STATUS_FAIL_STATE,
} CST_state_t;

typedef struct
{
  uint8_t rssi; 
  uint8_t ber;
} CS_SignalQuality_t;
typedef enum
{
  CS_NRS_NOT_REGISTERED_NOT_SEARCHING     = 0,
  CS_NRS_REGISTERED_HOME_NETWORK          = 1,
  CS_NRS_NOT_REGISTERED_SEARCHING         = 2,
  CS_NRS_REGISTRATION_DENIED              = 3,
  CS_NRS_UNKNOWN                          = 4,
  CS_NRS_REGISTERED_ROAMING               = 5,
  CS_NRS_REGISTERED_SMS_ONLY_HOME_NETWORK = 6,
  CS_NRS_REGISTERED_SMS_ONLY_ROAMING      = 7,
  CS_NRS_EMERGENCY_ONLY                   = 8,
  CS_NRS_REGISTERED_CFSB_NP_HOME_NETWORK  = 9,
  CS_NRS_REGISTERED_CFSB_NP_ROAMING       = 10,
} CS_NetworkRegState_t;
typedef enum
{
  CELLULAR_OK = 0,                /* No error */
  CELLULAR_ERROR,                 /* Generic error */
  CELLULAR_NOT_IMPLEMENTED,       /* Function not implemented yet */
  /* - SIM errors - */
  CELLULAR_SIM_BUSY,               /* SIM error: SIM is busy */
  CELLULAR_SIM_NOT_INSERTED,       /* SIM error: SIM not inserted */
  CELLULAR_SIM_PIN_OR_PUK_LOCKED,  /* SIM error: SIM locked due to PIN, PIN2, PUK or PUK2 */
  CELLULAR_SIM_INCORRECT_PASSWORD, /* SIM error: SIM password is incorrect */
  CELLULAR_SIM_ERROR,              /* SIM error: SIM other error */

} CS_Status_t;
typedef enum
{
  CST_NO_FAIL,
  CST_MODEM_POWER_ON_FAIL,
  CST_MODEM_RESET_FAIL,
  CST_MODEM_CSQ_FAIL,
  CST_MODEM_GNS_FAIL,
  CST_MODEM_REGISTER_FAIL,
  CST_MODEM_ATTACH_FAIL,
  CST_MODEM_PDP_DEFINE_FAIL,
  CST_MODEM_PDP_ACTIVATION_FAIL,
  CST_MODEL_CELLULAR_DATA_FAIL
} CST_fail_cause_t;
typedef enum
{
  CS_PDN_EVENT_OTHER, /*none of the event described below */
  CS_PDN_EVENT_NW_DETACH,
  CS_PDN_EVENT_NW_DEACT,
  CS_PDN_EVENT_NW_PDN_DEACT,
} CS_PDN_event_t;

typedef struct
{
  CST_state_t          current_state;
  CST_fail_cause_t     fail_cause;
  CS_PDN_event_t       pdn_status;
  CS_SignalQuality_t   signal_quality;
  CS_NetworkRegState_t current_EPS_NetworkRegState;
  CS_NetworkRegState_t current_GPRS_NetworkRegState;
  CS_NetworkRegState_t current_CS_NetworkRegState;
  CS_Status_t          sim_mode;
  uint16_t             set_pdn_mode;
  uint16_t             activate_pdn_nfmc_tempo_count;
  uint16_t             register_retry_tempo_count;
  uint16_t             power_on_reset_count ;
  uint16_t             reset_reset_count ;
  uint16_t             init_modem_reset_count ;
  uint16_t             csq_reset_count ;
  uint16_t             gns_reset_count ;
  uint16_t             attach_reset_count ;
  uint16_t             activate_pdn_reset_count ;
  uint16_t             cellular_data_retry_count ;
  uint16_t             global_retry_count ;
} CST_context_t;

typedef struct
{
  uint16_t  type ;
  uint16_t  id  ;
} CST_message_t;


typedef enum
{
  SOCKETSTATE_NOT_ALLOC         = 0,
  SOCKETSTATE_CREATED           = 1,
  SOCKETSTATE_CONNECTED         = 2,
  SOCKETSTATE_ALLOC_BUT_INVALID = 3, /* valid from client point of view but not from modem (exple: after reset) */
} csint_Socket_State_t;
typedef enum
{
  CS_SON_NO_OPTION                 = 0x00,
  CS_SON_IP_MAX_PACKET_SIZE        = 0x01,  /* 0 to 1500 bytes */
  CS_SON_TRP_MAX_TIMEOUT           = 0x02,  /* 0 to 65535, in second, 0 means infinite  */
  CS_SON_TRP_CONNECT_SETUP_TIMEOUT = 0x04,  /* 10 to 1200, in 100 of ms, 0 means infnite */
  CS_SON_TRP_TRANSFER_TIMEOUT      = 0x08,  /* 1 to 255, in ms, 0 means infinite */
  CS_SON_TRP_CONNECT_MODE          = 0x10,  /**/
  CS_SON_TRP_SUSPEND_TIMEOUT       = 0x20,  /* 0 to 2000, in ms , 0 means infinite */
  CS_SON_TRP_RX_TIMEOUT            = 0x40,  /* 0 to 255, in ms, 0 means infinite */
} CS_SocketOptionName_t;
typedef enum
{
  CS_TCP_PROTOCOL = 0,
  CS_UDP_PROTOCOL = 1,
} CS_TransportProtocol_t;
typedef struct
{
  /* CS_SocketState_t    state; */ /* to add ? */
  uint8_t           loc_ip_addr_value[MAX_IP_ADDR_SIZE];
  uint16_t            loc_port;
  uint8_t           rem_ip_addr_value[MAX_IP_ADDR_SIZE];
  uint16_t            rem_port;
} CS_SocketCnxInfos_t;


typedef struct
{
	uint8_t       conf_id;
	uint8_t 	  PDP_type[MAX_APN_SIZE];
	uint8_t       apn[MAX_APN_SIZE];
  //CS_PDN_configuration_t  pdn_conf;
} csint_pdn_infos_t;

typedef struct
{
  const uint8_t  *p_buffer_addr_send; /* send buffer (const) */
  uint8_t        *p_buffer_addr_rcv;  /* receive buffer */
  uint32_t         buffer_size;         /* real buffer size */
} csint_socket_data_buffer_t;

typedef struct
{
   
  /* socket states */
  csint_Socket_State_t state; /* socket state */

  CS_SocketOptionName_t config;

  /* parameters set during socket creation */
  //CS_IPaddrType_t             addr_type;   /* local IP address */
  CS_TransportProtocol_t      protocol;
  uint16_t                    local_port;
 // CS_PDN_conf_id_t            conf_id;     /* PDP context identifier used for this transfer */

  /* parameters set during socket connect */
//  CS_IPaddrType_t     ip_addr_type;
  uint8_t           ip_addr_value[MAX_IP_ADDR_SIZE]; /* remote IP address */
  uint8_t           remote_ip_addr_value[MAX_IP_ADDR_SIZE]; /* remote IP address */

  uint16_t            remote_port;

  /* parameters set during socket configuration */
  uint16_t ip_max_packet_size; /* 0 to 1500 bytes, 0 means default, default = DEFAULT_IP_MAX_PACKET_SIZE */
  uint16_t trp_max_timeout; /* 0 to 65535 seconds, 0 means infinite, default = DEFAULT_TRP_MAX_TIMEOUT */
  uint16_t trp_conn_setup_timeout; /* 10 to 1200 hundreds of ms, 0 means infinite, default = DEFAULT_TRP_CONN_SETUP_TIMEOUT */
  uint8_t  trp_transfer_timeout; /* 0 to 255 ms, 0 means infinite, default = DEFAULT_TRP_TRANSFER_TIMEOUT */
  //CS_ConnectionMode_t trp_connect_mode;
  uint16_t trp_suspend_timeout; /* 0 to 2000 ms , 0 means infinite, default = DEFAULT_TRP_SUSPEND_TIMEOUT */
  uint8_t  trp_rx_timeout; /* 0 to 255 ms, 0 means infinite, default = DEFAULT_TRP_RX_TIMEOUT */

  /* socket infos callbacks */
  void*   socket_data_ready_callback;
  void*   socket_data_sent_callback;
  void*   socket_remote_close_callback;

} csint_socket_infos_t;
typedef enum
{
  CS_RESET_SW             = 0,
  CS_RESET_HW             = 1,
  CS_RESET_AUTO           = 2,
  CS_RESET_FACTORY_RESET  = 3,
} CS_Reset_t;

typedef enum
{
  CS_NRM_AUTO = 0,
  CS_NRM_MANUAL = 1,
  CS_NRM_DEREGISTER = 2,
  CS_NRM_MANUAL_THEN_AUTO = 4,
} CS_NetworkRegMode_t;
typedef enum
{
  CS_RSF_NONE                    = 0x00,
  CS_RSF_FORMAT_PRESENT          = 0x01,
  CS_RSF_OPERATOR_NAME_PRESENT   = 0x02,
  CS_RSF_ACT_PRESENT             = 0x04,
} CS_RegistrationStatusFields_t;
typedef enum
{
  CS_ONF_LONG = 0,    /* up to 16 chars */
  CS_ONF_SHORT = 1,   /* up to 8 chars */
  CS_ONF_NUMERIC = 2, /* LAI */
  CS_ONF_NOT_PRESENT = 9, /* Operator Name not present */
} CS_OperatorNameFormat_t;
typedef uint8_t CS_CHAR_t;
typedef enum
{
  CS_ACT_GSM               = 0,
  CS_ACT_GSM_COMPACT       = 1,
  CS_ACT_UTRAN             = 2,
  CS_ACT_GSM_EDGE          = 3,
  CS_ACT_UTRAN_HSDPA       = 4,
  CS_ACT_UTRAN_HSUPA       = 5,
  CS_ACT_UTRAN_HSDPA_HSUPA = 6,
  CS_ACT_E_UTRAN           = 7,
  CS_ACT_EC_GSM_IOT        = 8, /* = LTE Cat.M1 */
  CS_ACT_E_UTRAN_NBS1      = 9, /* = LTE Cat.NB1 */
} CS_AccessTechno_t;

 typedef struct
{
  CS_NetworkRegMode_t               mode;                                    /* mandatory field */
  CS_NetworkRegState_t              EPS_NetworkRegState;                     /* mandatory field */
  CS_NetworkRegState_t              GPRS_NetworkRegState;                    /* mandatory field */
  CS_NetworkRegState_t              CS_NetworkRegState;                      /* mandatory field */

  CS_RegistrationStatusFields_t     optional_fields_presence;                /* indicates which fields below are valid */
  CS_OperatorNameFormat_t           format;                                  /* optional field */
  CS_CHAR_t                         operator_name[MAX_SIZE_OPERATOR_NAME];   /* optional field */
  CS_AccessTechno_t                 AcT;                                     /* optional field */

} CS_RegistrationStatus_t;
typedef enum
{
  CS_SOL_IP        = 0,
  CS_SOL_TRANSPORT = 1,
} CS_SocketOptionLevel_t;
typedef struct
{
  uint16_t field_requested; /* device info field request below */
  union
  {
    CS_CHAR_t imei[MAX_SIZE_IMEI];
    CS_CHAR_t manufacturer_name[MAX_SIZE_MANUFACT_NAME];
    CS_CHAR_t model[MAX_SIZE_MODEL];
    CS_CHAR_t revision[MAX_SIZE_REV];
    CS_CHAR_t serial_number[MAX_SIZE_SN];
    CS_CHAR_t imsi[MAX_SIZE_IMSI];
    CS_CHAR_t phone_number[MAX_SIZE_PHONE_NBR];
  } u;

} CS_DeviceInfo_t; 
typedef struct
{
  CS_NetworkRegMode_t      mode;
  CS_OperatorNameFormat_t  format;
  CS_CHAR_t                operator_name[MAX_SIZE_OPERATOR_NAME];

} CS_OperatorSelector_t;
typedef enum
{
  CS_PS_DETACHED = 0,
  CS_PS_ATTACHED = 1,
} CS_PSattach_t;

typedef enum
{
  CS_MDMEVENT_NONE          = 0x0000, /* none */
  CS_MDMEVENT_BOOT          = 0x0001, /* Modem Boot indication received (if exists) */
  CS_MDMEVENT_POWER_DOWN    = 0x0002, /* Modem Power Down indication received (if exists) */
  CS_MDMEVENT_FOTA_START    = 0x0004, /* Modem FOTA Start indication received (if exists) */
  CS_MDMEVENT_FOTA_END      = 0x0008, /* Modem FOTA End indication received (if exists) */
} CS_ModemEvent_t;
typedef enum
{
  CS_URCEVENT_NONE                  = 0, /* none */
  CS_URCEVENT_EPS_NETWORK_REG_STAT  = 1, /* EPS registration status (CEREG) */
  CS_URCEVENT_EPS_LOCATION_INFO     = 2, /* EPS location status (CEREG) */
  CS_URCEVENT_GPRS_NETWORK_REG_STAT = 3, /* GPRS registration status (CGREG) */
  CS_URCEVENT_GPRS_LOCATION_INFO    = 4, /* GPRS registration status (CGREG) */
  CS_URCEVENT_CS_NETWORK_REG_STAT   = 5, /* Circuit-switched registration status (CREG) */
  CS_URCEVENT_CS_LOCATION_INFO      = 6, /* Circuit-switched status (CREG) */
  CS_URCEVENT_SIGNAL_QUALITY        = 7, /* signal quality registration (if supported) */
  CS_URCEVENT_PING_RSP              = 8, /* Ping response */
} CS_UrcEvent_t;
/* callback */
typedef void (* cellular_event_callback_t)(uint32_t event_callback);
typedef void (* cellular_urc_callback_t)(void);
typedef void (* cellular_modem_event_callback_t)(CS_ModemEvent_t modem_event_received);
typedef void (* cellular_pdn_event_callback_t)( CS_PDN_event_t pdn_event);
typedef void (* cellular_socket_data_ready_callback_t)(socket_handle_t sockHandle);
typedef void (* cellular_socket_data_sent_callback_t)(socket_handle_t sockHandle);
typedef void (* cellular_socket_closed_callback_t)(socket_handle_t sockHandle);
//typedef void (* cellular_ping_response_callback_t)(CS_Ping_response_t ping_response);

  void socket_init(void);
CS_Status_t CS_power_on(void);
CS_Status_t CS_power_off(void);
CS_Status_t CS_reset(CS_Reset_t rst_type); 
CS_Status_t CS_check_connection(void);
CS_Status_t CS_get_signal_quality(CS_SignalQuality_t *p_sig_qual);
CS_Status_t CS_register_net(void);
CS_Status_t CS_init_modem(void);
CS_Status_t CDS_socket_open( uint8_t force);
CS_Status_t CDS_socket_close( uint8_t force);
CS_Status_t CDS_tcp_close( uint8_t force);
CS_Status_t CDS_socket_set_option(void);
CS_Status_t CDS_socket_connect(   uint8_t *p_ip_addr_value, uint16_t remote_port);
CS_Status_t CDS_socket_listen(void);
CS_Status_t CS_define_pdn(uint8_t cid, const uint8_t *apn,const uint8_t *pdptype);
CS_Status_t CS_get_dev_IP_address(uint8_t cid,  uint8_t *p_ip_addr_value);
CS_Status_t CDS_ipmode_set(uint8_t par);
CS_Status_t CDS_socket_send( const uint8_t *p_buf, uint32_t length);
int32_t CDS_socket_receive(uint8_t *socket_rec_buf,uint16_t length);
CS_Status_t CDS_socket_cnx_status( CS_SocketCnxInfos_t *p_infos);
CS_Status_t CS_get_net_status(CS_RegistrationStatus_t *p_reg_status);
CS_Status_t CS_get_device_info(CS_DeviceInfo_t *p_devinfo);
CS_Status_t CS_get_attach_status(CS_PSattach_t *p_attach);
CS_Status_t CS_activate_pdn(uint8_t cid);
CS_Status_t  CS_register_pdn_event(uint8_t cid, cellular_pdn_event_callback_t pdn_event_callback); 
#endif