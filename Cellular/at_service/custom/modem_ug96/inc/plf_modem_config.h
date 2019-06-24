/**
  ******************************************************************************
  * @file    plf_modem_config.h
  * @author  MCD Application Team
  * @brief   This file contains the modem configuration for UG96
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
#ifndef PLF_MODEM_CONFIG_H
#define PLF_MODEM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#if defined(HWREF_AKORIOT_UG96)
/* explicitly defined: using UG96 AkorIot board */
#elif defined(HWREF_B_CELL_UG96)
/* explicitly defined: using UG96 B_CELL board */
#else
/* default: no board specified -> using UG96 B_CELL board */
#define HWREF_B_CELL_UG96
#endif

#if defined(HWREF_AKORIOT_UG96)
#define USE_MODEM_UG96
#define CONFIG_MODEM_UART_BAUDRATE (115200U)
#define CONFIG_MODEM_UART_RTS_CTS  (0)
#define CONFIG_MODEM_USE_ARDUINO_CONNECTOR

#else /* default case: HWREF_B_CELL_UG96 */
#define USE_MODEM_UG96
#define CONFIG_MODEM_UART_BAUDRATE (115200U)
#define CONFIG_MODEM_UART_RTS_CTS  (1)
#define CONFIG_MODEM_USE_STMOD_CONNECTOR

#endif /* modem board choice */

/* PDP context parameters (for AT+CGDONT) BEGIN */
#define PDP_CONTEXT_DEFAULT_MODEM_CID          1   /* CID numeric value */
#define PDP_CONTEXT_DEFAULT_MODEM_CID_STRING  "1"  /* CID string value */
#define PDP_CONTEXT_DEFAULT_TYPE              "IP" /* defined in project config files */
#define PDP_CONTEXT_DEFAULT_APN               ""   /* defined in project config files */
/* PDP context parameters (for AT+CGDONT) END */

#ifdef __cplusplus
}
#endif

#endif /*_PLF_MODEM_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
