/************************************************************************************************************
Module:       cmd_rgb_cam

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

// ###########################################################################################################
//       #INCLUDES
// ###########################################################################################################
#include <stdint.h>
#include <stdbool.h>
#include <pb_encode.h>
#include "command.pb.h"
#include "rgb_cam_task.h"
#include "rgb_cam_control.h"
#include "system_utils.h"

// ###########################################################################################################
//       TESTING #DEFINES
// ###########################################################################################################

// ###########################################################################################################
//       CONSTANT #DEFINES
// ###########################################################################################################

// ###########################################################################################################
//       MACROS
// ###########################################################################################################

// ###########################################################################################################
//       MODULE TYPES
// ###########################################################################################################

// ###########################################################################################################
//       CONSTANTS
// ###########################################################################################################

// ###########################################################################################################
//       MODULE LEVEL VARIABLES
// ###########################################################################################################

// ###########################################################################################################
//       PRIVATE FUNCTION PROTOTYPES
// ###########################################################################################################

// ###########################################################################################################
//       PUBLIC FUNCTIONS
// ###########################################################################################################
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
bool cmd_rgb_cam_handler(cmdPkg_RGBCamMsg *msg)
{
  if (!msg)
  {
    return false;
  }

  bool ret = true;

  switch (msg->which_type)
  {
  case cmdPkg_RGBCamMsg_startRGBCam_tag:
    rgb_cam_start_video(msg->type.startRGBCam);
    break;
  case cmdPkg_RGBCamMsg_stopRGBCam_tag:
    rgb_cam_stop_video();
    break;
  case cmdPkg_RGBCamMsg_captureRGBCam_tag:
    rgb_cam_start_capture();
    break;
  case cmdPkg_RGBCamMsg_updateIntervalRGBCam_tag:
    rgb_cam_set_capture_interval(msg->type.updateIntervalRGBCam);
    break;
  case cmdPkg_RGBCamMsg_shutdownRGBCam_tag:
    if (!rgb_cam_is_running() && rgb_cam_shutdown())
    {
      db_printf("RGB camera shutdown done\n\r");
    }
    break;
  case cmdPkg_RGBCamMsg_bootupRGBCam_tag:
    if (!rgb_cam_is_running() && rgb_cam_bootup())
    {
      db_printf("RGB camera bootup done\n\r");
    }
    break;
  case cmdPkg_RGBCamMsg_powerOffRGBCam_tag:
    if (!rgb_cam_is_running() && rgb_cam_power_off())
    {
      db_printf("RGB camera power off done\n\r");
    }
    break;
  case cmdPkg_RGBCamMsg_powerOnRGBCam_tag:
    if (!rgb_cam_is_running() && rgb_cam_power_on())
    {
      db_printf("RGB camera power on done\n\r");
    }

    break;
  case cmdPkg_RGBCamMsg_alwaysOnModeRGBCam_tag:
    if (rgb_cam_set_op_mode(RGB_CAM_ALWAYS_ON_MODE))
    {
      db_printf("RGB camera mode always on\n\r");
    }
    break;
  case cmdPkg_RGBCamMsg_optimizeModeRGBCam_tag:
    if (rgb_cam_set_op_mode(RGB_CAM_POWER_OPTIMIZE_MODE))
    {
      db_printf("RGB camera mode power optimize\n\r");
    }
    break;
  case cmdPkg_RGBCamMsg_getInfoRGBCam_tag:
    rgb_cam_send_info();
    break;
  case cmdPkg_RGBCamMsg_writeRegRGBCam_tag:
    if (rgb_cam_reg_write((uint16_t)msg->type.writeRegRGBCam.reg, (uint8_t)msg->type.writeRegRGBCam.value))
    {
      db_printf("RGB camera reg 0x%x write value 0x%x\n\r", msg->type.writeRegRGBCam.reg,
                msg->type.writeRegRGBCam.value);
    }
    break;
  case cmdPkg_RGBCamMsg_readRegRGBCam_tag:
  {
    uint16_t reg = msg->type.readRegRGBCam;
    uint8_t value = 0xFF;

    if (rgb_cam_reg_read(reg, &value))
    {
      db_printf("RGB Register 0x%X: 0x%X\n\r", reg, value);
    }
    else
    {
      db_printf("RGB Read Error Register \n\r");
    }
  }
  break;
  case cmdPkg_RGBCamMsg_videoModeRGBCam_tag:
  {
    uint32_t mode = msg->type.videoModeRGBCam.mode;
    uint32_t frame_cnt = msg->type.videoModeRGBCam.frameCnt;
    if (rgb_cam_set_mode((rgb_cam_video_mode_e)mode, frame_cnt, false))
    {
      db_printf("RGB set video mode: %s %d - frame count %d\n\r",
                mode == RGB_CAM_OP_MODE_STANDBY ? "Standby" : "Streaming",
                mode, frame_cnt);
    }
  }
  break;
  case cmdPkg_RGBCamMsg_bightnessRGBCam_tag:
    if (rgb_cam_set_brightness(msg->type.bightnessRGBCam))
    {
      rgb_cam_cmd_update();
      db_printf("RGB set brightness level %d done\n\r", msg->type.bightnessRGBCam);
    }
    break;
  case cmdPkg_RGBCamMsg_expouseRGBCam_tag:
  {
    uint32_t enable = msg->type.expouseRGBCam.enable;
    uint32_t exposure_us = msg->type.expouseRGBCam.exposureUs;
    if (rgb_cam_set_auto_exposure(enable, exposure_us))
    {
      rgb_cam_cmd_update();
      db_printf("RGB set exposure: %s - exposure (us) %d done\n\r", enable ? "enable" : "disable", exposure_us);
    }
  }
  break;
  case cmdPkg_RGBCamMsg_autoGainRGBCam_tag:
  {
    uint32_t enable = msg->type.autoGainRGBCam.enable;
    uint32_t gain_db = msg->type.autoGainRGBCam.gainDb;
    uint32_t gain_db_ceiling = msg->type.autoGainRGBCam.gainDbCeiling;

    if (rgb_cam_set_auto_gain((bool)enable, gain_db, gain_db_ceiling))
    {
      rgb_cam_cmd_update();
      db_printf("RGB set auto gain: %s - gain db %d - gain_db_ceiling %d\n\r", enable ? "enable" : "disable", gain_db, gain_db_ceiling);
    }
  }
  break;
  case cmdPkg_RGBCamMsg_gainCeilingRGBCam_tag:
  {
    uint32_t gainceiling = msg->type.gainCeilingRGBCam;
    if (rgb_cam_set_gainceiling((rgb_cam_gainceiling_t)gainceiling))
    {
      rgb_cam_cmd_update();
      db_printf("RGB set gain ceiling %dX done\n\r", gainceiling);
    }
  }
  break;
  case cmdPkg_RGBCamMsg_frameRateRGBCam_tag:
  {
    uint32_t frame_rate = msg->type.frameRateRGBCam;
    if (rgb_cam_set_framerate(frame_rate))
    {
      db_printf("RGB set frame rate %d done\n\r", frame_rate);
    }
  }
  break;
  case cmdPkg_RGBCamMsg_pixelFormatRGBCam_tag:
  {
    uint32_t pixel_format = msg->type.pixelFormatRGBCam;
    if (rgb_cam_set_pixformat((rgb_cam_pixformat_t)pixel_format))
    {
      db_printf("RGB set pixel format %s done\n\r",
                pixel_format == PIXFORMAT_GRAYSCALE ? "Grayscale" : "Bayer");
    }
  }
  break;
  case cmdPkg_RGBCamMsg_frameSizeRGBCam_tag:
  {
    uint32_t frame_size = msg->type.frameSizeRGBCam;
    if (rgb_cam_set_framesize((rgb_cam_framesize_t)frame_size))
    {
      db_printf("RGB set frame size %s done\n\r",
                frame_size == FRAMESIZE_QQVGA ? "QQVGA" : frame_size == FRAMESIZE_QVGA  ? "QVGA"
                                                      : frame_size == FRAMESIZE_320X320 ? "320x320"
                                                                                        : "QVGA4BIT");
    }
  }
  break;

  default:
    ret = false;
    break;
  }

  return ret;
}
// ###########################################################################################################
//       PRIVATE FUNCTIONS
// ###########################################################################################################

// ###########################################################################################################
//       INTERRUPTS
// ###########################################################################################################

// ###########################################################################################################
//       TEST HARNESSES
// ###########################################################################################################

// ###########################################################################################################
//       END OF cmd_rgb_cam.c
// ###########################################################################################################
