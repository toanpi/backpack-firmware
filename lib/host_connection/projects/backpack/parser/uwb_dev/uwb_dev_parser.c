/************************************************************************************************************
Module:       uwb_dev_parser

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
04/14/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <string.h>
#include <stdint.h>
#include <pb_encode.h>
#include "host_connection.h"
#include "uwb-dev.pb.h"
#include "backpack.pb.h"
#include "command.pb.h"
#include "backpack_parser.h"
// #include "hc_msg_code.h"
#include "proto_utilities.h"
#include "instance.h"
#include "instance_config.h"
#include "dw_main.h"
#include "lib.h"
#include "instance_utilities.h"
#include "host_msg.h"
#include "timestamp.h"
#include "tx_power.h"
#include "app_monitor.h"
#include "ranging_test.h"

#if CONFIG_ENABLE_LOCATION_ENGINE
#include "location_smoothing.h"
#endif



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
extern bool host_connection_proto_write_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
static bool uwb_process_dev_msg(void *p_msg);

uwbDevPkg_OpModeCode get_op_mode(instance_data_t *inst)
{
  const uwbDevPkg_OpModeCode modes[] = {
      [CONFIG_OPERATION_TAG] = uwbDevPkg_OpModeCode_TAG,
      [CONFIG_OPERATION_ANCHOR] = uwbDevPkg_OpModeCode_ANCHOR,
      [CONFIG_OPERATION_TAG_ANCHOR] = uwbDevPkg_OpModeCode_TAG_ANCHOR,
  };

  return modes[inst->operationMode];
}

uwbDevPkg_stateCode get_state(instance_data_t *inst)
{
  const uwbDevPkg_stateCode states[] = {
      [DISCOVERY] = uwbDevPkg_stateCode_SEARCHING,
      [TAG] = uwbDevPkg_stateCode_CONNECTED,
      [ANCHOR] = uwbDevPkg_stateCode_CONNECTED,
  };

  return states[inst->mode];
}

uwbDevPkg_neighborStateCode get_neighbor_state(const struct TDMAHandler *tdma_handler, uint32 node_idx)
{
  if(node_idx >= UWB_LIST_SIZE)
  {
    return uwbDevPkg_neighborStateCode_INACTIVE;
  }

  uint16 state = tdma_handler->uwbListTDMAInfo[node_idx].connectionType;

  if(state >= UWB_LIST_TWICE_HIDDEN || state <= UWB_LIST_SELF)
  {
    return uwbDevPkg_neighborStateCode_INACTIVE;
  }

  const uwbDevPkg_neighborStateCode states[] = {
      [UWB_LIST_NEIGHBOR] = uwbDevPkg_neighborStateCode_IN_RANGE,
      [UWB_LIST_HIDDEN] = uwbDevPkg_neighborStateCode_OUT_OF_RANGE,
  };

  return states[state];
}

bool get_op_pos(const position_t *src_pos, uwbDevPkg_uwbPos *position)
{
  /* Position */
  position->x = src_pos->x;
  position->y = src_pos->y;
  position->z = src_pos->z;

  return true;
}

extern void device_reset_mcu(void);

