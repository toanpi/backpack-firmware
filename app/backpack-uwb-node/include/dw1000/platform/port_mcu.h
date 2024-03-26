#include <stdint.h>
#include <string.h>
typedef uint64_t        uint64 ;
typedef int64_t         int64 ;

void Sleep(uint32_t Delay);

void port_set_dw1000_slowrate(void);
void port_set_dw1000_fastrate(void);

void reset_DW1000(void);

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define portTICK_PERIOD_MS  1


#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif


#define sys_printf printk

#define port_DisableEXT_IRQ(...) {decaIrqStatus_t irq_stat = decamutexon();}
// #define port_DisableEXT_IRQ(...)


// typedef enum
// {
//     LEDIDX_0, //LED0
//     LEDIDX_1, //LED1
//     LEDIDX_2, //LED2
//     LED_ALL,
//     LEDn
// } led_t;
