/************************************************************************************************************
Module:       app_state

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/13/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "app_state.h"
#include "instance.h"
#include "instance_utilities.h"
#include "timestamp.h"
#include "network.h"
#include "uwb_dev_driver.h"
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

//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################

//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  app_state_exec()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Execute the state machine.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/13/2022
*********************************************************************************/
bool app_state_exec(instance_data_t *inst,
                    process_func process_hdl,
                    passed_func passed_hdl,
                    failed_func failed_hdl,
                    uint32 *p_timeout)
{
  if (!process_hdl || !passed_hdl || !failed_hdl)
  {
    return false;
  }

  bool ret = false;
  uint32 timeout = 0;

  if (process_hdl(inst, &timeout))
  {
    ret = passed_hdl(inst, timeout);

    if (p_timeout)
    {
      *p_timeout = timeout;
    }
  }
  else
  {
    sys_printf("[%lu][ERR] %s\r\n", timestamp_get_ms(), get_inst_states_string(inst->testAppState));

    failed_hdl(inst);
    ret = false;
  }

  return ret;
}
/********************************************************************************
Function:
  app_state_wait_timeout()
Input Parameters:
  ---
Output Parameters:
  Return remaining time in ms.
Description:
  Calculate the remaining time in ms before reaching the timeout.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/20/2022
*********************************************************************************/
uint32 app_state_wait_timeout(instance_data_t *inst, uint32 start_time, uint32 duration_ms)
{
  uint32 remaining_time_ms = 0;
  uint32 now = timestamp_get_ms();

  uint32 dt = get_dt32(start_time, now);
  if (dt < duration_ms)
  {
    remaining_time_ms = duration_ms - dt;
  }

  return remaining_time_ms;
}

/********************************************************************************
Function:
  app_state_rx_wait()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  App enter the RX wait state.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/13/2022
*********************************************************************************/
bool app_state_rx_wait(instance_data_t *inst)
{
  inst->previousState = TA_INIT;
  inst->nextState = TA_INIT;
  inst->testAppState = TA_RXE_WAIT;

  /* Clear the flag as the TX has failed the TRX is off */
  inst->wait4ack = 0;

  return true;
}

/********************************************************************************
Function:
  app_state_tx_confirm()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  App enter the wait TX confirm state.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/13/2022
*********************************************************************************/
bool app_state_tx_confirm(instance_data_t *inst, uint32 tx_timeout)
{
  inst->previousState = inst->testAppState;
  /* wait confirmation */
  inst->testAppState = TA_TX_WAIT_CONF; 
  inst->timeofTx = timestamp_get_ms();
  inst->txDoneTimeoutDuration = tx_timeout;
  return true;
}
/********************************************************************************
Function:
  app_state_tx_select()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  App enter the TX select state.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/14/2022
*********************************************************************************/
bool app_state_tx_select(instance_data_t *inst)
{
  inst->testAppState = TA_TX_SELECT;
  inst->previousState = TA_INIT;
  inst->nextState = TA_INIT;
  return true;
}

/********************************************************************************
Function:
	app_state_can_sleep()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Check if device can sleep now.
Notes:
	---
Author, Date:
	Toan Huynh, 03/19/2022
*********************************************************************************/
bool app_state_can_sleep(instance_data_t *inst)
{
  // When enter tag mode, the device will schedule to sleep before next sending
	return (inst->mode == TAG && inst->goToSleep);
}
/********************************************************************************
Function:
  app_state_change()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  App change the state.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/21/2022
*********************************************************************************/
bool app_state_change(instance_data_t *inst, INST_STATES new_state)
{
  inst->previousState = inst->testAppState;
  inst->testAppState = new_state;
  inst->nextState = new_state;
  return true;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/17/2023
*********************************************************************************/
static bool app_state_go_sleep(instance_data_t *inst, struct TDMAHandler *tdma_handler, uint32_t *p_timeout)
{
  bool ret = false;

  inst->sleepTime_ms = cal_sleep_time(tdma_handler, inst);

  if (inst->sleepTime_ms)
  {
    // the app should put chip into low power state and wake up in sleepTime_ms time...
    // the app could go to IDLE state and wait for up to wake it up...
    inst->testAppState = TA_SLEEP_DONE;

    /* Put device into sleep mode */
    instance_enter_sleep(inst);

    inst->timeofSleep = timestamp_get_ms();

    if(p_timeout)
    {
      *p_timeout = inst->sleepTime_ms;
    }

    sys_printf("[%lu] ENTER SLEEP IN %lu\r\n", inst->timeofSleep, inst->sleepTime_ms);

    ret = true;
  }

  return ret;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/17/2023
*********************************************************************************/
static bool app_state_sleep_delay_check(instance_data_t *inst, uint32_t *p_timeout)
{
  uint32_t now = timestamp_get_ms();
  uint32_t delta = get_dt32(inst->timeofSleep, now);
  // Device need to delay before sleep -> mark from tag transition time
  uint32_t delay = CONFIG_SLEEP_EXTRA_DELAY_MS + CEIL_DIV(inst->durationSlotMax_us, 1000);

  if(p_timeout && delta < delay)
  {
    *p_timeout = delay - delta;
  }

  // Check if it's time to sleep
  return delta >= delay;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/17/2023
*********************************************************************************/
bool app_state_process_sleep(instance_data_t *inst, struct TDMAHandler *tdma_handler, bool need_sleep, uint32_t *p_timeout)
{
  bool ret = false;
  static bool can_sleep = false;

  // if we are scheduled to go to sleep before next sending then sleep first.
  // go to sleep before sending the next poll
  if (can_sleep || app_state_can_sleep(inst) || need_sleep)
  {
    if(!can_sleep)
    {
      uint32_t now = timestamp_get_ms();
      // Store mark time to calcluate sleep time
      inst->timeofSleep = now;
      // Can sleep now --> Check if need to delay before sleep
      can_sleep = true;
      sys_printf("[%u] CAN SLEEP\r\n", now);
    }

    if (app_state_sleep_delay_check(inst, p_timeout))
    {
      ret = app_state_go_sleep(inst, tdma_handler, p_timeout);
      can_sleep = false;
    }
  }

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
//      END OF app_state.c
//###########################################################################################################
