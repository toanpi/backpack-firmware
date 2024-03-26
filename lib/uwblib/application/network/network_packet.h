/************************************************************************************************************
Module:       network_packet

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/20/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _NETWORK_PACKET_H
#define _NETWORK_PACKET_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include "instance.h"
#include "app_main.h"


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
bool net_packet_process_inf(instance_data_t *inst, struct TDMAHandler *tdma_handler, uwb_msg_info_t *p_msg);


//###########################################################################################################
// END OF network_packet.h
//###########################################################################################################
#endif