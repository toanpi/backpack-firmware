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

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "network_packet.h"
#include "timestamp.h"



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
  net_packet_process_inf()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Processes an incoming INF message.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/20/2022
*********************************************************************************/
bool net_packet_process_inf(instance_data_t *inst, struct TDMAHandler *tdma_handler, uwb_msg_info_t *p_msg)
{
  bool ret = false;

  uint32 time_now = timestamp_get_ms();
  uint8 srcIndex = instgetuwblistindex(inst, &p_msg->srcAddr[0], inst->addrByteSize);

  uint8 framelength;
  uint64 timeSinceFrameStart_us = 0;
  memcpy(&framelength, &p_msg->p_data[TDMA_FRAMELENGTH], sizeof(uint8));
  memcpy(&timeSinceFrameStart_us, &p_msg->p_data[TDMA_TSFS], 6);

  // return to discovery mode if no slots assigned to this UWB
  if (inst->mode == ANCHOR || inst->mode == TAG)
  {
    if (tdma_handler->uwbListTDMAInfo[0].slotsLength == 0)
    {
      inst->mode = DISCOVERY;
      tdma_handler->set_discovery_mode(tdma_handler, WAIT_INF_REG, time_now);
    }
  }

  if (inst->mode == DISCOVERY)
  {
    // NOTE: RX callback only accepts INF_UPDATE/INF_SUG/INF_REG for discovery modes WAIT_INF_REG and COLLECT_INF_REG.

    // 1.) sync our frame start time to the local network
    // 2.) collect and combine tdma info so we can construct a SUG packet and send it out

    if (tdma_handler->discovery_mode == WAIT_INF_INIT)
    {
      // if we receive network traffic while waiting for INF_INIT, transition to collecting INF messages
      tdma_handler->set_discovery_mode(tdma_handler, COLLECT_INF_REG, time_now);
    }

    if (tdma_handler->discovery_mode == WAIT_INF_REG) // treat INF_UPDATE and INF_SUG the same
    {
      // synchronize the frames
      tdma_handler->frame_sync(tdma_handler, p_msg->dw_event, framelength, timeSinceFrameStart_us, srcIndex, FS_ADOPT);
      // initialize collection of tdma info, clear any previously stored info
      tdma_handler->process_inf_msg(tdma_handler, p_msg->p_data, srcIndex, CLEAR_ALL_COPY);
      // set discovery mode to COLLECT_INF_REG
      tdma_handler->set_discovery_mode(tdma_handler, COLLECT_INF_REG, time_now);
    }
    else if (tdma_handler->discovery_mode == COLLECT_INF_REG)
    {
      // synchronize the frames
      tdma_handler->frame_sync(tdma_handler, p_msg->dw_event, framelength, timeSinceFrameStart_us, srcIndex, FS_COLLECT);
      // collecting tdma info, append to previously stored info
      tdma_handler->process_inf_msg(tdma_handler, p_msg->p_data, srcIndex, COPY);
    }
    else if (tdma_handler->discovery_mode == WAIT_SEND_SUG)
    {
      // process frame sync while waiting to send sug so we maintain syn with selected (sub)network
      // also give ourselves the opportunity to detect the need to transmit frame sync rebase messages
      tdma_handler->frame_sync(tdma_handler, p_msg->dw_event, framelength, timeSinceFrameStart_us, srcIndex, FS_AVERAGE);
    }
  }
  else if (inst->mode == ANCHOR || inst->mode == TAG)
  {
    // if we are a TAG or ANCHOR
    // 1.) sync our frame start time to the local network
    // 2.) check for and adopt any tdma changes, sending an INF_UPDATE or INF_REG accordingly

    // synchronize the frames
    tdma_handler->frame_sync(tdma_handler, p_msg->dw_event, framelength, timeSinceFrameStart_us, srcIndex, FS_AVERAGE);

    // collecting tdma info, append to previously stored info
    bool tdma_modified = tdma_handler->process_inf_msg(tdma_handler, p_msg->p_data, srcIndex, CLEAR_LISTED_COPY);

    if (tdma_modified)
    {
      // only repopulate the INF message if there was a modification to the TDMA configuration
      tdma_handler->populate_inf_msg(tdma_handler, RTLS_DEMO_MSG_INF_UPDATE);
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
//      END OF network_packet.c
//###########################################################################################################
