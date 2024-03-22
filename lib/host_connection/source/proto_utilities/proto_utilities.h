/************************************************************************************************************
Module:       proto_utilities

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
04/15/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _PROTO_UTILITIES_H
#define _PROTO_UTILITIES_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <string.h>
#include <pb_encode.h>
#include <pb_decode.h>




//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################



//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
typedef struct
{
  uint8_t *buffer;
  size_t size;
} pb_buffer_t;

//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
uint32_t proto_encode_msg(uint8_t *p_buf, uint16_t buf_size, const pb_msgdesc_t *fields, const void *src_struct);
uint32_t proto_encode_delimited_msg(uint8_t *p_buf, uint16_t buf_size, const pb_msgdesc_t *fields, const void *src_struct);
bool proto_decode_msg(uint8_t *buffer, uint32_t message_length, const pb_msgdesc_t *fields, void *dest_struct);
bool proto_write_bytes(pb_ostream_t *ostream, const pb_field_t *field, void *const *arg);
bool proto_write_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
bool proto_write_buffer(pb_ostream_t *ostream, const pb_field_t *field, void *const *arg);

//###########################################################################################################
// END OF proto_utilities.h
//###########################################################################################################
#endif
