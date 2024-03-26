/************************************************************************************************************
Module:       dw_main

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
#include "port_mcu.h"
#include "instance.h"
#include "tdma_handler.h"
#include "deca_types.h"
#include "stdio.h"
#include "deca_regs.h"
#include "dev_cfg.h"
#include "dw_main.h"
#include "instance_config.h"
#include "host_com_msg.h"
#include "app_main.h"
#include "uwb_dev_driver.h"
#include "dev_test.h"

#if CONFIG_ENABLE_LOCATION_ENGINE
#include "location_smoothing.h"
#endif

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
static bool isInit = false;
// static bool isInitCb = false;


//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################


//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
	inittestapplication()
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
uint32 inittestapplication(void)
{
	uint32 devID;
	int result;

	port_set_dw1000_slowrate(); // max SPI before PLLs configured is ~4M

	// this is called here to wake up the device (i.e. if it was in sleep mode before the restart)
	devID = instance_read_device_id();
	if (DWT_DEVICE_ID != devID) // if the read of device ID fails, the DW1000 could be asleep
	{
		port_wakeup_dw1000();

		devID = instance_read_device_id();
		// SPI not working or Unsupported Device ID
		if (DWT_DEVICE_ID != devID)
			return (-1);
		// clear the sleep bit - so that after the hard reset below the DW does not go into sleep
		dwt_softreset();
	}

	// reset the DW1000 by driving the RSTn line low
	reset_DW1000();

	result = instance_init();
	if (0 > result)
		return (-1); // Some failure has occurred

	port_set_dw1000_fastrate();
	devID = instance_read_device_id();

	if (DWT_DEVICE_ID != devID) // Means it is NOT DW1000 device
	{
		// SPI not working or Unsupported Device ID
		return (-1);
	}

	instance_init_s();
	instance_data_t *inst = instance_get_local_structure_ptr(0);

	instance_config(inst); // Set operating channel etc

	instance_init_timings();

#if CONFIG_ENABLE_LOCATION_ENGINE
	/* Init location smoothing algorithm */
	location_smoothing_init();
#endif

	inst->mode = DISCOVERY;

	return devID;
}

/********************************************************************************
Function:
	dw_main()
Input Parameters:
	---
Output Parameters:
	---
Description:
 	Main entry point
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
int dw_main_init(void)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);

	/* turn off all the LEDs */
	led_off(LED_ALL); 

	peripherals_init();

	/* Init storage */
	cfgInit();

	/* Load stored configuration */
	inst_load_config(inst);

	/* Disable DW1000 IRQ until we configure the application */
	port_DisableEXT_IRQ(); 

	led_off(LED_ALL);

	if (inittestapplication() == (uint32)-1)
	{
		println("ERROR - inittestapplication() failed");
		return 0; // error
	}

	tdma_init_s(inst->durationSlotMax_us); // call after instance_init_timings() to get slot duration

	println("UWB NET VERSION : %d.%d", SOFTWARE_MAJOR_VER, SOFTWARE_MINOR_VER);
	println("MAX NETWORK SIZE: %d", UWB_LIST_SIZE);
	println("PAN ID          : 0x%x", inst->panID);
	println("OPERATION MODE  : %s", inst->operationMode == CONFIG_OPERATION_TAG ? "TAG" : "ANCHOR");
	println("SLOT DURATION   : %u us", (uint32_t)inst->durationSlotMax_us);
	println("TX DELAY        : %.5u", inst->txAntennaDelay);
	println("RX DELAY        : %.5u", inst->rxAntennaDelay);
	println("POSITION        : X=%.2f Y=%.2f Z=%.2f", inst->selfPosition.x, inst->selfPosition.y, inst->selfPosition.z);
	println("BUILD TIME      : %s", __TIMESTAMP__);
	showTxPower();

	/* Enable continous wave transmission */
#if ENABLE_CONTINUOUS_WAVE
	// this function does not return!
	configure_continuous_txspectrum_mode();
#endif

	port_EnableEXT_IRQ();

	isInit = true;

	return 0;
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
//      END OF dw_main.c
//###########################################################################################################
