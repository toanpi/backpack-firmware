/************************************************************************************************************
Module:       dw_serial

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----
-------------------------------------------------------------------------------------
03/27/2024    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2024 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

// ###########################################################################################################
//       #INCLUDES
// ###########################################################################################################
#include <string.h>
#include <uwb_dev_parser.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <host_connection.h>
#include <packet.h>
#include <backpack_parser.h>
#include <instance_utilities.h>
#include <host_msg.h>
#include <host_com_msg.h>


// ###########################################################################################################
//       TESTING #DEFINES
// ###########################################################################################################

// ###########################################################################################################
//       CONSTANT #DEFINES
// ###########################################################################################################
/* change this to any other UART peripheral if desired */
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)

#define MSG_SIZE      256

#define TX_EVENT      0
#define RX_EVENT      1

// ###########################################################################################################
//       MACROS
// ###########################################################################################################

// ###########################################################################################################
//       MODULE TYPES
// ###########################################################################################################
struct hc_package {
  uint8_t event;
  uint8_t data[MSG_SIZE];
  uint32_t data_len;
};

// ###########################################################################################################
//       CONSTANTS
// ###########################################################################################################

// ###########################################################################################################
//       MODULE LEVEL VARIABLES
// ###########################################################################################################

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(hc_msgq, sizeof(struct hc_package), 10, 4);

static const struct device* const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);


// ###########################################################################################################
//       PRIVATE FUNCTION PROTOTYPES
// ###########################################################################################################
static void hc_serial_cb(const struct device* dev, void* user_data);
static bool uart_write(uint8_t* p_data, uint32_t len);
static int uart_init(void);
static bool host_com_rx_process(uint8_t *data, uint32_t len);


// ###########################################################################################################
//       PUBLIC FUNCTIONS
// ###########################################################################################################
/********************************************************************************
Input:
        ---
Output:
        ---
Description:
        ---
Author, Date:
        Toan Huynh, 03/27/2024
*********************************************************************************/
void host_conn_entry(void* p1, void* p2, void* p3) {
  struct hc_package msg;

  while (1) {
    if (k_msgq_get(&hc_msgq, &msg, K_FOREVER) == 0) {
      if (msg.event == TX_EVENT) {
        host_com_forward(msg.data, msg.data_len);
      } else if (msg.event == RX_EVENT) {
        host_com_rx_process(msg.data, msg.data_len);
      }
    }
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
  Toan Huynh, 03/27/2024
*********************************************************************************/
int host_conn_thread(void) {

  uart_init();

  uwb_dev_parser_init();

  /* Initialize the host communication */
  host_connection_init(uart_write, HOST_CONNECTION_MODE_PROTOBUF);

  return 0;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  Send data to the host
Author, Date:
  Toan Huynh, 03/26/2024
*********************************************************************************/
bool host_com_send_signal(uint8_t *p_data, uint32_t len) {

  struct hc_package buf;

  buf.event = TX_EVENT;
  buf.data_len = len;

  memcpy(buf.data, p_data, len);

  /* if queue is full, message is silently dropped */
  return k_msgq_put(&hc_msgq, &buf, K_NO_WAIT) == 0;
}

// ###########################################################################################################
//       PRIVATE FUNCTIONS
// ###########################################################################################################
/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
static void hc_serial_cb(const struct device* dev, void* user_data) {
  static struct hc_package hc_rx_buf = {.event = RX_EVENT};

  if (!uart_irq_update(uart_dev)) {
    return;
  }

  if (!uart_irq_rx_ready(uart_dev)) {
    return;
  }
  
  /* receive buffer used in UART ISR callback */
  uint8_t c;

  /* read until FIFO empty */
  while (uart_fifo_read(uart_dev, &c, 1) == 1) {
    if (HC_END_PACKAGE(c, hc_rx_buf.data, hc_rx_buf.data_len)) {
      
      hc_rx_buf.data[hc_rx_buf.data_len++] = c;
      
      /* if queue is full, message is silently dropped */
      k_msgq_put(&hc_msgq, &hc_rx_buf, K_NO_WAIT);

      /* reset the buffer (it was copied to the msgq) */
      hc_rx_buf.data_len = 0;
    } else if (hc_rx_buf.data_len < (sizeof(hc_rx_buf.data) - 1)) {
      hc_rx_buf.data[hc_rx_buf.data_len++] = c;
    } else {
      /* Error: reset the buffer */
      hc_rx_buf.data_len = 0;
    }
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
static bool uart_write(uint8_t* p_data, uint32_t len) {
  for (size_t i = 0; i < len; i++) {
    uart_poll_out(uart_dev, p_data[i]);
  }

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
        Toan Huynh, 03/27/2024
*********************************************************************************/
static int uart_init(void) {
  // TODO: Use logging

  if (!device_is_ready(uart_dev)) {
    // printk("UART device not found!");
    return 0;
  }

  /* configure interrupt and callback to receive data */
  int ret = uart_irq_callback_user_data_set(uart_dev, hc_serial_cb, NULL);

  if (ret < 0) {
    if (ret == -ENOTSUP) {
      // printk("Interrupt-driven UART API support not enabled\n");
    } else if (ret == -ENOSYS) {
      // printk("UART device does not support interrupt-driven API\n");
    } else {
      // printk("Error setting UART callback: %d\n", ret);
    }
    return 0;
  }

  uart_irq_rx_enable(uart_dev);

  return 0;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 03/27/2024
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

// ###########################################################################################################
//       INTERRUPTS
// ###########################################################################################################

// ###########################################################################################################
//       TEST HARNESSES
// ###########################################################################################################

// ###########################################################################################################
//       END OF dw_serial.c
// ###########################################################################################################
