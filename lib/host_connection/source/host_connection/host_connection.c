/************************************************************************************************************
Module:       host_connection.c

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
04/12/2022    TH        Began Coding    (TH = Toan Huynh)


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
#include <pb_decode.h>
#include "host_connection.h"
#include "hc_msg_code.h"
#include "packet.h"
#include "host_connection_port.h"



//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################
/* Disable/Enable host connection write */
#define HOST_CONNECTION_ENABLE_WRITE         (1)


//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################



//###########################################################################################################
//      MACROS
//###########################################################################################################
#define CHECK_NULL_AND_RETURN(ptr) if((ptr) == NULL) return HOST_CONNECTION_PARAM_INVALID;
#define CHECK_ZERO_AND_RETURN(len) if((len) == 0) return HOST_CONNECTION_PARAM_INVALID;



//###########################################################################################################
//      MODULE TYPES
//###########################################################################################################



//###########################################################################################################
//      CONSTANTS
//###########################################################################################################
#define PROTOBUF_BUF_SIZE   (512)


//###########################################################################################################
//      MODULE LEVEL VARIABLES
//###########################################################################################################
static send_data_f hw_send_data_func = NULL;
static host_connection_mode_e op_mode = HOST_CONNECTION_MODE_LOG;
static uint8_t proto_buffer[PROTOBUF_BUF_SIZE];



//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  HostConnection_init()
Input Parameters:
  send_data_f send_data_func: function pointer to send data to host
Output Parameters:
  ---
Description:
  Init host connection with the given function pointer
Notes:
  ---
Author, Date:
  Toan Huynh, 05/27/2021
