/************************************************************************************************************
Module:       dw1000_board

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
03/26/2024    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2024 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <port_mcu.h>
// #include "pw_log/log.h"



//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

#define DW1000_NODE DT_NODELABEL(ieee802154)



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
static const struct gpio_dt_spec gpio_irq = GPIO_DT_SPEC_GET_OR(DW1000_NODE, int_gpios, {0});
static struct k_work dw1000_isr_work;
static struct gpio_callback gpio_cb;


static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);



//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static int dw1000_hw_init_interrupt(void);
static void dw1000_hw_interrupt_enable(void);
static void dw1000_hw_interrupt_disable(void);
static void init_leds(void);

//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
int dw1000_board_hw_init(void) {
  
	// Setup leds
	init_leds();

  /* Setup interrupt connected to DW1000 IRQ */
  dw1000_hw_init_interrupt();
  dw1000_hw_interrupt_enable();

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
void device_reset_mcu(void)
{
  // Perform a system reset
  sys_reboot(SYS_REBOOT_COLD);
}


void led_off (led_t led)
{
	switch (led)
	{
	case LED_ALL:
	  gpio_pin_set_dt(&led0, 0);
	  gpio_pin_set_dt(&led1, 0);
	  gpio_pin_set_dt(&led2, 0);
		break;
	default:
		// do nothing for undefined led number
		break;
	}
}


/* @fn		port_DisableEXT_IRQ
 * @brief	wrapper to disable DW_IRQ pin IRQ
 * 			in current implementation it disables all IRQ from lines 5:9
 * */
inline void port_DisableEXT_IRQ(void)
{
	dw1000_hw_interrupt_disable();
}

/* @fn		port_EnableEXT_IRQ
 * @brief	wrapper to enable DW_IRQ pin IRQ
 * 			in current implementation it enables all IRQ from lines 5:9
 * */
inline void port_EnableEXT_IRQ(void)
{
	dw1000_hw_interrupt_enable();
}


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

  return gpio_pin_set_dt(led, 1);
}

static void init_leds(void)
{
  init_led(&led0);
  init_led(&led1);
  init_led(&led2);
}
//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################
extern void dw1000_hw_isr_work_handler(struct k_work * item);

static void dw1000_hw_isr(const struct device* dev, struct gpio_callback* cb,
						  uint32_t pins)
{
	k_work_submit(&dw1000_isr_work);
}

static int dw1000_hw_init_interrupt(void)
{
	if (gpio_irq.port) {
		k_work_init(&dw1000_isr_work, dw1000_hw_isr_work_handler);

		gpio_pin_configure_dt(&gpio_irq, GPIO_INPUT);
		gpio_init_callback(&gpio_cb, dw1000_hw_isr, BIT(gpio_irq.pin));
		gpio_add_callback(gpio_irq.port, &gpio_cb);
		gpio_pin_interrupt_configure_dt(&gpio_irq, GPIO_INT_EDGE_RISING);

		// PW_LOG_INFO("IRQ on %s pin %d", gpio_irq.port->name, gpio_irq.pin);
		return 0;
	} else {
		// PW_LOG_ERROR("IRQ pin not configured");
		__ASSERT(true, "IRQ pin not configured");
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
static void dw1000_hw_interrupt_enable(void)
{
	if (gpio_irq.port) {
		gpio_pin_interrupt_configure_dt(&gpio_irq, GPIO_INT_EDGE_RISING);
	}
}

static void dw1000_hw_interrupt_disable(void)
{
	if (gpio_irq.port) {
		gpio_pin_interrupt_configure_dt(&gpio_irq, GPIO_INT_DISABLE);
	}
}


//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF dw1000_board.c
//###########################################################################################################
