/************************************************************************************************************
Module:       discovery

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
#include "discovery.h"
#include "system_dbg.h"
#include "timestamp.h"
#include "instance_utilities.h"
#include "lib.h"
#include "instance.h"
#include "network_mac.h"
#include "net_node.h"
#include <app_monitor.h>




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
	net_discovery_init()
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
bool net_discovery_init(instance_data_t *inst, struct TDMAHandler *tdma_handler)
{
	bool ret = true;

	ret = net_node_setup_network(inst);

	inst->uwbListLen = 1;
	tdma_handler->uwbListTDMAInfo[0].connectionType = UWB_LIST_SELF;

	// change to next state - wait to receive a message
	tdma_handler->discoveryStartTime = portGetTickCnt();
	tdma_handler->last_blink_time = portGetTickCnt();

	inst->wait4ack = 0;

	return ret;
}

/********************************************************************************
Function:
  enter_discovery_mode()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Enter discovery mode.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void enter_discovery_mode(struct TDMAHandler *this)
{
	uint32 time_now = portGetTickCnt();
	this->discoveryStartTime = time_now;
	this->last_blink_time = time_now;
	this->set_discovery_mode(this, WAIT_INF_REG, time_now);
	this->collectInfStartTime = time_now;
	this->sleepDiscoveryEndTime = timestamp_add32(time_now, this->waitInfDuration);

	instance_data_t *inst = instance_get_local_structure_ptr(0);
	inst->canPrintLCD = TRUE;

	this->tdma_free_all_slots(this);
}

/********************************************************************************
Function:
  set_discovery_mode()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Set discovery mode.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void set_discovery_mode(struct TDMAHandler *this, DISCOVERY_MODE discovery_mode, uint32 time_now)
{
	this->discovery_mode_start_time = time_now;
	this->discovery_mode = discovery_mode;

	switch (discovery_mode)
	{
	case WAIT_INF_REG:
	{
		this->discovery_mode_duration = 0;
		this->discovery_mode_expires = FALSE;
		break;
	}
	case COLLECT_INF_REG:
	{
		this->collectInfStartTime = time_now;
		this->discovery_mode_duration = this->collectInfDuration;
		this->discovery_mode_expires = TRUE;
		sys_printf("COLLECT INF REG IN %lu\r\n", this->discovery_mode_duration);
		break;
	}
	case WAIT_INF_INIT:
	{
		this->discovery_mode_duration = this->slotDuration_ms;
		this->discovery_mode_expires = TRUE;
		break;
	}
	case WAIT_RNG_INIT:
	{
		instance_data_t *inst = instance_get_local_structure_ptr(0);
		this->discovery_mode_duration = inst->durationWaitRangeInit_ms;
		this->discovery_mode_expires = TRUE;
		break;
	}
	case WAIT_SEND_SUG:
	{
		// find common frame start time among neighbors
		instance_data_t *inst = instance_get_local_structure_ptr(0);

		// count the number of UWBs that belong to each subnetwork
		// select the subnetwork with the largest number of UWBs
		// find the common frame start time among the UWBs in that network

		// keep track of which number each UWB belongs to
		// then keep track of which was selected so we can iterate over it later in this function
		uint8 num_sub_networks = 0;
		uint8 sub_network_membership[UWB_LIST_SIZE] = {};
		uint8 sub_network_selected = 0;
		uint8 sub_network_members[UWB_LIST_SIZE - 1] = {0}; // cannot be more subnetworks than other UWBs
		uint64 sub_network_tsfs[UWB_LIST_SIZE - 1] = {0};
		uint8 sub_network_base_framelength[UWB_LIST_SIZE - 1] = {0};
		uint64 time_now_us = portGetTickCntMicro();
		uint64 tcommon = 0;
		uint64 shortestFrameDuration = this->maxFramelength * this->slotDuration_us;

		for (int i = 1; i < inst->uwbListLen; i++) // zero reserved for self
		{
			if (this->uwbListTDMAInfo[i].connectionType != UWB_LIST_NEIGHBOR)
			{
				continue;
			}

			struct TDMAInfo *info_i = &this->uwbListTDMAInfo[i];
			uint64 timeSinceFrameStart_us = get_dt64(this->uwbListTDMAInfo[i].frameStartTime, time_now_us);

			if (timeSinceFrameStart_us > 100000000)
			{
				sub_network_membership[i] = 255; // disregard
				continue;
			}

			// test the ith UWB against the jth subnetwork
			for (int j = 0; j < num_sub_networks; j++)
			{
				uint64 diff_us = 0;

				uint8 min_fl = info_i->framelength;
				if (sub_network_base_framelength[j] < min_fl)
				{
					min_fl = sub_network_base_framelength[j];
				}

				uint64 min_framelengthDuration = min_fl * this->slotDuration_us;
				uint64 diff_tsfs = 0;

				if (timeSinceFrameStart_us <= sub_network_tsfs[j])
				{
					diff_tsfs = sub_network_tsfs[j] - timeSinceFrameStart_us;
				}
				else
				{
					diff_tsfs = timeSinceFrameStart_us - sub_network_tsfs[j];
				}

				uint64 diff_tsfs_mod = diff_tsfs % min_framelengthDuration;

				if (diff_tsfs_mod <= 0.5 * min_framelengthDuration)
				{
					diff_us = diff_tsfs_mod;
				}
				else
				{
					diff_us = min_framelengthDuration - diff_tsfs_mod;
				}

				// if difference is below the threshold, it belongs to this subnetwork
				// if not, it may belong to another one already listed,
				// if not, create a new one...
				if (diff_us < this->frameSyncThreshold_us)
				{
					sub_network_members[j]++;
					sub_network_membership[i] = j;
					break;
				}
				else if (j == num_sub_networks - 1)
				{
					// reached the last listed sub_netowrk, list a new subnetwork.
					sub_network_members[num_sub_networks] = 1;
					sub_network_base_framelength[num_sub_networks] = this->uwbListTDMAInfo[i].framelength;
					sub_network_tsfs[num_sub_networks] = get_dt64(this->uwbListTDMAInfo[i].frameStartTime, time_now_us);
					sub_network_membership[num_sub_networks] = num_sub_networks;
					num_sub_networks++;
					break;
				}
			}

			// no subnetworks listed yet, set the first one.
			if (num_sub_networks == 0)
			{
				sub_network_members[num_sub_networks] = 1;
				sub_network_base_framelength[num_sub_networks] = this->uwbListTDMAInfo[i].framelength;
				sub_network_tsfs[num_sub_networks] = timeSinceFrameStart_us;
				sub_network_membership[num_sub_networks] = num_sub_networks;
				num_sub_networks++;
			}
		}

		// now select the subnetwork with the greatest number of uwbs
		uint8 max_num = 0;
		for (int i = 0; i < num_sub_networks; i++)
		{
			if (sub_network_members[i] > max_num)
			{
				max_num = sub_network_members[i];
				sub_network_selected = i;
			}
		}

		uint64 tnext[max_num];
		uint8 neighborIndices[max_num];
		uint64 latest_tnext = 0;
		uint8 nidx = 0;
		uint64 slotDuration_us = this->slotDuration_us;

		for (int i = 1; i < inst->uwbListLen; i++) // 0 reserved for self
		{
			if (this->uwbListTDMAInfo[i].connectionType == UWB_LIST_NEIGHBOR && sub_network_membership[i] == sub_network_selected)
			{
				neighborIndices[nidx] = i;
				tnext[nidx] = this->uwbListTDMAInfo[i].frameStartTime;
				while (time_now_us > tnext[nidx])
				{
					tnext[nidx] += this->uwbListTDMAInfo[i].framelength * slotDuration_us;
				}
				nidx++;

				if (this->uwbListTDMAInfo[i].framelength * slotDuration_us < shortestFrameDuration)
				{
					shortestFrameDuration = this->uwbListTDMAInfo[i].framelength * slotDuration_us;
				}
			}
		}

		tcommon = tnext[0];
		latest_tnext = tcommon;
		bool converged = FALSE;
		while (converged == FALSE)
		{
			converged = TRUE;
			for (int i = 0; i < nidx; i++)
			{
				uint64 frameduration = this->uwbListTDMAInfo[neighborIndices[i]].framelength * slotDuration_us;
				while (tnext[i] < tcommon && tcommon - tnext[i] >= frameduration - this->frameSyncThreshold_us)
				{
					tnext[i] += frameduration;
				}

				if (tnext[i] > tcommon && tnext[i] - tcommon >= this->frameSyncThreshold_us)
				{
					// increment the value based on tnext[0] to guarantee all uwbs
					// in this subnetwork will have frame sync errors within the threshold
					tcommon += shortestFrameDuration;
					converged = FALSE;
				}

				if (tnext[i] > latest_tnext)
				{
					latest_tnext = tnext[i];
				}
			}
		}

		// expire as the beginning of the common frame start time
		this->discovery_mode_duration = (uint32)(get_dt64(time_now_us, latest_tnext) / 1000);
		this->discovery_mode_expires = TRUE;

		sys_printf("NUM NET %u - DISCOVERY DURATION: %lu\r\n", num_sub_networks, this->discovery_mode_duration);

		this->free_slots(&this->uwbListTDMAInfo[0]);
		this->deconflict_slot_assignments(this);
		// assign self slot
		this->find_assign_slot(this);
		// construct SUG packet
		this->populate_inf_msg(this, RTLS_DEMO_MSG_INF_SUG);

		// back-track the frame start time so we can inform the need to rebase
		// and keep in sync with the subnetwork we initially chose to sync with
		this->uwbListTDMAInfo[0].frameStartTime = latest_tnext;
		uint64 myFrameDuration = this->uwbListTDMAInfo[0].framelength * this->slotDuration_us;

		// TODO: Bug if frameStartTime > time_now_us and myFrameDuration > time_now_us --> loop forever
		while (this->uwbListTDMAInfo[0].frameStartTime > time_now_us)
		{
			this->uwbListTDMAInfo[0].frameStartTime = timestamp_subtract64(this->uwbListTDMAInfo[0].frameStartTime, myFrameDuration); 
		}

		// sys_printf("FRAME START: %u\n", (uint32_t)this->uwbListTDMAInfo[0].frameStartTime);

		// while (this->uwbListTDMAInfo[0].frameStartTime > time_now_us)
		// {
		// 	this->uwbListTDMAInfo[0].frameStartTime -= myFrameDuration;
		// }

		uint64 myTimeSinceFrameStart = get_dt64(this->uwbListTDMAInfo[0].frameStartTime, time_now_us);
		// uint64 myTimeSinceFrameStart = get_dt64(this->uwbListTDMAInfo[0].frameStartTime, time_now);
		uint8 slot = myTimeSinceFrameStart / this->slotDuration_us; // integer division rounded down
		this->lastSlotStartTime64 = timestamp_add64(this->uwbListTDMAInfo[0].frameStartTime, (uint64)(this->slotDuration_us * slot));

		updateTestWithSlot(this, time_now_us, slot);
		break;
	}
	case SEND_SUG:
	{
		this->discovery_mode_duration = this->slotStartDelay_us * 2;
		this->discovery_mode_expires = TRUE;
		break;
	}
	case EXIT:
	{
		this->discovery_mode_duration = 0;
		this->discovery_mode_expires = FALSE;
		break;
	}
	case SLEEP_IN_DISCOVERY:
		this->discovery_mode_duration = CONFIG_SLEEP_IN_SEARCHING_MS;
		this->discovery_mode_expires = TRUE;
		break;
	default:
	{
		break;
	}
	}

}

/********************************************************************************
Function:
  check_discovery_mode_expiration()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Check if the discovery mode has expired.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void check_discovery_mode_expiration(struct TDMAHandler *this, uint32 *p_timeout, bool *p_need_sleep)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);

	if (inst->mode == DISCOVERY)
	{
		if (this->discovery_mode_expires == TRUE)
		{
			uint32 time_now = portGetTickCnt();
			uint32 timeSinceModeStart = get_dt32(this->discovery_mode_start_time, time_now);
			if (timeSinceModeStart >= this->discovery_mode_duration)
			{
				// discovery mode expired
				DISCOVERY_MODE new_mode = WAIT_INF_REG;
				if (this->discovery_mode == COLLECT_INF_REG)
				{
					new_mode = WAIT_SEND_SUG;
					inst->testAppState = TA_RXE_WAIT; // still collect RNG_REPORT messages while we wait to send our SUG message
					updateTestWithTxRx(inst, -1, -1);
				}
				else if (this->discovery_mode == WAIT_SEND_SUG)
				{
					// After collecting inf regs, we can now send our SUG message
					inst->testAppState = TA_TX_SELECT;
					new_mode = SEND_SUG;
					updateTestWithTxRx(inst, -1, -1);
				}
				else if (inst->goToSleep && this->discovery_mode == WAIT_RNG_INIT)
				{
					uint32 duration = get_dt32(this->sleepDiscoveryEndTime, time_now);
					if (duration >= CONFIG_SEARCHING_TIMEOUT_MS)
					{
						inst->testAppState = TA_RXE_WAIT;
						new_mode = SLEEP_IN_DISCOVERY;

						if (p_need_sleep)
						{
							*p_need_sleep = TRUE;
						}
					}
				}
				else if (this->discovery_mode == SLEEP_IN_DISCOVERY)
				{
					this->sleepDiscoveryEndTime = time_now;
					// Back to active searching mode

					/* Monitor searching process */
					app_monitor_callback();

				}

				this->set_discovery_mode(this, new_mode, time_now);
			}
			else
			{
				*p_timeout = CEIL_DIV(this->discovery_mode_duration - timeSinceModeStart, 1);
				dbg("[DIS EXPIRE] %u\r\n", *p_timeout);
			}
		}
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
//      END OF discovery.c
//###########################################################################################################
