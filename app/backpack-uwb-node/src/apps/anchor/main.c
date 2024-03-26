/************************************************************************************************************
Module:       main

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
01/06/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/
#define PW_LOG_MODULE_NAME "anchor-node"

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <pw_log/log.h>
#include "instance.h"
#include "host_msg.h"
#include <uwb_dev_parser.h>
#include <app_main.h>
#include <dw_main.h>
#include <host_connection.h>


//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################


//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
// #define MAIN_UWB_STACK_SIZE (3 * 1024)
// #define MAIN_UWB_PRIORITY   5

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
static struct k_sem uwb_sem;

K_MUTEX_DEFINE(uwb_mutex);

//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################


//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################



//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
static bool send_data_func(uint8_t* p_data, uint32_t len);


/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 03/26/2024
*********************************************************************************/
static void main_task(void *pvParameters, void *pvParameters2, void *pvParameters3) 
{
	uint32_t waiting_duration = 0;
  instance_data_t *inst = instance_get_local_structure_ptr(0);
  struct TDMAHandler *tdma_handler = (struct TDMAHandler *)get_tdma_handler();

  /* Setup some leds and interupt */
  dw1000_board_hw_init();
  
	// Initializing the low level radio
  dw_main_init();
  
  // init semaphore uwb_sem
  k_sem_init(&uwb_sem, 0, 0xFFFFFFFF);

	/* Main loop */
	while (1)
	{
		/* Wait until interrupt or timeout occurs */
    k_sem_take(&uwb_sem, K_MSEC(waiting_duration));

		waiting_duration = 0;

    if (k_mutex_lock(&uwb_mutex, K_FOREVER) == 0) {
			/* Run the state machine */
			waiting_duration = app_state_machine_run(inst, tdma_handler);
      
      k_mutex_unlock(&uwb_mutex);
		}
  }
}

/********************************************************************************
Function:
  main()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 01/06/2022
*********************************************************************************/
int main(void) {

  PW_LOG_INFO("Backpack UWB Node starting..");
  printk("Backpack UWB Node starting..\n");

  // host_com_register_interval_callback(5000, send_device_state_info);


  /* Initialize the host communication */
  host_connection_init(send_data_func, HOST_CONNECTION_MODE_PROTOBUF);

  uwb_dev_parser_init();

  main_task(NULL, NULL, NULL);

  /* This task is used for forwarding the received packets to the host */
  // host_com_task_init(512, configMAX_PRIORITIES - 3);

  return 0;
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################
void dw1000_hw_isr_work_handler(struct k_work * item) {
  if (k_mutex_lock(&uwb_mutex, K_FOREVER) == 0) {
    // Process the interrupt
    dwt_isr();
    k_mutex_unlock(&uwb_mutex);
  }
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 03/26/2024
*********************************************************************************/
void dw_app_signal(void) {
  k_sem_give(&uwb_sem);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 03/26/2024
*********************************************************************************/
bool host_com_send_signal(uint8_t *p_data, uint32_t len) {
  // TODO: Implement this function
  return false;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 03/26/2024
*********************************************************************************/
static bool send_data_func(uint8_t *p_data, uint32_t len) {
  // TODO: Implement this function
  return false;
}

//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF main.c
//###########################################################################################################
