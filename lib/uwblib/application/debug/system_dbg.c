/************************************************************************************************************
Module:       system_dbg

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
03/02/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "system_dbg.h"
#include "instance.h"


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
#if DEEP_DEBUG_ENABLE
test_t test[TEST_SIZE];
uint32 test_idx = 0;
#endif



//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
#if DEEP_DEBUG_ENABLE
/********************************************************************************
Function:
  updateLogDbg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 03/02/2022
*********************************************************************************/
uint32_t updateLogDbg(
    test_t *p_test,
    test_t *p_pre_test,
    instance_data_t *inst,
    struct TDMAHandler *p_tdma,
    event_data_t *p_newevent,
    tx_state_t tx_state,
    rx_state_t rx_state,
    int timeout,
    int slot,
    uint64 time_now_us)
{
  /* Dw event */
  p_test->dw_event = DWT_UNKNOWN;
  if (p_newevent)
  {
    p_test->dw_event = p_newevent->type;
  }

  /* Instance */
  if (p_pre_test)
  {
    p_test->mode = p_pre_test->mode;
    p_test->testAppState = p_pre_test->testAppState;
    p_test->tx_state = p_pre_test->tx_state;
    p_test->rx_state = p_pre_test->rx_state;
    p_test->discovery_mode = p_pre_test->discovery_mode;
    p_test->lastSlotStartTime64 = p_pre_test->lastSlotStartTime64;
    p_test->timeout = p_pre_test->timeout;
  }

  if (inst)
  {
    p_test->mode = inst->mode;
    p_test->testAppState = inst->testAppState;
  }

  /* Tx/RX state */
  if (tx_state >= 0)
  {
    p_test->tx_state = tx_state;
  }

  if (rx_state >= 0)
  {
    p_test->rx_state = rx_state;
  }
  /* Timeout */
  if (timeout >= 0)
  {
    p_test->timeout = timeout;
  }

  /* TDMAHandler */
  if (p_tdma)
  {
    p_test->discovery_mode = p_tdma->discovery_mode;
    p_test->lastSlotStartTime64 = p_tdma->lastSlotStartTime64;
  }

  if (time_now_us == 0)
  {
    p_test->diffFromSlotStart = p_test->time * 1000 - p_test->lastSlotStartTime64;
  }
  else
  {
    p_test->diffFromSlotStart = time_now_us - p_test->lastSlotStartTime64;
  }

  p_test->slot = 0xff;
  if (slot >= 0)
  {
    p_test->slot = slot;
  }

  return 0;
}

#endif

#if SNIFF_DATA_ENABLE
uint32_t sniff_idx = 0;
sniff_data_t sniff_data[200];
#endif

void log_sniff_slot_data(instance_data_t *inst, dw_event_e event, uint64_t lastSlotStartTime64)
{
#if SNIFF_DATA_ENABLE
  sniff_data[sniff_idx].timestamp = portGetTickCnt();
  // sniff_data[sniff_idx].timestamp = portGetTickCntMicro();
  sniff_data[sniff_idx].mode = inst->mode;
  sniff_data[sniff_idx].event = event;
  sniff_data[sniff_idx].lastSlotStartTime64 = lastSlotStartTime64;
  // sniff_data[sniff_idx].src_address = src_address;
  // sniff_data[sniff_idx].des_address = des_address;
  // sniff_data[sniff_idx].data_len = data_len;
  // sniff_data[sniff_idx].fcode = fcode;

  sniff_idx++;
  if (sniff_idx >= sizeof(sniff_data) / sizeof(sniff_data_t))
  {
    sniff_idx = 0;
  }
#endif
}

void log_sniff_data(instance_data_t *inst, dw_event_e event, uint32_t src_address, uint32_t des_address, uint32_t data_len, dw_msg_fcode_t fcode)
{
#if SNIFF_DATA_ENABLE
  sniff_data[sniff_idx].timestamp = portGetTickCnt();
  // sniff_data[sniff_idx].timestamp = portGetTickCntMicro();
  sniff_data[sniff_idx].mode = inst->mode;
  sniff_data[sniff_idx].event = event;
  sniff_data[sniff_idx].src_address = src_address;
  sniff_data[sniff_idx].des_address = des_address;
  sniff_data[sniff_idx].data_len = data_len;
  sniff_data[sniff_idx].fcode = fcode;

  sniff_idx++;
  if (sniff_idx >= sizeof(sniff_data) / sizeof(sniff_data_t))
  {
    sniff_idx = 0;
  }
#endif
}

void log_sniff_rx_data(instance_data_t *inst, event_data_t *p_dw_event, uint8_t fcode_index, uint8_t srcAddr_index)
{
  // return;

#if SNIFF_DATA_ENABLE
  log_sniff_data(inst,
                 p_dw_event->type,
                 *(uint16_t *)&p_dw_event->msgu.frame[srcAddr_index],
                 0,
                 p_dw_event->rxLength,
                 p_dw_event->msgu.frame[fcode_index]);
#endif
};

void log_sniff_tx_data(instance_data_t *inst)
{
  // return;
#if SNIFF_DATA_ENABLE
  if (inst->tx_poll || inst->tx_anch_resp || inst->tx_final)
  {
    log_sniff_data(inst,
                   DWT_SIG_TX_DONE,
                   *(uint16_t *)inst->msg.sourceAddr,
                   *(uint16_t *)inst->msg.destAddr,
                   0,
                   inst->msg.messageData[FCODE]);
  }
  else if (inst->previousState == TA_TXBLINK_WAIT_SEND)
  {
    log_sniff_data(inst,
                   DWT_SIG_TX_BLINK,
                   *(uint16_t *)inst->msg.sourceAddr,
                   *(uint16_t *)inst->msg.destAddr,
                   0,
                   inst->msg.messageData[FCODE]);
  }
  else if (inst->previousState == TA_TXRANGINGINIT_WAIT_SEND)
  {
    log_sniff_data(inst,
                   DWT_SIG_TX_RANG_INIT,
                   *(uint16_t *)inst->rng_initmsg.sourceAddr,
                   *(uint16_t *)inst->rng_initmsg.destAddr,
                   0,
                   inst->rng_initmsg.messageData[FCODE]);
  }
  else if (inst->previousState == TA_TXINF_WAIT_SEND)
  {
    log_sniff_data(inst,
                   DWT_SIG_TX_DONE,
                   *(uint16_t *)inst->inf_msg.sourceAddr,
                   *(uint16_t *)inst->inf_msg.destAddr,
                   0,
                   inst->inf_msg.messageData[FCODE]);
  }
  else if (inst->previousState == TA_TXREPORT_WAIT_SEND)
  {
    log_sniff_data(inst,
                   DWT_SIG_TX_DONE,
                   *(uint16_t *)inst->report_msg.sourceAddr,
                   *(uint16_t *)inst->report_msg.destAddr,
                   0,
                   inst->report_msg.messageData[FCODE]);
  }
  else
  {
    log_sniff_data(inst, DWT_SIG_TX_DONE, 0, 0, 0, 0);
  }

#endif
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
//      END OF system_dbg.c
//###########################################################################################################
