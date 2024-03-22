/************************************************************************************************************
Module:       cmd_ir_cam

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
05/18/2023    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2023 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <stdint.h>
#include <stdbool.h>
#include <pb_encode.h>
#include "command.pb.h"
#include "flir_cam_thread.h"
#include "flir_cam_control.h"
#include "system_utils.h"

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
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 05/18/2023
*********************************************************************************/
bool cmd_ir_cam_handler(cmdPkg_IRCamMsg *msg)
{
  if (!msg)
  {
    return false;
  }

  bool ret = true;

  switch (msg->which_type)
  {
  case cmdPkg_IRCamMsg_startIrCam_tag:
    flir_cam_start_video(msg->type.startIrCam);
    break;
  case cmdPkg_IRCamMsg_stopIrCam_tag:
    flir_cam_stop_video();
    break;
  case cmdPkg_IRCamMsg_captureIrCam_tag:
    flir_cam_start_capture();
    break;
  case cmdPkg_IRCamMsg_updateIntervalIrCam_tag:
    flir_cam_set_capture_interval(msg->type.updateIntervalIrCam);
    break;
  case cmdPkg_IRCamMsg_shutdownIrCam_tag:
    if (!flir_cam_is_running() && flir_cam_shutdown())
    {
      db_printf("Flir camera shutdown done\n\r");
    }
    break;
  case cmdPkg_IRCamMsg_bootupIrCam_tag:
    if (!flir_cam_is_running() && flir_cam_bootup())
    {
      db_printf("Flir camera bootup done\n\r");
    }
    break;
  case cmdPkg_IRCamMsg_powerOffIrCam_tag:
    if (!flir_cam_is_running() && flir_cam_power_off())
    {
      db_printf("Flir camera power off done\n\r");
    }
    break;
  case cmdPkg_IRCamMsg_powerOnIrCam_tag:
    if (!flir_cam_is_running() && flir_cam_power_on())
    {
      db_printf("Flir camera power on done\n\r");
    }
    break;
  case cmdPkg_IRCamMsg_alwaysOnModeIrCam_tag:
    if (flir_cam_set_op_mode(FLIR_CAM_ALWAYS_ON_MODE))
    {
      db_printf("Flir camera mode always on\n\r");
    }
    break;
  case cmdPkg_IRCamMsg_optimizeModeIrCam_tag:
    if (flir_cam_set_op_mode(FLIR_CAM_POWER_OPTIMIZE_MODE))
    {
      db_printf("Flir camera mode power optimize\n\r");
    }
    break;
  case cmdPkg_IRCamMsg_runFCCIrCam_tag:
    if (!flir_cam_is_running())
    {
      db_printf("Please wait for flir camera perform FFC..\n\r");
      bool status = flir_cam_perform_ffc();
      db_printf("%s\n\r", status ? "done" : "error");
    }
    break;
  case cmdPkg_IRCamMsg_getInfoIrCam_tag:
    flir_cam_send_info();// TODO
    break;
  default:
    ret = false;
    break;
  }

  return ret;
}

/*********************************************************************************
Function:
  encode_collector_list_msg()
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
bool encode_cmd_ir_info_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
   cmdPkg_cmdMsg message = cmdPkg_cmdMsg_init_zero;
  //  uint32_t *ret = (uint32_t *)p_data;

  //  message.which_type = cmdPkg_writeRegMsg_reg_tag;

  //  message.type.blobDetection.resultX = ret[0];
  //  message.type.blobDetection.resultY = ret[1];

   return backpack_encode_cmd_msg(p_buf, buf_size, p_written_len, &message);
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
//      END OF cmd_ir_cam.c
//###########################################################################################################
