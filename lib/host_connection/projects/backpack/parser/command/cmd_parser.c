/************************************************************************************************************
Module:       cmd_parser

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
#include "command.pb.h"
#include "backpack.pb.h"
#include "backpack_parser.h"
#include "proto_utilities.h"
#include "cmd_parser.h"




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
static bool process_dev_msg(void *p_msg);
extern bool cmd_ir_cam_handler(cmdPkg_IRCamMsg *irMsg);
extern bool cmd_rgb_cam_handler(cmdPkg_RGBCamMsg *msg);

#ifdef BACKPACK_APOLLO3
extern bool sensor_transfer_control_get(cmdPkg_transferControlImuMsg *cfg);
extern bool sensor_transfer_control_cfg(cmdPkg_transferControlImuMsg *cfg);
extern bool uwb_transfer_control_get(cmdPkg_transferControlUwbMsg *cfg);
extern bool uwb_transfer_control_cfg(cmdPkg_transferControlUwbMsg *cfg);
#endif

//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  collector_parser_init()
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
void cmd_parser_init(void)
{
   backpack_decode_register(backpackPkg_backpackMsg_cmdMsg_tag, process_dev_msg);
}


/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 07/31/2023
*********************************************************************************/
bool encode_cmd_transfer_control_cfg_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  cmdPkg_cmdMsg message = cmdPkg_cmdMsg_init_zero;
  message.which_type = cmdPkg_cmdMsg_transferControlCfg_tag;

  // IMU 
  message.type.transferControlCfg.has_imu = sensor_transfer_control_get(&message.type.transferControlCfg.imu);
  // UWB
  message.type.transferControlCfg.has_uwb = uwb_transfer_control_get(&message.type.transferControlCfg.uwb);

  return backpack_encode_cmd_msg(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 07/31/2023
*********************************************************************************/
bool cmd_transfer_control_handler(cmdPkg_transferControlMsg *msg)
{
  if(msg->has_imu)
  {
    sensor_transfer_control_cfg(&msg->imu);
  }

  if(msg->has_uwb)
  {
    uwb_transfer_control_cfg(&msg->uwb);
  }

  return true;
}

static bool cmd_send_transfer_control(void)
{
  host_connection_send(NULL, 0, encode_cmd_transfer_control_cfg_msg);
  return true;
}

/********************************************************************************
Function:
  collector_process_dev_msg()
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
static bool process_dev_msg(void *p_msg)
{
   if (!p_msg)
   {
     return false;
   }

   cmdPkg_cmdMsg *msg = &(((backpackPkg_backpackMsg *)p_msg)->type.cmdMsg);
   bool ret = false;

   switch (msg->which_type)
   {

#ifdef BACKPACK_APOLLO3
   case cmdPkg_cmdMsg_transferControlGet_tag:
     ret = cmd_send_transfer_control();
     break;
     
   case cmdPkg_cmdMsg_transferControlCfg_tag:
     cmd_transfer_control_handler(&msg->type.transferControlCfg);
     ret = true;
     break;
#else

   case cmdPkg_cmdMsg_irMsg_tag:
     cmd_ir_cam_handler(&msg->type.irMsg);
     ret = true;
     break;
#endif
   case cmdPkg_cmdMsg_rgbMsg_tag:
     cmd_rgb_cam_handler(&msg->type.rgbMsg);
     ret = true;
     break;
   }

   return ret;
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
//      END OF collector_parser.c
//###########################################################################################################

