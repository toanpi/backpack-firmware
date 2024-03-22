/************************************************************************************************************
Module:       file_transfer

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
06/16/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _FILE_TRANSFER_H
#define _FILE_TRANSFER_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include "host_connection.h"



//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################



//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
typedef struct
{
  uint8_t file_type;
  uint32_t seq;
  bool is_last_packet;
  uint32_t data_len;
  uint8_t *p_data;
  uint32_t session_id;
  uint8_t error;
  uint32_t width;
  uint32_t height;
} file_transfer_data_t;

typedef struct
{
  uint32_t session_id;
  uint8_t type;
  bool del_when_done;
  bool (*open)(void);
  bool (*close)(void);
  bool (*delete)(void);
  uint32_t (*file_size)(void);
  bool (*read)(uint8_t *p_buf, uint32_t buf_len, uint32_t offset);
  bool (*image_size)(uint32_t *p_width, uint32_t* p_height);
} ft_send_info_t;

typedef struct 
{
 uint32_t chunk_size;
 bool (*send)(uint8_t *data, uint32_t length, encode_f encode_func);
 void (*send_delay)(void);
} ft_send_cfg_t;

//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
void file_transfer_init(void);
bool file_transfer_encode_file(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t data_len, uint32_t *p_written_len);
bool file_transfer_send_file(ft_send_info_t *p_file_info, ft_send_cfg_t *handler);
bool file_transfer_register(ft_send_info_t *p_file, uint32_t fileType);
bool file_transfer_get_file_req(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t data_len, uint32_t *p_written_len);

//###########################################################################################################
// END OF file_transfer.h
//###########################################################################################################
#endif
