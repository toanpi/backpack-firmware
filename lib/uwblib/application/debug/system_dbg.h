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

#ifndef _SYSTEM_DBG_H
#define _SYSTEM_DBG_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include "application_definitions.h"
#include "instance.h"

//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################
#define DEEP_DEBUG_ENABLE           (0)
#define SNIFF_DATA_ENABLE           (0)


#define TEST_SIZE  200


#if SNIFF_DATA_ENABLE
typedef struct
{
    uint64_t timestamp;
    INST_MODE mode;
	dw_event_e event;
	uint64_t lastSlotStartTime64;
	uint16_t src_address;
	uint16_t des_address;
	uint32_t data_len;
	dw_msg_fcode_t fcode;
} sniff_data_t;

#endif

#if DEEP_DEBUG_ENABLE
//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
typedef enum {
    TX_NONE,
    TX_TAG_SELECT_SEND,
    TX_SEND_ANCH_RESP,
    TX_SEND_ANCH_RESP_ERROR,
    TX_SEND_TAG_FINAL,
    TX_SEND_TAG_FINAL_ERROR,
    TX_TIME_TO_BLINK,
    TX_SEND_BLINK,
    TX_SEND_BLINK_ERROR,
    TX_SEND_SYNC_MSG,
    TX_SEND_SYNC_MSG_ERROR,
    TX_RANGINGINIT,
    TX_RANGINGINIT_ERROR,
    TX_SEND_INF,
    TX_SEND_INF_ERROR,
    TX_SEND_POLL,
    TX_SEND_POLL_ERROR,
    TX_SEND_REPORT,
    TX_SEND_REPORT_ERROR,
} tx_state_t;

typedef enum {
    RX_UNKNOWN,
    RX_RECEIVIED_ANCH_RESP,
    RX_RECEIVIED_TAG_FINAL,
    RX_RECEIVIED_BLINK,
    RX_RECEIVIED_SYNC,
    RX_RECEIVIED_RANGINGINIT,
    RX_RECEIVIED_INF,
    RX_ANCHOR_WAIT_EVENT,
    RX_DIS_WAIT_EVENT,
    RX_TAG_WAIT_EVENT,
    RX_RECEIVIED_POLL,
    RX_RECEIVIED_REPORT,
    RX_RECEIVIED_UNKNOWN,
    RX_RECEIVIED_TIMEOUT,
    RX_SWITCH_TO_BLINK,
} rx_state_t;

typedef struct
{
    char const *function;
    uint32_t time;
    INST_STATES testAppState;
    dw_event_e dw_event;
    dw_msg_fcode_t dw_event_fcode;
    tx_state_t tx_state;
    rx_state_t rx_state;
    DISCOVERY_MODE discovery_mode;
    uint32 timeout;
    uint16_t line;
    INST_MODE mode;
    // TDMA
    uint64 lastSlotStartTime64;
    uint32 diffFromSlotStart;
    uint8 slot;
} test_t;

extern test_t test[TEST_SIZE];
extern uint32 test_idx;

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
    uint64 time_now_us);

//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################
// #define updateTestWithTxRx(...)
// #define updateTestWithDisMode(...)
// #define updateTestWithTimeout(...)
// #define updateTestWithEvent(...)
// #define updateTestWithMode(...)
// #define updateTestWithSlot(...)

#define updateTestWithTxRx(_inst, _tx_state, _rx_state) updateTest(_inst, NULL, NULL, _tx_state, _rx_state, -1, -1, -1)
#define updateTestWithDisMode(_inst, _tdma) updateTest(_inst, _tdma, NULL, -1, -1, -1, -1, -1)
#define updateTestWithTimeout(_inst, _timeout) updateTest(_inst, NULL, NULL, -1, -1, _timeout, -1, -1)
#define updateTestWithEvent(_dw_event) updateTest(NULL, NULL, _dw_event, -1, -1, -1, -1, -1)
#define updateTestWithMode(_inst) updateTest(_inst, NULL, NULL, -1, -1, -1, -1, -1)
#define updateTestWithSlot(_tdma, _time_now_us, _slot) updateTest(NULL, _tdma, NULL, -1, -1, -1, _slot, _time_now_us)

#define updateTest(_inst, _tdma, _dw_event, _tx_state, _rx_state, _timeout, _slot, _time_now_us)                                                        \
  {                                                                                                                                                     \
    test[test_idx].function = __FUNCTION__;                                                                                                             \
    test[test_idx].time = portGetTickCntMicro();                                                                                                             \
    test[test_idx].line = __LINE__;                                                                                                                     \
    updateLogDbg(&test[test_idx], test_idx ? &test[test_idx - 1] : NULL, _inst, _tdma, _dw_event, _tx_state, _rx_state, _timeout, _slot, _time_now_us); \
    test_idx++;                                                                                                                                         \
    if (test_idx >= TEST_SIZE)                                                                                                                          \
      test_idx = 0;                                                                                                                                     \
  }

#else
#define updateTestWithTxRx(...)
#define updateTestWithDisMode(...)
#define updateTestWithTimeout(...)
#define updateTestWithEvent(...)
#define updateTestWithMode(...)
#define updateTestWithSlot(...)
#define updateTest(...)
#endif



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
void log_sniff_tx_data(instance_data_t *inst);
void log_sniff_rx_data(instance_data_t *inst, event_data_t *p_dw_event, uint8_t fcode_index, uint8_t srcAddr_index);
void log_sniff_data(instance_data_t *inst, dw_event_e event, uint32_t src_address, uint32_t des_address, uint32_t data_len, dw_msg_fcode_t fcode);
void log_sniff_slot_data(instance_data_t *inst, dw_event_e event, uint64_t lastSlotStartTime64);

//###########################################################################################################
// END OF system_dbg.h
//###########################################################################################################
#endif
