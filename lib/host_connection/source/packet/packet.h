/************************************************************************************************************
Module:       packet

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

#ifndef _PACKET_H
#define _PACKET_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <stdint.h>


//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################



//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
typedef struct __attribute__((__packed__))
{
  uint32_t package_len;
  uint32_t crc32;
  uint32_t desAddr;
  uint32_t srcAddr;
} hc_header_t;


typedef struct 
{
  hc_header_t * p_header;
  uint8_t *p_data;
  uint32_t data_len; // is length of data only
  uint32_t total_len; // is sum of header len, data len
  uint32_t package_len; // is sum of header len, data len, footer len
} hc_package_t;


//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
uint32_t packet_get_header_len(void);
uint32_t packet_get_footer_len(void);
uint32_t packet_format_header(uint8_t *p_buf, uint32_t buf_len, uint32_t data_len);
uint32_t packet_format_footer(uint8_t *buf, uint32_t buf_len, uint32_t data_len);
hc_header_t *packet_parse_header(uint8_t *p_data, uint32_t len);
hc_package_t packet_parse_data(uint8_t *p_data, uint32_t len);


//###########################################################################################################
// END OF packet.h
//###########################################################################################################
#endif
