/************************************************************************************************************
Module:       sensor_parser

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
#include <string.h>
#include <pb_encode.h>
#include "host_connection.h"
#include "backpack.pb.h"
#include "command.pb.h"
#include "backpack_parser.h"
#include "proto_utilities.h"
#include "sensor_parser.h"
#include "backpack_parser.h"
#include "sensor_fusion_interface.h"
#include "controller.h"
#include "system_utils.h"
#include "positioning_calculation.h"
#include "power_manager.h"
#include "stimulation_exp.h"
#include "Stimulation_exp_variables.h"




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
static cmdPkg_transferControlImuMsg imu_transfer_control = 
{
    .dimension = false,
    .quaternion = false,
    .imuOmega = false,
    .imuAbsoluteLinearSpeed = false,
    .accel = false,
    .imuPosition = false,
    .angularVelocity = false,
};
// {
//     .dimension = true,
//     .quaternion = true,
//     .imuOmega = true,
//     .imuAbsoluteLinearSpeed = true,
//     .accel = true,
//     .imuPosition = true,
//     .angularVelocity = true,
// };
// {
//   0
// };


//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static bool process_msg(void *p_msg);

extern bool bp_process_stimulation_msg(stimulationPkg_stimulationMsg *msg);
extern bool bp_process_navigation_msg(navigationPkg_navigationMsg *msg);

//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  sensor_parser_init()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/04/2022
*********************************************************************************/
void sensor_parser_init(void)
{
  backpack_decode_register(backpackPkg_backpackMsg_sensorMsg_tag, process_msg);
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
bool sensor_transfer_control_cfg(cmdPkg_transferControlImuMsg *cfg)
{
  if(cfg)
  {
    imu_transfer_control = *cfg;
    return true;
  }

  return false;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 08/23/2023
*********************************************************************************/
bool sensor_transfer_control_get(cmdPkg_transferControlImuMsg *cfg)
{
  if(cfg)
  {
    *cfg = imu_transfer_control;
    return true;
  }
  return false;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 08/23/2023
*********************************************************************************/
bool is_imu_fusion_disabled(void)
{
  cmdPkg_transferControlImuMsg disable_config = {0};
  memset(&disable_config, 0, sizeof(disable_config));

  return !memcmp(&disable_config, &imu_transfer_control, sizeof(imu_transfer_control));
}

/********************************************************************************
Function:
  sensor_encode_position_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/04/2022
*********************************************************************************/
bool sensor_encode_position_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  sensorPkg_sensorMsg message = sensorPkg_sensorMsg_init_zero;
  message.which_type = sensorPkg_sensorMsg_positionMsg_tag;

  sensorPkg_positionMsg *p_msg = &message.type.positionMsg;

  if(!imu_transfer_control.imuPosition)
  {
    return false;
  }

  // Duoc uses this to send up the speed for checking purposes. Multiply for 100 is to convert to cm/s.
  p_msg->speedMagCms = speed_mag * 100.0; 

  p_msg->has_imuPosition = true;
  p_msg->imuPosition.x = IMUposition.x;
  p_msg->imuPosition.y = IMUposition.y;
  p_msg->imuPosition.z = IMUposition.z;

  p_msg->beacons_count = 4;
  for (int i = 0; i < 4; i++)
  {
    p_msg->beacons[i].id = (int)BeaconID[i];
    p_msg->beacons[i].distance = distanceArray[i];
  }

  p_msg->has_uwbPosition = true;
  p_msg->uwbPosition.x = UWBPosition.x;
  p_msg->uwbPosition.y = UWBPosition.y;
  p_msg->uwbPosition.z = UWBPosition.z;

  return backpack_encode_sensor_msg(p_buf, buf_size, p_written_len, &message);
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
  Toan Huynh, 05/04/2022
*********************************************************************************/
bool sensor_encode_fusion_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  sensorPkg_sensorMsg message = sensorPkg_sensorMsg_init_zero;
  message.which_type = sensorPkg_sensorMsg_fusionMsg_tag;

  fusion_data_t *p_fusion_data = (fusion_data_t *)p_data;
  sensorPkg_fusionMsg *p_fusion_msg = &message.type.fusionMsg;

  if(is_imu_fusion_disabled())
  {
    return false;
  }

  if(imu_transfer_control.dimension){
    p_fusion_msg->fPhiPl = p_fusion_data->SV_6DOF_GY_KALMAN.fPhiPl;
    p_fusion_msg->fThePl = p_fusion_data->SV_6DOF_GY_KALMAN.fThePl;
    p_fusion_msg->fPsiPl = p_fusion_data->SV_6DOF_GY_KALMAN.fPsiPl;
  }

  if(imu_transfer_control.quaternion)
  {
    p_fusion_msg->has_fqPl = true;
    p_fusion_msg->fqPl.q0 = p_fusion_data->SV_6DOF_GY_KALMAN.fqPl.q0;
    p_fusion_msg->fqPl.q1 = p_fusion_data->SV_6DOF_GY_KALMAN.fqPl.q1;
    p_fusion_msg->fqPl.q2 = p_fusion_data->SV_6DOF_GY_KALMAN.fqPl.q2;
    p_fusion_msg->fqPl.q3 = p_fusion_data->SV_6DOF_GY_KALMAN.fqPl.q3;
  }

  if(imu_transfer_control.imuOmega)
  {
    p_fusion_msg->imuOmega = IMU_Omega;
  }
  
  if(imu_transfer_control.imuAbsoluteLinearSpeed)
  {
    p_fusion_msg->imuAbsoluteLinearSpeed = IMU_AbsoluteLinearSpeed;
  }

  if(imu_transfer_control.accel)
  {
    p_fusion_msg->has_fAccGl = true;
    p_fusion_msg->fAccGl.x = p_fusion_data->SV_6DOF_GY_KALMAN.fAccGl[CHX];
    p_fusion_msg->fAccGl.y = p_fusion_data->SV_6DOF_GY_KALMAN.fAccGl[CHY];
    p_fusion_msg->fAccGl.z = p_fusion_data->SV_6DOF_GY_KALMAN.fAccGl[CHZ];
  }

  // Duoc change and stream up the average angular velocity (deg/s) 
  if(imu_transfer_control.angularVelocity)
  {
    p_fusion_msg->fOmega_count = 3;
    p_fusion_msg->fOmega[0] = p_fusion_data->SV_6DOF_GY_KALMAN.fOmega[0];
    p_fusion_msg->fOmega[1] = p_fusion_data->SV_6DOF_GY_KALMAN.fOmega[1];
    p_fusion_msg->fOmega[2] = p_fusion_data->SV_6DOF_GY_KALMAN.fOmega[2];
  }

  return backpack_encode_sensor_msg(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  sensor_encode_dev_info_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Decode the message from the device info
Notes:
  ---
Author, Date:
  Toan Huynh, 06/30/2022
*********************************************************************************/
bool sensor_encode_dev_info_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  sensorPkg_sensorMsg message = sensorPkg_sensorMsg_init_zero;

  message.which_type = sensorPkg_sensorMsg_devInfo_tag;

  message.type.devInfo.batteryMV = power_man_get_batt_mV();

  return backpack_encode_sensor_msg(p_buf, buf_size, p_written_len, &message);
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  process_channel_config()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/05/2022
*********************************************************************************/
static bool process_channel_config(sensorPkg_channelsConfigMsg *cfg)
{
#if 0
  channels_config_t channels_config = {0};

  channels_config.stiStartStop = cfg->stiStartStop;
  channels_config.gain = cfg->gain;
  channels_config.command = cfg->command;

  if (cfg->config_count > NUM_CHANNELS)
  {
    return false;
  }

  for (uint8_t i = 0; i < cfg->config_count; i++)
  {
    uint8_t idx = cfg->config[i].chId - 1;
    
    sys_dbg("Config channel %d\n", idx);
  
    if (idx >= NUM_CHANNELS)
    {
      return false;
    }

    channels_config.channels[idx].enable = cfg->config[i].enable;
    channels_config.channels[idx].duration = cfg->config[i].duration;
    channels_config.channels[idx].ton1 = cfg->config[i].ton1;
    channels_config.channels[idx].toff1 = cfg->config[i].toff1;
    channels_config.channels[idx].ton2 = cfg->config[i].ton2;
    channels_config.channels[idx].toff2 = cfg->config[i].toff2;
    channels_config.channels[idx].cycle = cfg->config[i].cycle;
    channels_config.channels[idx].rest = cfg->config[i].rest;
    channels_config.channels[idx].amp = cfg->config[i].amp;
    channels_config.channels[idx].channelId = cfg->config[i].chId;
  }

  /* TODO: Callback to update channels config */
  // set_channel_confif(&channels_config);
#endif

  return true;
}

/********************************************************************************
Function:
  process_stimulation_cmd()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/05/2022
*********************************************************************************/
static bool process_stimulation_cmd(sensorPkg_stimulationCommand *cmd)
{
  // TODO: Callback to execute stimulation command
  sys_dbg("Stimulation command: Ch1:%s Ch2:%s Ch3:%s\r\n",
          cmd->ch1En ? "En" : "Dis",
          cmd->ch2En ? "En" : "Dis",
          cmd->ch3En ? "En" : "Dis");

  return true;
}

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
static bool process_msg(void *p_msg)
{
  if(!p_msg)
  {
    return false;
  }

  sensorPkg_sensorMsg *msg = &(((backpackPkg_backpackMsg *)p_msg)->type.sensorMsg);
  bool ret = false;

  switch(msg->which_type)
  {
    case sensorPkg_sensorMsg_channelCfg_tag:
      ret = process_channel_config(&msg->type.channelCfg);
      break;
    case sensorPkg_sensorMsg_stimulationCommand_tag:
      ret = process_stimulation_cmd(&msg->type.stimulationCommand);
      break;
    case sensorPkg_sensorMsg_stimulationMsg_tag:
      ret = bp_process_stimulation_msg(&msg->type.stimulationMsg);
      break;
    case sensorPkg_sensorMsg_navigationMsg_tag:
      ret = bp_process_navigation_msg(&msg->type.navigationMsg);
      break;
  }

  return ret;
}


//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF dev_msg_parser.c
//###########################################################################################################