//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
void uwb_dev_parser_init(void)
{
   backpack_decode_register(backpackPkg_backpackMsg_uwbDevMsg_tag, uwb_process_dev_msg);
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
bool uwb_transfer_control_cfg(cmdPkg_transferControlUwbMsg *cfg)
{
  if(cfg)
  {
    transfer_control_host_msg msg = {
      .location = cfg->location,
      .ranging = cfg->ranging
    };

    host_msg_config_set(&msg);

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
bool uwb_transfer_control_get(cmdPkg_transferControlUwbMsg *cfg)
{
  if(cfg)
  {
    const transfer_control_host_msg *data = host_msg_config_get();
    
    cfg->location = data->location;
    cfg->ranging = data->ranging;

    return true;
  }
  return false;
}
/********************************************************************************
Function:
  encode_uwb_anchor_log_msg()
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
bool encode_uwb_log_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  if(p_buf == NULL || p_data == NULL || p_written_len == NULL || len == 0)
  {
    return false;
  }

  uwbDevPkg_uwbDevMsg message = uwbDevPkg_uwbDevMsg_init_zero;

  message.which_type = uwbDevPkg_uwbDevMsg_log_tag;
  message.type.log.msg.funcs.encode = &host_connection_proto_write_string;
  message.type.log.msg.arg = p_data;

  return backpack_encode_uwb_msg(p_buf, buf_size, p_written_len, &message);
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
  Toan Huynh, 04/20/2022
*********************************************************************************/
bool encode_uwb_dev_config_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  if(p_buf == NULL || p_data == NULL || p_written_len == NULL || len == 0)
  {
    return false;
  }

  // bool ret = false;
  uwbDevPkg_uwbDevMsg message = uwbDevPkg_uwbDevMsg_init_zero;
  uwbDevPkg_uwbDevConfig *cfg = &message.type.devConfig;
  instance_data_t *inst = (instance_data_t *)p_data;

  /* Set the message type */
  message.which_type = uwbDevPkg_uwbDevMsg_devConfig_tag;

  /* Set the device mode */
  cfg->opMode = get_op_mode(inst);
  
  /* Address */
  cfg->address = (uint32_t) instance_get_addr();

  /* Position */
  cfg->has_position = get_op_pos(&inst->selfPosition, &cfg->position);

  /* Antenna */
  cfg->txDelay = inst->txAntennaDelay;
  cfg->rxDelay = inst->rxAntennaDelay;

  /* Channel */
	cfg->channelNumber = inst->chConfig.channelNumber;
	cfg->preambleCode = inst->chConfig.preambleCode;
	cfg->pulseRepFreq = inst->chConfig.pulseRepFreq;
	cfg->dataRate = inst->chConfig.dataRate;
	cfg->preambleLen = inst->chConfig.preambleLen;
	cfg->pacSize = inst->chConfig.pacSize;
	cfg->nsSFD = inst->chConfig.nsSFD;
	cfg->sfdTO = inst->chConfig.sfdTO;

  cfg->rangingIntervalMs = inst->rangingIntervalMs;
  cfg->rxDetectThreshold = inst->rxDetectThreshold;

  cfg->enSmoothingAlgo = inst->enSmoothingAlgo;

  inst_get_smooth_algo(&cfg->smoothMaxSpeed, &cfg->smoothE1);

  return backpack_encode_uwb_msg(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  encode_uwb_dev_calib_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 04/07/2023
*********************************************************************************/
bool encode_uwb_dev_calib_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  if (p_buf == NULL || p_data == NULL || p_written_len == NULL || len == 0)
  {
    return false;
  }

  uwbDevPkg_uwbDevMsg message = uwbDevPkg_uwbDevMsg_init_zero;
  uwbDevPkg_uwbCalibCfg *cfg = &message.type.setCalibCfg;
  instance_data_t *inst = (instance_data_t *)p_data;

  /* Set the message type */
  message.which_type = uwbDevPkg_uwbDevMsg_setCalibCfg_tag;

  /* Data */
  uint8_t mode[] =
  {
      [TX_POWER_CALIBRATED_SMART] = uwbDevPkg_txPowerMode_TX_POWER_CALIBRATED_SMART,
      [TX_POWER_SMART] = uwbDevPkg_txPowerMode_TX_POWER_SMART,
      [TX_POWER_MANUAL] = uwbDevPkg_txPowerMode_TX_POWER_MANUAL,
  };

  cfg->mode = mode[inst->txPower.mode];

  cfg->phrPercent = inst->txPower.phr_percent;
  cfg->payloadPercent = inst->txPower.payload_percent;

  /* For smart power */
  cfg->payload500usPercent = inst->txPower.payload_500us_percent;
  cfg->payload250usPercent = inst->txPower.payload_250us_percent;
  cfg->payload125usPercent = inst->txPower.payload_125us_percent;

  cfg->needReset = false;
  cfg->maxTxPowerDB = tx_power_max_db();

  return backpack_encode_uwb_msg(p_buf, buf_size, p_written_len, &message);
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
#define MAX_DEVS  4
bool encode_uwb_ranging_info_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  uwbDevPkg_uwbDevMsg message = uwbDevPkg_uwbDevMsg_init_zero;
  uwbDevPkg_uwbRangingInfo *info = &message.type.rangingInfo;
  message.which_type = uwbDevPkg_uwbDevMsg_rangingInfo_tag;

  instance_data_t *inst = instance_get_local_structure_ptr(0);
  const struct TDMAHandler *tdma_handler = get_tdma_handler();

  info->address = (uint32_t) instance_get_addr();
  info->isInTestMode = ranging_test_in_test();
  // info->panId= inst->panID;
  info->state = get_state(inst);

  /* Ranging Info */
  uint32_t num_devs = 0;
  const app_ranged_node_info_t * rangingInfo = get_last_ranged_info(&num_devs);

  info->neighbors_count = 0;
  
  for (size_t i = 0; i < num_devs; i++)
  {
   if(rangingInfo[i].addr)
   {
     info->neighbors[i].address = rangingInfo[i].addr;
     info->neighbors[i].distance_rsl = rangingInfo[i].distance_rsl; 
     info->neighbors[i].error = rangingInfo[i].error;
     info->neighbors[i].state = get_neighbor_state(tdma_handler, rangingInfo->index);
     info->neighbors_count++;
   }
  }

  return backpack_encode_uwb_msg(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  encode_uwb_anchor_net_info_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 04/18/2022
*********************************************************************************/
bool encode_uwb_dev_info_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  if(p_buf == NULL || p_data == NULL || p_written_len == NULL || len == 0)
  {
    return false;
  }

  uwbDevPkg_uwbDevMsg message = uwbDevPkg_uwbDevMsg_init_zero;
  // bool ret = false;
  uwbDevPkg_uwbDevInfo * info = &message.type.devInfo;
  instance_data_t *inst = (instance_data_t *)p_data;

  /* Set the message type */
  message.which_type = uwbDevPkg_uwbDevMsg_devInfo_tag;

  /* Address */
  info->address = (uint32_t) instance_get_addr();
  
  /* Position */
  info->has_position = get_op_pos(&inst->selfPosition, &info->position);

  /* State */
  info->state = get_state(inst);

  /* Neighbors */
  info->neighborsCount = instfindnumneighbors(inst);
  info->hiddensCount = instfindnumhidden(inst);

  info->swMajorVersion = SOFTWARE_MAJOR_VER;
  info->swMinorVersion = SOFTWARE_MINOR_VER;

  info->slotDurationMs = CEIL_DIV(inst->durationSlotMax_us, 1000);

  info->opMode = get_op_mode(inst);

  info->panId = inst->panID;

  info->isInTestMode = ranging_test_in_test();

  return backpack_encode_uwb_msg(p_buf, buf_size, p_written_len, &message);
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
  Toan Huynh, 04/28/2022
*********************************************************************************/
bool encode_uwb_error_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  if(p_buf == NULL || p_data == NULL || p_written_len == NULL || len == 0)
  {
    return false;
  }

  uwbDevPkg_uwbDevMsg message = uwbDevPkg_uwbDevMsg_init_zero;

  message.which_type = uwbDevPkg_uwbDevMsg_uwbError_tag;
  message.type.uwbError = *(uwbDevPkg_uwbError *)p_data;

  return backpack_encode_uwb_msg(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  encode_uwb_anchor_net_info_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 04/18/2022
*********************************************************************************/
bool encode_uwb_net_info_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  if(p_buf == NULL || p_data == NULL || p_written_len == NULL || len == 0)
  {
    return false;
  }

  uwbDevPkg_uwbDevMsg message = uwbDevPkg_uwbDevMsg_init_zero;
  // bool ret = false;
  uwbDevPkg_uwbDevNetInfo * info = &message.type.devNetInfo;
  instance_data_t *inst = (instance_data_t *)p_data;
  static uint32_t last_send_node_idx = 0;

  /* Set the message type */
  message.which_type = uwbDevPkg_uwbDevMsg_devNetInfo_tag;

  /* Only sending if position is updated */
  if (!is_valid_position(&inst->selfPosition))
  {
    return false;
  }

  /* Address */
  info->address = (uint32_t) instance_get_addr();
  
  /* Position */
  info->has_position = get_op_pos(&inst->selfPosition, &info->position);

  info->timestamp = timestamp_get_ms();

  /* State */
  info->state = get_state(inst);

  info->opMode = get_op_mode(inst);

  /* Neighbors */
  uint32_t max_devs = ARRAY_SIZE(info->neighbors);
  const struct TDMAHandler *tdma_handler = get_tdma_handler();
  info->neighbors_count = 0;

  for (size_t i = 0; i < max_devs; i++)
  {
    uint8_t node_idx = last_send_node_idx + 1;
    const position_t *pos = get_node_position_start_from_idx(inst, tdma_handler, node_idx, &last_send_node_idx);

    if (pos)
    {
      info->neighbors[i].has_position = get_op_pos(pos, &info->neighbors[i].position);
      info->neighbors[i].address = (uint32_t)instance_get_uwbaddr(last_send_node_idx);
      info->neighbors_count++;
    }
    else
    {
      break;
    }
  }

  return backpack_encode_uwb_msg(p_buf, buf_size, p_written_len, &message);
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
bool encode_uwb_neighbor_list_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  uwbDevPkg_uwbDevMsg message = uwbDevPkg_uwbDevMsg_init_zero;
  message.which_type = uwbDevPkg_uwbDevMsg_neighborsList_tag;

  uwbDevPkg_neighborInfoList * list = &message.type.neighborsList;
  instance_data_t *inst = instance_get_local_structure_ptr(0);

  list->address =  (uint32_t)instance_get_addr();
  list->neighbors_count = inst_get_neighbors_list(inst, list->neighbors, ARRAY_SIZE(list->neighbors));

  return backpack_encode_uwb_msg(p_buf, buf_size, p_written_len, &message);
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
bool encode_uwb_host_setup_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  uwbDevPkg_uwbDevMsg message = uwbDevPkg_uwbDevMsg_init_zero;
  message.which_type = uwbDevPkg_uwbDevMsg_hostMsg_tag;
  uwbDevPkg_hostSetup * info = &message.type.hostMsg;

  instance_data_t *inst = instance_get_local_structure_ptr(0);

  info->address =  (uint32_t)instance_get_addr();
  info->neighbors_count = inst_get_neighbors_list(inst, info->neighbors, ARRAY_SIZE(info->neighbors));

  return backpack_encode_uwb_msg(p_buf, buf_size, p_written_len, &message);
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
bool encode_uwb_test_msg_res(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  uwbDevPkg_uwbDevMsg message = uwbDevPkg_uwbDevMsg_init_zero;
  message.which_type = uwbDevPkg_uwbDevMsg_testMsgResponse_tag;
  uwbDevPkg_testMsgResponse * res = &message.type.testMsgResponse;

  res->code = (uint32_t) p_data;
  res->success = true;

  return backpack_encode_uwb_msg(p_buf, buf_size, p_written_len, &message);
}


/********************************************************************************
Function:
  uwb_process_config_dev_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Processes the config message
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
static bool uwb_process_config_dev_msg(void *p_uwb_msg)
{
  if (!p_uwb_msg)
  {
    return false;
  }

  bool ret = false;
  uwbDevPkg_uwbDevConfig *cfg = (uwbDevPkg_uwbDevConfig *)p_uwb_msg;
  instance_data_t *inst = instance_get_local_structure_ptr(0);
  bool need_reset = false;

  instanceConfig_t setting = {
      .channelNumber = cfg->channelNumber,
      .preambleCode = cfg->preambleCode,
      .pulseRepFreq = cfg->pulseRepFreq,
      .dataRate = cfg->dataRate,
      .preambleLen = cfg->preambleLen,
      .pacSize = cfg->pacSize,
      .nsSFD = cfg->nsSFD,
      .sfdTO = cfg->sfdTO,
  };
  ret = inst_config_uwb_setting(inst, &setting, &need_reset);
  ret |= inst_config_operation_mode(inst, cfg->opMode, &need_reset);
  ret |= inst_config_addr(inst, cfg->address, &need_reset);
  if (cfg->has_position)
  {
    ret |= inst_config_position(inst, (position_t *)&cfg->position, &need_reset);
  }
  ret |= inst_config_ranging_interval(inst, cfg->rangingIntervalMs, &need_reset);
  ret |= inst_config_rx_detect_threshold(inst, cfg->rxDetectThreshold, &need_reset);
  ret |= inst_config_antenna_delay(inst, cfg->rxDelay, cfg->txDelay, &need_reset);
  ret |= inst_config_smooth_algo(inst, cfg->enSmoothingAlgo, cfg->smoothMaxSpeed, cfg->smoothE1);

  /* Store config in storage device */
  inst_save_config();

  if (need_reset)
  {
    /* Reset MCU */
    device_reset_mcu();
  }

  return ret;
}

/********************************************************************************
Function:
  set_smooth_algo()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 12/12/2022
*********************************************************************************/
static void set_smooth_algo(uwbDevPkg_uwbSmoothAlgoCfg *cfg)
{
  instance_data_t *inst = instance_get_local_structure_ptr(0);

  // Disable for now
  // inst_config_smooth_algo(inst, cfg->enable, cfg->maxSpeed, cfg->e1);

#if CONFIG_ENABLE_LOCATION_ENGINE
  position_t pos = {
      .x = cfg->startPoint.x,
      .y = cfg->startPoint.y,
      .z = cfg->startPoint.z,
  };
  user_set_smooth_start_point(inst->uwbShortAdd, pos);
#endif
}

/********************************************************************************
Function:
  uwb_process_calib_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Calibrate device
Notes:
  ---
Author, Date:
  Toan Huynh, 04/07/2023
*********************************************************************************/
static bool uwb_process_calib_msg(void *p_uwb_msg)
{
  if (!p_uwb_msg)
  {
    return false;
  }

  uint8_t mode[] =
  {
      [uwbDevPkg_txPowerMode_TX_POWER_CALIBRATED_SMART] = TX_POWER_CALIBRATED_SMART,
      [uwbDevPkg_txPowerMode_TX_POWER_SMART] = TX_POWER_SMART,
      [uwbDevPkg_txPowerMode_TX_POWER_MANUAL] = TX_POWER_MANUAL,
  };

  uwbDevPkg_uwbCalibCfg *cfg = (uwbDevPkg_uwbCalibCfg *)p_uwb_msg;
  instance_data_t *inst = instance_get_local_structure_ptr(0);

  bool ret = false;
  tx_power_percent_t tx_cfg =
  {
      .mode = mode[cfg->mode],
      .phr_percent = cfg->phrPercent,
      .payload_percent = cfg->payloadPercent,
      .payload_125us_percent = cfg->payload125usPercent,
      .payload_250us_percent = cfg->payload250usPercent,
      .payload_500us_percent = cfg->payload500usPercent,
  };

  tx_power_relative_set(&tx_cfg);

  inst_config_set_tx_power(inst, &tx_cfg);

  /* Store config in storage device */
  inst_save_config();

  if (cfg->needReset)
  {
    /* Reset MCU */
    device_reset_mcu();
  }

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
  Toan Huynh, 08/20/2023
*********************************************************************************/
static bool uwb_process_test_mode_msg(uwbDevPkg_uwbTestModeCfg *cfg)
{
  if(cfg->isExit)
  {
    ranging_test_exit();
  }
  else if(cfg->isEnter)
  {
    ranging_test_enter((testNodeInfo *)cfg->nodes, cfg->nodes_count);
  }

  return true;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 08/25/2023
*********************************************************************************/
static bool uwb_process_net_create_msg(uwbDevPkg_uwbNetworkCreateConfig *cfg)
{
  instance_data_t *inst = instance_get_local_structure_ptr(0);
  
  /* Store config in storage device */
  inst_config_set_pan_id(inst, cfg->panId);
  
  inst_config_operation_mode(inst, cfg->opMode, NULL);

  inst_save_config();

  /* Need to reset */
  device_reset_mcu();

  return true;
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
static bool list_node_neighbor(void)
{
  host_msg_send_neighbor_list();
  return true;
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
static bool send_host_setup_msg(void)
{
  host_msg_send_host_setup();
  return true;
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
static bool process_test_msg(uwbDevPkg_testMsg *testMsg)
{
  host_msg_send_test_msg_res(testMsg->code);
  return true;
}
/********************************************************************************
Function:
  process_uwb_dev_msg()
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
static bool uwb_process_dev_msg(void *p_msg)
{
  if(!p_msg)
  {
    return false;
  }

  uwbDevPkg_uwbDevMsg *msg = &(((backpackPkg_backpackMsg *)p_msg)->type.uwbDevMsg);
  bool ret = false;

  switch(msg->which_type)
  {
    case uwbDevPkg_uwbDevMsg_getCalibInfo_tag:
      send_dev_calib_info();
      ret = true;
      break;
    case uwbDevPkg_uwbDevMsg_setCalibCfg_tag:
      ret = uwb_process_calib_msg(&msg->type.setCalibCfg);
      break;
    case uwbDevPkg_uwbDevMsg_setDevCfg_tag:
      ret = uwb_process_config_dev_msg(&msg->type.setDevCfg);
      break;
    case uwbDevPkg_uwbDevMsg_getDevCfg_tag:
      send_dev_config();
      ret = true;
      break;
    case uwbDevPkg_uwbDevMsg_getDevInfo_tag:
      send_device_state_info();
      ret = true;
      break;
    case uwbDevPkg_uwbDevMsg_getDevNetInfo_tag:
      send_device_net_info();
      ret = true;
      break;
    case uwbDevPkg_uwbDevMsg_smoothAlgoCfg_tag:
      set_smooth_algo(&msg->type.smoothAlgoCfg);
      ret = true;
      break;
    case uwbDevPkg_uwbDevMsg_netCfg_tag:
      ret = uwb_process_net_create_msg(&msg->type.netCfg);
      break;
    case uwbDevPkg_uwbDevMsg_testModeCfg_tag:
      ret = uwb_process_test_mode_msg(&msg->type.testModeCfg);
      break;
    case uwbDevPkg_uwbDevMsg_hostSetupReq_tag:
      ret = send_host_setup_msg();
      break;
    case uwbDevPkg_uwbDevMsg_listNeighbors_tag:
      ret = list_node_neighbor();
      break;
    case uwbDevPkg_uwbDevMsg_testMsg_tag:
      ret = process_test_msg(&msg->type.testMsg);
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
//      END OF uwb_achor_parser.c
//###########################################################################################################
