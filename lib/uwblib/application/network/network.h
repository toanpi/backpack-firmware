/************************************************************************************************************
Module:       network

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/15/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _NETWORK_H
#define _NETWORK_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include "instance.h"
#include "network_mac.h"
#include "discovery.h"


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
bool network_create(struct TDMAHandler *tdma_handler, instance_data_t *inst);
void build_new_network(struct TDMAHandler *this);
void update_inf_tsfs(struct TDMAHandler *this);
void populate_inf_msg(struct TDMAHandler *this, uint8 inf_msg_type);
void slot_clear_action_check(struct TDMAHandler *this);
bool time_to_ranging(instance_data_t *inst, struct TDMAHandler *this);
bool tx_select(struct TDMAHandler *this, uint32 *p_timeout_ms);
bool need_blink(struct TDMAHandler *this);
bool check_blink(struct TDMAHandler *this);
bool check_timeouts(struct TDMAHandler *this);
bool process_inf_msg(struct TDMAHandler *this, uint8 *messageData, uint8 srcIndex, INF_PROCESS_MODE mode);
bool poll_delay(struct TDMAHandler *this, uint32 time_now_offset, uint32 offset);
uint32 cal_sleep_time(struct TDMAHandler *this, instance_data_t *inst);
uint32 next_blink_time(struct TDMAHandler *this);
bool is_last_node_to_range(struct TDMAHandler *tdma_handler);
bool store_last_node_ranged(struct TDMAHandler *tdma_handler, uint8 uwb_idx);



//###########################################################################################################
// END OF network.h
//###########################################################################################################
#endif
