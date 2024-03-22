/************************************************************************************************************
Module:       uwb_dev_isr

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

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "uwb_dev_isr.h"
#include "compiler.h"
#include "port_mcu.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_spi.h"
#include "instance.h"
#include "system_dbg.h"
#include "dev_cfg.h"
#include "uwb_dev_setting.h"
#include "app_main.h"
#include "timestamp.h"
#include "uwb_event.h"
#include "uwb_dev_driver.h"
#include "app_state.h"
#include "uwb_transfer.h"



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
extern instance_data_t instance_data[NUM_INST];
extern struct TDMAHandler tdma_handler;

//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  instance_irqstuckcallback()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void instance_irqstuckcallback()
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);

	port_DisableEXT_IRQ();			// disable IRQ until we configure the device
	port_set_dw1000_slowrate(); // max SPI before PLLs configured is ~4M

	dwt_softreset();

	// reset the DW1000 by driving the RSTn line low
	reset_DW1000();

	// we can enable any configuration loading from OTP/ROM
	dwt_initialise(DWT_LOADUCODE);

#if (CONFIG_ENABLE_LOW_POWER_MODE == 0)
	dwt_setleds(3); // configure the GPIOs which control the leds on EVBs
#endif

	// enable TX, RX states on GPIOs 6 and 5
	dwt_setlnapamode(1, 1);

	port_set_dw1000_fastrate();

	dwt_setinterrupt(SYS_MASK_VAL, 1);
	dwt_setcallbacks(instance_txcallback, instance_rxgoodcallback, instance_rxtimeoutcallback, instance_rxerrorcallback, instance_irqstuckcallback);

	instance_config(inst); // Set operating channel etc
	port_EnableEXT_IRQ();											 // enable IRQ before starting

	inst_processtxrxtimeout(inst);
}

/********************************************************************************
Function:
  instance_txcallback()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void instance_txcallback(const dwt_cb_data_t *txd)
{
	uint8 txTimeStamp[5] = {0, 0, 0, 0, 0};
	event_data_t dw_event;

	// NOTE: we can only get TX good (done) while here
	dwt_readtxtimestamp(txTimeStamp);
	dw_event.timeStamp32l = (uint32)txTimeStamp[0] + ((uint32)txTimeStamp[1] << 8) + ((uint32)txTimeStamp[2] << 16) + ((uint32)txTimeStamp[3] << 24);
	dw_event.timeStamp = txTimeStamp[4];
	dw_event.timeStamp <<= 32;
	dw_event.timeStamp += dw_event.timeStamp32l;
	dw_event.timeStamp32h = ((uint32)txTimeStamp[4] << 24) + (dw_event.timeStamp32l >> 8);

	dw_event.rxLength = 0;
	dw_event.typeSave = dw_event.type = DWT_SIG_TX_DONE;

	// NOTE: to avoid timestamping issues reported in the forums, we aren't using the DW1000 RX auto re-enable function.
	// Rather, we turn it back on here after every TX. However, logic in TA_RXE_WAIT needs wait4ack to be set
	// to function correctly. We don't want it to try to start RX during reception.
#ifdef AM_PART_APOLLO3
	/* RX isn't working if we reset it here */
#else
	dwt_forcetrxoff();
	dwt_rxreset();
#endif
	instance_data[0].wait4ack = DWT_RESPONSE_EXPECTED;
	instance_rxon(&instance_data[0], 0, 0); // turn RX on, without delay

	//******************************************************************************
	//   SNIFFER
	//******************************************************************************
	log_sniff_tx_data(&instance_data[0]);
	//******************************************************************************


	if (instance_data[0].tx_poll == TRUE)
	{
		instance_data[0].tx_poll = FALSE;

		// Write Poll TX time field of Final message
		memcpy(&(instance_data[0].msg.messageData[PTXT]), (uint8 *)&dw_event.timeStamp, 5);
	}
	else if (instance_data[0].tx_anch_resp == TRUE)
	{
		instance_data[0].tx_anch_resp = FALSE;
		instance_data[0].anchorRespTxTime = dw_event.timeStamp;
	}
	else if (instance_data[0].tx_final == TRUE)
	{
		instance_data[0].tx_final = FALSE;
	}

	instance_putevent(dw_event);
}

