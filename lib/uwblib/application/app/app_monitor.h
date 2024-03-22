/************************************************************************************************************
Module:       app_monitor

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
08/24/2023    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2023 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _APP_MONITOR_H
#define _APP_MONITOR_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <stdint.h>
#include <stdbool.h>
#include "application_definitions.h"
#include "instance.h"

//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################
#define NUM_OF_RANGING_NEIHBOR          (4)


//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################


typedef struct
{
    uint8 index;
    uint64 addr;
    position_t pos;
    double distance;
    double distance_rsl;
    app_uwb_err error;
} app_ranged_node_info_t;

typedef void (*app_monitor_change_cb)(void);


//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
bool app_monitor_add_ranging_node(int node_index);
bool app_monitor_update_ranging_info(void);
const app_ranged_node_info_t *get_last_ranged_info(uint32_t *num_devs);
bool app_monitor_register_change(app_monitor_change_cb func);
bool app_monitor_callback(void);
bool app_monitor_ranging_end(void);
bool app_monitor_ranging_start(void);
bool app_monitor_callback(void);



//###########################################################################################################
// END OF app_monitor.h
//###########################################################################################################
#endif