*********************************************************************************/
uint32_t host_connection_init(send_data_f send_data_func, host_connection_mode_e mode)
{
  CHECK_NULL_AND_RETURN(send_data_func);

  host_connection_err_e ret = HOST_CONNECTION_ERR;

  hw_send_data_func = send_data_func;
  op_mode = mode;

  ret = HOST_CONNECTION_SUCCESS;

  return ret;
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
send_data_f host_connection_get_send_func(void)
{
  return hw_send_data_func;
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
bool host_connection_set_send_func(send_data_f func)
{
  if (func)
  {
    hw_send_data_func = func;
    return true;
  }
  return false;
}

/********************************************************************************
Function:
  host_connection_encode()
Input Parameters:
  ---
Output Parameters:
  Return encode length
Description:
  Encode the given data to protobuf
Notes:
  ---
Author, Date:
  Toan Huynh, 07/13/2022
*********************************************************************************/
uint32_t host_connection_encode(uint8_t *p_buf, uint32_t buf_len, void *p_data, uint32_t data_len, encode_f encode_func)
{
  uint32_t written_len = 0;
  uint32_t total_len = 0;
  uint32_t header_len = packet_get_header_len();

  if (buf_len < header_len)
  {
    return 0;
  }

  /* Clear the buffer */
  memset(p_buf, 0, buf_len);

  /* Encode the buffer */
  if (encode_func(p_buf + header_len, buf_len - header_len, p_data, data_len, &written_len))
  {
    total_len = packet_format_header(p_buf, buf_len, written_len);

    if (total_len)
    {
      uint32_t footer_len = packet_format_footer(p_buf, buf_len, written_len);

      total_len = footer_len > 0 ? total_len + footer_len : 0;
    }
  }

  return total_len;
}

/********************************************************************************
Function:
  host_connection_write()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 07/13/2022
*********************************************************************************/
bool host_connection_write(uint8_t *p_data, uint32_t data_len)
{
  bool ret = false;

#if HOST_CONNECTION_ENABLE_WRITE
  if (hw_send_data_func && p_data && data_len)
  {
    ret = hw_send_data_func(p_data, data_len);
  }
#endif

  return ret;
}


/********************************************************************************
Function:
  host_connection_forward()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Verify the given data and forward to host
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
bool host_connection_forward(uint8_t *p_data, uint32_t data_len, bool update_src_addr, uint32_t src_addr)
{
  hc_package_t package = packet_parse_data(p_data, data_len);
  bool ret = false;

  /* Check whether the data received is correct */
  if (package.p_data)
  {
    // Correct data length
    uint32_t actual_len = packet_get_header_len();
    actual_len += package.data_len;
    actual_len += packet_get_footer_len();

    if (update_src_addr)
    {
      // Update source address
      package.p_header->srcAddr = src_addr;
    }

    ret = host_connection_write(p_data, actual_len);
  }

  return ret;
}
/********************************************************************************
Function:
  host_connection_send()
Input Parameters:
  uint8_t *p_data: pointer to data to send
  uint32_t len: length of data to send
Output Parameters:
  ---
Description:
  Send data to host
Notes:
  ---
Author, Date:
  Toan Huynh, 04/13/2022
*********************************************************************************/
uint32_t host_connection_send(void * p_data, uint32_t len, encode_f encode_func)
{
  CHECK_NULL_AND_RETURN(encode_func);

  if (!hw_send_data_func)
  {
    return HOST_CONNECTION_ERR;
  }

  host_connection_err_e ret = HOST_CONNECTION_ERR;

  /* Lock the process */
  host_connection_lock();

  switch (op_mode)
  {
  case HOST_CONNECTION_MODE_PROTOBUF:
  {
    uint32_t total_len = host_connection_encode(proto_buffer, sizeof(proto_buffer), p_data, len, encode_func);

    if (total_len && host_connection_write(proto_buffer, total_len))
    {
      ret = HOST_CONNECTION_SUCCESS;
    }
    else
    {
      ret = HOST_CONNECTION_ERR;
    }
  }
  break;
  case HOST_CONNECTION_MODE_LOG:
    if (hw_send_data_func(p_data, len))
    {
      ret = HOST_CONNECTION_SUCCESS;
    }
    break;
  }

  /* Unlock the process */
  host_connection_unlock();

  return ret;
}

/********************************************************************************
Function:
  host_connection_parse()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 04/13/2022
*********************************************************************************/
uint32_t host_connection_parse(uint8_t * p_data, uint32_t len, decode_f decode_func)
{
  CHECK_NULL_AND_RETURN(p_data);
  CHECK_NULL_AND_RETURN(decode_func);
  CHECK_ZERO_AND_RETURN(len);

  hc_package_t package = packet_parse_data(p_data, len);

  if (package.p_data == NULL)
  {
    return HOST_CONNECTION_ERR;
  }

  bool status = decode_func(package.p_data, package.data_len);

  return status ? HOST_CONNECTION_SUCCESS : HOST_CONNECTION_ERR;
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
bool host_connection_process(uint8_t *p_data, uint32_t len, decode_f decode_func, send_data_f write)
{
  send_data_f default_write = host_connection_get_send_func();

  // Update write function for processing
  host_connection_set_send_func(write);

  // Process message by backpack parser
  host_connection_parse(p_data, len, decode_func);

  // Restore default write function
  host_connection_set_send_func(default_write);
}

/********************************************************************************
Function:
  host_connection_is_mine()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
bool host_connection_is_mine(uint32_t myAddr, uint8_t *p_data, uint32_t len, hc_header_t *p_header)
{
  p_header = packet_parse_header(p_data, len);

  if (p_header->desAddr == myAddr)
  {
    return true;
  }

  return false;
}

/********************************************************************************
Function:
  write_string()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Write encoded string to stream
Notes:
  ---
Author, Date:
  Toan Huynh, 04/14/2022
*********************************************************************************/
bool host_connection_proto_write_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
  return pb_encode_tag_for_field(stream, field) &&
         pb_encode_string(stream, *arg, strlen(*arg));
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
//      END OF host_connection.c
//###########################################################################################################

