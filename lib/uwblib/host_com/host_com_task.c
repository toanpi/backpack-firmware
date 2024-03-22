/************************************************************************************************************
Module:       host_com_task

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
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "host_com_task.h"
#include "host_com_msg.h"

//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define PACKAGE_PROCESS_BUFF_SIZE        256


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
static SemaphoreHandle_t hostSendSem = NULL;
static SemaphoreHandle_t hostSendMutex = NULL;
static TaskHandle_t host_com_send_task = NULL;
static uint8_t package[PACKAGE_PROCESS_BUFF_SIZE];
static uint32_t package_length = 0;

//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static bool host_send_lock(void);
static bool host_send_unlock(void);
static void host_send_entry(void *pvParameters);



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  host_com_task_init()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 04/13/2022
*********************************************************************************/
void host_com_task_init(uint32_t stack_size, uint16_t priority)
{
  memset(package, 0, PACKAGE_PROCESS_BUFF_SIZE);

  hostSendMutex = xSemaphoreCreateMutex();
  hostSendSem = xSemaphoreCreateBinary();

  xTaskCreate(host_send_entry, "host com send task", stack_size, NULL, priority, &host_com_send_task);
}


/********************************************************************************
Function:
  host_com_send_signal()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Signal to host_com_send_task to send data
Notes:
  ---
Author, Date:
  Toan Huynh, 04/13/2022
*********************************************************************************/
bool host_com_send_signal(uint8_t *p_data, uint32_t len)
{
  if (p_data == NULL || len == 0)
  {
    return false;
  }

  bool ret = false;

  if (host_send_lock())
  {
    if (len <= PACKAGE_PROCESS_BUFF_SIZE)
    {
      memcpy(package, p_data, len);
      package_length = len;
      ret = true;
    }

    host_send_unlock();
  }

  if (hostSendSem && ret)
  {
    xSemaphoreGive(hostSendSem);
  }

  return ret;
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  host_send_entry()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Host send task entry point.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
static void host_send_entry(void *pvParameters)
{
  while (1)
  {
    if (xSemaphoreTake(hostSendSem, portMAX_DELAY))
    {
      if (host_send_lock())
      {
        host_com_forward(package, package_length);

        // Clear buffer
        memset(package, 0, sizeof(package));
        package_length = 0;

        host_send_unlock();
      }
    }
  }
}

/********************************************************************************
Function:
	host_send_lock()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Lock the host send task
Notes:
	---
Author, Date:
	Toan Huynh, 02/20/2022
*********************************************************************************/
static bool host_send_lock(void)
{
  return hostSendMutex && xSemaphoreTake(hostSendMutex, portMAX_DELAY) == pdTRUE;
}

/********************************************************************************
Function:
	host_send_unlock()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Unlock the host send task
Notes:
	---
Author, Date:
	Toan Huynh, 02/20/2022
*********************************************************************************/
static bool host_send_unlock(void)
{
  return hostSendMutex && xSemaphoreGive(hostSendMutex) == pdTRUE;
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF host_com_task.c
//###########################################################################################################

