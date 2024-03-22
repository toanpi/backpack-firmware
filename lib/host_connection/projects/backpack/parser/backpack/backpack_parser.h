/************************************************************************************************************
Module:       backpack_parser

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

#ifndef _BACKPACK_PARSER_H
#define _BACKPACK_PARSER_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "backpack.pb.h"


//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################



//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
typedef bool (*backpack_decode_f)(void *p_msg);



//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
bool backpack_process_msg(uint8_t *p_buf, uint16_t buf_size);
bool backpack_encode_uwb_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_uwb_message);
bool backpack_encode_ble_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_uwb_message);
void backpack_decode_register(pb_size_t type, backpack_decode_f callback);
bool backpack_encode_dev_info_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, uint32_t prototype, uint64_t address);
bool backpack_encode_sensor_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message);
bool backpack_encode_collector_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message);
bool backpack_encode_file_transfer_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message);
bool backpack_encode_algo_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message);
bool backpack_encode_cmd_msg(uint8_t *p_buf, uint16_t buf_size, uint32_t *p_written_len, const void *p_message);

//###########################################################################################################
// END OF backpack_parser.h
//###########################################################################################################
#endif
