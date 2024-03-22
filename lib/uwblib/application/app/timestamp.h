/************************************************************************************************************
Module:       timestamp

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/15/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _TIMESTAMP_H
#define _TIMESTAMP_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include "deca_device_api.h"



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
uint32 get_dt32(uint32 t1, uint32 t2);
uint32 timestamp_add32(uint32 timestamp, uint32 duration);
uint32 timestamp_subtract32(uint32 timestamp, uint32 duration);
uint64 get_dt64(uint64 t1, uint64 t2);
uint64 timestamp_add64(uint64 timestamp, uint64 duration);
uint64 timestamp_subtract64(uint64 timestamp, uint64 duration);
uint32 convert_microsec_to_device_time_u32(double microsecu);
uint64 convert_microsec_to_device_time_u64(double microsecu);
double convert_device_time_to_sec(int32 dt);
double convert_device_time_bytes_to_sec(uint8 *dt);
uint32 timestamp_get_ms(void);
uint64 timestamp_get_us(void);


//###########################################################################################################
// END OF timestamp.h
//###########################################################################################################
#endif
