/************************************************************************************************************
Module:       net_node

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
#include "network_mac.h"
#include "net_node.h"
#include "instance.h"
#include "uwb_dev_driver.h"
#include "system_dbg.h"
#include "uwb_transfer.h"
#include "timestamp.h"
#include "app_main.h"


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
	net_node_joined_network()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Procedure for node to join network.
Notes:
	---
Author, Date:
	Toan Huynh, 07/21/2022
*********************************************************************************/
bool net_node_joined_network(instance_data_t *inst, struct TDMAHandler *tdma_handler, uwb_msg_info_t *p_msg)
{
	bool ret = false;
	uint32 time_now = timestamp_get_ms();

	/* process the INF packet */
	if (p_msg)
	{
		uint8 srcIndex = instgetuwblistindex(inst, &p_msg->srcAddr[0], inst->addrByteSize);
		uint8 framelength = 0;
		uint64 timeSinceFrameStart_us = 0;

		memcpy(&framelength, &p_msg->p_data[TDMA_FRAMELENGTH], sizeof(uint8));
		memcpy(&timeSinceFrameStart_us, &p_msg->p_data[TDMA_TSFS], 6);

		// synchronize the frames
		tdma_handler->frame_sync(tdma_handler, p_msg->dw_event, framelength, timeSinceFrameStart_us, srcIndex, FS_ADOPT);
		// copy the TDMA network configuration directly
		tdma_handler->process_inf_msg(tdma_handler, p_msg->p_data, srcIndex, CLEAR_ALL_COPY);
		// copy new TDMA configuration into the INF message that this UWB will send out
		tdma_handler->populate_inf_msg(tdma_handler, RTLS_DEMO_MSG_INF_UPDATE);
	}

	// set discovery mode to EXIT
	tdma_handler->set_discovery_mode(tdma_handler, EXIT, time_now);

	inst->mode = ANCHOR;

	ret = true;

	return ret;
}
/********************************************************************************
Function:
	net_node_setup_network()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 07/20/2022
*********************************************************************************/
bool net_node_setup_network(instance_data_t *inst)
{
	bool ret = false;

	dwt_forcetrxoff();

	dwt_enableframefilter(DWT_FF_DATA_EN | DWT_FF_ACK_EN | DWT_FF_RSVD_EN);
	inst->frameFilteringEnabled = 1;
	dwt_setpanid(inst->panID);
	dwt_seteui(inst->eui64);

	// seed random number generator with our 64-bit address
	uint64 seed = 0;
	seed |= (uint64)inst->eui64[0];
	seed |= (uint64)inst->eui64[1] << 8;
	seed |= (uint64)inst->eui64[2] << 16;
	seed |= (uint64)inst->eui64[3] << 24;
	seed |= (uint64)inst->eui64[4] << 32;
	seed |= (uint64)inst->eui64[5] << 40;
	seed |= (uint64)inst->eui64[6] << 48;
	seed |= (uint64)inst->eui64[7] << 56;
	srand(seed);

	inst->uwbShortAdd = inst->eui64[0] + (inst->eui64[1] << 8); // NOTE a hashing algorithm could be used instead

#if (USING_64BIT_ADDR == 0)
	dwt_setaddress16(inst->uwbShortAdd);
	memcpy(&inst->uwbList[0][0], &inst->uwbShortAdd, inst->addrByteSize);
#else
	memcpy(&inst->uwbList[0][0], &inst->eui64, inst->addrByteSize);
#endif

	net_config_frame_header(inst);
	net_config_messages(inst);

	/* Config device to low power mode */
	instance_config_sleep(inst);

	dwt_setrxtimeout(0);

	ret = true;

	return ret;
}

