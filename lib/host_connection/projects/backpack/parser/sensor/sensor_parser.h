/************************************************************************************************************
Module:       dev_msg_parser

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

#ifndef _DEV_MSG_PARSER_H
#define _DEV_MSG_PARSER_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <stdbool.h>
#include <stdint.h>
#include "stimulation.pb.h"



//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################



//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
typedef stimulationPkg_directionControlMsg sti_control_msg_t;



//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
void sensor_parser_init(void);
bool sensor_encode_fusion_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len);
bool sensor_encode_position_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len);
bool sti_encode_state_info_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len);
bool nav_encode_state_info_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len);
bool sensor_encode_dev_info_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len);
void stop_sti(void);
void channel_config(stimulationPkg_directionControlMsg *p_cfg);
bool stimulation_init_param(void);


//###########################################################################################################
// END OF dev_msg_parser.h
//###########################################################################################################
#endif
