/**
  ******************************************************************************
  * @file    httpclient.c
  * @author  MCD Application Team
  * @brief   Example of http client to send infos to grovestreams cloud
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
#include "httpclient.h"
#include "plf_config.h"
#include "httpclient_config.h"

#include <stdio.h>
#include <string.h>

#include <cmsis_os.h>

#include "com_sockets.h"
#include "bsp_led.h"
#include "dc_common.h"
//#include "dc_mems.h"
#include "dc_control.h"
//#include "dc_emul.h"
#include "cellular_init.h"
#include "error_handler.h"
#include "setup.h"
#include "app_select.h"
#include "menu_utils.h"
#include "time_date.h"

/* Private defines -----------------------------------------------------------*/
#if (USE_TRACE_HTTP_CLIENT == 1U)

#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PrintINFO(format, args...) \
TracePrint(DBG_CHAN_HTTP, DBL_LVL_P0, "HTTP: " format "\n\r", ## args)
#define PrintINFOMenu(format, args...) \
TracePrint(DBG_CHAN_HTTP, DBL_LVL_P0, format "\n\r", ## args)
#define PrintDBG(format, args...) \
TracePrint(DBG_CHAN_HTTP, DBL_LVL_P1, "HTTP: " format "\n\r", ## args)
#define PrintERR(format, args...) \
TracePrint(DBG_CHAN_HTTP, DBL_LVL_ERR, "HTTP ERROR: " format "\n\r", ## args)

#else /* USE_PRINTF == 1 */
#define PrintINFO(format, args...)      printf("HTTP: " format "\n\r", ## args);
#define PrintINFOMenu(format, args...)  printf(format "\n\r", ## args);
#define PrintDBG(format, args...)       printf("HTTP: " format "\n\r", ## args);
#define PrintERR(format, args...)       printf("HTTP ERROR: " format "\n\r", ## args);

#endif /* USE_PRINTF */

#else /* USE_TRACE_HTTP_CLIENT == 0 */
#define PrintINFO(format, args...)      do {} while(0);
#define PrintINFOMenu(format, args...)  do {} while(0);
#define PrintDBG(format, args...)       do {} while(0);
#define PrintERR(format, args...)       do {} while(0);

#endif /* USE_TRACE_HTTP_CLIENT */


#define HTTP_CLIENT_SETUP_VERSION     (uint16_t)1

#define HTTPCLIENT_DEFAULT_LOCALPORT   ((uint16_t)0U)

#define DNS_SERVER_NAME        ((uint8_t*)"gandi.net")
#define DNS_SERVER_IP          ((uint32_t)0x41B946D9U)   /* 217.70.185.65 */
#define DNS_SERVER_PORT        ((uint16_t)443U)

#define CLOUD_SERVER_NAME      ((uint8_t*)"liveobjects.orange-business.com")
#define CLOUD_SERVER_IP        ((uint32_t)0xD02A2754U)   /*  84.39.42.208 */
#define CLOUD_SERVER_PORT      ((uint16_t)8883U)

#define ST_UNKNOWN_NAME        ((uint8_t*)"stunknown.st")
#define ST_UNKNOWN_IP          ((uint32_t)0x9B36379AU)   /* 154.55.54.155 */
#define ST_UNKNOWN_PORT        ((uint16_t)1234U)

#define NAME_SIZE_MAX            64U /* MAX_SIZE_IPADDR of cellular_service.h */

#define HTTPCLIENT_LABEL  ((uint8_t*)"Grovestreams")

#define PUT_CHANNEL_ID_MAX  ((uint8_t)16U)
#define GET_CHANNEL_ID_MAX  ((uint8_t)2U)

#define HTTPCLIENT_DEFAULT_PARAM_NB 20U

#define BATLEVEL_CHANNEL_ID         0U
#define SIGLEVEL_CHANNEL_ID         1U
#define HRM_HEART_RATE_CHANNEL_ID   2U
#define PEDOMETER_CHANNEL_ID        3U
#define HUM_CHANNEL_ID              4U
#define TEMP_CHANNEL_ID             5U
#define PRESS_CHANNEL_ID            6U
#define ACCX_CHANNEL_ID             7U
#define ACCY_CHANNEL_ID             8U
#define ACCZ_CHANNEL_ID             9U
#define GYRX_CHANNEL_ID            10U
#define GYRY_CHANNEL_ID            11U
#define GYRZ_CHANNEL_ID            12U
#define MAGX_CHANNEL_ID            13U
#define MAGY_CHANNEL_ID            14U
#define MAGZ_CHANNEL_ID            15U

#define HTTPCLIENT_SEND_PERIOD        1000U
#define CLIENT_MESSAGE_SIZE           1500U
#define KEY_GET_INPUT_SIZE              50U
#define KEY_PUT_INPUT_SIZE              KEY_GET_INPUT_SIZE
#define CHANNEL_ID_SIZE_MAX             15U
#define COMPONENT_ID_STRING_SIZE_MAX    15U
#define SELECT_INPUT_STRING_SIZE         5U

#define PUT_REQUEST_PERIOD_SIZE_MAX      5U
#define GET_REQUEST_PERIOD_SIZE_MAX      5U

#define PUT_DEFAULT_CHANNEL_ID_NUMBER    4U
#define GET_DEFAULT_CHANNEL_ID_NUMBER    2U

#define RCV_SND_TIMEOUT              10000U /* Timeout to send/receive data */

/* NFM implementation : nb of consecutive errors before to start NFM timer */
#define NFM_ERROR_LIMIT_SHORT_MAX        5U

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  HTTPCLIENT_FALSE = 0,
  HTTPCLIENT_TRUE  = 1
} httpclient_bool_t;

typedef enum
{
  SOCKET_INVALID = 0,
  SOCKET_CREATED,
  SOCKET_CONNECTED,
  SOCKET_SENDING,
  SOCKET_WAITING_RSP,
  SOCKET_CLOSING
} socket_state_t;

typedef struct
{
  com_char_t *hostname;
  com_ip_addr_t hostip;
  uint16_t      hostport;
} dns_resolver_t;

/* Private macro -------------------------------------------------------------*/
#define HTTPCLIENT_MIN(a,b) ((a)<(b) ? (a) : (b))

/* Private variables ---------------------------------------------------------*/
static osMessageQId http_client_queue;
static osThreadId httpClientTaskHandle;

static com_ip_addr_t httpclient_distantip;
static uint16_t      httpclient_distantport;
static com_char_t    httpclient_distantname[NAME_SIZE_MAX];

static httpclient_bool_t socket_closing;
static socket_state_t socket_state;
static int32_t socket_http_client;

/* input strings from console */
static uint8_t key_get_input[KEY_GET_INPUT_SIZE];
static uint8_t key_put_input[KEY_PUT_INPUT_SIZE];
static uint8_t component_id_string[COMPONENT_ID_STRING_SIZE_MAX];
static uint8_t put_channel_id_string_tab[PUT_CHANNEL_ID_MAX][CHANNEL_ID_SIZE_MAX];
static uint8_t get_channel_id_string_tab[GET_CHANNEL_ID_MAX][CHANNEL_ID_SIZE_MAX];

static com_char_t httpclient_tmp_buffer[CLIENT_MESSAGE_SIZE];

/* FORMATTED http GET or PUT request  */
static com_char_t formatted_http_request[400U];
static com_char_t formatted_channel_http_request[200U];

/* partial http PUT and GET request definition */
static com_char_t *header1a_put  = (com_char_t *)"PUT /api/feed?compId=";
static com_char_t *header1b_put  = (com_char_t *)" HTTP/1.1\r\n";
static com_char_t *header1_get_a = (com_char_t *)"GET /api/comp/";
static com_char_t *header1_get_b = (com_char_t *)"/stream/";
static com_char_t *header1_get_c = (com_char_t *)"/last_value HTTP/1.1\r\n";
static com_char_t *header2       = (com_char_t *)"Host:www.grovestreams.com\r\n";
static com_char_t *header3       = (com_char_t *)"Content-type:application/json\r\n";
static com_char_t *header4       = (com_char_t *)"Connection:Keep-Alive\r\nKeep-Alive:timeout=15\r\nContent-Type:application/json\r\nContent-Length:0\r\n";
/*
static com_char_t *header4       = "Connection:close\r\nContent-Type:application/json\r\nContent-Length:0\r\n";
*/
static com_char_t *header5       = (com_char_t *)"Cookie:api_key=";
static com_char_t *header_end    = (com_char_t *)"\r\n";

#if (USE_DEFAULT_SETUP == 0)
static uint8_t put_request_period_string[PUT_REQUEST_PERIOD_SIZE_MAX];
static uint8_t get_request_period_string[GET_REQUEST_PERIOD_SIZE_MAX];
static uint8_t select_input_string[SELECT_INPUT_STRING_SIZE];
static com_char_t distanthost_string[NAME_SIZE_MAX];
/* list of available types of PUT and GET channels */
/*
static com_char_t *put_channel_id_name_tab[PUT_CHANNEL_ID_MAX] = {
     "battery level","signal level","hrm heart rate","pedometer"
};
*/
static com_char_t *put_channel_id_name_tab[PUT_CHANNEL_ID_MAX] =
{
  (com_char_t *)"battery level", (com_char_t *)"signal level", (com_char_t *)"hrm heart rate", (com_char_t *)"pedometer",
  (com_char_t *)"humidity", (com_char_t *)"temperature", (com_char_t *)"pressure",
  (com_char_t *)"acceleration x", (com_char_t *)"acceleration y", (com_char_t *)"acceleration z",
  (com_char_t *)"angular velocity x", (com_char_t *)"angular velocity y", (com_char_t *)"angular velocity z",
  (com_char_t *)"magnetic flux x", (com_char_t *)"magnetic flux y", (com_char_t *)"magnetic flux z"
};

static com_char_t *get_channel_id_name_tab[GET_CHANNEL_ID_MAX] =
{
  (com_char_t *)"led state", (com_char_t *)"led frequency"
};
#endif /* USE_DEFAULT_SETUP */

static uint8_t http_client_set_date_flag;

static uint32_t http_client_put_period;
static uint32_t http_client_get_period;

/* static com_char_t *buf_led_on ="data":true"; */
/* static com_char_t *buf_led_off ="data":false"; */
static com_char_t buf_led_on[]  = {0x64, 0x61, 0x74, 0x61, 0x22, 0x3a, 0x74, 0x72, 0x75, 0x65, 0x00};
static com_char_t buf_led_off[] = {0x64, 0x61, 0x74, 0x61, 0x22, 0x3a, 0x66, 0x61, 0x6c, 0x73, 0x65, 0x00};

/* NFM implementation */
/* limit nb of errors before to activate nfm */
static uint8_t nfm_nb_error_limit_short;
/* current nb of errors */
static uint8_t nfm_nb_error_short;
/* sleep timer index in the NFM array */
static uint8_t nfm_sleep_timer_index;

static uint8_t *httpclient_default_setup_table[HTTPCLIENT_DEFAULT_PARAM_NB] =
{
  HTTPCLIENT_DEFAULT_DISTANTNAME,
  HTTPCLIENT_DEFAULT_CLOUD_KEY_GET,
  HTTPCLIENT_DEFAULT_CLOUD_KEY_PUT,
  HTTPCLIENT_DEFAULT_COMPONENT_ID,
  HTTPCLIENT_DEFAULT_PUT_REQUEST_PERIOD,
  HTTPCLIENT_DEFAULT_SENSOR1_TYPE,
  HTTPCLIENT_DEFAULT_SENSOR1_ID,
  HTTPCLIENT_DEFAULT_SENSOR2_TYPE,
  HTTPCLIENT_DEFAULT_SENSOR2_ID,
  HTTPCLIENT_DEFAULT_SENSOR3_TYPE,
  HTTPCLIENT_DEFAULT_SENSOR3_ID,
  HTTPCLIENT_DEFAULT_SENSOR4_TYPE,
  HTTPCLIENT_DEFAULT_SENSOR4_ID,
  HTTPCLIENT_DEFAULT_SENSOR5_TYPE,
  HTTPCLIENT_DEFAULT_SENSOR5_ID,
  HTTPCLIENT_DEFAULT_SENSOR_END,
  HTTPCLIENT_DEFAULT_GET_REQUEST_PERIOD,
  HTTPCLIENT_DEFAULT_GET_TYPE,
  HTTPCLIENT_DEFAULT_GET_ID,
  HTTPCLIENT_DEFAULT_GET_END
};

static const dns_resolver_t dns_resolver[] =
{
#if (HTTPCLIENT_CONFIG_UNITARY_TEST == 1)
  { HTTP_SERVER_TEST_NAME, {HTTP_SERVER_TEST_IP},  HTTP_SERVER_TEST_PORT },
#endif
  { DNS_SERVER_NAME,       {DNS_SERVER_IP},        DNS_SERVER_PORT },
  { CLOUD_SERVER_NAME,     {CLOUD_SERVER_IP},      CLOUD_SERVER_PORT },
  { ST_UNKNOWN_NAME,       {ST_UNKNOWN_IP},        ST_UNKNOWN_PORT }
};

/* State of HTTP_CLIENT process */
static httpclient_bool_t http_client_process_flag; /* false: inactive,
                                                      true: active
                                                      default value : true */

/* State of Network connection */
static httpclient_bool_t network_is_on;

/* Global variables ----------------------------------------------------------*/
/* cellular signal quality */
extern uint8_t CST_signal_quality;

/* Private function prototypes -----------------------------------------------*/
/* Callback */
static void http_client_notif_cb(dc_com_event_id_t dc_event_id,
                                 void *private_gui_data);

static httpclient_bool_t http_dns_resolver(uint8_t  *hoststring,
                                           uint8_t  *hostname,
                                           uint16_t *hostport,
                                           com_ip_addr_t *hostip);

static httpclient_bool_t is_nfm_sleep_requested(void);
static void http_put_request_format(void);
static void http_client_check_distantname(void);
static void http_client_create_socket(void);
static httpclient_bool_t http_client_process(com_char_t *send,
                                             com_char_t *response);
static void http_client_get_process(void);
static void http_client_put_process(void);
static uint32_t http_client_get_nfmc(uint8_t index);
static void http_client_socket_thread(void const *argument);

#if (USE_DEFAULT_SETUP == 1)
static void http_client_config_handler(void);
#else
static uint32_t http_client_sensor_list_config(com_char_t **channelIdName,
                                               uint8_t (*channelIdStringInput)[CHANNEL_ID_SIZE_MAX],
                                               uint8_t channel_id_max,
                                               uint8_t channel_id_size_max);
static void http_client_setup_handler(void);
static void http_client_setup_dump(void);
#endif

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Callback called when a value in datacache changed
  * @note   Managed datacache value changed
  * @param  dc_event_id - value changed
  * @note   -
  * @param  private_gui_data - value provided at service subscription
  * @note   Unused
  * @retval None
  */
static void http_client_notif_cb(dc_com_event_id_t dc_event_id,
                                 void *private_gui_data)
{
  /* UNUSED(private_gui_data); */

  if (dc_event_id == DC_COM_NIFMAN_INFO)
  {
    dc_nifman_info_t  dc_nifman_info;
    dc_com_read(&dc_com_db, DC_COM_NIFMAN_INFO,
                (void *)&dc_nifman_info, sizeof(dc_nifman_info));
    if (dc_nifman_info.rt_state == DC_SERVICE_ON)
    {
      network_is_on = HTTPCLIENT_TRUE;
      osMessagePut(http_client_queue, (uint32_t)dc_event_id, 0U);
    }
    else
    {
      network_is_on = HTTPCLIENT_FALSE;
    }
  }
  else if (dc_event_id == DC_COM_BUTTON_UP)
  {
    if (http_client_process_flag == HTTPCLIENT_FALSE)
    {
      http_client_process_flag = HTTPCLIENT_TRUE;
      PrintINFOMenu("\n\r <<< HTTP CLIENT ACTIVE >>>")
    }
    else
    {
      http_client_process_flag = HTTPCLIENT_FALSE;
      PrintINFOMenu("\n\r <<< HTTP CLIENT NOT ACTIVE >>>")
    }
  }

  else
  {
  }
}

/**
  * @brief  Internal DNS resolver
  * @note   Used to separate remote host information (URL from port)
            and to complete information (remote port) if not provided
  * @param  hoststring - string containing url and port
  * @note   -
  * @param  hostname - string that will contain host name
  * @note   only set
  * @param  hostport - string that will contain host port
  * @note   -
  * @param  hostip - string that will contain host IP
  * @note   -
  * @retval bool - false : semantic is NOK
                   true  : semantic is OK
  */
static httpclient_bool_t http_dns_resolver(uint8_t  *hoststring,
                                           uint8_t  *hostname,
                                           uint16_t *hostport,
                                           com_ip_addr_t *hostip)
{
  httpclient_bool_t result;
  uint8_t  i, begin;
  uint8_t  dns_resolver_size;
  uint8_t  ip_string[NAME_SIZE_MAX];
  int32_t  count;
  uint32_t ip_port;

  i = 0U;
  dns_resolver_size = sizeof(dns_resolver) / sizeof(dns_resolver_t);
  result = HTTPCLIENT_FALSE;

  /* Separate the NAME from the Port */
  count = sscanf((char *)hoststring, "%s %5lu", (char *)&ip_string[0],
                 &ip_port);
  if (count >= 1)
  {
    /* Check NAME known ? */
    if (strncmp((const char *)hoststring, "www.", 4U)
        == 0)
    {
      begin = 4U;
    }
    else
    {
      begin = 0U;
    }

    while ((i < dns_resolver_size)
           && (result == HTTPCLIENT_FALSE))
    {
      if (strlen((const char *)&ip_string[begin])
          == strlen((char *)&dns_resolver[i].hostname[0]))
      {
        if (strncmp((const char *)&ip_string[begin],
                    (const char *)&dns_resolver[i].hostname[0],
                    strlen((char *)&dns_resolver[i].hostname[0]))
            == 0)
        {
          /* If port is set, it overwrites the preconfigured value */
          if ((count == 2)
              && (ip_port < 65535U))
          {
            *hostport = (uint16_t)ip_port;
          }
          else
          {
            *hostport = (uint16_t)dns_resolver[i].hostport;
          }
          hostip->addr = dns_resolver[i].hostip.addr;
          /* NAME without port is copy in hostname */
          /* Sure that length is ok for next copy */
          strcpy((char *)hostname,
                 (const char *)ip_string);
          result = HTTPCLIENT_TRUE;
        }
        else
        {
          i++;
        }
      }
      else
      {
        i++;
      }
    }
    /* Internal DNS resolver didn't find the NAME */
    if (result == HTTPCLIENT_FALSE)
    {
      /* Copy NAME without port in hostname */
      memcpy(hostname,
             ip_string,
             HTTPCLIENT_MIN(strlen((char *)&ip_string[0]),
                            NAME_SIZE_MAX));
      if ((count == 2)
          && (ip_port < 65535U))
      {
        *hostport = (uint16_t)ip_port;
      }
      else
      {
        /* Put the default distant port value */
        *hostport = (uint16_t)HTTPCLIENT_DEFAULT_DISTANTPORT;
      }
      hostip->addr = 0U;
      result  = HTTPCLIENT_TRUE;
    }
  }

  return result;
}

/**
  * @brief  Check if NFM sleep has to be done
  * @note   -
  * @param  None
  * @retval bool - false/true NFM sleep hasn't to be done/has to be done
  */
static httpclient_bool_t is_nfm_sleep_requested(void)
{
  httpclient_bool_t result;

  if (nfm_nb_error_short >= nfm_nb_error_limit_short)
  {
    result = HTTPCLIENT_TRUE;
  }
  else
  {
    result = HTTPCLIENT_FALSE;
  }

  return result;
}

/**
  * @brief  PUT request format
  * @note   Format the PUT message
  * @param  None
  * @retval None
  */
static void http_put_request_format(void)
{
  uint8_t i;
	#if 0
  dc_cellular_info_t      cellular_info;
  dc_battery_info_t       battery_info;
  dc_hrm_info_t           hrm_info;
  dc_pedometer_info_t     pedometer_info;
  dc_pressure_info_t      pressure_info;
  dc_humidity_info_t      humidity_info;
  dc_temperature_info_t   temperature_info;
  dc_accelerometer_info_t accelerometer_info;
  dc_gyroscope_info_t     gyroscope_info;
  dc_magnetometer_info_t  magnetometer_info;

  formatted_channel_http_request[0] = 0U;

  for (i = 0U; i < PUT_CHANNEL_ID_MAX; i++)
  {
    if (put_channel_id_string_tab[i][0])
    {
      PrintDBG("Channel %d %s", i, put_channel_id_string_tab[i])
      switch (i)
      {
        case SIGLEVEL_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_CELLULAR,
                      (void *)&cellular_info,
                      sizeof(cellular_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%d",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  CST_signal_quality);
          break;
        }
        case BATLEVEL_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_BATTERY,
                      (void *)&battery_info,
                      sizeof(battery_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%d",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  battery_info.current_battery_percentage);
          break;
        }
        case HRM_HEART_RATE_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_HRM,
                      (void *)&hrm_info,
                      sizeof(hrm_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%d",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  hrm_info.current_heart_rate);
          break;
        }
        case PEDOMETER_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_PEDOMETER,
                      (void *)&pedometer_info,
                      sizeof(pedometer_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%ld",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  pedometer_info.step_counter);
          break;
        }
        case ACCX_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_ACCELEROMETER,
                      (void *)&accelerometer_info,
                      sizeof(accelerometer_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%ld",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  accelerometer_info.accelerometer.AXIS_X);
          break;
        }
        case ACCY_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_ACCELEROMETER,
                      (void *)&accelerometer_info,
                      sizeof(accelerometer_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%ld",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  accelerometer_info.accelerometer.AXIS_Y);
          break;
        }
        case ACCZ_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_ACCELEROMETER,
                      (void *)&accelerometer_info,
                      sizeof(accelerometer_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%ld",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  accelerometer_info.accelerometer.AXIS_Z);
          break;
        }
        case GYRX_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_GYROSCOPE,
                      (void *)&gyroscope_info,
                      sizeof(gyroscope_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%ld",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  gyroscope_info.gyroscope.AXIS_X);
          break;
        }
        case GYRY_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_GYROSCOPE,
                      (void *)&gyroscope_info,
                      sizeof(gyroscope_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%ld",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  gyroscope_info.gyroscope.AXIS_Y);
          break;
        }
        case GYRZ_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_GYROSCOPE,
                      (void *)&gyroscope_info,
                      sizeof(gyroscope_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%ld",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  gyroscope_info.gyroscope.AXIS_Z);
          break;
        }
        case MAGX_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_MAGNETOMETER,
                      (void *)&magnetometer_info,
                      sizeof(magnetometer_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%ld",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  magnetometer_info.magnetometer.AXIS_X);
          break;
        }
        case MAGY_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_MAGNETOMETER,
                      (void *)&magnetometer_info,
                      sizeof(magnetometer_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%ld",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  magnetometer_info.magnetometer.AXIS_Y);
          break;
        }
        case MAGZ_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_MAGNETOMETER,
                      (void *)&magnetometer_info,
                      sizeof(magnetometer_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%ld",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  magnetometer_info.magnetometer.AXIS_Z);
          break;
        }
        case HUM_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_HUMIDITY,
                      (void *)&humidity_info,
                      sizeof(humidity_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%f",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  humidity_info.humidity);
          break;
        }
        case TEMP_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_TEMPERATURE,
                      (void *)&temperature_info,
                      sizeof(temperature_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%f",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  temperature_info.temperature);
          break;
        }
        case PRESS_CHANNEL_ID:
        {
          dc_com_read(&dc_com_db, DC_COM_PRESSURE,
                      (void *)&pressure_info,
                      sizeof(pressure_info));
          sprintf((char *)formatted_channel_http_request, "%s&%s=%f",
                  formatted_channel_http_request,
                  put_channel_id_string_tab[i],
                  pressure_info.pressure);
          break;
        }
        default:
        {
          break;
        }
      }
    }
	
    PrintDBG("formatted_channel_http_request %s",
             formatted_channel_http_request)
  }
  #endif
}

/**
  * @brief  Check Distant name
  * @note   Decide if Network DNS resolver has to be called
  * @param  None
  * @retval None
  */
static void http_client_check_distantname(void)
{
  /* If distantname is provided and distantip is unknown
     call DNS network resolution service */
  if ((strlen((const char *)httpclient_distantname) > 0U)
      && (httpclient_distantip.addr == 0U))
  {
    com_sockaddr_t httpclient_distantaddr;

    /* DNS network resolution request */
    PrintINFO("Distant Name provided %s. DNS resolution started", httpclient_distantname)
    if (com_gethostbyname(httpclient_distantname,
                          &httpclient_distantaddr)
        == 0)
    {
      httpclient_distantip.addr = ((com_sockaddr_in_t *)&httpclient_distantaddr)->sin_addr.s_addr;
      PrintINFO("DNS resolution OK - IP: %d.%d.%d.%d",
                (uint8_t)(httpclient_distantip.addr),
                (uint8_t)(httpclient_distantip.addr >> 8),
                (uint8_t)(httpclient_distantip.addr >> 16),
                (uint8_t)(httpclient_distantip.addr >> 24))
      /* No reset of nb_error_short wait to see if distant can be reached */
    }
    else
    {
      nfm_nb_error_short++;
      PrintERR("DNS resolution NOK. Will retry later")
    }
  }
}

/**
  * @brief  Create socket
  * @note   If requested close and create socket
  * @param  None
  * @retval None
  */
static void http_client_create_socket(void)
{
  com_sockaddr_in_t address;
  uint32_t timeout;

  if ((socket_closing == HTTPCLIENT_TRUE)
      || (socket_state == SOCKET_CLOSING))
  {
    PrintINFO("socket in closing mode request the close")
    if (com_closesocket(socket_http_client) != COM_SOCKETS_ERR_OK)
    {
      socket_state = SOCKET_CLOSING;
      PrintERR("socket close NOK")
    }
    else
    {
      socket_state = SOCKET_INVALID;
      PrintINFO("socket close OK")
    }
    socket_closing = HTTPCLIENT_FALSE;
  }

  if (socket_state == SOCKET_INVALID)
  {
    /* Create a TCP socket */
    PrintINFO("socket creation in progress")
    socket_http_client = com_socket(COM_AF_INET,
                                    COM_SOCK_STREAM,
                                    COM_IPPROTO_TCP);

    if (socket_http_client >= 0)
    {
      address.sin_family      = (uint8_t)COM_AF_INET;
      address.sin_port        = com_htons(HTTPCLIENT_DEFAULT_LOCALPORT);
      address.sin_addr.s_addr = COM_INADDR_ANY;
      PrintINFO("socket bind in progress")
      if (com_bind(socket_http_client,
                   (com_sockaddr_t *)&address,
                   (int32_t)sizeof(com_sockaddr_in_t))
          == COM_SOCKETS_ERR_OK)
      {
        timeout = RCV_SND_TIMEOUT;

        PrintINFO("socket setsockopt in progress")
        if (com_setsockopt(socket_http_client,
                           COM_SOL_SOCKET, COM_SO_RCVTIMEO,
                           &timeout,
                           (int32_t)sizeof(timeout))
            == COM_SOCKETS_ERR_OK)
        {
          if (com_setsockopt(socket_http_client,
                             COM_SOL_SOCKET, COM_SO_SNDTIMEO,
                             &timeout,
                             (int32_t)sizeof(timeout))
              == COM_SOCKETS_ERR_OK)
          {
            socket_state = SOCKET_CREATED;
            PrintINFO("socket create OK")
          }
          else
          {
            PrintERR("socket setsockopt SNDTIMEO NOK")
          }
        }
        else
        {
          PrintERR("socket setsockopt RCVTIMEO NOK")
        }
      }
      else
      {
        PrintERR("socket bind NOK")
      }

      if (socket_state != SOCKET_CREATED)
      {
        PrintERR("socket bind or setsockopt NOK - close the socket")
        if (com_closesocket(socket_http_client)
            != COM_SOCKETS_ERR_OK)
        {
          socket_state = SOCKET_CLOSING;
          PrintERR("socket close NOK")
        }
        else
        {
          socket_state = SOCKET_INVALID;
          PrintINFO("socket close OK")
        }
      }
    }
    else
    {
      PrintERR("socket create NOK")
    }
  }

  if (socket_state == SOCKET_CREATED)
  {
    address.sin_family      = (uint8_t)COM_AF_INET;
    address.sin_port        = com_htons(httpclient_distantport);
    address.sin_addr.s_addr = httpclient_distantip.addr;
    if (com_connect(socket_http_client,
                    (com_sockaddr_t const *)&address,
                    (int32_t)sizeof(com_sockaddr_in_t))
        == COM_SOCKETS_ERR_OK)
    {
      socket_state = SOCKET_CONNECTED;
      nfm_nb_error_short = 0U;
      nfm_sleep_timer_index = 0U;
      PrintINFO("socket connect OK")
    }
    else
    {
      nfm_nb_error_short++;
      PrintERR("socket connect NOK closing the socket")
      /* force next time to recreate socket */
      /* because if close is not done, next connect might be refused by LwIP */
      if (com_closesocket(socket_http_client)
          != COM_SOCKETS_ERR_OK)
      {
        socket_state = SOCKET_CLOSING;
        PrintERR("socket close NOK")
      }
      else
      {
        socket_state = SOCKET_INVALID;
        PrintINFO("socket close OK")
      }
      /* maybe distantip is no more ok
         if distantname is known force next time a DNS network resolution */
      if (strlen((const char *)httpclient_distantname) > 0U)
      {
        PrintINFO("distant ip is reset to force a new DNS network resolution of distant url")
        httpclient_distantip.addr = (uint32_t)0U;
      }
    }
  }
}

/**
  * @brief  Process a request
  * @note   Create, Send, Receive and Close socket
  * @param  send     - pointer on data buffer to send
  * @note   -
  * @param  response - pointer on data buffer for the response
  * @note   -
  * @retval bool     - false/true : process NOK/OK
  */
static httpclient_bool_t http_client_process(com_char_t *send,
                                             com_char_t *response)
{
  httpclient_bool_t result;
  com_char_t compare[10];
  int32_t count;
  int32_t read_buf_size;

  result = HTTPCLIENT_FALSE;
  /* If distantip to contact is unknown
     call DNS resolver service */
  if (httpclient_distantip.addr == (uint32_t)0U)
  {
    http_client_check_distantname();
  }

  if (httpclient_distantip.addr != (uint32_t)0U)
  {
    http_client_create_socket();

    if (socket_state == SOCKET_CONNECTED)
    {
      if (com_send(socket_http_client,
                   (const com_char_t *)send,
                   (int32_t)strlen((const char *)send), 0)
          > 0)
      {
        PrintINFO("socket send data OK")
        nfm_nb_error_short = 0U;
        nfm_sleep_timer_index = 0U;

        socket_state = SOCKET_WAITING_RSP;
        read_buf_size = com_recv(socket_http_client,
                                 (uint8_t *)response,
                                 (int32_t)CLIENT_MESSAGE_SIZE, 0);
        if (read_buf_size > 0)
        {
          socket_state = SOCKET_CONNECTED;
          count = sscanf((const char *)response, "HTTP/1.1 200 %s\r\n", compare);
          if (count == 1)
          {
            if (strcmp((const char *)&compare[0], "OK") == 0)
            {
              PrintINFO("socket response received OK")
              if (http_client_set_date_flag == 0U)
              {
                http_client_set_date_flag = 1U;
                timedate_http_date_set((char *)response);
              }
              result = HTTPCLIENT_TRUE;
            }
            else
            {
              PrintINFO("socket rsp received NOK")
            }
          }
          else
          {
            PrintINFO("socket rsp received NOK")
          }
        }
        else
        {
          socket_closing = HTTPCLIENT_TRUE;
          PrintERR("socket response NOT received - closing the socket")
        }
      }
      else
      {
        nfm_nb_error_short++;
        socket_closing = HTTPCLIENT_TRUE;
        PrintERR("socket send data NOK - closing the socket")
      }

      if ((socket_closing == HTTPCLIENT_TRUE)
          || (socket_state == SOCKET_WAITING_RSP))
      {
        /* Timeout to receive an answer or Closing has been requested */
        if (com_closesocket(socket_http_client) != COM_SOCKETS_ERR_OK)
        {
          socket_state = SOCKET_CLOSING;
          PrintERR("socket close NOK - stay in closing state")
        }
        else
        {
          socket_state = SOCKET_INVALID;
          PrintINFO("socket close OK")
        }
        socket_closing = HTTPCLIENT_FALSE;
      }
    }
  }
  return result;
}

/**
  * @brief  Get Process
  * @note   Format a Get request and analyze the answer
  * @param  None
  * @retval None
  */
static void http_client_get_process(void)
{
  httpclient_bool_t process_ok;

  sprintf((char *)formatted_http_request,
          "%s%s%s%s%s%s%s%s%s%s%s%s",
          header1_get_a,
          component_id_string,
          header1_get_b,
          get_channel_id_string_tab[0],
          header1_get_c,
          header2, header3, header4, header5,
          key_get_input,
          header_end, header_end);

  PrintINFO("GET request")
  process_ok = http_client_process(formatted_http_request,
                                   httpclient_tmp_buffer);
  if (process_ok == HTTPCLIENT_TRUE)
  {
    PrintINFO("GET request OK")
    if (strstr((const char *)httpclient_tmp_buffer,
               (const char *)buf_led_on)
        != 0)
    {
      PrintINFO("led on")
      HAL_GPIO_WritePin(HTTP_LED_GPIO_PORT, HTTP_LED_PIN, LED_ON);
    }
    else
    {
      if (strstr((const char *)httpclient_tmp_buffer,
                 (const char *)buf_led_off)
          != 0)
      {
        PrintINFO("led off")
        HAL_GPIO_WritePin(HTTP_LED_GPIO_PORT, HTTP_LED_PIN, LED_OFF);
      }
      else
      {
        PrintINFO("GET request response NOK")
      }
    }
  }
  else
  {
    PrintINFO("GET request NOK")
  }
}


/**
  * @brief  PUT Process
  * @note   Format a PUT request and analyze the answer
  * @param  None
  * @retval None
  */
static void http_client_put_process(void)
{
  httpclient_bool_t process_ok;

  http_put_request_format();
  sprintf((char *)formatted_http_request,
          "%s%s%s%s%s%s%s%s%s%s%s",
          header1a_put,
          component_id_string,
          formatted_channel_http_request,
          header1b_put,
          header2,
          header3,
          header4,
          header5,
          key_put_input,
          header_end, header_end);

  PrintINFO("PUT request")
  process_ok = http_client_process(formatted_http_request,
                                   httpclient_tmp_buffer);
  if (process_ok == HTTPCLIENT_TRUE)
  {
    PrintINFO("PUT request OK")
  }
  else
  {
    PrintINFO("PUT request NOK")
  }
}

/**
  * @brief  Get NFMC
  * @note   Return NFMC timer value accorind to the current index
  * @param  index    - current index of NFMC
  * note    -
  * @retval uint32_t - NFMC timer value
  */
static uint32_t http_client_get_nfmc(uint8_t index)
{
  uint32_t result;
  dc_nfmc_info_t dc_nfmc_info;

  dc_com_read(&dc_com_db, DC_COM_NFMC_TEMPO_INFO,
              (void *)&dc_nfmc_info,
              sizeof(dc_nfmc_info));

  if ((dc_nfmc_info.rt_state == DC_SERVICE_ON)
      && (dc_nfmc_info.activate == 1U))
  {
    result = dc_nfmc_info.tempo[index];
    PrintINFO("http_client NFMC tempo: %d-%ld", index, result)
  }
  else
  {
    result = 0U;
  }

  return result;
}


#if (USE_DEFAULT_SETUP == 1)
/**
  * @brief  Config handler
  * @note   At initialization update http client parameters by default value
  * @param  None
  * @retval None
  */
static void http_client_config_handler(void)
{
  httpclient_bool_t ip_valid;
  uint8_t i;
  uint32_t table_indice;
  int32_t  atoi_res;

  ip_valid = HTTPCLIENT_FALSE;
  table_indice = 0U;
  if (http_dns_resolver(httpclient_default_setup_table[table_indice++],
                        &httpclient_distantname[0],
                        &httpclient_distantport,
                        &httpclient_distantip)
      == HTTPCLIENT_TRUE)
  {
    ip_valid = HTTPCLIENT_TRUE;
  }

  if (ip_valid == HTTPCLIENT_FALSE)
  {
    httpclient_distantport = (uint16_t)HTTPCLIENT_DEFAULT_DISTANTPORT;
    httpclient_distantip.addr   = HTTPCLIENT_DEFAULT_DISTANTIP;
    memcpy(httpclient_distantname,
           HTTPCLIENT_DEFAULT_DISTANTNAME,
           strlen((char const *)HTTPCLIENT_DEFAULT_DISTANTNAME));
  }

  strcpy((char *)key_get_input,
         (char *)httpclient_default_setup_table[table_indice++]);
  strcpy((char *)key_put_input,
         (char *)httpclient_default_setup_table[table_indice++]);
  strcpy((char *)component_id_string,
         (char *)httpclient_default_setup_table[table_indice++]);

  atoi_res = atoi((char *)httpclient_default_setup_table[table_indice++]);
  if (atoi_res > 0)
  {
    http_client_put_period = (uint32_t)atoi_res;
  }
  else
  {
    http_client_put_period = 0U;
  }
  for (i = 0U; i < PUT_CHANNEL_ID_MAX; i++)
  {
    atoi_res = atoi((char *)(httpclient_default_setup_table[table_indice++]));
    if (atoi_res <= 0)
    {
      break;
    }
    atoi_res--;
    strcpy((char *)(put_channel_id_string_tab[atoi_res]),
           (char *)(httpclient_default_setup_table[table_indice++]));
  }

  atoi_res = atoi((char *)httpclient_default_setup_table[table_indice++]);
  if (atoi_res > 0)
  {
    http_client_get_period = (uint32_t)atoi_res;
  }
  else
  {
    http_client_get_period = 0U;
  }

  for (i = 0U; i < GET_CHANNEL_ID_MAX; i++)
  {
    atoi_res = atoi((char *)(httpclient_default_setup_table[table_indice++]));
    if (atoi_res <= 0)
    {
      break;
    }
    atoi_res--;
    strcpy((char *)(get_channel_id_string_tab[atoi_res]),
           (char *)(httpclient_default_setup_table[table_indice++]));
  }
}
#else /* USE_DEFAULT_SETUP == 0 */
/**
  * @brief  Sensor list configuration
  * @note   Configure sensor list
  * @param  channelIdName - Name of channelId
  * @note
  * @param  channelIdStringInput - channelId
  * @note
  * @param  channel_id_max - maximun channelId number
  * @note
  * @param  channel_id_size_max-  maximun channelId size
  * @note
  * @retval uint32_t - number of sensor configured
  */
static uint32_t http_client_sensor_list_config(com_char_t **channelIdName,
                                               uint8_t (*channelIdStringInput)[CHANNEL_ID_SIZE_MAX],
                                               uint8_t channel_id_max,
                                               uint8_t channel_id_size_max)
{
  uint32_t count;
  int32_t  atoi_res;

  count = 0U;
  for (uint8_t i = 0U; i < channel_id_max; i++)
  {
    PrintINFOMenu("\r\nConfig a sensor")
    PrintINFOMenu("Select sensor type")
    PrintINFOMenu("  0 : quit")
    /*
         for(uint8_t j=0U; j<channel_id_max; j++)
       {
         PrintINFOMenu("  %d : %s\r\n",j+1, channelIdName[j])
       }
    */
    menu_utils_get_string((uint8_t *)"Selection (0 to quit) ",
                          select_input_string,
                          SELECT_INPUT_STRING_SIZE);

    atoi_res = atoi((char *)select_input_string);
    if (atoi_res <= 0)
    {
      break;
    }
    atoi_res--;
    if (atoi_res >= (int32_t)channel_id_max)
    {
      continue;
    }
    PrintINFOMenu("Enter channel ID of sensor")

    menu_utils_get_string((uint8_t *)"Channel ID ",
                          channelIdStringInput[(uint8_t)atoi_res],
                          (uint32_t)channel_id_size_max);
    count++;
  }

  return count;
}

/**
  * @brief  Setup handler
  * @note   At initialization update http client parameters by menu
  * @param  None
  * @retval None
  */
static void http_client_setup_handler(void)
{
  httpclient_bool_t ip_valid;
  int32_t  atoi_res;
  int32_t  count;
  uint32_t  ip_port;
  uint32_t  ip_addr[4];
  uint32_t put_request_channel_count;
  uint32_t get_request_channel_count;

  ip_valid = HTTPCLIENT_FALSE;
  put_request_channel_count = 0U;
  get_request_channel_count = 0U;
  menu_utils_get_string((uint8_t *)"Host to contact IP(xxx.xxx.xxx.xxx:xxxxx)or URL(www.url.com port)",
                        distanthost_string,
                        NAME_SIZE_MAX);
  count = sscanf((char *)distanthost_string,
                 "%3ld.%3ld.%3ld.%3ld:%5lu",
                 &ip_addr[0], &ip_addr[1],
                 &ip_addr[2], &ip_addr[3],
                 &ip_port);
  if (count == 5)
  {
    if ((ip_addr[0] <= 255U)
        && (ip_addr[1] <= 255U)
        && (ip_addr[2] <= 255U)
        && (ip_addr[3] <= 255U)
        && (ip_port <= 65535U))
    {
      COM_IP4_ADDR(&httpclient_distantip,
                   ip_addr[0], ip_addr[1],
                   ip_addr[2], ip_addr[3]);
      httpclient_distantport = (uint16_t)ip_port;
      httpclient_distantname[0] = '\0';
      ip_valid = HTTPCLIENT_TRUE;
    }
  }
  else
  {
    if (http_dns_resolver(distanthost_string,
                          &httpclient_distantname[0],
                          &httpclient_distantport,
                          &httpclient_distantip)
        == HTTPCLIENT_TRUE)
    {
      ip_valid = HTTPCLIENT_TRUE;
    }
  }

  if (ip_valid == HTTPCLIENT_FALSE)
  {
    PrintINFOMenu("Host to contact IP address syntax NOK - Backup to default value")
    httpclient_distantport = (uint16_t)HTTPCLIENT_DEFAULT_DISTANTPORT;
    httpclient_distantip.addr   = HTTPCLIENT_DEFAULT_DISTANTIP;
    memcpy(httpclient_distantname,
           HTTPCLIENT_DEFAULT_DISTANTNAME,
           strlen((char *)HTTPCLIENT_DEFAULT_DISTANTNAME));
  }
  if (httpclient_distantip.addr != 0U)
  {
    PrintINFOMenu("Host to contact: IP:%d.%d.%d.%d URL:%s Port:%d ",
                  COM_IP4_ADDR1(&httpclient_distantip),
                  COM_IP4_ADDR2(&httpclient_distantip),
                  COM_IP4_ADDR3(&httpclient_distantip),
                  COM_IP4_ADDR4(&httpclient_distantip),
                  (char *)&httpclient_distantname[0],
                  httpclient_distantport)
  }
  else
  {
    PrintINFOMenu("Host to contact: URL:%s Port:%d ",
                  (char *)&httpclient_distantname[0],
                  httpclient_distantport)
  }

  menu_utils_get_string((uint8_t *)"Grovestreams GET Key ",
                        key_get_input,
                        KEY_GET_INPUT_SIZE);
  menu_utils_get_string((uint8_t *)"Grovestreams PUT Key ",
                        key_put_input,
                        KEY_PUT_INPUT_SIZE);

  menu_utils_get_string((uint8_t *)"Component ID",
                        component_id_string,
                        COMPONENT_ID_STRING_SIZE_MAX);

  PrintINFOMenu("\r\nPUT request")
  menu_utils_get_string((uint8_t *)"PUT request period ",
                        put_request_period_string,
                        PUT_REQUEST_PERIOD_SIZE_MAX);
  atoi_res = atoi((char *)put_request_period_string);
  if (atoi_res > 0)
  {
    http_client_put_period = (uint32_t)atoi_res;
    put_request_channel_count = http_client_sensor_list_config(put_channel_id_name_tab,
                                                               put_channel_id_string_tab,
                                                               PUT_CHANNEL_ID_MAX,
                                                               CHANNEL_ID_SIZE_MAX);
    if (put_request_channel_count == 0U)
    {
      http_client_put_period = 0U;
    }
  }
  else
  {
    http_client_put_period = 0U;
  }

  PrintINFOMenu("\r\nGET request")
  menu_utils_get_string((uint8_t *)"GET request period ",
                        get_request_period_string,
                        GET_REQUEST_PERIOD_SIZE_MAX);
  atoi_res = atoi((char *)get_request_period_string);
  if (atoi_res > 0)
  {
    http_client_get_period = (uint32_t)atoi_res;
    get_request_channel_count = http_client_sensor_list_config(get_channel_id_name_tab,
                                                               get_channel_id_string_tab,
                                                               GET_CHANNEL_ID_MAX,
                                                               CHANNEL_ID_SIZE_MAX);
    if (get_request_channel_count == 0U)
    {
      http_client_get_period = 0U;
    }
  }
  else
  {
    http_client_get_period = 0U;
  }
}

/**
  * @brief  Setup dump
  * @note   At initialization dump the value
  * @param  None
  * @retval None
  */
static void http_client_setup_dump(void)
{
}
#endif /* USE_DEFAULT_SETUP */

/**
  * @brief  Socket thread
  * @note   Infinite loop HTTP client body
  * @param  argument - parameter osThread
  * @note   UNUSED
  * @retval None
  */
static void http_client_socket_thread(void const *argument)
{
  uint32_t put_period_count, get_period_count;
  uint32_t nfmc_tempo;

  /* UNUSED(argument); */

  put_period_count = http_client_put_period;
  get_period_count = http_client_get_period;
  nfmc_tempo = 0U;

//  BL_LED_Init(BL_LED1);

  while (1)
  {
    /* Waiting for network ready */
    LED_Off(LED1);
    osMessageGet(http_client_queue, osWaitForever);
    if (network_is_on == HTTPCLIENT_TRUE)
    {
      LED_On(LED1);
      osDelay(1000U);
      LED_Off(LED1);
      osDelay(500U);
      LED_On(LED1);
      osDelay(1000U);
      LED_Off(LED1);
    }
    else
    {
      LED_On(LED1);
      osDelay(300U);
      LED_Off(LED1);
    }

    /* Network can changed quickly from on/off */
    while (network_is_on == HTTPCLIENT_TRUE)
    {
      if (http_client_process_flag == HTTPCLIENT_FALSE)
      {
        /* HTTP CLIENT NOT ACTIVE */
        osDelay(1000U);
      }
      else
      {
        /*  HTTP CLIENT ACTIVE */
        LED_On(LED1);
        if ((is_nfm_sleep_requested() == HTTPCLIENT_FALSE)
            && (http_client_put_period > 0U))
        {
          put_period_count++;
          if (put_period_count >= http_client_put_period)
          {
            http_client_put_process();
            put_period_count = 0U;
          }
          else
          {
            PrintDBG("Skip PUT process: not its time")
          }
        }
        else
        {
          PrintDBG("Skip PUT process: NFM or Period=0")
        }

        if ((is_nfm_sleep_requested() == HTTPCLIENT_FALSE)
            && (http_client_get_period > 0U))

        {
          get_period_count++;
          if (get_period_count >= http_client_get_period)
          {
            http_client_get_process();
            get_period_count = 0U;
          }
          else
          {
            PrintDBG("Skip GET process: not its time")
          }
        }
        else
        {
          PrintDBG("Skip GET process: NFM or Period=0")
        }

        LED_Off(LED1);

        if (is_nfm_sleep_requested() == HTTPCLIENT_TRUE)
        {
          nfmc_tempo = http_client_get_nfmc(nfm_sleep_timer_index);
          PrintERR("Too many errors: error/limit:%d/%d - timer activation:%ld ms",
                   nfm_nb_error_short, nfm_nb_error_limit_short, nfmc_tempo)
          osDelay(nfmc_tempo);
          nfm_nb_error_short = 0U;
          if (nfm_sleep_timer_index < ((uint8_t)DC_NFMC_TEMPO_NB - 1U))
          {
            nfm_sleep_timer_index++;
          }
        }
        else
        {
          PrintDBG("Delay")
          osDelay(HTTPCLIENT_SEND_PERIOD);
        }
      }
    }
  }
}

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Initialization
  * @note   HTTP client initialization
  * @param  None
  * @retval None
  */
void http_client_init(void)
{
#if (USE_DEFAULT_SETUP == 1)
  http_client_config_handler();
#else
  setup_record(SETUP_APPLI_HTTP_CLIENT,
               HTTP_CLIENT_SETUP_VERSION,
               HTTPCLIENT_LABEL,
               http_client_setup_handler,
               http_client_setup_dump,
               httpclient_default_setup_table,
               HTTPCLIENT_DEFAULT_PARAM_NB);
#endif

  /* Socket initialization */
  socket_http_client = -1;
  socket_state = SOCKET_INVALID;
  socket_closing = HTTPCLIENT_FALSE;

  /* NFM initialization */
  nfm_nb_error_limit_short = NFM_ERROR_LIMIT_SHORT_MAX;
  nfm_nb_error_short = 0U;
  nfm_sleep_timer_index = 0U;

  /* HTTP client activated */
  http_client_process_flag = HTTPCLIENT_TRUE;

  /* HTTP client set date done */
  http_client_set_date_flag = 0U;

  /* Network state */
  network_is_on = HTTPCLIENT_FALSE;

  osMessageQDef(http_client_queue, 1, uint32_t);
  http_client_queue = osMessageCreate(osMessageQ(http_client_queue), NULL);

  if (http_client_queue == NULL)
  {
    ERROR_Handler(DBG_CHAN_HTTP, 1, ERROR_FATAL);
  }
}

/**
  * @brief  Start
  * @note   HTTP client start
  * @param  None
  * @retval None
  */
void http_client_start(void)
{
  /* Registration to datacache - for Network On/Off and Button */
  dc_com_register_gen_event_cb(&dc_com_db, http_client_notif_cb, (void *) NULL);

  /* Create HTTP Client thread  */
  osThreadDef(httpClientTask,
              http_client_socket_thread,
              HTTPCLIENT_THREAD_PRIO, 0,
              HTTPCLIENT_THREAD_STACK_SIZE);
  httpClientTaskHandle = osThreadCreate(osThread(httpClientTask), NULL);
  if (httpClientTaskHandle == NULL)
  {
    ERROR_Handler(DBG_CHAN_HTTP, 2, ERROR_FATAL);
  }
  else
  {
#if (STACK_ANALYSIS_TRACE == 1)
    stackAnalysis_addStackSizeByHandle(httpClientTaskHandle,
                                       HTTPCLIENT_THREAD_STACK_SIZE);
#endif /* STACK_ANALYSIS_TRACE */
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