/********************************************************************************
Function:
	net_node_blink_send()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Send a blink message to the network
Notes:
	---
Author, Date:
	Toan Huynh, 07/20/2022
*********************************************************************************/
bool net_node_blink_send(instance_data_t *inst, uint32 *p_timeout)
{
	bool ret = false;
	struct TDMAHandler *tdma_handler = (struct TDMAHandler *)get_tdma_handler();
	uint32 psduLength = BLINK_FRAME_LEN_BYTES;

	/* Blink frames with IEEE EUI-64 tag ID */
	inst->blinkmsg.seqNum = inst->frameSN++;
	inst->wait4ack = 0;

	ret = uwb_send_msg((uint8 *)&inst->blinkmsg, psduLength, DWT_START_TX_IMMEDIATE | inst->wait4ack, 0);

	if (ret)
	{

		tdma_handler->last_blink_time = timestamp_get_ms();
		tdma_handler->blinkPeriodRand = (uint32)rand() % BLINK_PERIOD_RAND_MS;

		sys_printf("[%u][TX] SEND BLINK\r\n", tdma_handler->last_blink_time);

		if (p_timeout)
		{
			*p_timeout = inst->durationBlinkTxDoneTimeout_ms;
		}
	}

	return ret;
}
/********************************************************************************
Function:
	net_node_ranging_init_send()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Send a ranging init message to the network
Notes:
	---
Author, Date:
	Toan Huynh, 07/20/2022
*********************************************************************************/
bool net_node_ranging_init_send(instance_data_t *inst, uint32 *p_timeout)
{
	bool ret = false;
	const struct TDMAHandler *tdma_handler = get_tdma_handler();
	uint32 psduLength = RNG_INIT_FRAME_LEN_BYTES;

	inst->rng_initmsg.seqNum = inst->frameSN++;

	inst->wait4ack = 0;

	/* Add a small random number to this to reduce chance of collisions */
	uint8 sys_time_arr[5] = {0, 0, 0, 0, 0};
	dwt_readsystime(sys_time_arr);
	uint64 dwt_time_now = 0;
	dwt_time_now = (uint64)sys_time_arr[0] + ((uint64)sys_time_arr[1] << 8) + ((uint64)sys_time_arr[2] << 16) + ((uint64)sys_time_arr[3] << 24) + ((uint64)sys_time_arr[4] << 32);
	inst->delayedReplyTime = (dwt_time_now + inst->rnginitReplyDelay + convert_microsec_to_device_time_u64(rand() % RANGE_INIT_RAND_US)) >> 8; // time we should send the blink response

	ret = uwb_send_msg((uint8 *)&inst->rng_initmsg, psduLength, DWT_START_TX_DELAYED | inst->wait4ack, inst->delayedReplyTime);

	if (ret)
	{
		uint32 time_now = timestamp_get_ms();

		tdma_handler->set_discovery_mode((struct TDMAHandler *)tdma_handler, WAIT_INF_INIT, time_now);

		sys_printf("[%u][TX] SEND RNG_INIT\r\n", time_now);

		if (p_timeout)
		{
			*p_timeout = inst->durationRngInitTxDoneTimeout_ms;
		}
	}
	else
	{
		inst->lateTX++;
	}

	return ret;
}

/********************************************************************************
Function:
	net_node_inf_send()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Send a inf message to the network
Notes:
	---
Author, Date:
	Toan Huynh, 07/20/2022
*********************************************************************************/
bool net_node_inf_send(instance_data_t *inst, uint32 *p_timeout)
{
	bool ret = false;
	struct TDMAHandler *tdma_handler = (struct TDMAHandler *)get_tdma_handler();

	// NOTE: handles INF_SUG, INF_INIT, INF_UPDATE, and INF_REG
	uint32 psduLength = tdma_handler->infMessageLength;

	inst->inf_msg.seqNum = inst->frameSN++;
	// update time since frame start!
	tdma_handler->update_inf_tsfs(tdma_handler);

	inst->wait4ack = 0;

	ret = uwb_send_msg((uint8 *)&inst->inf_msg, psduLength, DWT_START_TX_IMMEDIATE | inst->wait4ack, 0);

	if (ret)
	{
		if (p_timeout)
		{
			*p_timeout = uwb_get_tx_timeout_ms(5000, psduLength);
		}
	}
	else
	{
		// get the message FCODE
		uint8 fcode;
		memcpy(&fcode, &inst->inf_msg.messageData[FCODE], sizeof(uint8));

		if (fcode == RTLS_DEMO_MSG_INF_SUG)
		{
			tdma_handler->set_discovery_mode(tdma_handler, WAIT_SEND_SUG, portGetTickCnt());
		}
	}

	return ret;
}

/********************************************************************************
Function:
	net_node_exit_sleep()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Node exits from sleep mode
Notes:
	---
Author, Date:
	Toan Huynh, 07/20/2022
*********************************************************************************/
bool net_node_exit_sleep(instance_data_t *inst)
{
	bool ret = true;

	/* Exit from sleep mode */
	ret = instance_exit_sleep(inst);

	/* This will update the antenna delay if it has changed */
	instancesetantennadelays();

	return ret;
}

/********************************************************************************
Function:
	net_node_frame_sync()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Sync frame with the network
Notes:
	---
Author, Date:
	Toan Huynh, 07/20/2022
*********************************************************************************/
bool net_node_frame_sync(instance_data_t *inst, struct TDMAHandler *tdma_handler, uwb_msg_info_t *p_msg)
{
	bool ret = false;
	uint8 srcIndex = instgetuwblistindex(inst, &p_msg->srcAddr[0], inst->addrByteSize);
	uint8 framelength;
	uint64 timeSinceFrameStart_us = 0;

	memcpy(&framelength, &p_msg->p_data[SYNC_FRAMELENGTH], sizeof(uint8));
	memcpy(&timeSinceFrameStart_us, &p_msg->p_data[SYNC_TSFS], 6);

	if (inst->mode == ANCHOR || inst->mode == TAG)
	{
		// evaluate our frame synchronization to see if we need to snap to the incoming value
		// and rebroadcast a SYNC message
		tdma_handler->frame_sync(tdma_handler, p_msg->dw_event, framelength, timeSinceFrameStart_us, srcIndex, FS_EVAL);
	}

	ret = true;

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
//      END OF net_node.c
//###########################################################################################################
