/************************************************************************************************************
Module:       instance

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
#include "instance.h"
#include "deca_device_api.h"
#include "deca_spi.h"
#include "deca_regs.h"
#include "system_dbg.h"
#include "timestamp.h"
#include "uwb_dev_isr.h"
#include "uwb_event.h"
#include "instance_utilities.h"
#include "lib.h"



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
extern uint8 dwnsSFDlen[];
instance_data_t instance_data[NUM_INST];
struct TDMAHandler tdma_handler;




//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
	instance_init_s()
Input Parameters:
	---
Output Parameters:
	Returns 0 on success and -1 on error
Description:
	Function to initialise instance structures
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
int instance_init_s()
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);

	inst->mode = DISCOVERY;
	inst->testAppState = TA_INIT;
	updateTestWithTxRx(inst, -1, -1);

	// if using auto CRC check (DWT_INT_RFCG and DWT_INT_RFCE) are used instead of DWT_INT_RDFR flag
	// other errors which need to be checked (as they disable receiver) are
	dwt_setinterrupt(SYS_MASK_VAL, 1);

#if (CONFIG_ENABLE_LOW_POWER_MODE == 0)
	// this is platform dependent - only program if DW EVK/EVB
	dwt_setleds(3); // configure the GPIOs which control the LEDs on EVBs
#endif

	dwt_setcallbacks(instance_txcallback, instance_rxgoodcallback, instance_rxtimeoutcallback, instance_rxerrorcallback, instance_irqstuckcallback);

#if (USING_64BIT_ADDR == 0)
	inst->addrByteSize = ADDR_BYTE_SIZE_S;
#else
	inst->addrByteSize = ADDR_BYTE_SIZE_L;
#endif

	inst->uwbToRangeWith = 255;

	return 0;
}


/********************************************************************************
Function:
	instance_init_timings()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Pre-compute frame lengths, timeouts and delays needed in ranging process.
 	This function assumes that there is no user payload in the frame.
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
void instance_init_timings(void)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	uint32 pre_len;
	int sfd_len;

	static const int data_len_bytes[FRAME_TYPE_NB] = {
			BLINK_FRAME_LEN_BYTES, RNG_INIT_FRAME_LEN_BYTES, POLL_FRAME_LEN_BYTES,
			RESP_FRAME_LEN_BYTES, FINAL_FRAME_LEN_BYTES, REPORT_FRAME_LEN_BYTES, SYNC_FRAME_LEN_BYTES, INF_FRAME_LEN_BYTES_MAX};

	// Margin used for timeouts computation.
	uint64 margin_us = 200;

	// All internal computations are done in tens of picoseconds before
	// conversion into microseconds in order to ensure that we keep the needed
	// precision while not having to use 64 bits variables.

	// Compute frame lengths.
	// First step is preamble plus SFD length.
	sfd_len = dwnsSFDlen[inst->configData.dataRate];
	switch (inst->configData.txPreambLength)
	{
	case DWT_PLEN_4096:
		pre_len = 4096;
		break;
	case DWT_PLEN_2048:
		pre_len = 2048;
		break;
	case DWT_PLEN_1536:
		pre_len = 1536;
		break;
	case DWT_PLEN_1024:
		pre_len = 1024;
		break;
	case DWT_PLEN_512:
		pre_len = 512;
		break;
	case DWT_PLEN_256:
		pre_len = 256;
		break;
	case DWT_PLEN_128:
		pre_len = 128;
		break;
	case DWT_PLEN_64:
	default:
		pre_len = 64;
		break;
	}
	pre_len += sfd_len;
	// Convert preamble length from symbols to time. Length of symbol is defined
	// in IEEE 802.15.4 standard.
	if (inst->configData.prf == DWT_PRF_16M)
		pre_len *= 99359;
	else
		pre_len *= 101763;

	inst->storedPreLen = pre_len; // store to be used later with inf messages and frame_sync
	inst->storedPreLen_us = CEIL_DIV(pre_len, 100000);

	// Second step is data length for all frame types.
	for (int i = 0; i < FRAME_TYPE_NB; i++)
	{
		inst->frameLengths_us[i] = instance_getmessageduration_us(data_len_bytes[i]);
	}

	// delayed tx durations
	uint8 reply_margin_us = 25;
	uint64 duration = 0;
	duration += inst->frameLengths_us[POLL] - inst->storedPreLen_us + RX_TO_CB_DLY_US;	// poll rx timestamp to poll rx cb
	duration += RX_CB_TO_TX_CMD_DLY_US + MIN_DELAYED_TX_DLY_US + inst->storedPreLen_us; // poll rx cb to resp tx timestamp
	uint64 respDelay_us = duration + reply_margin_us;
	uint64 respDelay = convert_microsec_to_device_time_u64(respDelay_us);

	duration = 0;
	duration += inst->frameLengths_us[RESP] - inst->storedPreLen_us + RX_TO_CB_DLY_US;	// resp rx timestamp to resp rx cb
	duration += RX_CB_TO_TX_CMD_DLY_US + MIN_DELAYED_TX_DLY_US + inst->storedPreLen_us; // resp rx cb to final tx timestamp
	uint64 finalDelay_us = duration + reply_margin_us;
	uint64 finalDelay = convert_microsec_to_device_time_u64(finalDelay_us);

	// make reply times the same to minimize clock drift error. See Application Note APS011 for more information
	inst->respReplyDelay = inst->finalReplyDelay = MAX(respDelay, finalDelay);
	uint64 replyDelay_us = MAX(respDelay_us, finalDelay_us);

	// POLL TX TS TO FINAL TX TS
	duration = 0;
	duration += respDelay_us + finalDelay_us; // poll tx ts to final tx ts
	uint64 pollTxToFinalTx = duration;

	// Delay between blink reception and ranging init message transmission.
	inst->rnginitReplyDelay = convert_microsec_to_device_time_u64(MIN_DELAYED_TX_DLY_US + inst->storedPreLen_us); // rng_init tx cmd to rng_init tx ts

	margin_us = 5000;
	// rx timeout durations (_nus units are 1.0256us)
	duration = 0;
	duration += TX_CMD_TO_TX_CB_DLY_US + replyDelay_us;							 // poll tx cmd to resp tx ts
	duration += inst->frameLengths_us[RESP] - inst->storedPreLen_us; // resp tx ts to resp tx cb
	duration += RX_TO_CB_DLY_US + RX_CB_TO_TX_CMD_DLY_US;						 // resp tx cb to final tx cmd
	duration += margin_us;
	inst->durationPollTimeout_nus = (uint16)(duration / 1.0256) + 1;
	inst->durationPollTimeout_ms = (uint16)CEIL_DIV(duration, 1000);

	duration = 0;
	duration += TX_CMD_TO_TX_CB_DLY_US + inst->frameLengths_us[FINAL];																						 // final tx cmd to final tx cb
	duration += RX_TO_CB_DLY_US + RX_CB_TO_TX_CMD_DLY_US;																													 // final tx cb to place final
	duration += (uint64)MEASURED_SLOT_DURATIONS_US / 2;																														 // place final to report tx cmd
	duration += TX_CMD_TO_TX_CB_DLY_US + inst->frameLengths_us[REPORT] + RX_TO_CB_DLY_US + RX_CB_TO_TX_CMD_DLY_US; // report tx cmd to place report
	duration += margin_us;
	inst->durationFinalTimeout_ms = (uint16)CEIL_DIV(duration, 1000);

	margin_us = 5000;
	// tx conf timeout durations
	inst->durationBlinkTxDoneTimeout_ms = CEIL_DIV(TX_CMD_TO_TX_CB_DLY_US + inst->frameLengths_us[BLINK] + margin_us, 1000);			// tx cmd to tx cb
	inst->durationRngInitTxDoneTimeout_ms = CEIL_DIV(TX_CMD_TO_TX_CB_DLY_US + inst->frameLengths_us[RNG_INIT] + margin_us, 1000); // tx cmd to tx cb
	inst->durationPollTxDoneTimeout_ms = CEIL_DIV(TX_CMD_TO_TX_CB_DLY_US + inst->frameLengths_us[POLL] + margin_us, 1000);				// tx cmd to tx cb
	inst->durationReportTxDoneTimeout_ms = CEIL_DIV(TX_CMD_TO_TX_CB_DLY_US + inst->frameLengths_us[REPORT] + margin_us, 1000);		// tx cmd to tx cb
	inst->durationSyncTxDoneTimeout_ms = CEIL_DIV(TX_CMD_TO_TX_CB_DLY_US + inst->frameLengths_us[SYNC] + margin_us, 1000);				// tx cmd to tx cb

	uint32 fl = 0;
	if (inst->frameLengths_us[RESP] < inst->frameLengths_us[FINAL])
	{
		fl = inst->frameLengths_us[RESP] - inst->storedPreLen_us;
	}
	else
	{
		fl = inst->frameLengths_us[FINAL] - inst->storedPreLen_us;
	}
	duration = 0;
	duration += replyDelay_us - fl;
	duration -= RX_TO_CB_DLY_US + RX_CB_TO_TX_CMD_DLY_US;
	inst->durationRespTxDoneTimeout_ms = CEIL_DIV(duration + inst->frameLengths_us[RESP] - inst->storedPreLen_us + margin_us, 1000);	 // tx cmd to tx cb
	inst->durationFinalTxDoneTimeout_ms = CEIL_DIV(duration + inst->frameLengths_us[FINAL] - inst->storedPreLen_us + margin_us, 1000); // tx cmd to tx cb

	// figure maximum duration of a TDMA slot in microseconds
	duration = 0;
	duration += SLOT_START_BUFFER_US;																																																// frame start buffer
	duration += SLOT_BUFFER_EXP_TO_POLL_CMD_US;																																											// buffer expiration to cmd poll
	duration += TX_CMD_TO_TX_CB_DLY_US + pollTxToFinalTx + inst->frameLengths_us[FINAL] + RX_TO_CB_DLY_US + RX_CB_TO_TX_CMD_DLY_US; // poll cmd to place final
	// duration += B //place final to cmd report
	duration += TX_CMD_TO_TX_CB_DLY_US + inst->frameLengths_us[REPORT] + RX_TO_CB_DLY_US + RX_CB_TO_TX_CMD_DLY_US; // cmd report to place report
	// duration += C //place report to cmd inf
	duration += TX_CMD_TO_TX_CB_DLY_US + inst->frameLengths_us[INF_MAX] + RX_TO_CB_DLY_US + RX_CB_TO_TX_CMD_DLY_US; // cmd INF to place INF
	// duration += D place inf to process inf

	// MEASURED_SLOT_DURATIONS_US is experimentally found value found for B+C+D described above
	duration += (uint64)MEASURED_SLOT_DURATIONS_US;

	// add some time to account for possibly timing out on first poll
	duration += SLOT_BUFFER_EXP_TO_POLL_CMD_US;																																																				 // assume this is the same amount of time go get from timeout to poll command
	duration += TX_CMD_TO_TX_CB_DLY_US + inst->frameLengths_us[POLL] - inst->storedPreLen_us + (uint64)(inst->durationPollTimeout_nus * 1.0256) + 200; // add small margin of 200

	// if LCD is on, add time to allow for Sleep calls in the LCD display logic
	bool enableLCD = FALSE;

#if 0
	if (port_is_switch_on(TA_SW1_4) == S1_SWITCH_ON)
	{
		enableLCD = TRUE;
		duration += LCD_ENABLE_BUFFER_US;
	}
#endif

	duration += SLOT_END_BUFFER_US;

	inst->durationSlotMax_us = duration;

	// Count to reach ranging timeout
	inst->rangCnt = 0;  // This will be calculated after joining the network.

	duration = 0;
	// from set discovery to cmd tx blink, to rx blink cb to cmd resp to rx resp ts
	duration += TX_CMD_TO_TX_CB_DLY_US + inst->frameLengths_us[BLINK] + RX_TO_CB_DLY_US + BLINK_RX_CB_TO_RESP_TX_CMD_DLY_US + (uint64)(convert_device_time_to_sec(inst->rnginitReplyDelay) * 1000000.0) + RANGE_INIT_RAND_US + margin_us;
	if (enableLCD == TRUE)
	{
		duration += LCD_ENABLE_BUFFER_US * 2;
	}
	inst->durationWaitRangeInit_ms = CEIL_DIV(duration, 1000);

	// Smart Power is automatically applied by DW chip for frame of which length
	// is < 1 ms. Let the application know if it will be used depending on the
	// length of the longest frame.
	if (inst->frameLengths_us[FINAL] <= 1000)
	{
		inst->smartPowerEn = 1;
	}
	else
	{
		inst->smartPowerEn = 0;
	}
}


/********************************************************************************
Function:
	instance_get_local_structure_ptr()
Input Parameters:
	---
Output Parameters:
	---
Description:
 	Function to return the pointer to local instance data structure
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
instance_data_t *instance_get_local_structure_ptr(unsigned int x)
{
	if (x >= NUM_INST)
	{
		return NULL;
	}

	return &instance_data[x];
}

/********************************************************************************
Function:
	tdma_init_s()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Function to initialise tdma structures
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
int tdma_init_s(uint64 slot_duration)
{
	tdma_handler = TDMAHandler.new(slot_duration);
	return 0;
}

/********************************************************************************
Function:
	instgetuwblistindex()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Function to get the list index of a UWB. UWBs not already in the list are added
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
int instgetuwblistindex(instance_data_t *inst, uint8 *uwbAddr, uint8 addrByteSize)
{
	uint8 blank[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	char uwbChar[2];
	memcpy(&uwbChar[0], &uwbAddr[0], 2);

	// add the new UWB to the list, if not already there and there is space
	for (uint8 i = 0; i < UWB_LIST_SIZE; i++)
	{
		if (memcmp(&inst->uwbList[i][0], &uwbAddr[0], addrByteSize) != 0)
		{
			if (memcmp(&inst->uwbList[i][0], &blank[0], addrByteSize) == 0) // blank entry
			{
				memcpy(&inst->uwbList[i][0], &uwbAddr[0], addrByteSize);
				inst->uwbListLen = i + 1;
				return i;
			}
		}
		else
		{
			return i;
		}
	}

	return 255;
}

/********************************************************************************
Function:
	instfindnumneighbors()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Function to find the number of neighbor UWBs in our list that are not in a timeout status
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
int instfindnumneighbors(instance_data_t *inst)
{
	uint8 num = 0;

	for (int i = 1; i < inst->uwbListLen; i++) // 0 reserved for self, cant be neighbor
	{
		if (tdma_handler.uwbListTDMAInfo[i].connectionType == UWB_LIST_NEIGHBOR)
		{
			num++;
		}
	}

	return num;
}
/********************************************************************************
Input:
	---
Output:
	---
Description:
	---
Author, Date:
	Toan Huynh, 11/13/2023
*********************************************************************************/
uint32 inst_get_neighbors_list(instance_data_t *inst, uint32_t *buf, uint32_t max_cnt)
{
	if(!buf)
	{
		return 0;
	}

	uint32_t num = 0;

	for (int i = 1; i < inst->uwbListLen && num < max_cnt; i++) // 0 reserved for self, cant be neighbor
	{
		if (tdma_handler.uwbListTDMAInfo[i].connectionType == UWB_LIST_NEIGHBOR)
		{
			buf[num++] = (uint32_t)instance_get_uwbaddr(i);
		}
	}

	return num;
}

