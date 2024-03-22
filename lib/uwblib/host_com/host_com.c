/************************************************************************************************************
Module:       host_com

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
04/13/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "host_com.h"
#include "host_connection.h"
#include "packet.h"
#include "port_mcu.h"
#include "uwb_dev_parser.h"
#include "backpack_parser.h"
#include "host_msg.h"



//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define SERIAL_PROCESS_BUFF_SIZE 255
#define PACKET_END              '\n'

#define SEND_DATA_TO_HOST_TIMEOUT_MS    5000



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
static TaskHandle_t host_com_task;
static bool is_init = false;
static SemaphoreHandle_t host_com_sem = NULL;
const uint8_t PACKET_DELIMITER[2] = {'\r', '\n'};
static host_com_interval_callback interval_callback = NULL;
static uint32_t send_data_interval_ms = portMAX_DELAY;

//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static void host_com_main_task(void *pvParameters);
static bool send_data_func(uint8_t *p_data, uint32_t len);

//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  host_com_init()
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
void host_com_init(uint32_t stack_size, uint16_t priority, host_com_mode_e mode)
{
  port_serial_init(host_com_data_ready_signal);

  host_com_mode_e op_mode = (mode == HOST_COM_MODE_ENCODE) ? HOST_CONNECTION_MODE_PROTOBUF : HOST_CONNECTION_MODE_LOG;

  host_connection_init(send_data_func, op_mode);

  uwb_dev_parser_init();

  xTaskCreate(host_com_main_task, "host com", stack_size, NULL, priority, &host_com_task);
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
  Toan Huynh, 04/13/2022
*********************************************************************************/
void host_com_data_ready_signal(void)
{
  if(host_com_sem)
  {
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(host_com_sem, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
  }
}

/********************************************************************************
Function:
  host_com_register_interval_callback()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/01/2022
*********************************************************************************/
void host_com_register_interval_callback(uint32_t interval, host_com_interval_callback callback)
{
  send_data_interval_ms = interval;
  interval_callback = callback;
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  is_package_end()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
static bool is_package_end(uint8_t *p_data, uint32_t len)
{
  if (len < sizeof(PACKET_DELIMITER))
  {
    return false;
  }

  return (memcmp(p_data + len - sizeof(PACKET_DELIMITER), PACKET_DELIMITER, sizeof(PACKET_DELIMITER)) == 0);
}


/********************************************************************************
Function:
  host_com_rx_process()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
static bool host_com_rx_process(uint8_t *data, uint32_t len)
{
  hc_package_t package = packet_parse_data(data, len);

  // Check whether the data received is correct
  if (!package.p_data)
  {
    return false;
  }

  uint32_t inst_addr = (uint32_t)instance_get_addr();
  uint32_t des_addr = package.p_header->desAddr;
  
  // TODO: Improve --> Any device connected to host has vitual address 0x0
  if (des_addr == 0x0 || des_addr == inst_addr) 
  {
    host_connection_parse(data, len, backpack_process_msg);
  }
  else
  {
    // Not for me
    host_msg_process_host_msg(package.p_header->desAddr, data, package.total_len);
  }

  return true;
}

/********************************************************************************
Function:
  process_incoming_data()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Process incoming data from host
Notes:
  ---
Author, Date:
  Toan Huynh, 05/01/2022
*********************************************************************************/
static void process_incoming_data(void)
{
  uint32_t read_bytes = 0;
  uint8_t buffer[SERIAL_PROCESS_BUFF_SIZE];
  uint32_t buf_idx = 0;

  /* Wait for complete data */
  vTaskDelay(20);
  buf_idx = 0;
  memset(buffer, 0, sizeof(buffer));

  do
  {
    read_bytes = 0;
    port_serial_read(buffer + buf_idx, 1, &read_bytes);

    if (buffer[buf_idx] == PACKET_END && is_package_end(buffer, buf_idx + 1))
    {
      host_com_rx_process(buffer, buf_idx + 1 - sizeof(PACKET_DELIMITER));

      /* Process next packets if there are more packets in fifo */
      buf_idx = 0;
      memset(buffer, 0, sizeof(buffer));
    }
    else
    {
      buf_idx++;
      buf_idx %= SERIAL_PROCESS_BUFF_SIZE;
    }

  } while (read_bytes);
}
/********************************************************************************
Function:
  host_com_main_task()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Process incoming data from host and send data to host in intervals
Notes:
  ---
Author, Date:
  Toan Huynh, 04/13/2022
*********************************************************************************/
static void host_com_main_task(void *pvParameters)
{
  host_com_sem = xSemaphoreCreateBinary();

  while (1)
  {
    /* Wait for data ready event */
    if(xSemaphoreTake(host_com_sem, send_data_interval_ms / portTICK_PERIOD_MS))
    {
      process_incoming_data();
    }
    else 
    {
      // Send data to the host intervally
      if (interval_callback)
      {
        interval_callback();
      }
    }
  }
}

/********************************************************************************
Function:
  send_data_func()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Send data to host via hardware apis
Notes:
  ---
Author, Date:
  Toan Huynh, 04/13/2022
*********************************************************************************/
static bool send_data_func(uint8_t *p_data, uint32_t len)
{
  return port_serial_send(p_data, len, NULL);
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF host_com.c
//###########################################################################################################

