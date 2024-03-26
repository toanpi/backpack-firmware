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

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "ranging_test.h"
#include <stdlib.h>
#include "instance.h"
#include "instance_utilities.h"
#include "host_msg.h"


//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################



//###########################################################################################################
//      MACROS
//###########################################################################################################



//###########################################################################################################
//      MODULE TYPES
//###########################################################################################################



//###########################################################################################################
//      CONSTANTS
//###########################################################################################################



//###########################################################################################################
//      MODULE LEVEL VARIABLES
//###########################################################################################################
static testNodeInfo *neighborTestInfo = NULL;
static uint32_t numNeighborTest = 0;
static uint32_t nodeTestIdx = 0;



//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 08/25/2023
*********************************************************************************/
bool ranging_test_enter(testNodeInfo *p_nodes, uint32_t num)
{
  numNeighborTest = num;
  neighborTestInfo = p_nodes;

  /* Transfer ranging info to host */
  host_msg_config_ranging_transfer(true);

  return true;
}
/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 08/25/2023
*********************************************************************************/
bool ranging_test_exit(void)
{
  numNeighborTest = 0;
  neighborTestInfo = NULL;

  /* Need to reset */
  device_reset_mcu();

  return true;
}


/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 08/25/2023
*********************************************************************************/
bool ranging_test_in_test(void)
{
  return numNeighborTest > 0 && neighborTestInfo;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 08/25/2023
*********************************************************************************/
int ranging_test_get_node_idx(void)
{
  if(!ranging_test_in_test())
  {
    return -1;
  }

  instance_data_t *inst = instance_get_local_structure_ptr(0);
  int uwb_idx = -1;

  if(nodeTestIdx == numNeighborTest)
  {
    nodeTestIdx = 0;
  }

  uint16_t addr = neighborTestInfo[nodeTestIdx++].address;

  for (int i = 1; i < inst->uwbListLen; i++) // 0 reserved for self
  {
		uint16_t shortAddr = *(uint16_t *)&inst->uwbList[i];

    if(addr == shortAddr)
    {
      uwb_idx = i;
      break;
    }
  }

  return uwb_idx;
}
//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################


//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################




//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF ranging_test.c
//###########################################################################################################