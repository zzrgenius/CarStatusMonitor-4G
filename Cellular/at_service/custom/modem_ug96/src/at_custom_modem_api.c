/**
  ******************************************************************************
  * @file    at_custom_modem_api.c
  * @author  MCD Application Team
  * @brief   This file provides all the specific code to the
  *          UG96 Quectel modem (3G)
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
#include "string.h"
#include "at_custom_modem_api.h"
#include "at_custom_modem_specific.h"
#include "sysctrl_specific.h"
#include "plf_config.h"

/* UG96 COMPILATION FLAGS to define in project option if needed:
*
*  - CHECK_SIM_PIN : if defined, check if PIN code is required in SID_CS_INIT_MODEM
*
*
*/

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_ATCUSTOM_SPECIFIC == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PrintINFO(format, args...) TracePrint(DBG_CHAN_ATCMD, DBL_LVL_P0, "UG96:" format "\n\r", ## args)
#define PrintDBG(format, args...)  TracePrint(DBG_CHAN_ATCMD, DBL_LVL_P1, "UG96:" format "\n\r", ## args)
#define PrintAPI(format, args...)  TracePrint(DBG_CHAN_ATCMD, DBL_LVL_P2, "UG96 API:" format "\n\r", ## args)
#define PrintErr(format, args...)  TracePrint(DBG_CHAN_ATCMD, DBL_LVL_ERR, "UG96 ERROR:" format "\n\r", ## args)
#define PrintBuf(pbuf, size)       TracePrintBufChar(DBG_CHAN_ATCMD, DBL_LVL_P1, (char *)pbuf, size);
#else
#define PrintINFO(format, args...)  printf("UG96:" format "\n\r", ## args);
#define PrintDBG(format, args...)   printf("UG96:" format "\n\r", ## args);
#define PrintAPI(format, args...)   printf("UG96 API:" format "\n\r", ## args);
#define PrintErr(format, args...)   printf("UG96 ERROR:" format "\n\r", ## args);
#define PrintBuf(format, args...)   do {} while(0);
#endif /* USE_PRINTF */
#else
#define PrintINFO(format, args...)  do {} while(0);
#define PrintDBG(format, args...)   do {} while(0);
#define PrintAPI(format, args...)   do {} while(0);
#define PrintErr(format, args...)   do {} while(0);
#define PrintBuf(format, args...)   do {} while(0);
#endif /* USE_TRACE_ATCUSTOM_SPECIFIC */

/* Private variables ---------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
at_status_t atcma_init_at_func_ptrs(atcustom_funcPtrs_t *funcPtrs)
{
  at_status_t retval = ATSTATUS_ERROR;

#if defined(USE_MODEM_UG96)
  PrintDBG("Init AT func ptrs for device = MODEM QUECTEL UG96")

  /* init function pointers with UG96 functions */
  funcPtrs->f_init = ATCustom_UG96_init;
  funcPtrs->f_checkEndOfMsgCallback = ATCustom_UG96_checkEndOfMsgCallback;
  funcPtrs->f_getCmd = ATCustom_UG96_getCmd;
  funcPtrs->f_extractElement = ATCustom_UG96_extractElement;
  funcPtrs->f_analyzeCmd = ATCustom_UG96_analyzeCmd;
  funcPtrs->f_analyzeParam = ATCustom_UG96_analyzeParam;
  funcPtrs->f_terminateCmd = ATCustom_UG96_terminateCmd;
  funcPtrs->f_get_rsp = ATCustom_UG96_get_rsp;
  funcPtrs->f_get_urc = ATCustom_UG96_get_urc;
  funcPtrs->f_get_error = ATCustom_UG96_get_error;

  retval = ATSTATUS_OK;
#else
#error AT custom does not match with selected modem
#endif /* USE_MODEM_UG96 */

  return (retval);
}

sysctrl_status_t atcma_init_sysctrl_func_ptrs(sysctrl_funcPtrs_t *funcPtrs)
{
  sysctrl_status_t retval = SCSTATUS_ERROR;

#if defined(USE_MODEM_UG96)
  PrintDBG("Init SysCtrl func ptrs for device = MODEM QUECTEL UG96")

  /* init function pointers with UG96 functions */
  funcPtrs->f_getDeviceDescriptor = SysCtrl_UG96_getDeviceDescriptor;
  funcPtrs->f_power_on =  SysCtrl_UG96_power_on;
  funcPtrs->f_power_off = SysCtrl_UG96_power_off;
  funcPtrs->f_reset_device = SysCtrl_UG96_reset;

  retval = SCSTATUS_OK;
#else
#error SysCtrl does not match with selected modem
#endif /* USE_MODEM_UG96 */

  return (retval);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

