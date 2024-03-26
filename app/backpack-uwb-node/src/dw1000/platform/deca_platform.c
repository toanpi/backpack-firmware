#include <assert.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/__assert.h>

#include <deca_device_api.h>
#include <deca_spi.h>
#include <port.h>
#include <sleep.h>

static const struct device *const spi_device = DEVICE_DT_GET(DT_ALIAS(dw1000_bus));
static const struct gpio_dt_spec reset_pin = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), signal_gpios);
static const struct spi_cs_control spi_cs_control = {
    .gpio = GPIO_DT_SPEC_GET_BY_IDX(DT_ALIAS(dw1000_bus), cs_gpios, 0),
    .delay = 0
};
static const struct spi_config spi_slowrate_config = {
    .cs = spi_cs_control,
    .frequency = DT_PROP(DT_PATH(zephyr_user), dw1000_slowrate),
    .operation = SPI_WORD_SET(8)
};
static const struct spi_config spi_fastrate_config = {
    .cs = spi_cs_control,
    .frequency = DT_PROP(DT_PATH(zephyr_user), dw1000_fastrate),
    .operation = SPI_WORD_SET(8)
};


// static_assert(
//     DT_PROP(DT_PATH(zephyr_user), dw1000_slowrate) < DT_PROP(DT_PATH(zephyr_user), dw1000_fastrate),
//     "Slowrate freq must be lower than fastrate"
// );

// static_assert(
//     DT_PROP(DT_PATH(zephyr_user), dw1000_slowrate) < 3 * 1000 * 1000,
//     "Maximum supported slowrate frequency is 3 Mhz"
// );

// static_assert(
//     DT_PROP(DT_PATH(zephyr_user), dw1000_fastrate) < 20 * 1000 * 1000,
//     "Maximum supported fastrate frequency is 20 Mhz"
// );

static const struct spi_config *spi_selected_config = &spi_slowrate_config;

void deca_sleep(unsigned int time_ms) {
	k_sleep(K_MSEC(time_ms));
}

void Sleep(uint32_t delay) {
    deca_sleep(delay);
}

decaIrqStatus_t decamutexon(void) {
	return irq_lock();
}

void decamutexoff(decaIrqStatus_t s) {
	irq_unlock(s);
}

void port_set_dw1000_slowrate(void) {
    decaIrqStatus_t irq_stat = decamutexon();

    spi_selected_config = &spi_slowrate_config;

    decamutexoff(irq_stat);
}

void port_set_dw1000_fastrate(void) {
    decaIrqStatus_t irq_stat = decamutexon();

    spi_selected_config = &spi_fastrate_config;

    decamutexoff(irq_stat);
}

void reset_DW1000(void) {
    while (!device_is_ready(reset_pin.port))
        ;

    gpio_pin_set_dt(&reset_pin, 1);
    k_sleep(K_MSEC(1));
    gpio_pin_set_dt(&reset_pin, 0);
}

int openspi(void) {
    while (!device_is_ready(spi_device))
        ;

    while (!device_is_ready(spi_cs_control.gpio.port))
        ;

    port_set_dw1000_slowrate();

    return 0;
}

int closespi(void) {
    return 0;
}

int writetospi(uint16 headerLength,
               const uint8 *headerBuffer,
               uint32 bodyLength,
               const uint8 *bodyBuffer) {
    decaIrqStatus_t irq_stat = decamutexon();

    int res = spi_write(
        spi_device,
        spi_selected_config,
        &(struct spi_buf_set) {
            .buffers = (struct spi_buf[]) {
                {
                    .buf = (uint8_t *)headerBuffer,
                    .len = headerLength
                },
                {
                    .buf = (uint8_t *)bodyBuffer,
                    .len = bodyLength
                }
            },
            .count = 2
        }
    );

    decamutexoff(irq_stat);

    return res;
}

int readfromspi(uint16 headerLength,
                const uint8 *headerBuffer,
                uint32 readLength,
                uint8 *readBuffer)
{
    decaIrqStatus_t irq_stat = decamutexon();

    int res = spi_transceive(
        spi_device,
        spi_selected_config,
        &(struct spi_buf_set) {
            .buffers = (struct spi_buf[]) {
                {
                    .buf = (uint8_t *)headerBuffer,
                    .len = headerLength
                },
                {
                    .buf = NULL,
                    .len = readLength
                }
            },
            .count = 2
        },
        &(struct spi_buf_set) {
            .buffers = (struct spi_buf[]) {
                {
                    .buf = NULL,
                    .len = headerLength
                },
                {
                    .buf = (uint8_t *)readBuffer,
                    .len = readLength
                }
            },
            .count = 2
        }
    );

    decamutexoff(irq_stat);

    return res;
}

/* @fn    portGetTickCnt
 * @brief wrapper for to read a SysTickTimer, which is incremented with
 *        CLOCKS_PER_SEC frequency.
 *        The resolution of time32_incr is usually 1/1000 sec.
 * */
unsigned long portGetTickCnt(void)
{
    return k_uptime_get_32();
}

/* @fn 	  portGetTickCntMicro
* @brief  function to read a SysTickTimer modified by its count to get a higher resolution timestamp.
* 		  The resolution is usually one microsecond.
* */
unsigned long long portGetTickCntMicro(void)
{
    return k_uptime_get();
}


void port_disable_pin_connection(void)
{
}

void port_enable_pin_connection(void)
{
}

void setup_DW1000RSTnIRQ(int enable)
{

}

static volatile uint32_t signalResetDone;

void port_wakeup_dw1000_fast(void)
{
	#define WAKEUP_TMR_MS	(10)

	uint32_t x = 0;
	uint32_t timestamp = portGetTickCnt();	//protection

	setup_DW1000RSTnIRQ(0); 		//disable RSTn IRQ
	signalResetDone = 0;			//signalResetDone connected to the process_dwRSTn_irq() callback
	setup_DW1000RSTnIRQ(1); 		//enable RSTn Rising IRQ
	// port_SPIx_clear_chip_select();  //CS low

	//need to poll to check when the DW1000 is in the IDLE, the CPLL interrupt is not reliable
	//when RSTn goes high the DW1000 is in INIT, it will enter IDLE after PLL lock (in 5 us)
	while((signalResetDone == 0) && \
		  ((portGetTickCnt() - timestamp) < WAKEUP_TMR_MS))
	{
		x++;	 //when DW1000 will switch to an IDLE state RSTn pin will high
	}
	setup_DW1000RSTnIRQ(0); 		//disable RSTn IRQ
	// port_SPIx_set_chip_select();  	//CS high

	//it takes ~35us in total for the DW1000 to lock the PLL, download AON and go to IDLE state
	// usleep(35);
    // Zephyr does not have usleep, so we use k_sleep instead
    k_sleep(K_USEC(35));
}
void port_wakeup_dw1000(void)
{
    // port_SPIx_clear_chip_select();
     k_sleep(K_MSEC(1));
    // port_SPIx_set_chip_select();
     k_sleep(K_MSEC(7));						//wait 7ms for DW1000 XTAL to stabilise
}
int peripherals_init (void)
{
	// RCC_Configuration();
	// GPIO_Configuration();

	// SysTick_Configuration();
	// NVIC_Configuration();
	
	return 0;
}


void *sys_malloc(size_t size)
{
    return k_malloc(size);
}

void sys_free(void *ptr)
{
    k_free(ptr);
}

void port_EnableEXT_IRQ(void)
{
    
}

void host_connection_lock(void)
{

}

void host_connection_unlock(void)
{

}

