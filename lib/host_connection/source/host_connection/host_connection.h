/************************************************************************************************************
Module:       host_connection.h

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
04/12/2022    TH        Began Coding    (TH = Toan Huynh)


COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.

************************************************************************************************************/
#ifndef _HOST_CONNECTION_H
#define _HOST_CONNECTION_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <stdint.h>
#include <stdbool.h>
#include <string.h>




//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################
#define HOST_ADDR          (0x00)



//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
/* Error code */
typedef enum
{
  HOST_CONNECTION_SUCCESS = 0,
  HOST_CONNECTION_ERR,
  HOST_CONNECTION_PARAM_INVALID,
  HOST_CONNECTION_INIT_ERR,
} host_connection_err_e;

/* Host connection operation mode */
typedef enum
{
  HOST_CONNECTION_MODE_LOG = 0,
  HOST_CONNECTION_MODE_PROTOBUF,
} host_connection_mode_e;

typedef bool (*send_data_f)(uint8_t * p_data, uint32_t len);
typedef bool (*encode_f)(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len);
typedef bool (*decode_f) (uint8_t *p_data, uint16_t data_len);

//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
uint32_t host_connection_init(send_data_f send_data_func, host_connection_mode_e mode);
uint32_t host_connection_send(void * p_data, uint32_t len, encode_f encode_func);
uint32_t host_connection_parse(uint8_t * p_data, uint32_t len, decode_f decode_func);
uint32_t host_connection_encode(uint8_t *p_buf, uint32_t buf_len, void *p_data, uint32_t data_len, encode_f encode_func);
bool host_connection_write(uint8_t *p_data, uint32_t data_len);
bool host_connection_forward(uint8_t *p_data, uint32_t data_len, bool update_src_addr, uint32_t src_addr);
bool host_connection_process(uint8_t *p_data, uint32_t len, decode_f decode_func, send_data_f write);

//###########################################################################################################
// END OF host_connection.h
//###########################################################################################################
#endif
