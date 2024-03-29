/************************************************************************************************************
Module:       host_com_msg

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
05/02/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <stdarg.h>
#include "host_com_msg.h"
#include "host_connection.h"
#include "port_mcu.h"
#include "uwb_dev_parser.h"
#include "backpack_parser.h"
#include "instance.h"



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



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  host_com_write()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  this function is used to write data to the host.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/13/2022
*********************************************************************************/
bool host_com_write(uint8_t *data, uint8_t len)
{
  return host_connection_write(data, len);
}

/********************************************************************************
Function:
  host_com_forward()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Forward data to the host.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/13/2022
*********************************************************************************/
bool host_com_forward(uint8_t *p_data, uint32 data_len)
{
  return host_connection_forward(p_data, data_len, false, 0);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/17/2023
*********************************************************************************/
void host_com_print_log(const char *format, ...)
{
  char buffer[256] = {0};
  va_list args;

  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  host_connection_write((uint8_t *)buffer, strlen(buffer) + 1);
}

/********************************************************************************
Function:
  host_com_send_dev_info()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Send device info to host
Notes:
  ---
Author, Date:
  Toan Huynh, 04/17/2022
*********************************************************************************/
bool host_com_send_dev_info(uint8_t *data, uint16_t len)
{
  return host_connection_send(data, len, encode_uwb_dev_info_msg);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  Create a log funtion like printf
  Max buffer size is 256 bytes
Author, Date:
  Toan Huynh, 03/28/2024
*********************************************************************************/
void host_com_log(const char *format, ...)
{
  char buffer[256] = {0};
  va_list args;

  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  host_connection_send((uint8_t *)buffer, strlen(buffer) + 1, encode_uwb_log_msg);
}

/********************************************************************************
Function:
  ()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/02/2022
*********************************************************************************/
bool host_com_send_net_info(uint8_t *data, uint16_t len)
{
  return host_connection_send(data, len, encode_uwb_net_info_msg);
}

/********************************************************************************
Function:
  host_com_encode_net_info()
Input Parameters:
  ---
Output Parameters:
  Return encoded message length
Description:
  Encode network info message
Notes:
  ---
Author, Date:
  Toan Huynh, 07/13/2022
*********************************************************************************/
uint32 host_com_encode_net_info(uint8_t *p_buf, uint32 buf_len, uint8_t *data, uint16_t len)
{
  return host_connection_encode(p_buf, buf_len, data, len, encode_uwb_net_info_msg);
}
/********************************************************************************
Function:
  host_com_send_dev_config()
Input Parameters:
  data: pointer to data to send
  len: length of data to send
Output Parameters:
  ---
Description:
  Sends device configuration data to host
Notes:
  ---
Author, Date:
  Toan Huynh, 04/21/2022
*********************************************************************************/
bool host_com_send_dev_config(uint8_t *data, uint16_t len)
{
  return host_connection_send(data, len, encode_uwb_dev_config_msg);
}


/********************************************************************************
Function:
  host_com_send_dev_calib()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Send calib info
Notes:
  ---
Author, Date:
  Toan Huynh, 04/07/2023
*********************************************************************************/
bool host_com_send_dev_calib(uint8_t *data, uint16_t len)
{
  return host_connection_send(data, len, encode_uwb_dev_calib_msg);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 08/20/2023
*********************************************************************************/
bool host_com_send_ranging_info(void)
{
  return host_connection_send(NULL, 0, encode_uwb_ranging_info_msg);
  
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/13/2023
*********************************************************************************/
bool host_com_send_neighbor_list(void)
{
  return host_connection_send(NULL, 0, encode_uwb_neighbor_list_msg);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/18/2023
*********************************************************************************/
bool host_com_send_host_setup(void)
{
  return host_connection_send(NULL, 0, encode_uwb_host_setup_msg);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/18/2023
*********************************************************************************/
bool host_com_send_test_msg_res(uint32_t arg)
{
  return host_connection_send((void *)arg, sizeof(arg), encode_uwb_test_msg_res);
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################


//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF host_com_msg.c
//###########################################################################################################