/********************************************************************************
Function:
  instance_rxtimeoutcallback()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void instance_rxtimeoutcallback(const dwt_cb_data_t *rxd)
{
	event_data_t dw_event;

	dw_event.typeSave = dw_event.type = DWT_SIG_RX_TIMEOUT;
	dw_event.rxLength = 0;
	dw_event.timeStamp = 0;
	dw_event.timeStamp32l = 0;
	dw_event.timeStamp32h = 0;

	//******************************************************************************
	//   SNIFFER
	//******************************************************************************
	log_sniff_data(&instance_data[0], dw_event.type, 0, 0, 0, 0);
	//******************************************************************************

	instance_putevent(dw_event);
}

/********************************************************************************
Function:
  instance_rxerrorcallback()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void instance_rxerrorcallback(const dwt_cb_data_t *rxd)
{
	int instance = 0;

	// error caused by automatic frame filtering, ignore it and immediately turn rx back on
	if (rxd->status & SYS_STATUS_AFFREJ)
	{
		instance_rxon(&instance_data[instance], 0, 0);
		return;
	}

	if (tdma_handler.discovery_mode == WAIT_RNG_INIT || tdma_handler.discovery_mode == WAIT_INF_INIT)
	{
		uint32 time_now = timestamp_get_ms();
		tdma_handler.set_discovery_mode(&tdma_handler, WAIT_INF_REG, time_now);
	}

	event_data_t dw_event;
	// re-enable the receiver
	// for ranging application rx error frame is same as TO - as we are not going to get the expected frame
	if (instance_data[instance].mode == TAG || instance_data[instance].mode == DISCOVERY)
	{
		dw_event.type = DWT_SIG_RX_TIMEOUT;
		dw_event.typeSave = 0x40 | DWT_SIG_RX_TIMEOUT;
		dw_event.rxLength = 0;

		instance_putevent(dw_event);

		//******************************************************************************
		//   SNIFFER
		//******************************************************************************
		log_sniff_data(&instance_data[0], dw_event.type, 0, 0, 0, 0);
		//******************************************************************************
	}
	else
	{
		instance_data[instance].uwbToRangeWith = 255;

		instance_rxon(&instance_data[instance], 0, 0); // immediate enable if anchor
	}
}
/********************************************************************************
Function:
  instance_rxgoodcallback()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void instance_rxgoodcallback(const dwt_cb_data_t *rxd)
{
	int instance = 0;
	uint8 rxTimeStamp[5] = {0, 0, 0, 0, 0};
	uint8 srcAddr_index = 0;
	uint8 rxd_event = 0;
	uint8 fcode_index = 0;
	event_data_t dw_event;

	uint32 time_now = timestamp_get_ms();

	// if we got a frame with a good CRC - RX OK
	rxd_event = DWT_SIG_RX_OKAY;

	dw_event.rxLength = rxd->datalength;

	// need to process the frame control bytes to figure out what type of frame we have received
	switch (rxd->fctrl[0])
	{
	// blink type frame
	case 0xC5:
		if (rxd->datalength == 12)
		{
			rxd_event = DWT_SIG_RX_BLINK;
		}
		else
		{
			rxd_event = SIG_RX_UNKNOWN;
		}

		break;

	// ACK type frame - not supported in this SW - set as unknown (re-enable RX)
	case 0x02:
		rxd_event = SIG_RX_UNKNOWN;
		break;

	// data type frames (with/without ACK request) - assume PIDC is on.
	case 0x41:
	case 0x61:
		// read the frame
		if (rxd->datalength > EXTENDED_FRAME_SIZE)
			rxd_event = SIG_RX_UNKNOWN;

		// need to check the destination/source address mode
		if ((rxd->fctrl[1] & 0xCC) == 0x88) // dest & src short (16 bits)
		{
			fcode_index = FRAME_CRTL_AND_ADDRESS_S; // function code is in first byte after source address
			srcAddr_index = FRAME_CTRLP + ADDR_BYTE_SIZE_S;
		}
		else if ((rxd->fctrl[1] & 0xCC) == 0xCC) // dest & src long (64 bits)
		{
			fcode_index = FRAME_CRTL_AND_ADDRESS_L; // function code is in first byte after source address
			srcAddr_index = FRAME_CTRLP + ADDR_BYTE_SIZE_L;
		}
		else // using one short/one long
		{
			fcode_index = FRAME_CRTL_AND_ADDRESS_LS; // function code is in first byte after source address

			if (((rxd->fctrl[1] & 0xCC) == 0x8C)) // source short
			{
				srcAddr_index = FRAME_CTRLP + ADDR_BYTE_SIZE_L;
			}
			else
			{
				srcAddr_index = FRAME_CTRLP + ADDR_BYTE_SIZE_S;
			}
		}

		break;

	// any other frame types are not supported by this application
	default:
		rxd_event = SIG_RX_UNKNOWN;
		break;
	}

	// read rx timestamp
	if ((rxd_event == DWT_SIG_RX_BLINK) || (rxd_event == DWT_SIG_RX_OKAY))
	{
		dwt_readrxtimestamp(rxTimeStamp);
		dw_event.timeStamp32l = (uint32)rxTimeStamp[0] + ((uint32)rxTimeStamp[1] << 8) + ((uint32)rxTimeStamp[2] << 16) + ((uint32)rxTimeStamp[3] << 24);
		dw_event.timeStamp = rxTimeStamp[4];
		dw_event.timeStamp <<= 32;
		dw_event.timeStamp += dw_event.timeStamp32l;
		dw_event.timeStamp32h = ((uint32)rxTimeStamp[4] << 24) + (dw_event.timeStamp32l >> 8);

		dwt_readrxdata((uint8 *)&dw_event.msgu.frame[0], rxd->datalength, 0); // Read Data Frame
	}

	dw_event.typeSave = dw_event.type = rxd_event;

	//******************************************************************************
	//   SNIFFER
	//******************************************************************************
	log_sniff_rx_data(&instance_data[instance], &dw_event, fcode_index, srcAddr_index);

	//----------------------------------------------------------------------------------------------
	// TWR - here we check if we need to respond to a TWR Poll or Response Messages
	//----------------------------------------------------------------------------------------------

	// don't process unknown signals or non-broadcast messages that aren't addressed to this UWB
	if (rxd_event == DWT_SIG_RX_OKAY)
	{
		if ((dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_RNG_INIT) ||
				(dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_TAG_POLL) ||
				(dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_ANCH_RESP) ||
				(dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_TAG_FINAL))
		{
			uint8 destAddr_index = FRAME_CTRLP;

			if (memcmp(&instance_data[instance].uwbList[0][0], &dw_event.msgu.frame[destAddr_index], instance_data[instance].addrByteSize) != 0)
			{
				rxd_event = SIG_RX_UNKNOWN;
			}
		}
		else if (dw_event.msgu.frame[fcode_index] != RTLS_DEMO_MSG_INF_REG &&
						 dw_event.msgu.frame[fcode_index] != RTLS_DEMO_MSG_INF_INIT &&
						 dw_event.msgu.frame[fcode_index] != RTLS_DEMO_MSG_INF_SUG &&
						 dw_event.msgu.frame[fcode_index] != RTLS_DEMO_MSG_INF_UPDATE &&
						 dw_event.msgu.frame[fcode_index] != RTLS_DEMO_MSG_RNG_REPORT &&
						 dw_event.msgu.frame[fcode_index] != RTLS_HOST_MSG &&
						 dw_event.msgu.frame[fcode_index] != RTLS_DEMO_MSG_SYNC)
		{
			rxd_event = SIG_RX_UNKNOWN;
		}
	}

	// first check if the address is already tracked.
	// if not, add it.
	// then get the index for that address
	uint8 uwb_index = 255;
	if (rxd_event == DWT_SIG_RX_BLINK)
	{
		uint8 blink_address[8] = {0, 0, 0, 0, 0, 0, 0, 0};
#if (USING_64BIT_ADDR == 0)
		memcpy(&blink_address, &dw_event.msgu.rxblinkmsg.tagID[0], instance_data[instance].addrByteSize);
#else
		memcpy(&blink_address, &dw_event.msgu.rxblinkmsg.tagID[0], instance_data[instance].addrByteSize);
#endif

		// must be a neighbor
		uwb_index = instgetuwblistindex(&instance_data[instance], &blink_address[0], instance_data[instance].addrByteSize);

		if (uwb_index > 0 && uwb_index < UWB_LIST_SIZE)
		{
			tdma_handler.uwbListTDMAInfo[uwb_index].connectionType = UWB_LIST_NEIGHBOR;
			tdma_handler.uwbListTDMAInfo[uwb_index].lastCommNeighbor = time_now;
		}
	}
	else if (rxd_event == DWT_SIG_RX_OKAY)
	{
		// must be a neighbor
		uwb_index = instgetuwblistindex(&instance_data[instance], &dw_event.msgu.frame[srcAddr_index], instance_data[instance].addrByteSize);

		if (uwb_index > 0 && uwb_index < UWB_LIST_SIZE)
		{
			tdma_handler.uwbListTDMAInfo[uwb_index].connectionType = UWB_LIST_NEIGHBOR;
			tdma_handler.uwbListTDMAInfo[uwb_index].lastCommNeighbor = time_now;
		}
	}

	bool accept_inf = FALSE;
	// check if the incoming message indicates that we should range with the source UWB or just accept and process the message
	if (rxd_event != SIG_RX_UNKNOWN)
	{
		if (instance_data[instance].uwbToRangeWith == 255)
		{
			if (rxd_event == DWT_SIG_RX_OKAY)
			{
				if (instance_data[instance].mode == ANCHOR)
				{
					if (dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_TAG_POLL)
					{
						instance_data[instance].uwbToRangeWith = uwb_index;
					}
					else if (dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_INF_REG || dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_INF_SUG || dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_INF_UPDATE) // only anchor if already received INF_INIT or collected regular INF messages and joined with SUG message
					{
						accept_inf = TRUE;
					}
				}
				else if (instance_data[instance].mode == DISCOVERY)
				{
					if (dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_RNG_INIT)
					{
						if (tdma_handler.discovery_mode == WAIT_RNG_INIT)
						{
							instance_data[instance].uwbToRangeWith = uwb_index;
						}
					}
					else if (dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_INF_INIT)
					{
						// only accept if we are waiting for INF_INIT
						if (tdma_handler.discovery_mode == WAIT_INF_INIT)
						{
							instance_data[instance].uwbToRangeWith = uwb_index;
							accept_inf = TRUE;
						}
					}
					else if (dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_INF_REG ||
									 dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_INF_UPDATE ||
									 dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_INF_SUG)
					{
						if (tdma_handler.discovery_mode == WAIT_INF_REG || tdma_handler.discovery_mode == COLLECT_INF_REG || tdma_handler.discovery_mode == WAIT_INF_INIT)
						{
							accept_inf = TRUE;
						}
					}
				}
			}
			else if (rxd_event == DWT_SIG_RX_BLINK)
			{
				// only accept BLINK if in DISCOVERY mode and waiting for INF_REG
				if (instance_data[instance].mode == DISCOVERY)
				{
					if (tdma_handler.discovery_mode == WAIT_INF_REG)
					{
						instance_data[instance].uwbToRangeWith = uwb_index;
					}
				}
			}
		}
	}

	int place_event = 0;

	if (rxd_event == DWT_SIG_RX_OKAY && uwb_index != 255)
	{
		// always accept.
		if (dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_RNG_REPORT || 
				dw_event.msgu.frame[fcode_index] == RTLS_HOST_MSG || 
				dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_SYNC)
		{
			place_event = 1;
		}
	}

	if (accept_inf == TRUE)
	{
		place_event = 1;
	}
	else if (uwb_index != 255 && instance_data[instance].uwbToRangeWith == uwb_index)
	{
		if (rxd_event == DWT_SIG_RX_OKAY)
		{
			// BUG: Should response if only in anchor mode ?
			// if (dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_TAG_POLL && instance_data[instance].mode == ANCHOR)
			if (dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_TAG_POLL)
			{
				int psduLength = RESP_FRAME_LEN_BYTES;

				// Poll's Rx time
				instance_data[instance].tagPollRxTime = dw_event.timeStamp;																																	 
				// message function code (specifies if message is a poll, response or other...)
				instance_data[instance].msg.messageData[FCODE] = RTLS_DEMO_MSG_ANCH_RESP;																										 

#if ENABLE_ANCHOR_POSITION			
				/* Add position of tag to message */
				memcpy(&instance_data[instance].msg.messageData[POLL_POSITION], &instance_data[instance].selfPosition, sizeof(instance_data[instance].selfPosition));
