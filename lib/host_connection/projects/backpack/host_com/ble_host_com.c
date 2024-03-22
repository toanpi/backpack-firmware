/************************************************************************************************************
Module:       ble_host_com

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
05/03/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "packet.h"
#include "ti_host_com.h"
#include "host_connection.h"
#include "ble_central_parser.h"
#include "ble_host_com.h"
#include "backpack_parser.h"






//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define UART_BAUD_RATE_2M    (2000000)


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
static void postEvt(void);
static bool ble_central_rx_forward(uint32_t des_addr, uint8_t *p_data, uint32_t data_len);


//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  ble_central_host_com_init()
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
void ble_central_host_com_init(void)
{
  host_com_init(UART_BAUD_RATE_2M, postEvt, ble_central_rx_forward);
  host_connection_init(host_com_write, HOST_CONNECTION_MODE_PROTOBUF);
  ble_central_parser_init();
}

/********************************************************************************
Function:
  ble_host_com_log()
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
void ble_host_com_log(uint8_t *log_string, uint32_t log_len)
{
  host_connection_send((uint8_t *)log_string, log_len, encode_ble_log_msg);
}

void ble_host_com_send_scan_devs(scanRec_t scanList[], uint32_t num_dev)
{
  host_connection_send((uint8_t *)scanList, num_dev * sizeof(scanRec_t), encode_ble_scan_dev_msg);
}

// void ble_host_com_send_connect_devs(uint8_t *addr)
// {
//   host_connection_send(addr, B_ADDR_LEN, encode_ble_connect_dev_msg);
// }

void ble_host_com_send_connect_devs(scanRec_t scanList[], uint32_t num_dev)
{
  host_connection_send((uint8_t *)scanList, num_dev * sizeof(scanRec_t), encode_ble_connect_dev_msg);
}

/********************************************************************************
Function:
  ble_host_com_forward()
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
void ble_host_com_forward(uint8_t *p_data, uint32_t data_len, uint32_t src_addr)
{
  return ti_host_com_forward(p_data, data_len, src_addr);
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  ble_central_rx_process()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Handle request from the host
Notes:
  ---
Author, Date:
  Toan Huynh, 08/05/2021
*********************************************************************************/
static bool ble_central_rx_forward(uint32_t des_addr, uint8_t *p_data, uint32_t data_len)
{
  return Ble_sendPacket(p_data, data_len);
}

/********************************************************************************
Function:
  postEvt()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Wake up the application thread to process the host connection event.
Notes:
  ---
Author, Date:
  Toan Huynh, 05/27/2021
*********************************************************************************/
static void postEvt(void)
{
  /* Wake up the application thread when it waits for clock event */
  SimpleCentral_enqueueMsg(SC_EVT_UART_RECEIVED, 0, NULL);
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF ble_host_com.c
//###########################################################################################################
