/************************************************************************************************************
Module:       ranging_test

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
08/25/2023    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2023 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _RANGING_TEST_H
#define _RANGING_TEST_H

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

/* Test Mode */
typedef struct {
    uint32_t address;
} testNodeInfo;



//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
bool ranging_test_exit(void);
bool ranging_test_enter(testNodeInfo *p_nodes, uint32_t num);
bool ranging_test_in_test(void);
int ranging_test_get_node_idx(void);


//###########################################################################################################
// END OF ranging_test.h
//###########################################################################################################
#endif