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

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
// #include "sdk_config.h"
// #include "FreeRTOS.h"
// #include "task.h"
// #include "bsp.h"
// #include "timers.h"
// #include "boards.h"
// #include "nordic_common.h"
// #include "nrf_drv_clock.h"
// #include "nrf_drv_spi.h"
// #include "nrf_uart.h"
// #include "app_util_platform.h"
// #include "nrf_gpio.h"
// #include "nrf_delay.h"
// #include "nrf_log.h"
// #include "nrf.h"
// #include "app_error.h"
// #include "app_util_platform.h"
// #include "app_error.h"
// #include <string.h>
// #include "port_platform.h"
// #include "deca_types.h"
// #include "deca_param_types.h"
// #include "deca_regs.h"
// #include "deca_device_api.h"
// #include "uart.h"
// #include "nrf_drv_gpiote.h"
// #include "nrf_drv_systick.h"
// #include "host_com.h"
// #include "host_com_task.h"
// #include "dw_main.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>


//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################


//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define PW_LOG_MODULE_NAME "uwb-node"


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
// static TaskHandle_t xMainTask;
// static TaskHandle_t xUwbCbTask;


//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
// extern int dw_main(void);
// extern void uwb_isr_callback(void);
void vInterruptInit(void);


//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################


/********************************************************************************
Function:
  checkIrq()
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
int checkIrq(void)
{
  // return nrf_gpio_pin_read(DW1000_IRQ);
}

/********************************************************************************
Function:
  enbaleDw1000Isr()
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
void enbaleDw1000Isr(void)
{
  // nrf_drv_gpiote_in_event_enable(DW1000_IRQ, true);
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
static void main_task(void *pvParameters, void *pvParameters2, void *pvParameters3) 
// static void main_task(void *pvParameters) 
{
  /* Setup some LEDs for debug Green and Blue on DWM1001-DEV */
  // LEDS_CONFIGURE(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK);
  // LEDS_ON(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK );

  /* Set SPI clock to 2MHz */
  // port_set_dw1000_slowrate();

  /* Setup NRF52832 interrupt on GPIO 25 : connected to DW1000 IRQ */
  vInterruptInit();

  /*Initialization UART*/
  // boUART_Init();

  /* Initialize dw main */
  // dw_main();
}

extern void dw_cb_main(void);

static void uwb_cb_main(void *pvParameters, void *pvParameters2, void *pvParameters3) 
// static void uwb_cb_main(void *pvParameters) 
{
  // dw_cb_main();
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

#define MY_STACK_SIZE 500
#define MY_PRIORITY 5

K_THREAD_STACK_DEFINE(my_stack_area, MY_STACK_SIZE);
struct k_thread my_thread_data;

int main(void) {

  k_thread_create(&my_thread_data,
                  my_stack_area,
                  K_THREAD_STACK_SIZEOF(my_stack_area),
                  main_task,
                  NULL,
                  NULL,
                  NULL,
                  MY_PRIORITY,
                  0,
                  K_NO_WAIT);

  k_thread_create(&my_thread_data,
                  my_stack_area,
                  K_THREAD_STACK_SIZEOF(my_stack_area),
                  uwb_cb_main,
                  NULL,
                  NULL,
                  NULL,
                  MY_PRIORITY,
                  0,
                  K_NO_WAIT);

  // Setup main task
  // xTaskCreate(main_task, "main", 3 * 1024 + 512, NULL, configMAX_PRIORITIES - 2, &xMainTask);

	// xTaskCreate(uwb_cb_main, "uwb callback", 3 * 1024, NULL, configMAX_PRIORITIES - 1, &xUwbCbTask);

  /* Initialize the host communication */
  // host_com_init(1024, configMAX_PRIORITIES - 3, HOST_COM_MODE_ENCODE);

  // host_com_register_interval_callback(UPDATE_STATUS_TIMEOUT, send_device_state_info);

  /* This task is used for forwarding the received packets to the host */
  // host_com_task_init(512, configMAX_PRIORITIES - 3);

  // Start the FreeRTOS scheduler
  // vTaskStartScheduler();

  // Should never reach there
  while (1)
    ;

  return 0;
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################
/********************************************************************************
Function:
  vInterruptHandler()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Interrupt handler calls the DW1000 ISR API. Call back corresponding to each event
Notes:
  ---
Author, Date:
  Toan Huynh, 01/06/2022
*********************************************************************************/
// void vInterruptHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
// {
//   /* Call DW1000 ISR */
//   // process_deca_irq();
//   uwb_isr_callback();
// }

/********************************************************************************
Function:
  vInterruptInit()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Configure an IO pin as a positive edge triggered interrupt source.
Notes:
  ---
Author, Date:
  Toan Huynh, 01/06/2022
*********************************************************************************/
void vInterruptInit(void)
{
  // ret_code_t err_code;

  // if (!nrf_drv_gpiote_is_init())
  // {
  //   nrf_drv_gpiote_init();
  // }

  // // input pin, +ve edge interrupt, no pull-up
  // nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
  // in_config.pull = NRF_GPIO_PIN_NOPULL;

  // // Link this pin interrupt source to its interrupt handler
  // err_code = nrf_drv_gpiote_in_init(DW1000_IRQ, &in_config, vInterruptHandler);
  // APP_ERROR_CHECK(err_code);

}

void device_reset_mcu(void)
{
    // Perform a system reset
    sys_reboot(SYS_REBOOT_COLD);
}
//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################




//###########################################################################################################
//      END OF main.c
//###########################################################################################################

