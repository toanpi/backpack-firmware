/************************************************************************************************************
Module:       collector_parser

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
05/03/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _ALGO_PARSER_H
#define _ALGO_PARSER_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <stdint.h>
#include <stdbool.h>


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
void algo_parser_init(void);
bool encode_algo_blob_detection_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len);
bool encode_algo_human_detection_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len);

//###########################################################################################################
// END OF collector_parser.h
//###########################################################################################################
#endif