/********************************************************************************
Function:
	instfindnumhidden()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Function to find the number of hidden neighbor UWBs in our list that are not in a timeout status
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
int instfindnumhidden(instance_data_t *inst)
{
	uint8 num = 0;

	for (int i = 1; i < inst->uwbListLen; i++) // 0 reserved for self, cannot be hidden
	{
		if (tdma_handler.uwbListTDMAInfo[i].connectionType == UWB_LIST_HIDDEN)
		{
			num++;
		}
	}

	return num;
}

/********************************************************************************
Function:
	instclearuwbList()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Clear the UWB list
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
void instclearuwbList(void)
{
	int instance = 0;
	uint8 blank[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	instance_data[instance].uwbListLen = 0;
	instance_data[instance].uwbToRangeWith = 255;

	for (int i = 0; i < UWB_LIST_SIZE; i++)
	{
		memcpy(&instance_data[instance].uwbList[i][0], &blank[0], 8);
	}
}

/********************************************************************************
Function:
	instanceclearcounts()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Function to clear counts/averages/range values
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
void instanceclearcounts(void)
{
	int instance = 0;

	instance_data[instance].frameSN = 0;

	dwt_configeventcounters(1); // enable and clear - NOTE: the counters are not preserved when in DEEP SLEEP

	instance_data[instance].frameSN = 0;
	instance_data[instance].txmsgcount = 0;
	instance_data[instance].rxmsgcount = 0;
	instance_data[instance].lateTX = 0;
	instance_data[instance].lateRX = 0;

	instclearuwbList();

}
/********************************************************************************
Function:
	instance_init()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Function to initialise instance structures
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
int instance_init(void)
{
	int instance = 0;
	int result;

	for (uint8 i = 0; i < UWB_LIST_SIZE; i++)
	{
		instance_data[instance].tof[i] = 0;
		instance_data[instance].idistance[i] = 0;
		// instance_data[instance].idistanceraw[i] = 0;
	}

	memset(instance_data[instance].iPosition, 0xFF, sizeof(instance_data[instance].iPosition));

	instance_data[instance].newRangeUWBIndex = 0;

	instance_data[instance].rslCnt = 0;
	instance_data[instance].idxRSL = 0;
	instance_data[instance].avgRSL = 0;
	// for (uint8 i = 0; i < NUM_RSL_AVG; i++)
	// {
	// 	instance_data[instance].RSL[i] = 0;
	// }

	// Reset the IC (might be needed if not getting here from POWER ON)
	dwt_softreset();

	// we can enable any configuration loading from OTP/ROM on initialization
	result = dwt_initialise(DWT_LOADUCODE);

#if (CONFIG_ENABLE_LOW_POWER_MODE == 0)
	// this is platform dependent - only program if DW EVK/EVB
	dwt_setleds(3); // configure the GPIOs which control the leds on EVBs
#endif

	if (DWT_SUCCESS != result)
	{
		return (-1); // device initialize has failed
	}

	// enable TX, RX states on GPIOs 6 and 5
	dwt_setlnapamode(1, 1);

	instanceclearcounts();

	// instance_data[instance].panID = CONFIG_DEFAULT_PAN_UUID;
	instance_data[instance].wait4ack = 0;

	instance_clearevents();

#if 0
	dwt_geteui(instance_data[instance].eui64);
#endif

	instance_data[instance].clockOffset = 0;
	instance_data[instance].gotTO = FALSE;

	/* Low power mode */
	instance_data[instance].goToSleep = (instance_data[instance].operationMode == CONFIG_OPERATION_TAG);
	instance_data[instance].lowPowerEnable = (CONFIG_ENABLE_LOW_POWER_MODE == 1);
	instance_data[instance].isInDeepSleep = FALSE;

	return 0;
}


