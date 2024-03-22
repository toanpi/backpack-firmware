/************************************************************************************************************
Module:       host_com

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
04/13/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _HOST_COM_H
#define _HOST_COM_H

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
typedef enum
{
  HOST_COM_MODE_LOG = 0,
  HOST_COM_MODE_ENCODE,
} host_com_mode_e;


typedef void (*host_com_interval_callback)(void);

//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
void host_com_init(uint32_t stack_size, uint16_t priority, host_com_mode_e mode);
void host_com_data_ready_signal(void);
void host_com_register_interval_callback(uint32_t interval, host_com_interval_callback callback);


//###########################################################################################################
// END OF host_com.h
//###########################################################################################################
#endif

