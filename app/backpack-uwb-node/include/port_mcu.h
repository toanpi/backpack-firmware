/************************************************************************************************************
Module:       port_mcu

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

#ifndef _PORT_MCU_H
#define _PORT_MCU_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <zephyr/kernel.h>


//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################

#if CONFIG_UWBLIB_DEBUG
#if 1 // Send to host console

extern void host_com_log(const char *format, ...);

#define sys_printf  host_com_log
#define db_printf   host_com_log
#else // kernel printf

#include <zephyr/sys/printk.h>

#define sys_printf  printk
#define db_printf   printk

#endif
#else
#define sys_printf(...)
#define db_printf(...)
#endif

//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
typedef enum
{
    LEDIDX_0, //LED0
    LEDIDX_1, //LED1
    LEDIDX_2, //LED2
    LED_ALL,
    LEDn
} led_t;



//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################

void Sleep(uint32_t Delay);
void deca_sleep(unsigned int time_ms);

void port_set_dw1000_slowrate(void);
void port_set_dw1000_fastrate(void);

void port_wakeup_dw1000(void);
void port_wakeup_dw1000_fast(void);

void reset_DW1000(void);

void port_EnableEXT_IRQ(void);
void port_DisableEXT_IRQ(void);

void port_disable_pin_connection(void);
void port_enable_pin_connection(void);

void device_reset_mcu(void);

void* sys_malloc(size_t size);
void sys_free(void* ptr);

unsigned long portGetTickCnt(void);
unsigned long long portGetTickCntMicro(void);

int dw1000_board_hw_init(void);

void usleep(uint32_t delay_us);

int peripherals_init(void);

void led_off(led_t led);

bool host_connection_lock(void);
bool host_connection_unlock(void);

//###########################################################################################################
// END OF port_mcu.h
//###########################################################################################################
#endif


