/************************************************************************************************************
Module:       network

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
#include "network.h"
#include "timestamp.h"
#include "system_dbg.h"
#include "instance_utilities.h"
#include "lib.h"
#include "app_monitor.h"
#include "ranging_test.h"





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
	network_create()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Create the network.
Notes:
	---
Author, Date:
	Toan Huynh, 07/20/2022
*********************************************************************************/
bool network_create(struct TDMAHandler *tdma_handler, instance_data_t *inst)
{
	bool result = false;
	uint32 time_now = timestamp_get_ms();

	/* Create the network */
	tdma_handler->build_new_network(tdma_handler);
	tdma_handler->populate_inf_msg(tdma_handler, RTLS_DEMO_MSG_INF_INIT);
	tdma_handler->set_discovery_mode(tdma_handler, EXIT, time_now);

	inst->mode = TAG;

	return result;
}

/********************************************************************************
Function:
  build_new_network()
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
void build_new_network(struct TDMAHandler *this)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	uint64 time_now_us = portGetTickCntMicro();
	bool safeAssign = FALSE;

	// clear all tdma information
	this->tdma_free_all_slots(this);

	// build the initial TDMA
	this->uwbListTDMAInfo[0].frameStartTime = timestamp_subtract64(time_now_us, this->slotDuration_us);

	// sys_printf("FRAME START: %u\n", (uint32_t)this->uwbListTDMAInfo[0].frameStartTime);

	this->lastSlotStartTime64 = time_now_us;

	this->uwbListTDMAInfo[0].framelength = (uint8)MIN_FRAMELENGTH;

	if (inst->uwbToRangeWith > 0 && inst->uwbToRangeWith < UWB_LIST_SIZE)
	{
		this->uwbListTDMAInfo[inst->uwbToRangeWith].framelength = (uint8)MIN_FRAMELENGTH;

		this->assign_slot(&this->uwbListTDMAInfo[0], 1, safeAssign);
		this->assign_slot(&this->uwbListTDMAInfo[inst->uwbToRangeWith], 2, safeAssign);
		this->assign_slot(&this->uwbListTDMAInfo[0], 3, safeAssign);
	}
	else
	{
		// Something wrong with the UWB list
	}
}

/********************************************************************************
Function:
	cal_sleep_time()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Return sleep time in ms.
Notes:
	---
Author, Date:
	Toan Huynh, 03/19/2022
*********************************************************************************/
uint32 cal_sleep_time(struct TDMAHandler *this, instance_data_t *inst)
{
	uint64 frameDuration_ms = get_frame_duration_ms(this);
	uint32 sleep_time = 0;

	if (inst->mode == DISCOVERY)
	{
		sleep_time = CONFIG_SLEEP_IN_SEARCHING_MS;
	}
	else
	{
		// Format: [Sleep][wakeup][Sleep Delay][Sleep]...

		/* Default sleep time is frame duration */
		sleep_time = frameDuration_ms;

		/* At this point, sleep time = frame duration * n (with n >= 1) */
		if (inst->rangingIntervalMs >= frameDuration_ms)
		{
			sleep_time = frameDuration_ms * (inst->rangingIntervalMs / frameDuration_ms);
		}

		/* At this point, sleep time reduce a few ms at least half of frame to listerning inf packet */
		if (sleep_time >= MIN(CONFIG_WAKE_UP_BUFFER_BEFORE_RANGING_MS, frameDuration_ms / 2))
		{
			sleep_time -= MIN(CONFIG_WAKE_UP_BUFFER_BEFORE_RANGING_MS, frameDuration_ms / 2);
		}
	}

	// sys_printf("rangingIntervalMs: %u - frameDuration_ms: %u - sleep_time = %u\n",
	// 					 inst->rangingIntervalMs, (uint32)frameDuration_ms, sleep_time);

	return sleep_time;
}

/********************************************************************************
Function:
	time_to_ranging()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Check if it is time to do ranging.
Notes:
	---
Author, Date:
	Toan Huynh, 03/17/2022
*********************************************************************************/
bool time_to_ranging(instance_data_t *inst, struct TDMAHandler *this)
{
	if (inst->operationMode == CONFIG_OPERATION_TAG)
	{
		if (inst->goToSleep)
		{
			/* Ranging now, it's time to range right after wake up */
			return TRUE;
		}
		else
		{
			/* If not sleep, need to count down berfore ranging */
			if (inst->rangCnt > 0)
			{
				inst->rangCnt--;
				if (inst->rangCnt == 0)
				{
					inst->rangCnt = inst->rangingIntervalMs / get_frame_duration_ms(this);
					return TRUE;
				}

				return FALSE;
			}
			else
			{
				inst->rangCnt = inst->rangingIntervalMs / get_frame_duration_ms(this);
				return TRUE;
			}
		}
	}
	else
	{
		/* Always anchor mode should not use ranging */
		return TRUE;
	}
}

