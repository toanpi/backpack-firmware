/************************************************************************************************************
Module:       tdma_handler

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
#include "tdma_handler.h"
#include "port_mcu.h"
#include "instance.h"
#include "lib.h"
#include "system_dbg.h"
#include "timestamp.h"
#include "uwb_dev_driver.h"
#include "uwb_event.h"
#include "network.h"
#include "discovery.h"
#include "app_main.h"
#include "dev_mem.h"
#include "app_monitor.h"


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
	get_frame_duration_ms()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Return frame duration in ms.
Notes:
	---
Author, Date:
	Toan Huynh, 03/19/2022
*********************************************************************************/
uint64 get_frame_duration_ms(struct TDMAHandler *this)
{
	uint64 frameDuration64 = this->slotDuration_us * this->uwbListTDMAInfo[0].framelength;
	return CEIL_DIV(frameDuration64, 1000);
};

/********************************************************************************
Function:
	slot_transition()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Slot transition: Switch to next slot tag <-> anchor.
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static bool slot_transition(struct TDMAHandler *this, uint32 *p_timeout_ms, bool *p_need_sleep)
{
	bool transition = FALSE;
	instance_data_t *inst = instance_get_local_structure_ptr(0);

	if (inst->mode == TAG ||
			inst->mode == ANCHOR ||
			(inst->mode == DISCOVERY && (this->discovery_mode == WAIT_SEND_SUG || this->discovery_mode == COLLECT_INF_REG)))
	{
		uint64 time_now_us = portGetTickCntMicro();
		uint64 timeSinceSlotStart64 = get_dt64(this->lastSlotStartTime64, time_now_us);

		if (timeSinceSlotStart64 >= this->slotDuration_us)
		{
			transition = TRUE;
			this->numNodeToRange = NUM_NODE_TO_RANGE;
			inst->canPrintLCD = TRUE;

			// we have transitioned into the next slot.
			// get the slot number and set the start time appropriately

			uint64 timeSinceFrameStart64 = get_dt64(this->uwbListTDMAInfo[0].frameStartTime, time_now_us);
			uint64 frameDuration64 = this->slotDuration_us * this->uwbListTDMAInfo[0].framelength;
			if (timeSinceFrameStart64 >= frameDuration64)
			{
				int div = timeSinceFrameStart64 / frameDuration64;
				this->uwbListTDMAInfo[0].frameStartTime = timestamp_add64(this->uwbListTDMAInfo[0].frameStartTime, frameDuration64 * div);
				timeSinceFrameStart64 -= frameDuration64 * div;
			}

			uint8 slot = timeSinceFrameStart64 / (this->slotDuration_us); // integer division rounded down
			this->lastSlotStartTime64 = timestamp_add64(this->uwbListTDMAInfo[0].frameStartTime, (uint64)(this->slotDuration_us * slot));
			// this->lastSlotStartTime64 = this->uwbListTDMAInfo[0].frameStartTime + (uint64)(this->slotDuration_us * slot);

			// sys_printf("FRAME START: %u\n", (uint32_t)this->uwbListTDMAInfo[0].frameStartTime);

			if (inst->mode != DISCOVERY)
			{
				if (this->slot_assigned(&this->uwbListTDMAInfo[0], slot) == TRUE && time_to_ranging(inst, this))
				{
					/* Start tag mode */
					if(inst->mode == ANCHOR)
					{
						app_monitor_ranging_start();
					}

					inst->mode = TAG;
					inst->testAppState = TA_TX_SELECT;

					log_sniff_slot_data(inst, DWT_SIG_SLOT_TRANSITION, this->lastSlotStartTime64);
					updateTest(inst, this, NULL, -1, -1, -1, slot, time_now_us);
					// go to TX select, select the oldest uwb, send POLL, then send INF
				}
				else
				{
					/* Start anchor mode */
					if(inst->mode == TAG)
					{
						app_monitor_ranging_end();
					}

					// go to RX
					inst->mode = ANCHOR;
					inst->wait4ack = 0;
					inst->testAppState = TA_RXE_WAIT;
					log_sniff_slot_data(inst,DWT_SIG_SLOT_TRANSITION, this->lastSlotStartTime64);
					updateTest(inst, this, NULL, -1, -1, -1, slot, time_now_us);
				}

					// sys_printf("[%u] %s SLOT %d AT %u - FR: %u\r\n",
					// 					 (uint32)(time_now_us / 1000),
					// 					 get_instanceModes_string(inst->mode),
					// 					 slot,
					// 					 (uint32)(this->uwbListTDMAInfo[0].frameStartTime / 1000));
			}

			slot_clear_action_check(this);

			instance_getevent(17); // get and clear this event
			inst_processtxrxtimeout(inst);
		}
		else
		{
			/* Wait until slot transition */
			if (p_timeout_ms)
			{
				*p_timeout_ms =  CEIL_DIV((this->slotDuration_us - timeSinceSlotStart64), 1000);
				updateTestWithTimeout(inst, *p_timeout_ms);
			}
		}
	}
	else if (inst->mode == DISCOVERY)
	{
		slot_clear_action_check(this);
	}

	return transition;
}

