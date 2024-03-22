/************************************************************************************************************
Module:       network_mac

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
#include "network_mac.h"


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
  net_config_frame_header()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Function to construct the message/frame header bytes
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void net_config_frame_header(instance_data_t *inst)
{
	// configure ranging message
	inst->msg.panID[0] = (inst->panID) & 0xff;
	inst->msg.panID[1] = inst->panID >> 8;

	// set frame type (0-2), SEC (3), Pending (4), ACK (5), PanIDcomp(6)
	inst->msg.frameCtrl[0] = 0x1 /*frame type 0x1 == data*/ | 0x40 /*PID comp*/;
#if (USING_64BIT_ADDR == 1)
	// source/dest addressing modes and frame version
	inst->msg.frameCtrl[1] = 0xC /*dest extended address (64bits)*/ | 0xC0 /*src extended address (64bits)*/;
#else
	inst->msg.frameCtrl[1] = 0x8 /*dest short address (16bits)*/ | 0x80 /*src short address (16bits)*/;
#endif

	// configure RNG_INIT message
	inst->rng_initmsg.frameCtrl[0] = 0x41;

#if (USING_64BIT_ADDR == 1)
	inst->rng_initmsg.frameCtrl[1] = 0xCC;
#else
	inst->rng_initmsg.frameCtrl[1] = 0x8C;
#endif
	inst->rng_initmsg.panID[0] = (inst->panID) & 0xff;
	inst->rng_initmsg.panID[1] = inst->panID >> 8;

	// configure INF message
	inst->inf_msg.panID[0] = (inst->panID) & 0xff;
	inst->inf_msg.panID[1] = inst->panID >> 8;

	// set frame type (0-2), SEC (3), Pending (4), ACK (5), PanIDcomp(6)
	inst->inf_msg.frameCtrl[0] = 0x1 /*frame type 0x1 == data*/ | 0x40 /*PID comp*/;
#if (USING_64BIT_ADDR == 1)
	// source/dest addressing modes and frame version
	inst->inf_msg.frameCtrl[1] = 0x8 /*dest short address (16bits)*/ | 0xC0 /*src extended address (64bits)*/;
#else
	inst->inf_msg.frameCtrl[1] = 0x8 /*dest short address (16bits)*/ | 0x80 /*src short address (16bits)*/;
#endif

	// configure host message
	inst->host_msg.panID[0] = (inst->panID) & 0xff;
	inst->host_msg.panID[1] = inst->panID >> 8;

	// set frame type (0-2), SEC (3), Pending (4), ACK (5), PanIDcomp(6)
	inst->host_msg.frameCtrl[0] = 0x1 /*frame type 0x1 == data*/ | 0x40 /*PID comp*/;
#if (USING_64BIT_ADDR == 1)
	// source/dest addressing modes and frame version
	inst->host_msg.frameCtrl[1] = 0x8 /*dest short address (16bits)*/ | 0xC0 /*src extended address (64bits)*/;
#else
	inst->host_msg.frameCtrl[1] = 0x8 /*dest short address (16bits)*/ | 0x80 /*src short address (16bits)*/;
#endif

	// configure RNG_REPORT
	inst->report_msg.panID[0] = (inst->panID) & 0xff;
	inst->report_msg.panID[1] = inst->panID >> 8;

	// set frame type (0-2), SEC (3), Pending (4), ACK (5), PanIDcomp(6)
	inst->report_msg.frameCtrl[0] = 0x1 /*frame type 0x1 == data*/ | 0x40 /*PID comp*/;
#if (USING_64BIT_ADDR == 1)
	// source/dest addressing modes and frame version
	inst->report_msg.frameCtrl[1] = 0x8 /*dest short address (16bits)*/ | 0xC0 /*src extended address (64bits)*/;
#else
	inst->report_msg.frameCtrl[1] = 0x8 /*dest short address (16bits)*/ | 0x80 /*src short address (16bits)*/;
#endif

	// configure SYNC message
	inst->sync_msg.panID[0] = (inst->panID) & 0xff;
	inst->sync_msg.panID[1] = inst->panID >> 8;

	// set frame type (0-2), SEC (3), Pending (4), ACK (5), PanIDcomp(6)
	inst->sync_msg.frameCtrl[0] = 0x1 /*frame type 0x1 == data*/ | 0x40 /*PID comp*/;
#if (USING_64BIT_ADDR == 1)
	// source/dest addressing modes and frame version
	inst->sync_msg.frameCtrl[1] = 0x8 /*dest short address (16bits)*/ | 0xC0 /*src extended address (64bits)*/;
#else
	inst->sync_msg.frameCtrl[1] = 0x8 /*dest short address (16bits)*/ | 0x80 /*src short address (16bits)*/;
#endif

	// configure BLINK
	// blink frames with IEEE EUI-64 tag ID
	inst->blinkmsg.frameCtrl = 0xC5;
}

/********************************************************************************
Function:
  net_config_messages()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Function to construct the fixed portions of the message definitions
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void net_config_messages(instance_data_t *inst)
{
	// initialize ranging message
	// set source address into the message structure
	memcpy(&inst->msg.sourceAddr[0], &inst->eui64[0], inst->addrByteSize);

	// initialize RNG_INIT message
	// set source address into the message structure
	memcpy(&inst->rng_initmsg.sourceAddr[0], &inst->eui64[0], inst->addrByteSize);
	inst->rng_initmsg.messageData[FCODE] = RTLS_DEMO_MSG_RNG_INIT;

	// configure INF message
	uint16 broadcast_address = BROADCAST_ADDRESS;
	memcpy(&inst->inf_msg.sourceAddr[0], &inst->eui64[0], inst->addrByteSize);
	memcpy(&inst->inf_msg.destAddr[0], &broadcast_address, 2);
	inst->inf_msg.messageData[FCODE] = 0; // message function code (specifies if message is a poll, response or other...)
	
	// configure host message
	memcpy(&inst->host_msg.sourceAddr[0], &inst->eui64[0], inst->addrByteSize);
	memcpy(&inst->host_msg.destAddr[0], &broadcast_address, 2);
	inst->host_msg.messageData[FCODE] = RTLS_HOST_MSG; // message function code (specifies if message is a poll, response or other...)

	// configure RNG_REPORT
	memcpy(&inst->report_msg.sourceAddr[0], &inst->eui64[0], inst->addrByteSize);
	memcpy(&inst->report_msg.destAddr[0], &broadcast_address, 2);
	inst->report_msg.messageData[FCODE] = RTLS_DEMO_MSG_RNG_REPORT; // message function code (specifies if message is a poll, response or other...)

	// configure SYNC message
	memcpy(&inst->sync_msg.sourceAddr[0], &inst->eui64[0], inst->addrByteSize);
	memcpy(&inst->sync_msg.destAddr[0], &broadcast_address, 2);
	inst->sync_msg.messageData[FCODE] = RTLS_DEMO_MSG_SYNC; // message function code (specifies if message is a poll, response or other...)

	// configure BLINK message
	memcpy(&inst->blinkmsg.tagID[0], &inst->eui64[0], ADDR_BYTE_SIZE_L);
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
