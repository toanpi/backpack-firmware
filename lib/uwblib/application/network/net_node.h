/************************************************************************************************************
Module:       net_node

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/20/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _NET_NODE_H
#define _NET_NODE_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <stdio.h>
#include <stdbool.h>
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
bool net_node_setup_network(instance_data_t *inst);
bool net_node_blink_send(instance_data_t *inst, uint32 *p_timeout);
bool net_node_ranging_init_send(instance_data_t *inst, uint32 *p_timeout);
bool net_node_inf_send(instance_data_t *inst, uint32 *p_timeout);
bool net_node_exit_sleep(instance_data_t *inst);
bool net_node_frame_sync(instance_data_t *inst, struct TDMAHandler *tdma_handler, uwb_msg_info_t *p_msg);
bool net_node_joined_network(instance_data_t *inst, struct TDMAHandler *tdma_handler, uwb_msg_info_t *p_msg);



//###########################################################################################################
// END OF net_node.h
//###########################################################################################################
#endif
