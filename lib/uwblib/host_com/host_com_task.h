/************************************************************************************************************
Module:       host_com_task

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

#ifndef _HOST_COM_TASK_H
#define _HOST_COM_TASK_H

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
void host_com_task_init(uint32_t stack_size, uint16_t priority);
bool host_com_send_signal(uint8_t *p_data, uint32_t len);


//###########################################################################################################
// END OF host_com_task.h
//###########################################################################################################
#endif
