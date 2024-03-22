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

#ifndef _INSTANCE_CONFIG_H
#define _INSTANCE_CONFIG_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <stdbool.h>
#include <stdint.h>
#include "instance.h"
#include "tx_power.h"


//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################



//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################



//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
bool inst_load_config(instance_data_t *inst);
void inst_save_config(void);
bool inst_config_operation_mode(instance_data_t *inst, operation_mode_e mode, bool *need_reset);
bool inst_config_addr(instance_data_t *inst, uint32_t addr, bool *need_reset);
bool inst_config_position(instance_data_t *inst, position_t *pos, bool *need_reset);
bool inst_config_uwb_setting(instance_data_t *inst, instanceConfig_t *p_setting, bool *need_reset);
bool inst_config_ranging_interval(instance_data_t *inst, uint32_t interval_ms, bool *need_reset);
bool inst_config_rx_detect_threshold(instance_data_t *inst, rx_detect_threshold_e rx_detect_threshold, bool *need_reset);
bool inst_config_antenna_delay(instance_data_t *inst, uint16_t rx_deay, uint16_t tx_delay, bool *need_reset);
bool inst_config_smooth_algo(instance_data_t *inst,
                               bool enSmoothingAlgo,
                               uint32_t smoothMaxSpeed,
                               float smoothE1);
void inst_get_smooth_algo(uint32_t *smoothMaxSpeed, float *smoothE1);

void inst_config_set_tx_power(instance_data_t *inst, tx_power_percent_t *cfg);
void inst_config_get_tx_power(tx_power_percent_t *cfg);
void inst_config_set_pan_id(instance_data_t *inst, uint32_t pan_id);

//###########################################################################################################
// END OF instance_config.h
//###########################################################################################################
#endif