/********************************************************************************
Function:
	slot_clear_action_check()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Clear action check.
Notes:
	---
Author, Date:
	Toan Huynh, 03/18/2022
*********************************************************************************/
void slot_clear_action_check(struct TDMAHandler *this)
{
	this->infSentThisSlot = FALSE;
	this->firstPollSentThisSlot = FALSE;
	this->firstPollResponse = FALSE;
	this->firstPollComplete = FALSE;
	this->secondPollSentThisSlot = FALSE;
}

/********************************************************************************
Function:
  tx_select()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Select the TX action.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
bool tx_select(struct TDMAHandler *this, uint32 *p_timeout_ms)
{
	uint32 timeout_ms = 0;
	instance_data_t *inst = instance_get_local_structure_ptr(0);

	uint64 time_now_us = portGetTickCntMicro();

	int uwb_index = 255;

	if (inst->mode == DISCOVERY)
	{
		if (this->discovery_mode == WAIT_INF_REG)
		{
			if (this->check_blink(this))
			{
				// time to blink
				uwb_index = 255;
				this->set_discovery_mode(this, WAIT_RNG_INIT, portGetTickCnt());
			}
			else
			{
				// not time to blink yet, keep waiting for RNG_INIT
				inst->wait4ack = 0;
				inst->testAppState = TA_RXE_WAIT;
				updateTestWithTxRx(inst, -1, -1);

				return TRUE;
			}
		}
		else if (this->discovery_mode == SEND_SUG)
		{
			// get time since slot start and make sure that's greater than delay
			uint64 timeSinceSlotStart = get_dt64(this->lastSlotStartTime64, time_now_us);

			if (timeSinceSlotStart <= this->slotStartDelay_us)
			{
				uwb_index = -1;
				timeout_ms = CEIL_DIV(this->slotStartDelay_us - timeSinceSlotStart, 1000);
			}
			else
			{
				inst->wait4ack = 0;
				inst->testAppState = TA_TXINF_WAIT_SEND; // Start send sug inf after collecting inf from other nodes
				inst->uwbToRangeWith = (uint8)255;

				updateTestWithTxRx(inst, -1, -1);
				return TRUE;
			}
		}
		else
		{
			return FALSE;
		}
	}
	else if (inst->mode == TAG)
	{
		// get time since slot start and make sure that's greater than delay
		uint64 timeSinceSlotStart = get_dt64(this->lastSlotStartTime64, time_now_us);

		// TAG pauses for INF_POLL_DELAY <-added at beginning of slot
		if (timeSinceSlotStart <= this->slotStartDelay_us)
		{
			uwb_index = -1;
			timeout_ms = CEIL_DIV(this->slotStartDelay_us - timeSinceSlotStart, 1000);
		}
		/* In test mode */
		else if (ranging_test_in_test())
		{
			uwb_index = ranging_test_get_node_idx();
		}
		/* In normal mode */
		else
		{

			uint32 timeSinceRange[UWB_LIST_SIZE] = {0}; // 0th entry unused
			uint32 numNeighbors = 0;
			uint32 time_now = portGetTickCnt();

			// get time since range for each neighbor
			for (int i = 1; i < inst->uwbListLen; i++) // 0 reserved for self
			{
				if (this->uwbListTDMAInfo[i].connectionType == UWB_LIST_NEIGHBOR)
				{
					numNeighbors++;
					timeSinceRange[i] = get_dt32(this->uwbListTDMAInfo[i].lastRange, time_now);
				}
			}

			if (this->nthOldest > numNeighbors)
			{
				this->nthOldest = 1;
			}

			// get the nth oldest
			for (int i = 1; i < inst->uwbListLen; i++) // 0 reserved for self
			{
				bool neighbor_is_tag = is_tag(inst, i);

				if (this->uwbListTDMAInfo[i].connectionType == UWB_LIST_NEIGHBOR)
				{
					/* Note that in operation tag mode: Only ranging with neighbor that is anchor */
					if (neighbor_is_tag)
					{
						continue;
					}

					uint8 numOlder = 0;
					for (int j = 1; j < inst->uwbListLen; j++) // 0 reserved for self
					{
						if (i != j && !is_tag(inst, j))
						{
							// Only check with anchor neighbors
							if (this->uwbListTDMAInfo[j].connectionType == UWB_LIST_NEIGHBOR)
							{
								if (timeSinceRange[i] < timeSinceRange[j])
								{
									numOlder++;
								}
							}
						}
					}

					if (numOlder + 1 == this->nthOldest)
					{
						uwb_index = i;
						break;
					}
				}
			}

			if (uwb_index == 255 && inst->uwbListLen > 1)
			{
				uwb_index = 1;
			}
		}

		// Monitor ranging anchor 
		app_monitor_add_ranging_node(uwb_index);
	}
	else
	{
		// ANCHOR shouldn't be in this mode, should be listening for INF and POLL
		inst->testAppState = TA_RXE_WAIT;
		
		updateTestWithTxRx(inst, -1, -1);
		return TRUE;
	}

	if (uwb_index < 1) // set to -1 when waiting for the slotStartDelay to pass
	{
		if (p_timeout_ms)
		{
			*p_timeout_ms = timeout_ms;
		}
		// do nothing
		return FALSE;
	}
	else if (uwb_index > 254)
	{
		inst->testAppState = TA_TXBLINK_WAIT_SEND;
		inst->uwbToRangeWith = (uint8)255;
		updateTestWithTxRx(inst, -1, -1);
	}
	else
	{
		bool needSendPoll = inst->operationMode != CONFIG_OPERATION_ANCHOR;

		if (this->firstPollComplete || this->secondPollSentThisSlot)
		{
			/* Done with an anchor so move to next anchor */
			this->numNodeToRange--;

			if (this->numNodeToRange == 0)
			{
				/* All nodes have been ranged */
				this->numNodeToRange = NUM_NODE_TO_RANGE;
			}
			else
			{
				slot_clear_action_check(this);
			}
		}

		uint8 fcode = RTLS_DEMO_MSG_INF_INIT;
		if (memcmp(&inst->inf_msg.messageData[FCODE], &fcode, sizeof(uint8)) == 0)
		{
			this->infSentThisSlot = TRUE;
			inst->testAppState = TA_TXINF_WAIT_SEND;
			updateTestWithTxRx(inst, -1, -1);
		}
		/* Don't send poll if in anchor mode */
		else if (this->firstPollSentThisSlot == FALSE && needSendPoll)
		{
			this->firstPollSentThisSlot = TRUE;
			inst->testAppState = TA_TXPOLL_WAIT_SEND;
			updateTestWithTxRx(inst, -1, -1);
		}
		else if (this->secondPollSentThisSlot == FALSE &&
						 this->firstPollComplete == FALSE &&
						 this->firstPollResponse == FALSE &&
						 needSendPoll)
		{
			this->secondPollSentThisSlot = TRUE;
			inst->testAppState = TA_TXPOLL_WAIT_SEND;
			updateTestWithTxRx(inst, -1, -1);
		}
		else if (this->infSentThisSlot == FALSE)
		{
			this->infSentThisSlot = TRUE;
			inst->testAppState = TA_TXINF_WAIT_SEND;
			updateTestWithTxRx(inst, -1, -1);
		}
		else
		{
			return TRUE;
		}

		inst->uwbToRangeWith = (uint8)uwb_index;
	}

	return TRUE;
}

