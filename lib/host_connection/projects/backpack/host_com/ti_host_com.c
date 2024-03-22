/************************************************************************************************************
Module:       ti_host_com

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
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>
#include <ti/sysbios/knl/Semaphore.h>
#include "ti_drivers_config.h"
#include "ti_host_com.h"
#include <ti/drivers/UART2.h>
#include "packet.h"
#include "host_connection.h"
#include "backpack_parser.h"



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
static UART2_Handle gUartHandle = NULL;
static unsigned char gUartRxBuffer[UART_RECEIVE_LEN];
static unsigned char gUartTxBuffer[UART_RECEIVE_LEN];
static unsigned char uart_RxBuffer_ToUse[UART_RECEIVE_LEN];
static post_event_f hc_post_event_cb;
static rx_process_f hc_rx_process_cb;


//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static void UartReadCallback(UART2_Handle handle, void *uart_RxBuffer, size_t count, void *userArg, int_fast16_t status);


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
  Init host communication
Notes:
  ---
Author, Date:
  Toan Huynh, 05/27/2021
*********************************************************************************/
void host_com_init(uint32_t baudRate, post_event_f post_event_cb, rx_process_f rx_process_cb)
{
  UART2_Params gUartParams;

  UART2_Params_init(&gUartParams);

  gUartParams.baudRate = baudRate;
  gUartParams.writeMode = UART2_Mode_NONBLOCKING;

  gUartParams.readMode = UART2_Mode_CALLBACK;
  gUartParams.readReturnMode = UART2_ReadReturnMode_FULL;
  gUartParams.readCallback = UartReadCallback;

  gUartHandle = UART2_open(CONFIG_UART2_0, &gUartParams);

  // Enable receiver, inhibit low power mode
  UART2_rxEnable(gUartHandle);

  if (gUartHandle)
  {
    UART2_read(gUartHandle, gUartRxBuffer, sizeof(gUartRxBuffer), NULL);
  }

  hc_post_event_cb = post_event_cb;
  hc_rx_process_cb = rx_process_cb;

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
bool host_com_rx_process(void)
{
  bool status = false;
  hc_package_t package = packet_parse_data(uart_RxBuffer_ToUse, UART_RECEIVE_LEN);

  // Check whether the data received is correct
  if (package.p_data)
  {
    if (package.p_header->desAddr != 0x0) // Not for me
    {
      if (hc_rx_process_cb)
      {
        hc_rx_process_cb(package.p_header->desAddr, uart_RxBuffer_ToUse, package.total_len);
      }
    }
    else
    {
      host_connection_parse(uart_RxBuffer_ToUse, UART_RECEIVE_LEN, backpack_process_msg);
    }

    status = true;
  }

  if (!status)
  {
    // Cancel the reading process because of the asynchronization
    UART2_readCancel(gUartHandle);
    // Restart reading
    UART2_read(gUartHandle, gUartRxBuffer, UART_RECEIVE_LEN, NULL);
  }

  memset(uart_RxBuffer_ToUse, 0, sizeof(uart_RxBuffer_ToUse));

  return status;
}

/********************************************************************************
Function:
  host_com_write()
Input Parameters:
  p_data        Pointer to data
  data_len      Data length
Output Parameters:
  True if transmission is completed otherwise false
Description:
  Send data from collector to host connection
Notes:
  ---
Author, Date:
  Toan Huynh, 06/16/2021
*********************************************************************************/
bool host_com_write(uint8_t *p_data, uint32_t data_len)
{
  if (p_data == NULL || data_len == 0)
  {
    return false;
  }

  size_t bytesWritten = 0;
  int_fast16_t status = UART2_write(gUartHandle, (const void *)p_data, data_len, &bytesWritten);

  // Check if all data is written successfully
  return status == UART2_STATUS_SUCCESS && bytesWritten == data_len; 
}

/********************************************************************************
Function:
  ti_host_com_forward()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Truncate the data to fit length of UART_RECEIVE_LEN then send it to host
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
void ti_host_com_forward(uint8_t *p_data, uint32_t data_len, uint32_t src_addr)
{
  hc_package_t package = packet_parse_data(p_data, data_len);

  /* Check whether the data received is correct */
  if (package.p_data)
  {
    // Correct data length
    uint32_t actual_len = packet_get_header_len();
    actual_len += package.data_len;
    actual_len += packet_get_footer_len();

    // Update source address
    package.p_header->srcAddr = src_addr;

    host_com_write(p_data, actual_len);
  }
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  UartReadCallback()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Uart read callback function
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
static void UartReadCallback(UART2_Handle handle, void *uart_RxBuffer, size_t count, void *userArg, int_fast16_t status)
{
  // Make sure we received all expected bytes
  if (count == UART_RECEIVE_LEN)
  {
    if (uart_RxBuffer_ToUse[0] == 0)
    {
      memcpy(uart_RxBuffer_ToUse, uart_RxBuffer, UART_RECEIVE_LEN);
    }

    memset(uart_RxBuffer, 0, UART_RECEIVE_LEN);

    // Start another read, with size the same as it was during first call (or its previous call to be more precise) to UART_read()
    UART2_read(handle, uart_RxBuffer, UART_RECEIVE_LEN, NULL);

    /* Wake up the application thread when it waits for clock event */
    if (hc_post_event_cb)
    {
      hc_post_event_cb();
    }
  }
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF ti_host_com.c
//###########################################################################################################
