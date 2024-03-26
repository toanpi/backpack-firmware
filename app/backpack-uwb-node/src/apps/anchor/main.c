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
#define PW_LOG_MODULE_NAME "uwb-node"

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "pw_log/log.h"
#include "instance.h"
#include "host_msg.h"

//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################


//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define MAX_TIMEOUT					0xffffffffUL

#define TICK_PERIOD_MS      CONFIG_SYS_CLOCK_TICKS_PER_SEC

#define MAIN_UWB_STACK_SIZE (3 * 1024)
#define MAIN_UWB_PRIORITY   5

//###########################################################################################################
//      MACROS
//###########################################################################################################
/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)



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

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

K_MUTEX_DEFINE(uwb_mutex);

//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
int dw1000_hw_init_interrupt(void);
void dw1000_hw_interrupt_enable(void);


//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################



//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################

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
int init_led(const struct gpio_dt_spec *led)
{
	int ret;

	if (!gpio_is_ready_dt(led)) {
		return -1;
	}

	ret = gpio_pin_configure_dt(led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return -1;
	}

  gpio_pin_set_dt(led, 1);

  return 1;
}

void init_leds(void)
{
  init_led(&led0);
  init_led(&led1);
  init_led(&led2);
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
static void main_task(void *pvParameters, void *pvParameters2, void *pvParameters3) 
{
	uint32_t waiting_duration = 0;
  instance_data_t *inst = instance_get_local_structure_ptr(0);
  struct TDMAHandler *tdma_handler = (struct TDMAHandler *)get_tdma_handler();

  /* Setup some LEDs for debug Green and Blue on DWM1001-DEV */
  init_leds();

  /* Setup interrupt connected to DW1000 IRQ */
	dw1000_hw_init_interrupt();
	dw1000_hw_interrupt_enable();

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

		if (waiting_duration != MAX_TIMEOUT)
		{
			waiting_duration = waiting_duration / TICK_PERIOD_MS;
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

K_THREAD_DEFINE(main_tid, MAIN_UWB_STACK_SIZE,
                main_task, NULL, NULL, NULL,
                MAIN_UWB_PRIORITY, 0, 0);

int main(void)
{
  PW_LOG_INFO("Backpack UWB Node starting..");
  printk("Backpack UWB Node starting..\n");
  
  /* Initialize the host communication */
  // host_com_init(1024, configMAX_PRIORITIES - 3, HOST_COM_MODE_ENCODE);

  // host_com_register_interval_callback(5000, send_device_state_info);

  uwb_dev_parser_init();

  /* This task is used for forwarding the received packets to the host */
  // host_com_task_init(512, configMAX_PRIORITIES - 3);

  return 0;
}



//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################

#define DW1000_NODE DT_NODELABEL(ieee802154)
static const struct gpio_dt_spec gpio_irq = GPIO_DT_SPEC_GET_OR(DW1000_NODE, int_gpios, {0});
static struct k_work dw1000_isr_work;
static struct gpio_callback gpio_cb;

//###########################################################################################################

static void dw1000_hw_isr_work_handler(struct k_work * item) {
  if (k_mutex_lock(&uwb_mutex, K_FOREVER) == 0) {
    // Process the interrupt
    dwt_isr();
    k_mutex_unlock(&uwb_mutex);
  }
}

static void dw1000_hw_isr(const struct device* dev, struct gpio_callback* cb,
						  uint32_t pins)
{
	k_work_submit(&dw1000_isr_work);
}


int dw1000_hw_init_interrupt(void)
{
	if (gpio_irq.port) {
		k_work_init(&dw1000_isr_work, dw1000_hw_isr_work_handler);

		gpio_pin_configure_dt(&gpio_irq, GPIO_INPUT);
		gpio_init_callback(&gpio_cb, dw1000_hw_isr, BIT(gpio_irq.pin));
		gpio_add_callback(gpio_irq.port, &gpio_cb);
		gpio_pin_interrupt_configure_dt(&gpio_irq, GPIO_INT_EDGE_RISING);

		PW_LOG_INFO("IRQ on %s pin %d", gpio_irq.port->name, gpio_irq.pin);
		return 0;
	} else {
		PW_LOG_ERROR("IRQ pin not configured");
		return -ENOENT;
	}

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
  Toan Huynh, 03/26/2024
*********************************************************************************/
void dw1000_hw_interrupt_enable(void)
{
	if (gpio_irq.port) {
		gpio_pin_interrupt_configure_dt(&gpio_irq, GPIO_INT_EDGE_RISING);
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
void dw_app_signal(void)
{
  k_sem_give(&uwb_sem);
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
  Toan Huynh, 03/26/2024
*********************************************************************************/
bool host_com_send_signal(uint8_t *p_data, uint32_t len)
{
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
