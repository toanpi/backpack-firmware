/************************************************************************************************************
Module:       app_state

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/13/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _APP_STATE_H
#define _APP_STATE_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <stdint.h>
#include <stdbool.h>
#include "instance.h"

//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################



//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
typedef bool (*process_func)(instance_data_t *inst, uint32 *p_timeout);
typedef bool (*passed_func)(instance_data_t *inst, uint32 tx_timeout);
typedef bool (*failed_func)(instance_data_t *inst);

//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
bool app_state_rx_wait(instance_data_t *inst);
bool app_state_tx_confirm(instance_data_t *inst, uint32 tx_timeout);
bool app_state_exec(instance_data_t *inst,
                    process_func process_hdl,
                    passed_func passed_hdl,
                    failed_func failed_hdl,
                    uint32 *p_timeout);
bool app_state_can_sleep(instance_data_t *inst);
uint32 app_state_wait_timeout(instance_data_t *inst, uint32 start_time, uint32 duration_ms);
bool app_state_change(instance_data_t *inst, INST_STATES new_state);
bool app_state_process_sleep(instance_data_t *inst, struct TDMAHandler *tdma_handler, bool need_sleep, uint32_t *p_timeout);

//###########################################################################################################
// END OF app_state.h
//###########################################################################################################
#endif
