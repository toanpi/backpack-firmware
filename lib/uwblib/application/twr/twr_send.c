/************************************************************************************************************
Module:       twr_send

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/13/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "twr_send.h"
#include "instance.h"
#include "uwb_transfer.h"
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
  twr_poll_send()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Send a poll to anchor node.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/20/2022
*********************************************************************************/
bool twr_poll_send(instance_data_t *inst, uint32 *p_timeout)
{
  bool ret = false;
  struct TDMAHandler *tdma_handler = (struct TDMAHandler *)get_tdma_handler();
  uint32 psduLength = POLL_FRAME_LEN_BYTES;

  inst->msg.seqNum = inst->frameSN++;
  inst->msg.messageData[FCODE] = RTLS_DEMO_MSG_TAG_POLL; // message function code (specifies if message is a poll, response or other...)

  memcpy(&inst->msg.destAddr[0], &inst->uwbList[inst->uwbToRangeWith], inst->addrByteSize);

  tdma_handler->nthOldest++;

  inst->wait4ack = 0;

  ret = uwb_send_msg((uint8 *)&inst->msg, psduLength, DWT_START_TX_IMMEDIATE | inst->wait4ack, 0);

  inst->tx_poll = ret;

  if (ret)
  {
    inst->timeofTxPoll = timestamp_get_ms();

    if (inst->uwbToRangeWith < inst->uwbListLen)
    {
      tdma_handler->uwbListTDMAInfo[inst->uwbToRangeWith].lastRange = timestamp_get_ms();
    }

    if (p_timeout)
    {
      *p_timeout = inst->durationPollTxDoneTimeout_ms;
    }
  }

  return ret;
}

/********************************************************************************
Function:
  twr_report_send()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Send a report to tag node.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/20/2022
*********************************************************************************/
bool twr_report_send(instance_data_t *inst, uint32 *p_timeout)
{
  bool ret = false;

  int psduLength = REPORT_FRAME_LEN_BYTES;

  // Write calculated TOF into response message
  memcpy(&inst->report_msg.messageData[REPORT_TOF], &inst->tof[inst->uwbToRangeWith], 6);
  memcpy(&inst->report_msg.messageData[REPORT_RSL], &inst->rxPWR, sizeof(double));
  memcpy(&inst->report_msg.messageData[REPORT_ADDR], &inst->uwbList[inst->uwbToRangeWith], inst->addrByteSize);
  inst->report_msg.seqNum = inst->frameSN++;

  inst->wait4ack = 0;

  ret = uwb_send_msg((uint8 *)&inst->report_msg, psduLength, DWT_START_RX_IMMEDIATE, 0);

  if (ret && p_timeout)
  {
    *p_timeout = inst->durationReportTxDoneTimeout_ms;
  }

  return ret;
}

/********************************************************************************
Function:
  twr_response_send()
Input Parameters:
  ---
Output Parameters:
  Send a response to tag node.
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 07/20/2022
*********************************************************************************/
bool twr_response_send(instance_data_t *inst, uint32 *p_timeout)
{
  bool ret = false;


  return ret;
}

/********************************************************************************
Function:
  twr_final_send()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Send a final to anchor node.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/20/2022
*********************************************************************************/
bool twr_final_send(instance_data_t *inst, uint32 *p_timeout)
{
  bool ret = false;


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
//      END OF twr_send.c
//###########################################################################################################
