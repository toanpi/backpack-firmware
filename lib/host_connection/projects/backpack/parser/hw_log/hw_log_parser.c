/************************************************************************************************************
Module:       hw_log_parser

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
06/16/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <string.h>
#include <pb_encode.h>
#include "proto_utilities.h"
#include "hw_log_parser.h"
#include "watchdog.h"
#include "hw_logger.pb.h"
#include "host_connection.h"
#include "hw_log.h"
#include "system_utils.h"
#include "os_api.h"
#include "isr_thread.h"



//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################



//###########################################################################################################
//      MACROS
//###########################################################################################################



//###########################################################################################################
//      MODULE TYPES
//###########################################################################################################



//###########################################################################################################
//      CONSTANTS
//###########################################################################################################



//###########################################################################################################
//      MODULE LEVEL VARIABLES
//###########################################################################################################



//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static bool hwlog_encode_delimited_data(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message);
static bool hw_encode_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message, pb_size_t type);
static bool hwlog_parser_write(const void *p_message, pb_size_t type);



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  hwlog_parser_write_watchdog()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 06/16/2022
*********************************************************************************/
bool hwlog_parser_write_watchdog(const watchdog_log_t * p_watchdog_log, bool do_reset)
{
  HwLogPkg_Watchdog log = HwLogPkg_Watchdog_init_zero;

  log.regLr = p_watchdog_log->reg_lr;
  log.regPc = p_watchdog_log->reg_pc;
  log.threadRegLr = p_watchdog_log->thread_reg_lr;
  log.threadRegPc = p_watchdog_log->thread_reg_pc;
  log.executionTick = p_watchdog_log->execution_tick;

  if (p_watchdog_log->thread_name)
  {
    log.threadName.funcs.encode = proto_write_string;
    log.threadName.arg = p_watchdog_log->thread_name;
  }

  return hwlog_parser_write(&log, HwLogPkg_Entry_watchdog_tag);
}

/********************************************************************************
Function:
  hwlog_parser_write_fault()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 06/16/2022
*********************************************************************************/
bool hwlog_parser_write_fault(isr_data_t *isr_data, isr_flag_t type)
{
  HwLogPkg_SystemFault log = HwLogPkg_SystemFault_init_zero;

  log.lrAddr = isr_data->int_addr.lr_addr;
  log.pcAddr = isr_data->int_addr.pc_addr;
  log.psrAddr = isr_data->int_addr.psr_addr;

  switch (type)
  {
  case HARDFAULT_ISR:
    log.type = HwLogPkg_SystemFault_Type_HARD_FAULT;
    break;
  case USAGEFAULT_ISR:
    log.type = HwLogPkg_SystemFault_Type_USAGE_FAULT;
    break;
  case BUSFAULT_ISR:
    log.type = HwLogPkg_SystemFault_Type_BUS_FAULT;
    break;
  default:
    log.type = HwLogPkg_SystemFault_Type__NOT_SET;
    break;
  }

  return hwlog_parser_write(&log, HwLogPkg_Entry_systemFault_tag);
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  hwlog_parser_write()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 06/21/2022
*********************************************************************************/
static bool hwlog_parser_write(const void *p_message, pb_size_t type)
{
  uint8_t buffer[128] = {0};
  uint32_t written_len = 0;

  bool ret = false;

  if (hw_encode_msg(buffer, sizeof(buffer), &written_len, p_message, type))
  {
    ret = hwlog_open_then_write(buffer, written_len);
  }

  return ret;
}

/********************************************************************************
Function:
  hw_encode_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 06/21/2022
*********************************************************************************/
static bool hw_encode_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message, pb_size_t type)
{
  HwLogPkg_Entry message = HwLogPkg_Entry_init_zero;

  message.systemTickMs = os_tick_count_get();
  message.level = HwLogPkg_Entry_Level_FATAL;
  message.which_type = type;

  switch (message.which_type)
  {
  case HwLogPkg_Entry_watchdog_tag:
    message.type.watchdog = *(HwLogPkg_Watchdog *)p_message;
    break;
  case HwLogPkg_Entry_systemFault_tag:
    message.type.systemFault = *(HwLogPkg_SystemFault *)p_message;
    break;
  default:
    return false;
  }

  return hwlog_encode_delimited_data(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  hwlog_encode_delimited_data()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Encode the message into the buffer for hwlog device.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
static bool hwlog_encode_delimited_data(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message)
{
  if (!p_buf || !buf_size || !p_written_len)
  {
    return false;
  }

  bool ret = false;
  *p_written_len = proto_encode_delimited_msg(p_buf, buf_size, HwLogPkg_Entry_fields, p_message);

  ret = (*p_written_len > 0);

  return ret;
}

/********************************************************************************
Function:
  hwlog_encode_data()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Encode the message into the buffer for hwlog device.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
#if 0
static bool hwlog_encode_data(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message)
{
  if (!p_buf || !buf_size || !p_written_len)
  {
    return false;
  }

  bool ret = false;
  *p_written_len = proto_encode_msg(p_buf, buf_size, HwLogPkg_Entry_fields, p_message);

  ret = (*p_written_len > 0);

  return ret;
}
#endif

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF hw_log_parser.c
//###########################################################################################################
