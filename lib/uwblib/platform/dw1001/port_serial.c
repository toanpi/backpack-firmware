/************************************************************************************************************
Module:       port_serial

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
#include "app_uart.h"
#include "nrf_serial.h"
#include "boards.h"
#include "port_mcu.h"


//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define SERIAL_FIFO_TX_SIZE 255
#define SERIAL_FIFO_RX_SIZE 255
#define SERIAL_BUFF_TX_SIZE 255
#define SERIAL_BUFF_RX_SIZE 255


#define NO_PARITY	false

// UART circular buffers - Tx and Rx size
#define UART_TX_BUF_SIZE 256
#define UART_RX_BUF_SIZE 256

//###########################################################################################################
//      MACROS
//###########################################################################################################



//###########################################################################################################
//      CONSTANTS
//###########################################################################################################
#define CONFIG_USE_APP_UART     (1)


//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static void nrf_serial_evt_handler(struct nrf_serial_s const *p_serial, nrf_serial_event_t event);
static void vHandleUartInternalErrors(uint32_t u32Error);
static void vUartErrorHandle(app_uart_evt_t *p_event);

//###########################################################################################################
//      MODULE LEVEL VARIABLES
//###########################################################################################################
static data_ready_callback_t data_ready_callback = NULL;

#if CONFIG_USE_APP_UART
// UART initialisation structure
const app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUM,
        TX_PIN_NUM,
        RTS_PIN_NUM,
        CTS_PIN_NUM,
        APP_UART_FLOW_CONTROL_DISABLED,
        NO_PARITY,
        NRF_UART_BAUDRATE_115200};

#else

/* Set up serial parameters */
NRF_SERIAL_DRV_UART_CONFIG_DEF(m_uarte0_drv_config,
                               RX_PIN_NUM, TX_PIN_NUM,
                               0, 0,
                               NRF_UART_HWFC_DISABLED, NRF_UART_PARITY_EXCLUDED,
                               NRF_UART_BAUDRATE_115200,
                               UART_DEFAULT_CONFIG_IRQ_PRIORITY);

NRF_SERIAL_QUEUES_DEF(serial_queues, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);
NRF_SERIAL_BUFFERS_DEF(serial_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);

NRF_SERIAL_CONFIG_DEF(serial_config, NRF_SERIAL_MODE_IRQ,
                      &serial_queues, &serial_buffs, nrf_serial_evt_handler, NULL);

NRF_SERIAL_UART_DEF(serial_uarte, 0);
#endif




//###########################################################################################################
//      MODULE TYPES
//###########################################################################################################


//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  port_serial_init()
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
void port_serial_init(data_ready_callback_t cb)
{
#if CONFIG_USE_APP_UART
  // Initialis the nrf UART driver returning state
  uint32_t err_code = NRF_SUCCESS;

  APP_UART_FIFO_INIT(&comm_params,
                     UART_RX_BUF_SIZE,
                     UART_TX_BUF_SIZE,
                     vUartErrorHandle,
                     APP_IRQ_PRIORITY_LOWEST,
                     err_code);

#else /* CONFIG_USE_APP_UART */
  ret_code_t ret;

  ret = nrf_serial_init(&serial_uarte, &m_uarte0_drv_config, &serial_config);

  APP_ERROR_CHECK(ret);
#endif

  data_ready_callback = cb;
}

/********************************************************************************
Function:
  port_serial_send()
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
bool port_serial_send(uint8_t *p_data, uint32_t len, uint32_t *p_written)
{
  ret_code_t ret;

#if CONFIG_USE_APP_UART
  for (size_t i = 0; i < len; i++)
  {
    ret = app_uart_put(p_data[i]);
    if (ret != NRF_SUCCESS)
    {
      break;
    }
  }

#else /* CONFIG_USE_APP_UART */
  ret = nrf_serial_write(&serial_uarte, p_data, len, NULL, 0);
#endif
  return ret == NRF_SUCCESS;
}

/********************************************************************************
Function:
  port_serial_read()
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
bool port_serial_read(uint8_t *p_data, uint32_t len, uint32_t *p_read)
{
  ret_code_t ret;

#if CONFIG_USE_APP_UART
  for (size_t i = 0; i < len; i++)
  {
    ret = app_uart_get(&p_data[i]);
    if (ret != NRF_SUCCESS)
    {
      break;
    }

    if (p_read)
    {
      *p_read = i + 1;
    }
  }
#else
  ret = nrf_serial_read(&serial_uarte, p_data, len, p_read, 0);
#endif

  return ret == NRF_SUCCESS;
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################



//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################

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
  Toan Huynh, 04/17/2022
*********************************************************************************/
static void vUartErrorHandle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        vHandleUartInternalErrors(p_event->evt_type);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        vHandleUartInternalErrors(p_event->evt_type);
    }
    else if(p_event->evt_type == APP_UART_DATA_READY)
    {
      data_ready_callback();
    }
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
  Toan Huynh, 04/17/2022
*********************************************************************************/
static void vHandleUartInternalErrors (uint32_t u32Error)
{
	// notify app of error - LED ?
}


/********************************************************************************
Function:
  nrf_serial_evt_handler()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 04/15/2022
*********************************************************************************/
static void nrf_serial_evt_handler(struct nrf_serial_s const *p_serial, nrf_serial_event_t event)
{
  if (event == NRF_SERIAL_EVENT_RX_DATA)
  {
    if (data_ready_callback != NULL)
    {
      data_ready_callback();
    }
  }
};

//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF port_serial.c
//###########################################################################################################
