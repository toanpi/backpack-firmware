/************************************************************************************************************
Module:       instance_config

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
#include "instance_config.h"
#include "dev_cfg.h"
#include "uwb_dev_setting.h"

#if CONFIG_ENABLE_LOCATION_ENGINE
#include "location_smoothing.h"
#endif



//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define CONFIG_RUNTIME_CHANGE    (0)



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
  instance_load_config()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/06/2022
*********************************************************************************/
bool inst_load_config(instance_data_t *inst)
{
  if (!inst)
  {
    return false;
  }

  /* Operation mode */
  operation_mode_e mode = CONFIG_OPERATION_MODE;
  cfgReadU8list(cfgMode, &mode, sizeof(mode));
  inst->operationMode = mode;

  /* Address */
  uint64_t eui = CONFIG_DEFAULT_EUI64;
  cfgReadU8list(cfgAddress, (uint8_t *)&eui, sizeof(eui));
  memcpy(&inst->eui64[0], &eui, 8);

  /* Position */
  position_t pos = {
      .x = CONFIG_NODE_POSITION_X,
      .y = CONFIG_NODE_POSITION_Y,
      .z = CONFIG_NODE_POSITION_Z};

  cfgReadU8list(cfgNodePos, (uint8_t *)&pos, sizeof(pos));
  memcpy(&inst->selfPosition, &pos, sizeof(pos));

  /* Settings */
  instanceConfig_t config = {
      .channelNumber = CONFIG_DEFAULT_CHANNEL,
      .preambleCode = CONFIG_DEFAULT_PREAMBLE_CODE,
      .pulseRepFreq = CONFIG_DEFAULT_PULSE_FREQ,
      .dataRate = CONFIG_DEFAULT_DATA_RATE,
      .preambleLen = CONFIG_DEFAULT_PREAMBLE_LENGTH,
      .pacSize = CONFIG_DEFAULT_PAC_SIZE,
      .nsSFD = CONFIG_DEFAULT_NSSFD,
      .sfdTO = CONFIG_DEFAULT_SFD_TO};

  cfgReadU8list(cfgUwbSetting, (uint8_t *)&config, sizeof(instanceConfig_t));
  memcpy(&inst->chConfig, &config, sizeof(instanceConfig_t));

  /* Range Interval */
  uint32_t range_interval = CONFIG_RANGING_RATE_MS;
  cfgReadU8list(cfgRangingIntervalMs, (uint8_t *)&range_interval, sizeof(range_interval));
  inst->rangingIntervalMs = range_interval;

  /* Receiver threshold */
  rx_detect_threshold_e rx_threshold = RDT_NORMAL;
  cfgReadU8list(cfgRxDetectThreshold, (uint8_t *)&rx_threshold, sizeof(rx_threshold));
  inst->rxDetectThreshold = rx_threshold;

  /* Antenna delay */
  uint16_t rx_delay = CONFIG_RX_ANT_DELAY;
  cfgReadU8list(cfgRxDelay, (uint8_t *)&rx_delay, sizeof(rx_delay));
  inst->rxAntennaDelay = rx_delay;

  uint16_t tx_delay = CONFIG_TX_ANT_DELAY;
  cfgReadU8list(cfgTxDelay, (uint8_t *)&tx_delay, sizeof(tx_delay));
  inst->txAntennaDelay = tx_delay;

  /* Smooth algo */
  inst->enSmoothingAlgo = (inst->operationMode == CONFIG_OPERATION_TAG);
  cfgReadU8list(cfgEnSmoothingAlgo, (uint8_t *)&inst->enSmoothingAlgo, sizeof(inst->enSmoothingAlgo));

  uint32_t max_speed = inst->enSmoothingAlgo ? 3 : 0;
  float e1 = inst->enSmoothingAlgo ? 0.05 : 0.0;

  cfgReadU8list(cfgSmoothMaxSpeed, (uint8_t *)&max_speed, sizeof(max_speed));
  cfgReadU8list(cfgSmoothE1, (uint8_t *)&e1, sizeof(e1));

#if CONFIG_ENABLE_LOCATION_ENGINE
  smooth_config_coefficient(max_speed, e1);
#endif

  /* Tx power */
  inst->txPower.mode = DEFAULT_TX_POWER;
  inst_config_get_tx_power(&inst->txPower);

  /* PAN ID */
  uint16 panId = CONFIG_DEFAULT_PAN_UUID;
  
  cfgReadU8list(cfgPanId, (uint8_t *)&panId, sizeof(panId));
  inst->panID = panId;

  return true;
}

/********************************************************************************
Function:
  instance_save_config()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Save the configuration to the flash
Notes:
  ---
Author, Date:
  Toan Huynh, 05/09/2022
*********************************************************************************/
void inst_save_config(void)
{
  cfgStore(true);
}

