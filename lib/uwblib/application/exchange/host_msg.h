/************************************************************************************************************
Module:       host_msg

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

#ifndef _HOST_MSG_H
#define _HOST_MSG_H

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

typedef struct {
  bool location;
  bool ranging;
} transfer_control_host_msg;



//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
bool inst_send_host_msg(instance_data_t *inst, uint32 *p_timeout);
void send_device_state_info(void);
void send_dev_config(void);
void send_device_net_info(void);
void send_dev_calib_info(void);
bool host_msg_config_set(transfer_control_host_msg *cfg);
transfer_control_host_msg * host_msg_config_get(void);
void send_device_raning_info(void);
bool host_msg_config_ranging_transfer(bool enable_rang_info);
void host_msg_send_neighbor_list(void);
bool inst_net_send_msg(instance_data_t *inst, uint8_t *p_data, uint32_t data_len, uint16_t des_addr);
bool inst_net_broadcast_msg(instance_data_t *inst, uint8_t *p_data, uint32_t data_len);
bool host_msg_process_host_msg(uint32_t node_addr,uint8_t *p_data, uint32_t len);
bool host_msg_forward_msg_to_node(uint32_t node_addr);
bool host_msg_find_msg(uint32_t node_addr);
bool host_msg_parse_msgs(instance_data_t *inst, uint32_t src_addr, uint8_t *data, uint32_t len);
void host_msg_send_host_setup(void);
void host_msg_send_test_msg_res(uint32_t arg);
int host_msg_send_node_msg(uint32_t node_addr, uint8_t* p_data, uint32_t len);



//###########################################################################################################
// END OF host_msg.h
//###########################################################################################################
#endif