/********************************************************************************
Function:
	update_frame_start()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Update frame start time.
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static uint64 update_frame_start(struct TDMAHandler *this)
{

	uint64 time_now_us = portGetTickCntMicro();
	uint64 frameDuration_us = this->slotDuration_us * this->uwbListTDMAInfo[0].framelength;
	uint64 timeSinceFrameStart_us = get_dt64(this->uwbListTDMAInfo[0].frameStartTime, time_now_us);

	if (timeSinceFrameStart_us > 1000000000) // if very large number, assume frame start time accidentally moved ahead of time now
	{
		uint64 diff_us = get_dt64(time_now_us, this->uwbListTDMAInfo[0].frameStartTime);
		int div = diff_us / frameDuration_us;
		if (diff_us % frameDuration_us != 0)
		{
			div += 1;
		}
		this->uwbListTDMAInfo[0].frameStartTime = timestamp_subtract64(this->uwbListTDMAInfo[0].frameStartTime, frameDuration_us * div);
		timeSinceFrameStart_us = get_dt64(this->uwbListTDMAInfo[0].frameStartTime, time_now_us);
	}
	else if (timeSinceFrameStart_us >= frameDuration_us)
	{
		int div = timeSinceFrameStart_us / frameDuration_us;
		this->uwbListTDMAInfo[0].frameStartTime = timestamp_add64(this->uwbListTDMAInfo[0].frameStartTime, frameDuration_us * div);
		timeSinceFrameStart_us -= timeSinceFrameStart_us * div;
	}

	// sys_printf("FRAME UPDATE: %u\n", (uint32_t)this->uwbListTDMAInfo[0].frameStartTime);

	return timeSinceFrameStart_us;
}

/********************************************************************************
Function:
	frame_sync()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Sync to frame start time with another node in the network.
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static void frame_sync(struct TDMAHandler *this, event_data_t *dw_event, uint8 framelength, uint64 timeSinceFrameStart_us, uint8 srcIndex, FRAME_SYNC_MODE mode)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);

	// do not process erroneous tsfs
	// can happen if frame start time is shifted ahead of time_now_us in transmitting UWB
	if (timeSinceFrameStart_us > 1000000000 || timeSinceFrameStart_us == 0 || srcIndex >= UWB_LIST_SIZE)
	{
		return;
	}

	uint8 sys_time_arr[5] = {0, 0, 0, 0, 0};
	dwt_readsystime(sys_time_arr);
	uint64 dwt_time_now = 0;
	dwt_time_now = (uint64)sys_time_arr[0] + ((uint64)sys_time_arr[1] << 8) + ((uint64)sys_time_arr[2] << 16) + ((uint64)sys_time_arr[3] << 24) + ((uint64)sys_time_arr[4] << 32);

	// time from command tx to tx timestamp
	uint64 infCmdToTsDelay_us = TX_CMD_TO_TX_CB_DLY_US + inst->storedPreLen_us;

	// tx antenna delay
	uint64 tx_antenna_delay = (uint64)inst->defaultAntennaDelay;

	// time to propagate
	// NOTE: assuming zero since difference for speed of light travel time over 10cm and 100m is negligible for frame sync purposes

	// rx antenna delay
	// NOTE: antenna delay is captured by the RX timestamp

	// time from rx timestamp to now
	uint64 rxfs_process_delay = dwt_getdt(dw_event->timeStamp, dwt_time_now);

	uint64 txrx_delay = (uint64)(convert_device_time_to_sec(tx_antenna_delay + rxfs_process_delay) * 1000000.0) + infCmdToTsDelay_us;

	uint64 hisTimeSinceFrameStart_us = timeSinceFrameStart_us + txrx_delay;

	uint64 time_now_us = portGetTickCntMicro();

#ifdef AM_PART_APOLLO3
	if (hisTimeSinceFrameStart_us > time_now_us)
	{
		return;
	}
#endif

	this->uwbListTDMAInfo[srcIndex].frameStartTime = timestamp_subtract64(time_now_us, hisTimeSinceFrameStart_us);
	uint64 myFrameDuration = this->slotDuration_us * this->uwbListTDMAInfo[0].framelength;

	uint64 diff_tsfs = 0;
	uint64 diff_tsfs_mod = 0;
	uint64 diff_us = 0;
	bool need_send_sync = FALSE;
	bool diff_add = FALSE;

	if (mode == FS_COLLECT)
	{
		return;
	}
	else if (mode == FS_ADOPT)
	{
		this->uwbListTDMAInfo[0].frameStartTime = this->uwbListTDMAInfo[srcIndex].frameStartTime; // NOTE gets processed further at end of function
	}
	else // if(mode == FS_AVERAGE || mode == FS_EVAL)
	{
		uint64 myTimeSinceFrameStart_us = get_dt64(this->uwbListTDMAInfo[0].frameStartTime, time_now_us);

		// SELF VS INCOMING

		uint8 min_fl = this->uwbListTDMAInfo[0].framelength;
		if (framelength < min_fl)
		{
			min_fl = framelength;
		}

		uint64 min_framelengthDuration = min_fl * this->slotDuration_us;

		if (hisTimeSinceFrameStart_us <= myTimeSinceFrameStart_us)
		{
			diff_tsfs = myTimeSinceFrameStart_us - hisTimeSinceFrameStart_us;
			diff_tsfs_mod = diff_tsfs % min_framelengthDuration;

			if (diff_tsfs_mod <= 0.5 * min_framelengthDuration)
			{
				diff_us = diff_tsfs_mod;
				diff_add = TRUE;
			}
			else
			{
				diff_us = min_framelengthDuration - diff_tsfs_mod;
			}
		}
		else
		{
			diff_tsfs = hisTimeSinceFrameStart_us - myTimeSinceFrameStart_us;
			diff_tsfs_mod = diff_tsfs % min_framelengthDuration;

			if (diff_tsfs_mod <= 0.5 * min_framelengthDuration)
			{
				diff_us = diff_tsfs_mod;
			}
			else
			{
				diff_us = min_framelengthDuration - diff_tsfs_mod;
				diff_add = TRUE;
			}
		}

		// check if frame sync out of tolerance (don't xmit sync message in case of 0th slot misalignment)
		if (diff_us % (MIN_FRAMELENGTH * this->slotDuration_us) > this->frameSyncThreshold_us)
		{
			if (MIN_FRAMELENGTH * this->slotDuration_us - diff_us % (MIN_FRAMELENGTH * this->slotDuration_us) > this->frameSyncThreshold_us)
			{
				/* TODO Toan Huynh: Double check here if sync erro rate is too high */
				need_send_sync = TRUE;
				// this->tx_sync_msg(this);
			}
		}
		else if (mode == FS_EVAL)
		{
			return;
		}

		uint8 div = 2;
		if (mode == FS_EVAL || diff_us > this->frameSyncThreshold_us) // if diff_us > threshold, there is 0th slot misalignment
		{
			div = 1;
		}

		if (diff_add == TRUE)
		{
			this->uwbListTDMAInfo[0].frameStartTime = timestamp_add64(this->uwbListTDMAInfo[0].frameStartTime, diff_us / div);
		}
		else
		{
			this->uwbListTDMAInfo[0].frameStartTime = timestamp_subtract64(this->uwbListTDMAInfo[0].frameStartTime, diff_us / div);
		}
	}

	uint64 myTimeSinceFrameStart_us = get_dt64(this->uwbListTDMAInfo[0].frameStartTime, time_now_us);
	if (myTimeSinceFrameStart_us > 100000000)
	{
		// if this is a very large number, then the frame start time was likely moved ahead of time_now_us.
		while (this->uwbListTDMAInfo[0].frameStartTime > time_now_us)
		{
			this->uwbListTDMAInfo[0].frameStartTime = timestamp_subtract64(this->uwbListTDMAInfo[0].frameStartTime, myFrameDuration);
		}

		myTimeSinceFrameStart_us = get_dt64(this->uwbListTDMAInfo[0].frameStartTime, time_now_us);
	}
	else
	{
		while (myTimeSinceFrameStart_us >= myFrameDuration)
		{
			this->uwbListTDMAInfo[0].frameStartTime = timestamp_add64(this->uwbListTDMAInfo[0].frameStartTime, myFrameDuration);
			myTimeSinceFrameStart_us -= myFrameDuration;
		}
	}

	uint8 slot = myTimeSinceFrameStart_us / this->slotDuration_us; // integer division rounded down
	this->lastSlotStartTime64 = timestamp_add64(this->uwbListTDMAInfo[0].frameStartTime, (uint64)(this->slotDuration_us * slot));

	/* TODO Toan Huynh: Double check here if sync erro rate is too high */
	/* Delta error is too high, send sync message */
	if (need_send_sync)
	{
		this->tx_sync_msg(this);
	}

	if (diff_us > myFrameDuration)
	{
		sys_printf("FRAME SYNC: %u %s %u| DIFF: %03u - IDX: %03u\r\n",
					 (uint32_t)this->uwbListTDMAInfo[0].frameStartTime / 1000,
					 diff_add == TRUE ? "+" : "-",
					 (uint32)diff_us / 1000,
					 (uint32)diff_us / 1000, srcIndex);
	}

	log_sniff_slot_data(inst,DWT_SIG_FRAME_SYNC, this->lastSlotStartTime64);
	updateTestWithSlot(this, time_now_us, -1);
}

