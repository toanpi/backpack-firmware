/************************************************************************************************************
Module:       clt_host_com

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
#include "collector_host_com.h"
#include "packet.h"
#include "ti_host_com.h"
#include "host_connection.h"
#include "collector_parser.h"
#include "backpack_parser.h"
#include "collector.h"






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
static bool clt_rx_process(uint32_t des_addr, uint8_t *p_data, uint32_t data_len);

//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  clt_central_host_com_init()
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
void clt_host_com_init(void)
{
  host_com_init(UART_BAUD_RATE_2M, postEvt, clt_rx_process);
  host_connection_init(host_com_write, HOST_CONNECTION_MODE_PROTOBUF);
  collector_parser_init();
}

/********************************************************************************
Function:
  clt_host_com_log()
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
void clt_host_com_list_dev(void)
{
  uint8_t dummy = 0;
  host_connection_send(&dummy, sizeof(dummy), encode_collector_list_msg);
}

void clt_host_com_loss_test_result_msg(loss_test_result_t *p_result)
{
  host_connection_send((uint8_t *)p_result, sizeof(loss_test_result_t), encode_collector_loss_result_msg);
}

void clt_host_com_loss_test_progress_msg(loss_test_progress_t *p_data)
{
  host_connection_send((uint8_t *)p_data, sizeof(loss_test_progress_t), encode_collector_loss_progress_msg);
}

/********************************************************************************
Function:
  clt_host_com_forward()
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
void clt_host_com_forward(uint8_t *p_data, uint32_t data_len, uint32_t src_addr)
{
  return ti_host_com_forward(p_data, data_len, src_addr);
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  clt_rx_process()
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
static bool clt_rx_process(uint32_t des_addr, uint8_t *p_data, uint32_t data_len)
{
  return Collector_sendNodeMsg(des_addr, p_data, data_len);
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
  Collector_postEvent(COLLECTOR_HOST_CONNECTION_EVT);
}
//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF clt_host_com.c
//###########################################################################################################