/********************************************************************************
Function:
  inst_config_operation_mode()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Store the operation mode of the instance.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
bool inst_config_operation_mode(instance_data_t *inst, operation_mode_e mode, bool *need_reset)
{
  if (inst->operationMode != mode)
  {
    cfgSet(cfgMode, (uint8_t *)&mode, sizeof(mode));

    if (need_reset)
    {
      *need_reset = true;
    }

#if CONFIG_RUNTIME_CHANGE
    inst->operationMode = mode;
#endif
  }

  return true;
}

/********************************************************************************
Function:
  inst_config_addr()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Store the address of the instance.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
bool inst_config_addr(instance_data_t *inst, uint32_t addr, bool *need_reset)
{
  uint64_t eui64 = (uint64_t)addr;

  if (memcmp(inst->eui64, &eui64, sizeof(inst->eui64)))
  {
    cfgSet(cfgAddress, (uint8_t *)&eui64, sizeof(eui64));

    if (need_reset)
    {
      *need_reset = true;
    }

#if CONFIG_RUNTIME_CHANGE
    memcpy(inst->eui64, &eui64, sizeof(inst->eui64));
#endif
  }

  return true;
}

/********************************************************************************
Function:
  inst_config_position()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Store the position of the instance.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
bool inst_config_position(instance_data_t *inst, position_t *pos, bool *need_reset)
{
  if (memcmp(&inst->selfPosition, pos, sizeof(inst->selfPosition)))
  {
    memcpy(&inst->selfPosition, pos, sizeof(inst->selfPosition));
    cfgSet(cfgNodePos, (uint8_t *)pos, sizeof(position_t));
  }

  return true;
}

/********************************************************************************
Function:
  inst_config_uwb_setting()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Store the UWB setting of the instance.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
bool inst_config_uwb_setting(instance_data_t *inst, instanceConfig_t *p_setting, bool *need_reset)
{
  if (memcmp(&inst->chConfig, p_setting, sizeof(instanceConfig_t)))
  {
    cfgSet(cfgUwbSetting, (uint8_t *)p_setting, sizeof(instanceConfig_t));

    if (need_reset)
    {
      *need_reset = true;
    }

#if CONFIG_RUNTIME_CHANGE
    memcpy(&inst->chConfig, p_setting, sizeof(instanceConfig_t));
    instance_config(inst);
#endif
  }

  return true;
}

/********************************************************************************
Function:
  inst_config_ranging_interval()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Store the ranging interval of the instance.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
bool inst_config_ranging_interval(instance_data_t *inst, uint32_t interval_ms, bool *need_reset)
{
  if (inst->rangingIntervalMs != interval_ms)
  {
    inst->rangingIntervalMs = interval_ms;
    cfgSet(cfgRangingIntervalMs, (uint8_t *)&interval_ms, sizeof(interval_ms));
  }

  return true;
}

/********************************************************************************
Function:
  inst_config_rx_detect_threshold()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Store the RX detect threshold of the instance.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
bool inst_config_rx_detect_threshold(instance_data_t *inst, rx_detect_threshold_e rx_detect_threshold, bool *need_reset)
{
  if (inst->rxDetectThreshold != rx_detect_threshold)
  {
    cfgSet(cfgRxDetectThreshold, (uint8_t *)&rx_detect_threshold, sizeof(rx_detect_threshold));

    inst->rxDetectThreshold = rx_detect_threshold;
    uwb_dev_set_rx_detect_threshold(inst, rx_detect_threshold);
  }

  return true;
}

/********************************************************************************
Function:
  inst_config_antenna_delay()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Store the antenna delay of the instance.
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
bool inst_config_antenna_delay(instance_data_t *inst, uint16_t rx_deay, uint16_t tx_delay, bool *need_reset)
{
  bool changed = false;

  if (inst->rxAntennaDelay != rx_deay)
  {
    cfgSet(cfgRxDelay, (uint8_t *)&rx_deay, sizeof(rx_deay));
    changed = true;
  }

  if (inst->txAntennaDelay != tx_delay)
  {
    cfgSet(cfgTxDelay, (uint8_t *)&tx_delay, sizeof(tx_delay));
    changed = true;
  }

  if (changed)
  {
    instanceconfigantennadelays(tx_delay, rx_deay);
    instancesetantennadelays();
  }

  return true;
}

/********************************************************************************
Function:
  inst_config_smooth_algo()
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
bool inst_config_smooth_algo(instance_data_t *inst,
                             bool enSmoothingAlgo,
                             uint32_t smoothMaxSpeed,
                             float smoothE1)
{
  inst->enSmoothingAlgo = enSmoothingAlgo;

  cfgSet(cfgEnSmoothingAlgo, (uint8_t *)&enSmoothingAlgo, sizeof(enSmoothingAlgo));
  cfgSet(cfgSmoothMaxSpeed, (uint8_t *)&smoothMaxSpeed, sizeof(smoothMaxSpeed));
  cfgSet(cfgSmoothE1, (uint8_t *)&smoothE1, sizeof(smoothE1));

#if CONFIG_ENABLE_LOCATION_ENGINE
  smooth_config_coefficient(smoothMaxSpeed, smoothE1);
#endif

  return true;
}

/********************************************************************************
Function:
  inst_get_smooth_algo()
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
void inst_get_smooth_algo(uint32_t *smoothMaxSpeed, float *smoothE1)
{
  if (smoothMaxSpeed)
  {
    cfgReadU8list(cfgSmoothMaxSpeed, (uint8_t *)smoothMaxSpeed, sizeof(*smoothMaxSpeed));
  }

  if (smoothE1)
  {
    cfgReadU8list(cfgSmoothE1, (uint8_t *)smoothE1, sizeof(*smoothE1));
  }
}

/********************************************************************************
Function:
  inst_config_set_tx_power()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Set/ Get tx power
Notes:
  ---
Author, Date:
  Toan Huynh, 04/07/2023
*********************************************************************************/
void inst_config_set_tx_power(instance_data_t *inst, tx_power_percent_t *cfg)
{
  inst->txPower = *cfg;

  cfgSet(cfgTxPower, (uint8_t *)cfg, sizeof(*cfg));
}

void inst_config_get_tx_power(tx_power_percent_t *cfg)
{
  if (cfg)
  {
    cfgReadU8list(cfgTxPower, (uint8_t *)cfg, sizeof(*cfg));
  }
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
void inst_config_set_pan_id(instance_data_t *inst, uint32_t pan_id)
{
  cfgSet(cfgPanId, (uint8_t *)&pan_id, sizeof(pan_id));
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
//      END OF instance_config.c
//###########################################################################################################
