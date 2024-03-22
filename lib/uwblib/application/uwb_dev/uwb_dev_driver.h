/************************************************************************************************************
Module:       uwb_dev_deriver

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

#ifndef _UWB_DEV_DERIVER_H
#define _UWB_DEV_DERIVER_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include "instance.h"



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
void instance_config(instance_data_t *inst);
void instance_read_accumulator_data(void);
bool instance_config_sleep(instance_data_t *inst);
bool instance_enter_sleep(instance_data_t *inst);
bool instance_exit_sleep(instance_data_t *inst);
void instance_rxon(instance_data_t *inst, int delayed, uint64 delayedReceiveTime);
int instance_send_packet(uint16 length, uint8 txmode, uint32 dtime);
uint32 instance_read_device_id(void);

bool uwb_dev_off_transceiver(void);


//###########################################################################################################
// END OF uwb_dev_deriver.h
//###########################################################################################################
#endif
