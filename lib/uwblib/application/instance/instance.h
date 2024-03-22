/*! ----------------------------------------------------------------------------
 *  @file    instance.h
 *  @brief   DecaWave header for application level instance
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */
#ifndef INSTANCE_H_
#define INSTANCE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "application_definitions.h"
#include "port_mcu.h"
#include "deca_types.h"
#include "deca_device_api.h"
#include "tdma_handler.h"
#include "tx_power.h"

	typedef struct
	{

		INST_MODE mode;						 // instance mode (tag or anchor)
		INST_STATES testAppState;	 // state machine - current state
		INST_STATES nextState;		 // state machine - next state
		INST_STATES previousState; // state machine - previous state

		// configuration structures
		dwt_config_t configData;		// DW1000 channel configuration
		dwt_txconfig_t configTX;		// DW1000 TX power configuration
		uint16 txAntennaDelay;			// DW1000 TX antenna delay
		uint16 rxAntennaDelay;			// DW1000 RX antenna delay
		uint16 defaultAntennaDelay; // reasonable estimate of DW1000 TX/RX antenna delay

		uint8 antennaDelayChanged;
		// "MAC" features
		uint8 frameFilteringEnabled; // frame filtering is enabled

		bool canPrintLCD;

		// timeouts and delays
		// this is the delay used for the delayed transmit (when sending the ranging init, response, and final messages)
		uint64 rnginitReplyDelay;
		uint64 finalReplyDelay;
		uint64 finalReplyDelay_us;
		uint64 respReplyDelay;

		// Receive Frame Wait Timeout Periods, units are 1.0256us (nus stands for near microsecond)
		uint16 durationPollTimeout_nus; // rx timeout duration after tx poll
		uint16 durationPollTimeout_ms;	// rx timeout duration after tx poll
		uint16 durationFinalTimeout_ms; // rx timeout duration after tx final

		uint32 durationBlinkTxDoneTimeout_ms;		// tx done timeout after tx blink
		uint32 durationRngInitTxDoneTimeout_ms; // tx done timeout after tx rng_init
		uint32 durationPollTxDoneTimeout_ms;		// tx done timeout after tx poll
		uint32 durationRespTxDoneTimeout_ms;		// tx done timeout after tx resp
		uint32 durationFinalTxDoneTimeout_ms;		// tx done timeout after tx final
		uint32 durationReportTxDoneTimeout_ms;	// tx done timeout after tx report
		uint32 durationSyncTxDoneTimeout_ms;		// tx done timeout after tx sync

		uint32 durationUwbCommTimeout_ms; // how long to wait before changing UWB_LIST_TYPE if we haven't communicated with or received an information about a UWB in a while

		uint32 durationWaitRangeInit_ms; // discovery mode WAIT_RNG_INIT duration

		uint64 durationSlotMax_us; // longest anticipated time required for a slot based on UWB_LIST_SIZE and S1 switch settings

		uint32 delayedReplyTime; // delayed reply time of delayed TX message - high 32 bits

		// Pre-computed frame lengths for frames involved in the ranging process,
		// in microseconds.
		uint32 frameLengths_us[FRAME_TYPE_NB];
		uint32 storedPreLen;		// precomputed conversion of preamble and sfd in DW1000 time units
		uint64 storedPreLen_us; // precomputed conversion of preamble and sfd in microseconds

// message structures used for transmitted messages
#if (USING_64BIT_ADDR == 1)
		srd_msg_dlsl rng_initmsg;		 // ranging init message (destination long, source long)
		srd_ext_msg_dlsl msg;				 // simple 802.15.4 frame structure (used for tx message) - using long addresses
		srd_ext_msg_dssl inf_msg;		 // extended inf message containing frame lengths and slot assignments
		srd_ext_msg_dssl report_msg; // extended report message containing the calculated range
		srd_ext_msg_dssl sync_msg;	 // extended message indicating the need to resync TDMA frame
		srd_ext_msg_dsss host_msg;	 // extended message for host 
#else
	srd_msg_dlss rng_initmsg;		 // ranging init message (destination long, source short)
	srd_ext_msg_dsss msg;				 // simple 802.15.4 frame structure (used for tx message) - using short addresses
	srd_ext_msg_dsss inf_msg;		 // extended inf message containing frame lengths and slot assignments
	srd_ext_msg_dsss report_msg; // extended report message containing the calculated range
	srd_ext_msg_dsss sync_msg;	 // extended message indicating the need to resync TDMA frame
	srd_ext_msg_dsss host_msg;	 // extended message for host 
#endif

		iso_IEEE_EUI64_blink_msg blinkmsg; // frame structure (used for tx blink message)

		// Tag function address/message configuration
		uint8 eui64[8];			// devices EUI 64-bit address
		uint16 uwbShortAdd; // UWB's short address (16-bit) used when USING_64BIT_ADDR == 0
		uint8 frameSN;			// modulo 256 frame sequence number - it is incremented for each new frame transmittion
		uint16 panID;				// panid used in the frames

		uint8 addrByteSize; // The bytelength used for addresses.

		uint32 resp_dly_us[RESP_DLY_NB];

		// 64 bit timestamps
		uint64 anchorRespTxTime; // anchor's reponse tx timestamp
		uint64 anchorRespRxTime; // receive time of response message
		uint64 tagPollRxTime;		 // receive time of poll message
		uint64 dwt_final_rx;		 // receive time of the final message

		// application control parameters
		uint8 wait4ack; // if this is set to DWT_RESPONSE_EXPECTED, then the receiver will turn on automatically after TX completion
		bool gotTO;		// got timeout event

		// diagnostic counters/data, results and logging
		int64 tof[UWB_LIST_SIZE];
		double clockOffset;

		// RSL reporting
		uint8 rslCnt;
		uint8 idxRSL;
		// double RSL[NUM_RSL_AVG]; // Not in use
		double avgRSL;

		// counts for debug
		int txmsgcount;
		int rxmsgcount;
		int lateTX;
		int lateRX;

		uint8 newRangeUWBIndex; // index for most recent ranging exchange
		int newRange;
		uint64 newRangeAncAddress; // anchor address for most recent ranging exchange
		uint64 newRangeTagAddress; // tag address for most recent ranging exchange

		double idistance[UWB_LIST_SIZE];
		app_uwb_err error[UWB_LIST_SIZE];

		double idistancersl[UWB_LIST_SIZE];
		// double idistanceraw[UWB_LIST_SIZE]; // Not in use
		// double iRSL[UWB_LIST_SIZE]; // Not in use
		position_t iPosition[UWB_LIST_SIZE];

		uint8 uwbToRangeWith;						 // it is the index of the uwbList array which contains the address of the UWB we are ranging with
		uint8 uwbListLen;								 // Number of UWB devices in the uwbList array
		uint8 uwbList[UWB_LIST_SIZE][8]; // index 0 reserved for self, rest for other tracked uwbs

		uint32 timeofTx;							// used to calculate tx done callback timeouts
		uint32 timeofTxPoll;					// used to calculate rx timeout after poll
		uint32 timeofTxFinal;					// used to calculate rx timeout after final
		uint32 txDoneTimeoutDuration; // duration used for tx done callback timeouts. (set differently for each tx message)

		bool tx_poll;			 // was the last tx message a POLL message?
		bool tx_anch_resp; // was the last tx message an ANCH_RESP message?
		bool tx_final;		 // was the last tx message a FINAL message?

		// event queue - used to store DW1000 events as they are processed by the dw_isr/callback functions
		event_data_t dwevent[MAX_EVENT_NUMBER]; // this holds any TX/RX events and associated message data
		uint8 dweventIdxOut;
		uint8 dweventIdxIn;
		uint8 dweventPeek;

		uint8 smartPowerEn;

		uint32 rxCheckOnTime;

		double rxPWR;
		uint8 acc_adj;
		position_t selfPosition;

		instanceConfig_t chConfig;

		operation_mode_e operationMode; // If true, the device is always a tag (use for backpack)
		uint32 rangCnt; // how many times have we waited for a ranging exchange.
		uint32 rangingIntervalMs; // Range interval in ms

		rx_detect_threshold_e rxDetectThreshold;

		// Low power mode
		bool goToSleep;			 // If set the instance will go to sleep before sending the blink/poll message
		uint32 timeofSleep;	 // used to calculate sleep done callback timeouts
		uint32 sleepTime_ms; // The time to sleep for
		bool lowPowerEnable; // If set the instance will go to deep sleep before sending the blink/poll message
		bool isInDeepSleep; // If set the instance is in deep sleep

		/* Smoothing algorithm */
		bool enSmoothingAlgo;
		tx_power_percent_t txPower;

		/* If device is linked to host for communication */
		bool linkedToHost;

	} instance_data_t;

	typedef struct
	{
		uint64 addr;
		position_t pos;
		double distance;
		double distance_rsl;
		app_uwb_err error;
	} ranged_node_info_t;

	// clear the status/ranging data
	void instanceclearcounts(void);
	void instclearuwblist(void);
	int instfindnumneighbors(instance_data_t *inst);
	int instfindnumhidden(instance_data_t *inst);
	int instgetuwblistindex(instance_data_t *inst, uint8 *uwbAddr, uint8 addrByteSize);

	// Call init, then call config, then call run. call close when finished
	// initialise the instance (application) structures and DW1000 device
	int instance_init(void);
	int instance_init_s();
	int tdma_init_s(uint64 slot_duration);

	void instancesetreplydelay(int datalength);

	// Pre-compute frame lengths, timeouts and delays needed in ranging process.
	// /!\ This function assumes that there is no user payload in the frame.
	void instance_init_timings(void);

	double instance_get_idist(uint8 uwb_index);
	double instance_get_idistrsl(uint8 uwb_index);

	// configure the antenna delays
	void instanceconfigantennadelays(uint16 tx, uint16 rx);
	void instancesetantennadelays(void);

	int instance_starttxtest(int framePeriod);

	instance_data_t *instance_get_local_structure_ptr(unsigned int x);

	void dw_app_signal(void);

	uint64 get_frame_duration_ms(struct TDMAHandler *this);

	const struct TDMAHandler *get_tdma_handler(void);

	uint32 inst_get_neighbors_list(instance_data_t *inst, uint32_t *buf, uint32_t max_cnt);

#ifdef __cplusplus
}
#endif

#endif
