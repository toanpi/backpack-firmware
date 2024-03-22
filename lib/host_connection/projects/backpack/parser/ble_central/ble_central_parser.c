/************************************************************************************************************
Module:       ble_central_parser

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
05/03/2022    TH       Began Coding    (TH = Toan Huynh)

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
#include "host_connection.h"
#include "ble-central.pb.h"
#include "backpack.pb.h"
#include "backpack_parser.h"
#include "proto_utilities.h"
#include "ble_central_parser.h"
#include "ble_host_com.h"
#include "simple_central.h"




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
static bool encode_ble_dev_msg(bleCentralPkg_bleDevsInfo *devs, uint8_t *p_data, uint32_t len);
extern bool host_connection_proto_write_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);


//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
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
  Toan Huynh, 05/03/2022
*********************************************************************************/
void ble_central_parser_init(void)
{
   backpack_decode_register(backpackPkg_backpackMsg_bleMsg_tag, ble_process_dev_msg);
}

/*********************************************************************************
Function:
  encode_ble_log_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Uwb anchor log encode by protobuf
Notes:
  ---
Author, Date:
  Toan Huynh, 04/15/2022
*********************************************************************************/
bool encode_ble_log_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  bleCentralPkg_bleCentralMsg message = bleCentralPkg_bleCentralMsg_init_zero;

  message.which_type = bleCentralPkg_bleCentralMsg_state_tag;
  message.type.state.funcs.encode = &host_connection_proto_write_string;
  message.type.state.arg = p_data;

  return backpack_encode_ble_msg(p_buf, buf_size, p_written_len, &message);
}


bool encode_ble_scan_dev_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  bleCentralPkg_bleCentralMsg message = bleCentralPkg_bleCentralMsg_init_zero;
  message.which_type = bleCentralPkg_bleCentralMsg_scanDev_tag;

  encode_ble_dev_msg(&message.type.scanDev, p_data, len);

  return backpack_encode_ble_msg(p_buf, buf_size, p_written_len, &message);
}

bool encode_ble_connect_dev_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  bleCentralPkg_bleCentralMsg message = bleCentralPkg_bleCentralMsg_init_zero;
  message.which_type = bleCentralPkg_bleCentralMsg_connectDev_tag;

  encode_ble_dev_msg(&message.type.connectDev, p_data, len);

  return backpack_encode_ble_msg(p_buf, buf_size, p_written_len, &message);
}

// bool encode_ble_connect_dev_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
// {
//   uint64_t addr = *(uint64_t *)p_data;
//   backpackPkg_prototype prototype = backpackPkg_prototype_BLE_DEV;

//   return backpack_encode_dev_info_msg(p_buf, buf_size, p_written_len, prototype, addr);
// }


/********************************************************************************
Function:
  ble_process_dev_msg()
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
bool ble_process_dev_msg(void *p_msg)
{
  if(!p_msg)
  {
    return false;
  }

  bleCentralPkg_bleCentralMsg *msg = &(((backpackPkg_backpackMsg *)p_msg)->type.bleMsg);
  bool ret = false;

  switch(msg->which_type)
  {
    case bleCentralPkg_bleCentralMsg_reqConnectDev_tag:
      for (size_t i = 0; i < msg->type.reqConnectDev.list_count; i++)
      {
        bleCentralPkg_bleDevInfo *dev = &msg->type.reqConnectDev.list[i];
        connectBleDev((uint8_t *)&dev->address);
      }
      ret = true;
      break;
    case bleCentralPkg_bleCentralMsg_listConnectedDev_tag:
      break;
  }

  return ret;
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
static bool encode_ble_dev_msg(bleCentralPkg_bleDevsInfo *devs, uint8_t *p_data, uint32_t len)
{
  scanRec_t *scanList = (scanRec_t *)p_data;
  devs->list_count = len / sizeof(scanRec_t);

  for (size_t i = 0; i < devs->list_count; i++)
  {
    memcpy(&devs->list[i].address, &scanList[i].addr, B_ADDR_LEN);
    devs->list[i].addrType = scanList[i].addrType;
  }

  return true;
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF ble_central_parser.c
//###########################################################################################################
