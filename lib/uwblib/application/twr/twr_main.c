/************************************************************************************************************
Module:       twr_main

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/21/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "twr_main.h"
#include "distance.h"
#include "timestamp.h"
#include "instance_utilities.h"
#include "network.h"




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



//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  twr_report_distance()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Report the distance.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/21/2022
*********************************************************************************/
bool twr_report_distance(instance_data_t *inst, struct TDMAHandler *tdma_handler, uwb_msg_info_t *p_msg)
{
  bool ret = true;

  /* time-of-flight */
  inst->tof[inst->uwbToRangeWith] = distance_cal_tof(inst, p_msg);

  inst->newRangeUWBIndex = inst->uwbToRangeWith;

  if (reportTOF(inst, inst->newRangeUWBIndex, inst->rxPWR) == 0)
  {
    inst->newRange = 1;
  }

  if (inst->uwbToRangeWith < inst->uwbListLen)
  {
    tdma_handler->uwbListTDMAInfo[inst->uwbToRangeWith].lastRange = timestamp_get_ms();
  }

  inst->newRangeTagAddress = instance_get_uwbaddr(inst->uwbToRangeWith);
  inst->newRangeAncAddress = instance_get_addr();
  inst->delayedReplyTime = 0;

  return ret;
}

/********************************************************************************
Function:
  twr_process_report_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Process the report message.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/21/2022
*********************************************************************************/
bool twr_process_report_msg(instance_data_t *inst,
                            struct TDMAHandler *tdma_handler,
                            uwb_msg_info_t *p_msg,
                            bool *my_report)
{
  bool ret = true;
  uint8 tag_index = instgetuwblistindex(inst, &p_msg->p_data[REPORT_ADDR], inst->addrByteSize);
  uint8 anchor_index = instgetuwblistindex(inst, &p_msg->srcAddr[0], inst->addrByteSize);

  // for now only process if we are the TAG that ranged with the reporting ANCHOR
  inst->tof[anchor_index] = 0;

  // copy previously calculated ToF
  memcpy(&inst->tof[anchor_index], &p_msg->p_data[REPORT_TOF], 6);
  memcpy(&inst->rxPWR, &p_msg->p_data[REPORT_RSL], sizeof(double));

  inst->newRangeAncAddress = instance_get_uwbaddr(anchor_index);
  inst->newRangeTagAddress = instance_get_uwbaddr(tag_index);

  inst->newRangeUWBIndex = anchor_index;

  if (tag_index == 0)
  {
    /* if ToF == 0 - then no new range to report */
    if (inst->tof[inst->newRangeUWBIndex] > 0)
    {
      if (reportTOF(inst, inst->newRangeUWBIndex, inst->rxPWR) == 0)
      {
        inst->newRange = 1;
      }
    }

    /* Store last node ranged with */
    store_last_node_ranged(tdma_handler, inst->newRangeUWBIndex);

    if (my_report)
    {
      *my_report = true;
    }
  }

  return ret;
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
//      END OF twr_main.c
//###########################################################################################################
