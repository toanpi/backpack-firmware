/************************************************************************************************************
Module:       backpack_parser

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
04/28/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "backpack_parser.h"
#include "backpack.pb.h"
#include "proto_utilities.h"



//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define MAX_DECODE_CBS  (20)


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
static struct
{
  pb_size_t type;
  backpack_decode_f cb;
} backpack_decode_cbs[MAX_DECODE_CBS] = {0};

//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static bool decode_backpack_msg(uint8_t *p_buf, uint16_t buf_size, backpackPkg_backpackMsg *p_msg);
static bool backpack_encode_data(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message);


//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  backpack_process_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Processes the message received from the host for backpack device.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
bool backpack_process_msg(uint8_t *p_buf, uint16_t buf_size)
{
  if (p_buf == NULL || buf_size == 0)
  {
    return false;
  }

  backpackPkg_backpackMsg message = backpackPkg_backpackMsg_init_zero;
  bool ret = decode_backpack_msg(p_buf, buf_size, &message);

  if (!ret)
  {
    return false;
  }

  // For backpack_decode_cbs to call callback functions
  for (size_t i = 0; i < MAX_DECODE_CBS; i++)
  {
    if (backpack_decode_cbs[i].type == message.which_type && backpack_decode_cbs[i].cb)
    {
      backpack_decode_cbs[i].cb(&message);
      ret = true;
    }
  }
  return ret;
}

/********************************************************************************
Function:
  backpack_decode_register()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
void backpack_decode_register(pb_size_t type, backpack_decode_f callback)
{
  static uint8_t idx = 0;

  if(idx == MAX_DECODE_CBS)
  {
    return;
  }

  if (idx == 0)
  {
    memset(backpack_decode_cbs, 0, sizeof(backpack_decode_cbs));
  }

  backpack_decode_cbs[idx].type = type;
  backpack_decode_cbs[idx].cb = callback;
  idx++;
}

/********************************************************************************
Function:
  backpack_encode_uwb_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Encode uwb device message into backpack message.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
bool backpack_encode_uwb_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_uwb_message)
{
  backpackPkg_backpackMsg message = backpackPkg_backpackMsg_init_zero;

  message.which_type = backpackPkg_backpackMsg_uwbDevMsg_tag;
  message.type.uwbDevMsg = *((uwbDevPkg_uwbDevMsg *)p_uwb_message);

  return backpack_encode_data(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  backpack_encode_ble_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
bool backpack_encode_ble_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_ble_message)
{
  backpackPkg_backpackMsg message = backpackPkg_backpackMsg_init_zero;

  message.which_type = backpackPkg_backpackMsg_bleMsg_tag;
  message.type.bleMsg = *((bleCentralPkg_bleCentralMsg *)p_ble_message);

  return backpack_encode_data(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  backpack_encode_sensor_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
bool backpack_encode_sensor_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message)
{
  backpackPkg_backpackMsg message = backpackPkg_backpackMsg_init_zero;

  message.which_type = backpackPkg_backpackMsg_sensorMsg_tag;
  message.type.sensorMsg = *((sensorPkg_sensorMsg *)p_message);

  return backpack_encode_data(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  backpack_encode_collector_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
bool backpack_encode_collector_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message)
{
  backpackPkg_backpackMsg message = backpackPkg_backpackMsg_init_zero;

  message.which_type = backpackPkg_backpackMsg_collectorMsg_tag;
  message.type.collectorMsg = *((collectorPkg_collectorMsg *)p_message);

  return backpack_encode_data(p_buf, buf_size, p_written_len, &message);
}


/********************************************************************************
Function:
  backpack_encode_filetransfer_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
bool backpack_encode_algo_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message)
{
  backpackPkg_backpackMsg message = backpackPkg_backpackMsg_init_zero;

  message.which_type = backpackPkg_backpackMsg_algoMsg_tag;
  message.type.algoMsg = *((algoPkg_algoMsg *)p_message);

  return backpack_encode_data(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 05/17/2023
*********************************************************************************/
bool backpack_encode_cmd_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message)
{
  backpackPkg_backpackMsg message = backpackPkg_backpackMsg_init_zero;

  message.which_type = backpackPkg_backpackMsg_cmdMsg_tag;
  message.type.cmdMsg = *((cmdPkg_cmdMsg *)p_message);

  return backpack_encode_data(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  backpack_encode_filetransfer_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
bool backpack_encode_file_transfer_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message)
{
  backpackPkg_backpackMsg message = backpackPkg_backpackMsg_init_zero;

  message.which_type = backpackPkg_backpackMsg_fileMsg_tag;
  message.type.fileMsg = *((fileTransferPkg_fileMsg *)p_message);

  return backpack_encode_data(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  backpack_encode_dev_info_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
bool backpack_encode_dev_info_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, uint32_t prototype, uint64_t address)
{
  backpackPkg_backpackMsg message = backpackPkg_backpackMsg_init_zero;

  message.which_type = backpackPkg_backpackMsg_devInfo_tag;
  message.type.devInfo.prototype = (backpackPkg_prototype)prototype;
  message.type.devInfo.address = address;

  return backpack_encode_data(p_buf, buf_size, p_written_len, &message);
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  decode_backpack_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Decode the message received from the host for backpack device.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
static bool decode_backpack_msg(uint8_t *p_buf, uint16_t buf_size, backpackPkg_backpackMsg *p_msg)
{
  if (p_buf == NULL || buf_size == 0)
  {
    return false;
  }

  return proto_decode_msg(p_buf, buf_size, backpackPkg_backpackMsg_fields, p_msg);
}

/********************************************************************************
Function:
  backpack_encode_data()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Encode the message into the buffer for backpack device.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
static bool backpack_encode_data(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message)
{
  if (!p_buf || !buf_size || !p_written_len)
  {
    return false;
  }

  bool ret = false;
  *p_written_len = proto_encode_msg(p_buf, buf_size, backpackPkg_backpackMsg_fields, p_message);

  ret = (*p_written_len > 0);

  return ret;
}


//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF backpack_parser.c
//###########################################################################################################
