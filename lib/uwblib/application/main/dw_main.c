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
// static SemaphoreHandle_t irqSemaphore;
// static SemaphoreHandle_t uwbSem;
// static SemaphoreHandle_t algoSemaphore;
static bool isInit = false;
static bool isInitCb = false;


//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################

#if ENABLE_SHOW_STATUS
static void show_status(void);
#endif

// static bool uwb_lock(void);
// static bool uwb_unlock(void);

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
	struct TDMAHandler *tdma_handler = (struct TDMAHandler *)get_tdma_handler();

	/* turn off all the LEDs */
	// led_off(LED_ALL); 

	peripherals_init();

	/* Init storage */
	cfgInit();

	/* Load stored configuration */
	inst_load_config(inst);

	/* Disable DW1000 IRQ until we configure the application */
	port_DisableEXT_IRQ(); 

	// led_off(LED_ALL);

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

// int dw_main_loop(void) 
// {
// 	// Initializing the low level radio handling
//   // algoSemaphore = xSemaphoreCreateMutex();
// 	// uwbSem = xSemaphoreCreateCounting(0xFFFFFFFF, 0);

// 	uint32 waiting_duration = 0;

// #if ENABLE_SHOW_STATUS
// 	uint32 time_accum = 0;
// #endif

// 	/* Main loop */
// 	while (1)
// 	{
// 		/* Run some statictis task */
// 		dw_task_setup();

// 		/* Wait until interrupt or timeout occurs */
// 		xSemaphoreTake(uwbSem, waiting_duration);

// 		waiting_duration = 0;

// 		if (uwb_lock())
// 		{
// 			/* Run the state machine */
// 			waiting_duration = app_state_machine_run(inst, tdma_handler);
// 			uwb_unlock();
// 		}

// 		if (waiting_duration != MAX_TIMEOUT)
// 		{
// 			waiting_duration = waiting_duration / portTICK_PERIOD_MS;
// 		}

// 	}
// }

/********************************************************************************
Function:
	dw_cb_main()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 02/22/2022
*********************************************************************************/
#if 0
void dw_cb_main(void)
{
	irqSemaphore = xSemaphoreCreateBinary();

	isInitCb = true;

	while (1)
	{
		/* Run some statictis task */
		uwb_cb_task_setup();

		if (xSemaphoreTake(irqSemaphore, portMAX_DELAY))
		{
			if (uwb_lock())
			{
				process_deca_irq();
				uwb_unlock();
			}
		}
	}
}

/********************************************************************************
Function:
	uwb_isr_callback()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 02/20/2022
*********************************************************************************/
void uwb_isr_callback(void)
{
  if(isInit && isInitCb)
  {
		BaseType_t higherPriorityTaskWoken;
    xSemaphoreGiveFromISR(irqSemaphore, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
  }
}

/********************************************************************************
Function:
	dw_app_signal()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 02/22/2022
*********************************************************************************/
void dw_app_signal(void)
{
  if(isInit)
  {
    xSemaphoreGive(uwbSem);
  }
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
	uwb_lock()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 02/20/2022
*********************************************************************************/
static bool uwb_lock(void)
{
  return xSemaphoreTake(algoSemaphore, portMAX_DELAY) == pdTRUE;
}

/********************************************************************************
Function:
	uwb_unlock()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 02/20/2022
*********************************************************************************/
static bool uwb_unlock(void)
{
  return xSemaphoreGive(algoSemaphore) == pdTRUE;
}

/********************************************************************************
Function:
	show_status()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 02/22/2022
*********************************************************************************/
#if ENABLE_SHOW_STATUS
static void show_status(void)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);

	if (!inst->isInDeepSleep)
	{
		if (inst->operationMode == CONFIG_OPERATION_TAG && inst->mode == TAG)
		{
			return;
		}

		if (!inst->canPrintLCD)
		{
			return;
		}
	}

#if CONFIG_ENABLE_HOST_CONNECTION
	send_device_net_info();
#else
	uint64 addr = instance_get_addr();
	uint8 num_neighbors = instfindnumneighbors(inst);
	uint8 num_hidden = instfindnumhidden(inst);
	char status[20] = {0};

	if (inst->mode == DISCOVERY)
	{
		strcpy(status, "SEARCHING");
	}
	else
	{
		strcpy(status, "CONNECTED");
	}

	sys_printf("0x%04X %s\r\n", (uint32_t)addr, status);
	sys_printf("N%02u H%02u\r\n", num_neighbors, num_hidden);
	// sys_printf("N%02u H%02u %05.2fm\r\n", num_neighbors, num_hidden, range_result);
	// sys_printf("%05.1fdB %05.2fm\r\n", inst->avgRSL, range_result);
#endif
}
#endif
#endif


//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF dw_main.c
//###########################################################################################################
