/************************************************************************************************************
Module:       host_com_msg

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

#ifndef _HOST_COM_MSG_H
#define _HOST_COM_MSG_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <stdint.h>
#include <stdbool.h>
#include "instance.h"
#include "host_com_task.h"


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
bool host_com_send_dev_info(uint8_t *data, uint16_t len);
bool host_com_send_dev_config(uint8_t *data, uint16_t len);
bool host_com_send_net_info(uint8_t *data, uint16_t len);
bool host_com_write(uint8_t *data, uint8_t len);
bool host_com_forward(uint8_t *p_data, uint32 data_len);
uint32 host_com_encode_net_info(uint8_t *p_buf, uint32 buf_len, uint8_t *data, uint16_t len);
bool host_com_send_dev_calib(uint8_t *data, uint16_t len);
bool host_com_send_ranging_info(void);
bool host_com_send_neighbor_list(void);
bool host_com_send_host_setup(void);
bool host_com_send_test_msg_res(uint32_t arg);
void host_com_print_log(const char *format, ...);

//###########################################################################################################
// END OF host_com_msg.h
//###########################################################################################################
#endif
