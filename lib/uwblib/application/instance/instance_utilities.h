/************************************************************************************************************
Module:       instance_utilities

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/14/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _INSTANCE_UTILITIES_H
#define _INSTANCE_UTILITIES_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include "app_main.h"



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
bool is_valid_position(const position_t * position);
uint16 address64to16(const uint8 *address);
uint64 instance_get_addr(void);
uint64 instance_get_uwbaddr(uint8 uwb_index);
void instance_set_position(instance_data_t *isnt, float x, float y, float z);
void get_last_ranged_node_info(instance_data_t *inst, ranged_node_info_t info[4], uint32 * num_devs);
bool instance_clear_ranged_node_info(void);
bool is_tag(instance_data_t *inst, uint16 uwbIdx);
uint32 instance_getmessageduration_us(int data_length_bytes);
const char *get_instanceModes_string(enum instanceModes mode);
const char *get_discovery_modes_string(enum discovery_modes mode);
const char *get_msg_fcode_string(int fcode);
const position_t *get_node_position_by_idx(instance_data_t *inst, uint8 idx);
const position_t *get_node_position_start_from_idx(instance_data_t *inst,
                                                   const struct TDMAHandler *tdma_handler,
                                                   uint8_t idx,
                                                   uint32_t *node_idx);
const char *get_instanceModes_string(enum instanceModes mode);
const char *get_discovery_modes_string(enum discovery_modes mode);
const char *get_inst_states_string(enum inst_states state);

bool instance_store_neigbor_position(instance_data_t *inst, uwb_msg_info_t *p_msg);

//###########################################################################################################
// END OF instance_utilities.h
//###########################################################################################################
#endif