/********************************************************************************
Function:
	need_blink()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 02/26/2022
*********************************************************************************/
bool need_blink(struct TDMAHandler *this)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	bool retval = FALSE;

	if (inst->mode == DISCOVERY && this->discovery_mode == WAIT_INF_REG)
	{
		retval = TRUE;
	}

	return retval;
}


/********************************************************************************
Function:
	next_blink_time()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 02/26/2022
*********************************************************************************/
uint32 next_blink_time(struct TDMAHandler *this)
{
	if (need_blink(this))
	{
		uint32 time_now = portGetTickCnt();
		uint32 timeSinceDiscoveryStart = get_dt32(this->discoveryStartTime, time_now);
		if (timeSinceDiscoveryStart > this->waitInfDuration)
		{
			uint32 timeSinceBlink = get_dt32(this->last_blink_time, time_now);

			if (timeSinceBlink > (uint32)BLINK_PERIOD_MS + this->blinkPeriodRand)
			{
				return (uint32)BLINK_PERIOD_MS + this->blinkPeriodRand;
			}
			else
			{
				return (uint32)BLINK_PERIOD_MS + this->blinkPeriodRand - timeSinceBlink;
			}
		}
		else
		{
			return this->waitInfDuration - timeSinceDiscoveryStart;
		}
	}
	else
	{
		return 0;
	}
}
/********************************************************************************
Function:
  check_blink()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Check if the blink period has elapsed.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
bool check_blink(struct TDMAHandler *this)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	bool retval = FALSE;

	if (inst->mode == DISCOVERY && this->discovery_mode == WAIT_INF_REG)
	{
		uint32 time_now = portGetTickCnt();
		uint32 timeSinceDiscoveryStart = get_dt32(this->discoveryStartTime, time_now);
		if (timeSinceDiscoveryStart >= this->waitInfDuration)
		{
			uint32 timeSinceBlink = get_dt32(this->last_blink_time, time_now);
			if (timeSinceBlink >= (uint32)BLINK_PERIOD_MS + this->blinkPeriodRand)
			{
				retval = TRUE;
			}
		}
	}

	return retval;
}

/********************************************************************************
Function:
  ()
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
void update_inf_tsfs(struct TDMAHandler *this)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	uint64 time_now_us = portGetTickCntMicro();
	uint64 timeSinceFrameStart64 = get_dt64(this->uwbListTDMAInfo[0].frameStartTime, time_now_us);

	if (timeSinceFrameStart64 > 10000000)
	{
		timeSinceFrameStart64 = 0;
	}

	memcpy(&inst->inf_msg.messageData[TDMA_TSFS], &timeSinceFrameStart64, 6);
}

/********************************************************************************
Function:
  populate_inf_msg()
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
void populate_inf_msg(struct TDMAHandler *this, uint8 inf_msg_type)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);

	int num_neighbors = instfindnumneighbors(inst);
	int num_hidden = instfindnumhidden(inst);

	// fcode
	int msgDataIndex = FCODE;
	memcpy(&inst->inf_msg.messageData[msgDataIndex], &inf_msg_type, sizeof(uint8));

	// time since frame start
	// populated immediately before being sent

	// number of neighbors
	msgDataIndex = TDMA_NUMN;
	memcpy(&inst->inf_msg.messageData[msgDataIndex], &num_neighbors, sizeof(uint8));

	// number of hidden neighbors
	msgDataIndex = TDMA_NUMH;
	memcpy(&inst->inf_msg.messageData[msgDataIndex], &num_hidden, sizeof(uint8));

	// self framelength
	msgDataIndex = TDMA_FRAMELENGTH;
	memcpy(&inst->inf_msg.messageData[msgDataIndex], &this->uwbListTDMAInfo[0].framelength, sizeof(uint8));

	// self number of slots
	msgDataIndex = TDMA_NUMS;
	memcpy(&inst->inf_msg.messageData[msgDataIndex], &this->uwbListTDMAInfo[0].slotsLength, sizeof(uint8));
	msgDataIndex++;

	// self slot assignments
	for (int s = 0; s < this->uwbListTDMAInfo[0].slotsLength; s++)
	{
		memcpy(&inst->inf_msg.messageData[msgDataIndex], &this->uwbListTDMAInfo[0].slots[s], sizeof(uint8));
		msgDataIndex++;
	}

	// neighbor address, framelength, number of slots, and slot assignments
	for (int i = 1; i < inst->uwbListLen; i++) // slot 0 reserved for self
	{
		if (this->uwbListTDMAInfo[i].connectionType == UWB_LIST_NEIGHBOR)
		{
			struct TDMAInfo *info = &this->uwbListTDMAInfo[i];

			// address
			memcpy(&inst->inf_msg.messageData[msgDataIndex], &inst->uwbList[i][0], inst->addrByteSize);
			msgDataIndex += inst->addrByteSize;

			// framelength
			memcpy(&inst->inf_msg.messageData[msgDataIndex], &info->framelength, sizeof(uint8));
			msgDataIndex++;

			// number of slots
			memcpy(&inst->inf_msg.messageData[msgDataIndex], &info->slotsLength, sizeof(uint8));
			msgDataIndex++;

			// slot assignments
			for (int s = 0; s < info->slotsLength; s++)
			{
				memcpy(&inst->inf_msg.messageData[msgDataIndex], &info->slots[s], sizeof(uint8));
				msgDataIndex++;
			}
		}
	}

	// hidden address, framelength, number of slots, and slot assignments
	for (int i = 1; i < inst->uwbListLen; i++) // slot 0 reserved for self
	{
		if (this->uwbListTDMAInfo[i].connectionType == UWB_LIST_HIDDEN)
		{
			struct TDMAInfo *info = &this->uwbListTDMAInfo[i];

			// address
			memcpy(&inst->inf_msg.messageData[msgDataIndex], &inst->uwbList[i][0], inst->addrByteSize);
			msgDataIndex += inst->addrByteSize;

			// framelength
			memcpy(&inst->inf_msg.messageData[msgDataIndex], &info->framelength, sizeof(uint8));
			msgDataIndex++;

			// number of slots
			memcpy(&inst->inf_msg.messageData[msgDataIndex], &info->slotsLength, sizeof(uint8));
			msgDataIndex++;

			// slot assignments
			for (int s = 0; s < info->slotsLength; s++)
			{
				memcpy(&inst->inf_msg.messageData[msgDataIndex], &info->slots[s], sizeof(uint8));
				msgDataIndex++;
			}
		}
	}

#if (USING_64BIT_ADDR == 1)
	this->infMessageLength = msgDataIndex + FRAME_CRTL_AND_ADDRESS_L + FRAME_CRC;
#else
	this->infMessageLength = msgDataIndex + FRAME_CRTL_AND_ADDRESS_S + FRAME_CRC;
#endif
}

// General procedure for processing INF SUG, INF REG, and INF UPDATE (NOTE: slightly different for each INF_PROCESS_MODE)
// 1. Check for differences with stored assignments
//		(a) exit if none exist
// 2. Drop stored assignments for self, neighbor, hidden, and twice hidden nodes that appear
//	 in the INF message
// 3. Copy all assignments for self, neighbors, hidden, and twice hidden nodes that appear
//	 in the INF message
// 4. Check for conflicts between nodes in the INF message and nodes not in the INF message (excluding self) and deconflict using PDS
// 5. Release self slot assignments and follow PSA
// 6. Send INF message at beginning of allocated slot (handled elsewhere)
// returns TRUE if a change was made to the TDMA assingments, FALSE if invalid message FCODE or process mode or if no TDMA changes made
bool process_inf_msg(struct TDMAHandler *this, uint8 *messageData, uint8 srcIndex, INF_PROCESS_MODE mode)
{
	// NOTE: this function does not handle TDMA deconflict

	if (srcIndex >= UWB_LIST_SIZE)
	{
		return FALSE;
	}

	bool tdma_modified = FALSE;

	uint32 time_now = portGetTickCnt();

	if ((mode != CLEAR_ALL_COPY) &&		 // happens when we creat a new network
			(mode != CLEAR_LISTED_COPY) && // happens most of the time while processing
			(mode != COPY))								 // happens when collecting inf messages
	{
		// only process if valid mode supplied
		return FALSE;
	}

	bool safeAssign = FALSE;
	if (mode == COPY)
	{
		safeAssign = TRUE;
	}

	uint8 inf_msg_type;
	memcpy(&inf_msg_type, &messageData[FCODE], sizeof(uint8));

	if ((inf_msg_type != RTLS_DEMO_MSG_INF_REG) &&
			(inf_msg_type != RTLS_DEMO_MSG_INF_UPDATE) &&
			(inf_msg_type != RTLS_DEMO_MSG_INF_INIT) &&
			(inf_msg_type != RTLS_DEMO_MSG_INF_SUG))
	{
		// only process INF messages
		return FALSE;
	}

	instance_data_t *inst = instance_get_local_structure_ptr(0);
	this->uwbListTDMAInfo[srcIndex].connectionType = UWB_LIST_NEIGHBOR;

	uint8 numNeighbors;
	uint8 numHidden;
	uint8 framelength;
	uint8 numSlots;
	uint8 slot;
	struct TDMAInfo *info;

	memcpy(&numNeighbors, &messageData[TDMA_NUMN], sizeof(uint8));
	memcpy(&numHidden, &messageData[TDMA_NUMH], sizeof(uint8));
	memcpy(&framelength, &messageData[TDMA_FRAMELENGTH], sizeof(uint8));
	memcpy(&numSlots, &messageData[TDMA_NUMS], sizeof(uint8));

	int msgDataIndex = TDMA_NUMS + 1;

	bool uwbListInMsg[UWB_LIST_SIZE];
	for (int i = 0; i < inst->uwbListLen; i++)
	{
		uwbListInMsg[i] = FALSE;
	}
	uwbListInMsg[srcIndex] = TRUE;

	if (mode == CLEAR_ALL_COPY)
	{
		// clear all TDMA assignments and reset framelength to MIN
		this->tdma_free_all_slots(this);
		tdma_modified = TRUE;
	}

	// copy slot assignments for source UWB
	info = &this->uwbListTDMAInfo[srcIndex];
	if (framelength != info->framelength)
	{
		tdma_modified = TRUE;
	}

	// check if the tdma has been modified
	if (tdma_modified == FALSE) // dont look for any more differences if we already know one exists
	{
		// frist check if same number of slots
		if (numSlots == info->slotsLength)
		{
			// then check if each incoming slot is already assigned
			for (int i = 0; i < numSlots; i++)
			{
				memcpy(&slot, &messageData[msgDataIndex], sizeof(uint8));
				msgDataIndex++;

				if (this->slot_assigned(info, slot) == FALSE)
				{
					tdma_modified = TRUE;
					break;
				}
			}
		}
		else
		{
			tdma_modified = TRUE;
		}
	}

	if (mode == CLEAR_LISTED_COPY)
	{
		// do after cheking framelength because framelength will be reset
		this->free_slots(info);
	}

	info->framelength = MAX(framelength, info->framelength);

	msgDataIndex = TDMA_NUMS + 1;
	for (int s = 0; s < numSlots; s++)
	{
		memcpy(&slot, &messageData[msgDataIndex], sizeof(uint8));
		msgDataIndex++;

		this->assign_slot(info, slot, safeAssign);
	}

	for (int i = 0; i < numNeighbors; i++)
	{
		uint8 address[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		memcpy(&address[0], &messageData[msgDataIndex], inst->addrByteSize);
		msgDataIndex += inst->addrByteSize;

		uint8 uwb_index = instgetuwblistindex(inst, &address[0], inst->addrByteSize);

		if (uwb_index >= UWB_LIST_SIZE)
		{
			continue; // invalid
		}

		if (uwb_index != 0 && uwb_index < UWB_LIST_SIZE)
		{
			if (this->uwbListTDMAInfo[uwb_index].connectionType == UWB_LIST_INACTIVE || this->uwbListTDMAInfo[uwb_index].connectionType == UWB_LIST_TWICE_HIDDEN)
			{
				this->uwbListTDMAInfo[uwb_index].connectionType = UWB_LIST_HIDDEN;
			}

			this->uwbListTDMAInfo[uwb_index].lastCommHidden = time_now;
		}

		info = &this->uwbListTDMAInfo[uwb_index];
		uwbListInMsg[uwb_index] = TRUE;

		memcpy(&framelength, &messageData[msgDataIndex], sizeof(uint8));
		msgDataIndex++;
		memcpy(&numSlots, &messageData[msgDataIndex], sizeof(uint8));
		msgDataIndex++;
		int msgDataIndexSave = msgDataIndex;

		// check if the tdma has been modified
		if (tdma_modified == FALSE) // dont look for any more differences if we already know one exists
		{
			// frist check if same framelength and number of slots
			if (framelength == info->framelength && numSlots == info->slotsLength)
			{
				// then check if each incoming slot is already assigned
				for (int s = 0; s < numSlots; s++)
				{
					memcpy(&slot, &messageData[msgDataIndex], sizeof(uint8));
					msgDataIndex++;

					if (this->slot_assigned(info, slot) == FALSE)
					{
						tdma_modified = TRUE;
						break;
					}
				}
			}
			else
			{
				tdma_modified = TRUE;
			}
		}

		if (mode == CLEAR_LISTED_COPY)
		{
			// do after checking framelength because framelength reset
			this->free_slots(info);
		}
		info->framelength = MAX(framelength, info->framelength);

		msgDataIndex = msgDataIndexSave;
		for (int s = 0; s < numSlots; s++)
		{
			memcpy(&slot, &messageData[msgDataIndex], sizeof(uint8));
			msgDataIndex++;

			this->assign_slot(info, slot, safeAssign);
		}
	}

	for (int i = 0; i < numHidden; i++)
	{
		uint8 address[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		memcpy(&address[0], &messageData[msgDataIndex], inst->addrByteSize);
		msgDataIndex += inst->addrByteSize;

		uint8 uwb_index = instgetuwblistindex(inst, &address[0], inst->addrByteSize);
		if (uwb_index != 0) // 0 reserved for self
		{
			if (this->uwbListTDMAInfo[uwb_index].connectionType == UWB_LIST_INACTIVE)
			{
				this->uwbListTDMAInfo[uwb_index].connectionType = UWB_LIST_TWICE_HIDDEN;
			}

			this->uwbListTDMAInfo[uwb_index].lastCommTwiceHidden = time_now;
		}

		uwbListInMsg[uwb_index] = TRUE;
		info = &this->uwbListTDMAInfo[uwb_index];

		memcpy(&framelength, &messageData[msgDataIndex], sizeof(uint8));
		msgDataIndex++;
		memcpy(&numSlots, &messageData[msgDataIndex], sizeof(uint8));
		msgDataIndex++;
		int msgDataIndexSave = msgDataIndex;

		// check if the tdma has been modified
		if (tdma_modified == FALSE) // don't look for any more differences if we already know one exists
		{
			// frist check if same framelength and number of slots
			if (framelength == info->framelength && numSlots == info->slotsLength)
			{
				// then check if each incoming slot is already assigned
				for (int s = 0; s < numSlots; s++)
				{
					memcpy(&slot, &messageData[msgDataIndex], sizeof(uint8));
					msgDataIndex++;

					if (this->slot_assigned(info, slot) == FALSE)
					{
						tdma_modified = TRUE;
						break;
					}
				}
			}
			else
			{
				tdma_modified = TRUE;
			}
		}

		if (mode == CLEAR_LISTED_COPY)
		{
			// do after checking for difference because will reset framelength as well
			this->free_slots(info);
		}
		info->framelength = MAX(framelength, info->framelength);

		msgDataIndex = msgDataIndexSave;
		for (int s = 0; s < numSlots; s++)
		{
			memcpy(&slot, &messageData[msgDataIndex], sizeof(uint8));
			msgDataIndex++;

			this->assign_slot(info, slot, safeAssign);
		}
	}

	if (mode == CLEAR_LISTED_COPY)
	{
		// deconflict uncopied against copied. (excluding self)
		for (int i = 1; i < inst->uwbListLen; i++)
		{
			for (int j = i + 1; j < inst->uwbListLen; j++)
			{
				if ((uwbListInMsg[i] == FALSE && uwbListInMsg[j] == TRUE) || (uwbListInMsg[i] == TRUE && uwbListInMsg[j] == FALSE))
				{
					if ((this->uwbListTDMAInfo[i].connectionType == UWB_LIST_NEIGHBOR && this->uwbListTDMAInfo[j].connectionType == UWB_LIST_NEIGHBOR) ||
							(this->uwbListTDMAInfo[i].connectionType == UWB_LIST_NEIGHBOR && this->uwbListTDMAInfo[j].connectionType == UWB_LIST_HIDDEN) ||
							(this->uwbListTDMAInfo[j].connectionType == UWB_LIST_NEIGHBOR && this->uwbListTDMAInfo[i].connectionType == UWB_LIST_NEIGHBOR) ||
							(this->uwbListTDMAInfo[j].connectionType == UWB_LIST_NEIGHBOR && this->uwbListTDMAInfo[i].connectionType == UWB_LIST_HIDDEN))
					{
						if (this->deconflict_uwb_pair(this, &this->uwbListTDMAInfo[i], &this->uwbListTDMAInfo[j]) == TRUE)
						{
							tdma_modified = TRUE;
						}
					}
				}
			}
		}

		if (tdma_modified == TRUE)
		{

			// if so, release all assignments from self
			this->free_slots(&this->uwbListTDMAInfo[0]);

			// find self a new slot assignment
			this->find_assign_slot(this);
		}
	}

	return tdma_modified;
}

/********************************************************************************
Function:
  poll_delay()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Delay for a specified number of milliseconds before sending the poll message.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
bool poll_delay(struct TDMAHandler *this, uint32 time_now_offset, uint32 offset)
{
	bool delay = FALSE;

	uint64 time_now_us = portGetTickCntMicro();
	uint64 timeSinceSlotStart = get_dt64(this->lastSlotStartTime64, time_now_us);
	if (timeSinceSlotStart >= this->slotStartDelay_us)
	{
		delay = FALSE;
	}

	return delay;
}

/********************************************************************************
Function:
  check_timeouts()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Check if any neighbor connection has timed out.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
bool check_timeouts(struct TDMAHandler *this)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	uint32 delta_t = 0;
	bool rangingUWBTimeout = FALSE;
	bool setInactive = FALSE;
	bool updateINF = FALSE;
	bool noNeighbors = FALSE;

	uint8 max_framelength = 4;
	for (int i = 0; i < inst->uwbListLen; i++)
	{
		struct TDMAInfo *info = &this->uwbListTDMAInfo[i];
		if (info->connectionType != UWB_LIST_SELF && info->connectionType != UWB_LIST_INACTIVE)
		{
			if (info->framelength > max_framelength)
			{
				max_framelength = info->framelength;
			}
		}
	}
	inst->durationUwbCommTimeout_ms = 2 * max_framelength * CEIL_DIV(inst->durationSlotMax_us, 1000);

	uint32 timeout_ms = inst->durationUwbCommTimeout_ms;
	uint32 time_now_ms = portGetTickCnt();

	for (int i = 1; i < inst->uwbListLen; i++) // 0 reserved for self, timeout not applicable
	{
		struct TDMAInfo *info = &this->uwbListTDMAInfo[i];
		if (info->connectionType == UWB_LIST_INACTIVE)
		{
			continue;
		}

		/* Extend more time for tag/ anchor */
		if (is_tag(inst, i))
		{
			/* For tag */
			timeout_ms += (inst->rangingIntervalMs) * MAX(inst->uwbListLen / NUM_NODE_TO_RANGE, 1) * 3;
		}
		else
		{
			/* For anchor */
			timeout_ms += (inst->rangingIntervalMs) * MAX(inst->uwbListLen / NUM_NODE_TO_RANGE, 1) * 3;
		}

		switch (info->connectionType)
		{
		case UWB_LIST_NEIGHBOR:
		{

			delta_t = get_dt32(info->lastCommNeighbor, time_now_ms); // get time now here in case rx interrupt occurs before get_dt call

			if (delta_t > timeout_ms)
			{
				info->connectionType = UWB_LIST_HIDDEN;
				updateINF = TRUE;

				if (instfindnumneighbors(inst) <= 0)
				{
					sys_printf("NEIGHBOR TIMEOUT: %u - DELTA: %u - NOW: %u - LAST COMM: %u\r\n",
								 timeout_ms,
								 delta_t,
								 time_now_ms,
								 info->lastCommNeighbor);

					noNeighbors = TRUE;
				}

				if (inst->uwbToRangeWith == i)
				{
					rangingUWBTimeout = TRUE;
				}
			}

			break;
		}
		case UWB_LIST_HIDDEN:
		{
			delta_t = get_dt32(info->lastCommHidden, time_now_ms); // get time now here in case rx interrupt occurs before get_dt call

			if (delta_t > timeout_ms)
			{
				info->connectionType = UWB_LIST_TWICE_HIDDEN;
				updateINF = TRUE;
			}

			break;
		}
		case UWB_LIST_TWICE_HIDDEN:
		{
			delta_t = get_dt32(info->lastCommTwiceHidden, time_now_ms); // get time now here in case rx interrupt occurs before get_dt call

			if (delta_t > timeout_ms)
			{
				info->connectionType = UWB_LIST_INACTIVE;
				this->free_slots(info);
				setInactive = TRUE;
				updateINF = TRUE;
				info->lastCommNeighbor = 0;
				info->lastCommHidden = 0;
				info->lastCommTwiceHidden = 0;
			}

			break;
		}
		case UWB_LIST_INACTIVE:
		{
			break;
		}
		default:
		{
			// invalid list type
			break;
		}
		}
	}

	// one of the UWBs we were tracking became inactive,
	// re-optimize our TDMA assignments and repopulate the inf message
	if (setInactive == TRUE)
	{
		this->free_slots(&this->uwbListTDMAInfo[0]);
		this->find_assign_slot(this);
	}

	// adjust the INF message to reflect any changes
	if (updateINF == TRUE)
	{
		this->populate_inf_msg(this, RTLS_DEMO_MSG_INF_UPDATE);
	}

	if (rangingUWBTimeout == TRUE)
	{
		inst->uwbToRangeWith = 255;
		if (inst->mode == ANCHOR || inst->mode == DISCOVERY)
		{
			inst->testAppState = TA_RXE_WAIT;
			updateTestWithTxRx(inst, -1, -1);
		}
		else
		{
			inst->testAppState = TA_TX_SELECT;
			updateTestWithTxRx(inst, -1, -1);
		}
	}

	return noNeighbors;
}


/********************************************************************************
Function:
	is_last_node_to_range()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Check if this report is from the last anchor to range with
Notes:
	---
Author, Date:
	Toan Huynh, 03/18/2022
*********************************************************************************/
bool is_last_node_to_range(struct TDMAHandler *tdma_handler)
{
	return tdma_handler->numNodeToRange == 1;
}

/********************************************************************************
Function:
	store_last_node_ranged()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 03/19/2022
*********************************************************************************/
bool store_last_node_ranged(struct TDMAHandler *tdma_handler, uint8 uwb_idx)
{
	if (tdma_handler->numNodeToRange - 1 < ARRAY_SIZE(tdma_handler->lastNodeRanged))
	{
		tdma_handler->lastNodeRanged[tdma_handler->numNodeToRange - 1] = uwb_idx;
		return true;
	}

	return false;
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
//      END OF network.c
//###########################################################################################################