/********************************************************************************
Function:
	instance_get_idist()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Get instantaneous range corrected by distance
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
double instance_get_idist(uint8 uwb_index) 
{
	double x = instance_data[0].idistance[uwb_index];

	return (x);
}
/********************************************************************************
Function:
	instance_get_idistrsl()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Get instantaneous range corrected by rsl
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
double instance_get_idistrsl(uint8 uwb_index) 
{
	double x = instance_data[0].idistancersl[uwb_index];

	return (x);
}

/********************************************************************************
Function:
	instanceconfigantennadelays()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Get antenna delays
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
void instanceconfigantennadelays(uint16 tx, uint16 rx)
{
	instance_data[0].txAntennaDelay = tx;
	instance_data[0].rxAntennaDelay = rx;

	instance_data[0].antennaDelayChanged = 1;
}

/********************************************************************************
Function:
	instancesetantennadelays()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Set antenna delays
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
void instancesetantennadelays(void)
{
	if (instance_data[0].antennaDelayChanged == 1)
	{
		dwt_setrxantennadelay(instance_data[0].rxAntennaDelay);
		dwt_settxantennadelay(instance_data[0].txAntennaDelay);

		instance_data[0].antennaDelayChanged = 0;
	}
}

/********************************************************************************
Function:
	get_tdma_handler()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Get TDMA handler
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
const struct TDMAHandler *get_tdma_handler(void)
{
	return (const struct TDMAHandler *)&tdma_handler;
}

/********************************************************************************
Function:
	instance_store_neigbor_position()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Get and store neighbor position
Notes:
	---
Author, Date:
	Toan Huynh, 07/21/2022
*********************************************************************************/
bool instance_store_neigbor_position(instance_data_t *inst, uwb_msg_info_t *p_msg)
{
	bool result = FALSE;

#if ENABLE_ANCHOR_POSITION
	/* Get anchor position */
	position_t *p_anchor_position = (position_t *)&p_msg->p_data[POLL_POSITION];
	uint8 anchor_index = instgetuwblistindex(inst, &p_msg->srcAddr[0], inst->addrByteSize);

	/* Store anchor position */
	if (anchor_index <= UWB_LIST_SIZE)
	{
		memcpy(&inst->iPosition[anchor_index], p_anchor_position, sizeof(position_t));
	}
	result = TRUE;
#endif

	return result;
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
//      END OF instance.c
//###########################################################################################################