/********************************************************************************
Function:
	tx_sync_msg()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Send a sync message to the other UWB device.
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static bool tx_sync_msg(struct TDMAHandler *this)
{
	int psduLength = SYNC_FRAME_LEN_BYTES;

	instance_data_t *inst = instance_get_local_structure_ptr(0);
	uint64 time_now_us = portGetTickCntMicro();

	uint64 myTimeSinceFrameStart_us = get_dt64(this->uwbListTDMAInfo[0].frameStartTime, time_now_us);

	memcpy(&inst->sync_msg.messageData[SYNC_FRAMELENGTH], &this->uwbListTDMAInfo[0].framelength, sizeof(uint8));
	memcpy(&inst->sync_msg.messageData[SYNC_TSFS], &myTimeSinceFrameStart_us, 6);
	inst->sync_msg.seqNum = inst->frameSN++;

	inst->wait4ack = 0;

	dwt_writetxdata(psduLength, (uint8 *)&inst->sync_msg, 0); // write the frame data
	if (instance_send_packet(psduLength, DWT_START_RX_IMMEDIATE | inst->wait4ack, 0))
	{
		sys_printf("[ERR][TX] SYNC\r\n");

		inst->previousState = TA_INIT;
		inst->nextState = TA_INIT;
		inst->testAppState = TA_RXE_WAIT;
		inst->wait4ack = 0;
		updateTestWithTxRx(inst, TX_SEND_SYNC_MSG_ERROR, -1);
		return FALSE;
	}
	else
	{
		inst->previousState = inst->testAppState;
		inst->testAppState = TA_TX_WAIT_CONF; // wait confirmation
		updateTestWithTxRx(inst, TX_SEND_SYNC_MSG, -1);

		inst->timeofTx = portGetTickCnt();
		inst->txDoneTimeoutDuration = inst->durationSyncTxDoneTimeout_ms; // NOTE timeout duration found experimentally
		return TRUE;
	}
}

/********************************************************************************
Function:
	slot_assigned()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Get the slot assigned to this UWB device.
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static bool slot_assigned(struct TDMAInfo *info, uint8 slot)
{
	bool assigned = FALSE;
	if (info->slots != NULL && info->slotsLength != 0)
	{
		for (int i = 0; i < info->slotsLength; i++)
		{
			if (memcmp(&info->slots[i], &slot, 1) == 0)
			{
				assigned = TRUE;
				break;
			}
		}
	}

	return assigned;
}

/********************************************************************************
Function:
	assign_slot()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Assign a slot to the current UWB device.
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static bool assign_slot(struct TDMAInfo *info, uint8 slot, bool safeAssign)
{
	// NOTE: deconflicting happens elsewhere
	bool retval = FALSE;

	if (safeAssign == TRUE) // when using safeAssign, first check if the slot is assigned
	{
		// if not assigned, increase slots size and add slot index to end of array (array is unsorted)
		if (slot_assigned(info, slot) == FALSE)
		{
			uint8 *newSlots = sys_malloc(sizeof(uint8) * (info->slotsLength + 1));
			memcpy(&newSlots[0], &info->slots[0], sizeof(uint8) * info->slotsLength);
			memcpy(&newSlots[info->slotsLength], &slot, 1);

			sys_free(info->slots);
			info->slots = NULL;
			info->slots = newSlots;
			info->slotsLength += 1;

			retval = TRUE;
		}
	}
	else
	{
		uint8 *newSlots = sys_malloc(sizeof(uint8) * (info->slotsLength + 1));
		memcpy(&newSlots[0], &info->slots[0], sizeof(uint8) * info->slotsLength);
		memcpy(&newSlots[info->slotsLength], &slot, 1);

		sys_free(info->slots);
		info->slots = NULL;
		info->slots = newSlots;
		info->slotsLength += 1;

		retval = TRUE;
	}

	return retval;
}

/********************************************************************************
Function:
	find_assign_slot()
Input Parameters:
	---
Output Parameters:
	---
Description:
	 finding and assigning a slot works according to the following:
	 1.) Set framelength to 4
	 2.) Get Unassigned Slots (GU)
			applicable if one or more open slots exist
			assign self up to two unassigned slots (except for 0th slot); exit
	 3.) Release Multiple Assigned Slots (RMA)
			applicable if 2.) not applicable
			applicable if one or more nodes has multiple slot assignments
			release one slot from node with greatest number of slot assignments and assign to self; exit
	 4.) Double the Frame (DF)
			applicable if 2.) and 3.) not applicable
			double own framelength and go back to 2.)
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static void find_assign_slot(struct TDMAHandler *this)
{

	// NOTE: this assumes that all other TDMAInfo stored in the TDMAHandler are not in conflict with each other
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	struct TDMAInfo *info = &this->uwbListTDMAInfo[0];

	// if this UWB was somehow reset, recover its prior slot assignment info from the network traffic
	if (info->slotsLength > 0)
	{
		return;
	}

	bool assignment_made = FALSE;

	// set framelength
	info->framelength = 4;

	// come here after DF
	while (TRUE)
	{
		// GU
		uint8 slotsAssigned = 0;
		for (uint8 i = 1; i < info->framelength; i++) // do not assign to 0th slot
		{
			bool assigned = FALSE;

			for (uint8 u = 1; u < inst->uwbListLen; u++) // 0 reserved for self
			{
				for (uint8 su = 0; su < this->uwbListTDMAInfo[u].slotsLength; su++)
				{
					uint8 slot_su;
					memcpy(&slot_su, &this->uwbListTDMAInfo[u].slots[su], sizeof(uint8));

					if (info->framelength > this->uwbListTDMAInfo[u].framelength)
					{
						uint8 mod_i = i % this->uwbListTDMAInfo[u].framelength;

						if (slot_su == mod_i)
						{
							// slot assigned to this uwb
							assigned = TRUE;
						}
					}
					else if (info->framelength < this->uwbListTDMAInfo[u].framelength)
					{
						uint8 mod_slot_su = slot_su % info->framelength;

						if (mod_slot_su == i)
						{
							// slot assigned to this uwb
							assigned = TRUE;
						}
					}
					else // same framelength
					{
						if (slot_su == i)
						{
							// slot assigned to this uwb
							assigned = TRUE;
						}
					}

					if (assigned == TRUE)
					{
						break;
					}
				}

				if (assigned == TRUE)
				{
					break;
				}
			}

			// slot not assigned, assign to self
			if (assigned == FALSE)
			{
				this->assign_slot(info, i, FALSE);
				assignment_made = TRUE;

				slotsAssigned++;
				if (slotsAssigned > 1) // assign self up to two empty slots
				{
					break;
				}
			}
		}

		if (assignment_made == TRUE)
		{
			break;
		}

		// RMA
		// find UWB with greatest number of slot assignments
		uint8 max_assignments = 0;
		uint8 max_uwb_index = 255;
		for (uint8 u = 1; u < inst->uwbListLen; u++) // 0 reserved for self
		{
			if (info->framelength < this->uwbListTDMAInfo[u].framelength)
			{
				continue;
			}

			uint8 slotsLength = this->uwbListTDMAInfo[u].slotsLength;
			if (info->framelength > this->uwbListTDMAInfo[u].framelength && this->uwbListTDMAInfo[u].slotsLength != 0)
			{
				slotsLength *= info->framelength / this->uwbListTDMAInfo[u].framelength;
			}

			if (slotsLength > max_assignments)
			{
				max_assignments = slotsLength;
				max_uwb_index = u;
			}
		}

		if (max_uwb_index != 255 && max_assignments > 1)
		{
			uint8 slot;
			memcpy(&slot, &this->uwbListTDMAInfo[max_uwb_index].slots[0], sizeof(uint8));
			this->assign_slot(info, slot, TRUE);
			this->deconflict_uwb_pair(this, info, &this->uwbListTDMAInfo[max_uwb_index]);
			assignment_made = TRUE;
		}

		if (assignment_made == TRUE)
		{
			break;
		}

		// DF
		info->framelength *= 2;
	}
}

/********************************************************************************
Function:
	deconflict_slot_assignments()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Deconflict slot assignments of two UWBs
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static bool deconflict_slot_assignments(struct TDMAHandler *this)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	bool conflict = FALSE;

	while (TRUE)
	{
		bool conflict_this_iter = FALSE;
		// first deconflict slots in neighbor, hidden, and twice hidden
		for (int i = 1; i < inst->uwbListLen; i++) // 0 reserved for self
		{
			if (this->uwbListTDMAInfo[i].connectionType != UWB_LIST_INACTIVE)
			{
				for (int j = i + 1; j < inst->uwbListLen; j++)
				{
					if (this->uwbListTDMAInfo[j].connectionType != UWB_LIST_INACTIVE && j != i)
					{
						// first check if their list type requires deconflicting
						if ((this->uwbListTDMAInfo[i].connectionType == UWB_LIST_NEIGHBOR && this->uwbListTDMAInfo[j].connectionType == UWB_LIST_TWICE_HIDDEN) ||
								(this->uwbListTDMAInfo[j].connectionType == UWB_LIST_NEIGHBOR && this->uwbListTDMAInfo[i].connectionType == UWB_LIST_TWICE_HIDDEN) ||
								(this->uwbListTDMAInfo[i].connectionType == UWB_LIST_TWICE_HIDDEN && this->uwbListTDMAInfo[j].connectionType == UWB_LIST_TWICE_HIDDEN) ||
								(this->uwbListTDMAInfo[i].connectionType == UWB_LIST_HIDDEN && this->uwbListTDMAInfo[j].connectionType == UWB_LIST_HIDDEN))
						{
							continue;
						}

						if (this->deconflict_uwb_pair(this, &this->uwbListTDMAInfo[i], &this->uwbListTDMAInfo[j]))
						{
							conflict = TRUE;
							conflict_this_iter = TRUE;
							break;
						}
					}
				}
			}

			if (conflict_this_iter)
			{
				break;
			}
		}

		if (conflict_this_iter)
		{
			continue;
		}

		// next deconflict slots between self and neighbor, hidden, and twice hidden
		for (int i = 1; i < inst->uwbListLen; i++) // 0 reserved for self
		{
			if (this->uwbListTDMAInfo[i].connectionType != UWB_LIST_INACTIVE)
			{
				if (this->deconflict_uwb_pair(this, &this->uwbListTDMAInfo[0], &this->uwbListTDMAInfo[i]))
				{
					conflict = TRUE;
					conflict_this_iter = TRUE;
				}
			}
			if (conflict_this_iter)
			{
				break;
			}
		}

		if (conflict_this_iter)
		{
			continue;
		}

		break; // no conflicts found this iteration, break out of while loop
	}

	return conflict;
}

/********************************************************************************
Function:
	deconflict_uwb_pair()
Input Parameters:
	---
Output Parameters:
	return true if a conflict was found
Description:
	Check if there is a conflict between two UWBs
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static bool deconflict_uwb_pair(struct TDMAHandler *this, struct TDMAInfo *info_a, struct TDMAInfo *info_b)
{
	bool conflict = FALSE;

	while (TRUE)
	{
		bool conflict_this_iter = false;

		for (int sa = 0; sa < info_a->slotsLength; sa++)
		{
			uint8 slot_sa;
			memcpy(&slot_sa, &info_a->slots[sa], 1);

			for (int sb = 0; sb < info_b->slotsLength; sb++)
			{
				uint8 slot_sb;
				memcpy(&slot_sb, &info_b->slots[sb], 1);

				// check if slot is taken
				if (slot_sa >= info_b->framelength)
				{
					uint8 mod_slot_sa = slot_sa % info_b->framelength;

					if (mod_slot_sa == slot_sb)
					{
						// slot already assigned, deconflict!
						this->deconflict_slot_pair(this, info_a, info_b, sa, sb);
						conflict = TRUE;
						conflict_this_iter = TRUE;
						break;
					}
				}
				else if (slot_sb >= info_a->framelength)
				{
					uint8 mod_slot_sb = slot_sb % info_a->framelength;
					if (mod_slot_sb == slot_sa)
					{
						// slot already assigned, deconflict!
						this->deconflict_slot_pair(this, info_a, info_b, sa, sb);
						conflict = TRUE;
						conflict_this_iter = TRUE;
						break;
					}
				}
				else
				{
					if (slot_sa == slot_sb)
					{
						// slot already assigned, deconflict!
						this->deconflict_slot_pair(this, info_a, info_b, sa, sb);
						conflict = TRUE;
						conflict_this_iter = TRUE;
						break;
					}
				}
			}

			if (conflict_this_iter)
			{
				break;
			}
		}

		if (conflict_this_iter)
		{
			continue;
		}

		break; // no conflicts found this iterations, break while loop
	}

	return conflict;
}

/********************************************************************************
Function:
	deconflict_slot_pair()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Deconflict two slots
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static void deconflict_slot_pair(struct TDMAHandler *this, struct TDMAInfo *info_a, struct TDMAInfo *info_b, uint8 slot_idx_a, uint8 slot_idx_b)
{
	// procedure for deconflicting slots (PDS)
	// 1.) delete a conflicting slot
	//		applicable if all but node with fewest slots has more than one slot assignment
	//		release conflicting slot from all but node with fewest slots
	// 2.) divide the assignment
	//		applicable if multiple conflicting slots between to nodes
	//		release lowest slot from first, greatest from second
	// 3.) double the frame and divide the assignment
	//		applicable if single conflict between two nodes and neither has another slot assignment
	//		double the framelength of one or both and assign one slot assignment to the first and other to the second
	//		make the change
	// 4.) if any of 1-3.) is applied, check for conflicts again, and start again at 1.)

	// logic for 1.) and 2.) not explicitly programmed.
	// Should be taken care of by repeatedly checking for conflicts and executing the code block below
	if (info_a->slotsLength > 1 || info_b->slotsLength > 1)
	{
		if (info_a->slotsLength >= info_b->slotsLength)
		{
			// release slot from uwb_a
			uint8 slot_a;
			memcpy(&slot_a, &info_a->slots[slot_idx_a], sizeof(uint8));
			this->free_slot(info_a, slot_a);
			return;
		}
		else
		{
			// release slot from uwb_b
			uint8 slot_b;
			memcpy(&slot_b, &info_b->slots[slot_idx_b], sizeof(uint8));
			this->free_slot(info_b, slot_b);
			return;
		}
	}

	// double the frame and divide the assignment
	if (info_a->framelength == info_b->framelength)
	{
		uint8 slot_b;
		memcpy(&slot_b, &info_b->slots[slot_idx_b], sizeof(uint8));
		slot_b += info_b->framelength;
		memcpy(&info_b->slots[slot_idx_b], &slot_b, sizeof(uint8));
		info_a->framelength *= 2;
		info_b->framelength *= 2;
	}
	else if (info_a->framelength > info_b->framelength)
	{
		uint8 slot_a;
		memcpy(&slot_a, &info_a->slots[slot_idx_a], sizeof(uint8));
		uint8 mod_a = slot_a % (2 * info_b->framelength);

		uint8 slot_b;
		memcpy(&slot_b, &info_b->slots[slot_idx_b], sizeof(uint8));

		if (mod_a == slot_b)
		{
			slot_b += info_b->framelength;
			memcpy(&info_b->slots[slot_idx_b], &slot_b, sizeof(uint8));
		}

		info_b->framelength *= 2;
	}
	else if (info_a->framelength < info_b->framelength)
	{
		uint8 slot_b;
		memcpy(&slot_b, &info_b->slots[slot_idx_b], sizeof(uint8));
		uint8 mod_b = slot_b % (2 * info_a->framelength);

		uint8 slot_a;
		memcpy(&slot_a, &info_a->slots[slot_idx_a], sizeof(uint8));

		if (mod_b == slot_a)
		{
			slot_a += info_a->framelength;
			memcpy(&info_a->slots[slot_idx_a], &slot_a, sizeof(uint8));
		}

		info_a->framelength *= 2;
	}

	// re-checking for conflicts handled in calling function
}

/********************************************************************************
Function:
	self_conflict()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Check if this uwb has any TDMA conflicts with others in the uwbList
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static bool self_conflict(struct TDMAHandler *this)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	struct TDMAInfo *info_a = &this->uwbListTDMAInfo[0];

	for (int b = 1; b < inst->uwbListLen; b++)
	{
		if (this->uwbListTDMAInfo[b].connectionType == UWB_LIST_NEIGHBOR ||
				this->uwbListTDMAInfo[b].connectionType == UWB_LIST_HIDDEN ||
				this->uwbListTDMAInfo[b].connectionType == UWB_LIST_TWICE_HIDDEN)
		{
			struct TDMAInfo *info_b = &this->uwbListTDMAInfo[b];

			for (int sa = 0; sa < info_a->slotsLength; sa++)
			{
				uint8 slot_sa;
				memcpy(&slot_sa, &info_a->slots[sa], sizeof(uint8));

				for (int sb = 0; sb < info_b->slotsLength; sb++)
				{
					uint8 slot_sb;
					memcpy(&slot_sb, &info_b->slots[sb], sizeof(uint8));

					// check if slot is taken
					if (slot_sa >= info_b->framelength)
					{
						uint8 mod_slot_sa = slot_sa % info_b->framelength;

						if (mod_slot_sa == slot_sb)
						{
							return TRUE;
						}
					}
					else if (slot_sb >= info_a->framelength)
					{
						uint8 mod_slot_sb = slot_sb % info_a->framelength;
						if (mod_slot_sb == slot_sa)
						{
							return TRUE;
						}
					}
					else
					{
						if (slot_sa == slot_sb)
						{
							return TRUE;
						}
					}
				}
			}
		}
	}

	return FALSE;
}

/********************************************************************************
Function:
	free_slot()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Free a slot in the tdma structure
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static void free_slot(struct TDMAInfo *info, uint8 slot)
{
	bool assigned = TRUE;

	while (assigned == TRUE) // duplicate assignments shouldn't exist, but will make sure to remove any just in case
	{
		uint8 slot_index = 255;
		assigned = FALSE;
		if (info->slots != NULL && info->slotsLength != 0)
		{
			for (int i = 0; i < info->slotsLength; i++)
			{
				if (memcmp(&info->slots[i], &slot, sizeof(uint8)) == 0)
				{
					assigned = TRUE;
					slot_index = i;
					break;
				}
			}
		}

		// if assigned, remove from array
		if (assigned == TRUE)
		{
			memcpy(&info->slots[slot_index], &info->slots[slot_index + 1], sizeof(uint8) * (info->slotsLength - slot_index - 1));
			info->slotsLength -= 1;
			if (info->slotsLength <= 0)
			{
				info->slotsLength = 0;
				sys_free(info->slots);
				info->slots = NULL;
			}
		}
	}

	return;
}

/********************************************************************************
Function:
	free_slots()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Free slots in the tdma structure
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static void free_slots(struct TDMAInfo *info)
{
	if (info->slots != NULL)
	{
		sys_free(info->slots);
		info->slots = NULL;
	}

	info->slotsLength = 0;
	info->framelength = MIN_FRAMELENGTH;
}

/********************************************************************************
Function:
	uwblist_free_slots()
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
static void uwblist_free_slots(struct TDMAHandler *this, uint8 uwb_index)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	if (uwb_index >= inst->uwbListLen)
	{
		// out of bounds!
		return;
	}

	this->free_slots(&this->uwbListTDMAInfo[uwb_index]);

	return;
}

/********************************************************************************
Function:
	tdma_free_all_slots()
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
static void tdma_free_all_slots(struct TDMAHandler *this)
{
	for (int i = 0; i < (int)UWB_LIST_SIZE; i++)
	{
		if (this->uwbListTDMAInfo[i].slots != NULL)
		{
			sys_free(this->uwbListTDMAInfo[i].slots);
			this->uwbListTDMAInfo[i].slots = NULL;
		}

		this->uwbListTDMAInfo[i].slotsLength = 0;
		this->uwbListTDMAInfo[i].framelength = MIN_FRAMELENGTH;
	}

	return;
}

/********************************************************************************
Function:
	new()
Input Parameters:
	---
Output Parameters:
	---
Description:
	New a TDMAHandler structure
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
static struct TDMAHandler new (uint64 slot_duration)
{
	struct TDMAHandler ret = {};

	ret.slot_transition = &slot_transition;
	ret.frame_sync = &frame_sync;
	ret.update_frame_start = &update_frame_start;
	ret.tx_sync_msg = &tx_sync_msg;
	ret.tx_select = &tx_select;
	ret.check_blink = &check_blink;

	ret.populate_inf_msg = &populate_inf_msg;
	ret.update_inf_tsfs = &update_inf_tsfs;
	ret.process_inf_msg = &process_inf_msg;

	ret.poll_delay = &poll_delay;
	ret.slot_assigned = &slot_assigned;
	ret.assign_slot = &assign_slot;
	ret.find_assign_slot = &find_assign_slot;
	ret.build_new_network = &build_new_network;

	ret.free_slot = &free_slot;
	ret.free_slots = &free_slots;
	ret.tdma_free_all_slots = &tdma_free_all_slots;
	ret.uwblist_free_slots = &uwblist_free_slots;
	ret.enter_discovery_mode = &enter_discovery_mode;
	ret.set_discovery_mode = &set_discovery_mode;
	ret.check_discovery_mode_expiration = &check_discovery_mode_expiration;
	ret.check_timeouts = &check_timeouts;

	ret.deconflict_slot_assignments = &deconflict_slot_assignments;
	ret.deconflict_uwb_pair = &deconflict_uwb_pair;
	ret.deconflict_slot_pair = &deconflict_slot_pair;
	ret.self_conflict = &self_conflict;

	ret.slotDuration_us = slot_duration;
	ret.slotDuration_ms = slot_duration / 1000 + (slot_duration % 1000 == 0 ? 0 : 1);

	uint64 time_now_us = portGetTickCntMicro();
	uint32 time_now = portGetTickCnt();

	ret.maxFramelength = (uint8)MIN_FRAMELENGTH;
	while (ret.maxFramelength < (uint8)UWB_LIST_SIZE + 1)
	{
		ret.maxFramelength *= 2;
	}

	for (int i = 0; i < UWB_LIST_SIZE; i++)
	{
		ret.uwbListTDMAInfo[i].framelength = (uint8)MIN_FRAMELENGTH;
		ret.uwbListTDMAInfo[i].slots = NULL;
		ret.uwbListTDMAInfo[i].slotsLength = 0;
		ret.uwbListTDMAInfo[i].frameStartTime = time_now_us;
		ret.uwbListTDMAInfo[i].connectionType = UWB_LIST_INACTIVE;
		ret.uwbListTDMAInfo[i].lastCommNeighbor = 0;
		ret.uwbListTDMAInfo[i].lastCommHidden = 0;
		ret.uwbListTDMAInfo[i].lastCommTwiceHidden = 0;
		ret.uwbListTDMAInfo[i].lastRange = time_now;
	}
	ret.uwbListTDMAInfo[0].connectionType = UWB_LIST_SELF;

	ret.lastSlotStartTime64 = time_now_us;
	ret.infSentThisSlot = FALSE;
	ret.firstPollSentThisSlot = FALSE;
	ret.firstPollResponse = FALSE;
	ret.firstPollComplete = FALSE;
	ret.secondPollSentThisSlot = FALSE;
	ret.nthOldest = 1;
	ret.nthOldestPlus = 2;
	ret.slotStartDelay_us = SLOT_START_BUFFER_US;
	ret.frameSyncThreshold_us = ret.slotStartDelay_us;
	ret.infMessageLength = 0;
	ret.numNodeToRange = NUM_NODE_TO_RANGE;

	memset(ret.lastNodeRanged, 0, sizeof(ret.lastNodeRanged));

	ret.enter_discovery_mode(&ret);
	ret.collectInfDuration = ret.maxFramelength * ret.slotDuration_ms;
	ret.waitInfDuration = ret.collectInfDuration;
	ret.blinkPeriodRand = (uint32)rand() % BLINK_PERIOD_RAND_MS;

	return ret;
}

const struct TDMAHandlerClass TDMAHandler = {.new = &new};


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
//      END OF tdma_handler.c
//###########################################################################################################