#endif

				memcpy(&instance_data[instance].msg.destAddr[0], &dw_event.msgu.frame[srcAddr_index], instance_data[instance].addrByteSize); // remember who to send the reply to (set destination address)
				instance_data[instance].msg.seqNum = instance_data[instance].frameSN++;

				instance_data[0].wait4ack = 0;

				uint32 reply_time = ((dw_event.timeStamp + instance_data[instance].respReplyDelay) & MASK_TXDTS) >> 8;

				bool ret = uwb_send_msg((uint8 *)&instance_data[instance].msg,
																psduLength,
																DWT_START_TX_DELAYED | instance_data[instance].wait4ack,
																reply_time);

				if(!ret)
				{
					sys_printf("[ERR][TX] ANCH RESP\r\n");
					instance_data[instance].tx_anch_resp = FALSE;
					app_state_rx_wait(&instance_data[instance]);
				}
				else
				{
					instance_data[instance].tx_anch_resp = TRUE;
					/* Exit this interrupt and notify the application/instance that TX is in progress. */
					dw_event.typePend = DWT_SIG_TX_PENDING; 
					instance_data[instance].timeofTx = time_now;

					instance_data[instance].txDoneTimeoutDuration = instance_data[instance].durationRespTxDoneTimeout_ms;
					instance_data[instance].canPrintLCD = FALSE;
				}
			}
			// else if (dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_ANCH_RESP && instance_data[instance].mode == TAG)
			else if (dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_ANCH_RESP)
			{
				// if we have received a response, it's too late in the slot to try and poll with someone else in case we dont finish the exchange
				// however, if the full exchange doesn't complete, we still want to broadcast an INF message
				// set a flag here to indicate that we are past the point of no return
				tdma_handler.firstPollResponse = TRUE;

				// Embed into Final message: 40-bit pollTXTime,  40-bit respRxTime,  40-bit finalTxTime
				uint64 tagCalculatedFinalTxTime; // time we should send the response
				uint64 finalReplyDelay = instance_data[0].respReplyDelay;
				tagCalculatedFinalTxTime = (dw_event.timeStamp + finalReplyDelay) & MASK_TXDTS;
				instance_data[0].delayedReplyTime = tagCalculatedFinalTxTime >> 8;

				// Calculate Time Final message will be sent and write this field of Final message
				// Sending time will be delayedReplyTime, snapped to ~125MHz or ~250MHz boundary by
				// zeroing its low 9 bits, and then having the TX antenna delay added
				// getting antenna delay from the device and add it to the Calculated TX Time
				tagCalculatedFinalTxTime = tagCalculatedFinalTxTime + instance_data[0].txAntennaDelay;
				tagCalculatedFinalTxTime &= MASK_40BIT;

				// Write Calculated TX time field of Final message
				memcpy(&(instance_data[0].msg.messageData[FTXT]), (uint8 *)&tagCalculatedFinalTxTime, 5);

				// process RTLS_DEMO_MSG_ANCH_RESP immediately.
				int psduLength = FINAL_FRAME_LEN_BYTES;

				instance_data[instance].anchorRespRxTime = dw_event.timeStamp; // Response's Rx time
				// Embbed into Final message:40-bit respRxTime
				// Write Response RX time field of Final message
				memcpy(&(instance_data[instance].msg.messageData[RRXT]), (uint8 *)&instance_data[instance].anchorRespRxTime, 5);
				instance_data[instance].msg.messageData[FCODE] = RTLS_DEMO_MSG_TAG_FINAL; // message function code (specifies if message is a poll, response or other...)

				instance_data[0].wait4ack = 0;

				bool ret = uwb_send_msg((uint8 *)&instance_data[instance].msg,
																psduLength,
																DWT_START_TX_DELAYED | instance_data[instance].wait4ack,
																instance_data[instance].delayedReplyTime);

				if(!ret)
				{
					sys_printf("[ERR][TX] FINAL\r\n");
					instance_data[instance].tx_final = FALSE;
					instance_data[instance].lateTX++;
					app_state_rx_wait(&instance_data[instance]);
				}
				else
				{
					instance_data[instance].tx_final = TRUE;
					/* Exit this interrupt and notify the application/instance that TX is in progress.*/
					dw_event.typePend = DWT_SIG_TX_PENDING; 
					instance_data[instance].timeofTx = time_now;
					instance_data[instance].timeofTxFinal = timestamp_get_ms();
					instance_data[instance].txDoneTimeoutDuration = instance_data[instance].durationFinalTxDoneTimeout_ms;
				}
			}
			// else if (dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_TAG_FINAL && instance_data[instance].mode == ANCHOR)
			else if (dw_event.msgu.frame[fcode_index] == RTLS_DEMO_MSG_TAG_FINAL)
			{
				dwt_rxdiag_t dwt_diag;
				dwt_readdiagnostics(&dwt_diag);

				uint8 RXPACC_NOSAT_OFFSET = 0x2C;
				uint16 RXPACC_NOSAT = dwt_read16bitoffsetreg(DRX_CONF_ID, RXPACC_NOSAT_OFFSET);

				// following adjustment from function Adjust_RXPACC in following link
				// https://github.com/damaki/DW1000/blob/00da81fce9c11c2632c5776add0629971b1d5ba6/src/dw1000-reception_quality.adb
				if (RXPACC_NOSAT == dwt_diag.rxPreamCount)
				{
					uint8 rxpacc_adj = 0;
					instance_data[instance].acc_adj = 1;

					if (instance_data[instance].configData.nsSFD == TRUE)
					{
						if (instance_data[instance].configData.dataRate == DWT_BR_110K)
						{
							rxpacc_adj = 82;
						}
						if (instance_data[instance].configData.dataRate == DWT_BR_850K)
						{
							rxpacc_adj = 18;
						}
						else // DWT_BR_6M8
						{
							rxpacc_adj = 10;
						}
					}
					else
					{
						if (instance_data[instance].configData.dataRate == DWT_BR_110K)
						{
							rxpacc_adj = 64;
						}
						else // DWT_BR_850K or DWT_BR_6M8
						{
							rxpacc_adj = 5;
						}
					}

					if (rxpacc_adj <= dwt_diag.rxPreamCount)
					{
						dwt_diag.rxPreamCount -= rxpacc_adj;
					}
				}
				else
				{
					instance_data[instance].acc_adj = 0;
				}

				// 10 log (C * 2^17 / N^2) - A
				double C = (double)dwt_diag.maxGrowthCIR;
				if (C <= 0.0)
				{
					C = 1;
				}

				double Nsquared = pow(dwt_diag.rxPreamCount, 2);
				if (Nsquared <= 0.0)
				{
					Nsquared = 1;
				}

				double A = 0;
				if (instance_data[instance].configData.prf == DWT_PRF_16M)
				{
					A = 113.77;
				}
				else // DWT_PRF_64M
				{
					A = 121.74;
				}

				instance_data[instance].rxPWR = (double)(10.0 * log10(C * pow(2, 17) / Nsquared)) - A;

				instance_data[instance].dwt_final_rx = dw_event.timeStamp;
			}

			place_event = 1;
		}
		else if (rxd_event == DWT_SIG_RX_BLINK)
		{
			place_event = 1;
		}
	}

	if (place_event)
	{
		instance_putevent(dw_event);
	}
	else
	{
		instance_rxon(&instance_data[instance], 0, 0); // immediately reenable RX
	}
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
//      END OF uwb_dev_isr.c
//###########################################################################################################
