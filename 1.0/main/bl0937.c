#include "bl0937.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "driver/gptimer_types.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_eth_mac.h"
#include "esp_eth_phy.h"
#include "esp_heap_caps.h"
#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "esp_private/periph_ctrl.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/gpio_hal.h"
#include "hal/gpio_ll.h"
#include "hal/timer_hal.h"
#include "hal/timer_ll.h"
#include "rom/gpio.h"
#include "sdkconfig.h"
#include "soc/gpio_periph.h"
#include "soc/io_mux_reg.h"
#include "soc/periph_defs.h"
#include "soc/soc_caps.h"
#include "soc/spi_reg.h"
#include "soc/timer_periph.h"
#include <soc/gpio_reg.h>
#include <soc/soc.h>
#include <soc/spi_struct.h>
#include <string.h>

#define PIN_CF 36  // 6st pin
#define PIN_CF1 39 // 7st pin
#define PIN_SEL 34 // 8st pin

#define CNT_2_E 2637.3333f		  // imp/Wh
#define HZ_2_U 0.11973f			  // V
#define HZ_2_I 0.011497579178315f // A
#define HZ_2_P 1.361921538073144f // W

#define NULL_TIME 3000000

static volatile uint32_t period_e = 0, period_i = 0, period_u = 0, cnt_e = 0;
static volatile int prev_sel1 = -1;
static volatile uint64_t prev_tmr_cf = 0;
static volatile uint64_t prev_tmr_cf1_i = 0;

static void IRAM_ATTR isr_cf(void *arg)
{
	uint64_t t = esp_timer_get_time();
	period_e = t - prev_tmr_cf;
	prev_tmr_cf = t;
	cnt_e++;
}

static void IRAM_ATTR isr_cf1(void *arg)
{
	uint64_t t = esp_timer_get_time();
	static volatile uint64_t prev_tmr = 0;
	uint32_t period = t - prev_tmr;
	prev_tmr = t;
	if(prev_sel1 == gpio_get_level(GPIO_NUM_34))
	{
		if(prev_sel1)
		{
			period_u = period;
		}
		else
		{
			period_i = period;
			prev_tmr_cf1_i = t;
		}
	}
	prev_sel1 = gpio_get_level(GPIO_NUM_34);
}

float bl0937_get_cf_e(void) { return (float)cnt_e / CNT_2_E; }
float bl0937_get_cf_p(void) { return period_e == 0 ? 0 : HZ_2_P * (1000000.0f / (float)period_e); }
float bl0937_get_cf_u(void) { return period_u == 0 ? 0 : HZ_2_U * (1000000.0f / (float)period_u); }
float bl0937_get_cf_i(void) { return period_i == 0 ? 0 : HZ_2_I * (1000000.0f / (float)period_i); }

void bl0937_init(void)
{
	// ensure gpio_install_isr_service(0) was called before
	esp_rom_gpio_pad_select_gpio(PIN_CF);
	gpio_set_direction(GPIO_NUM_36, GPIO_MODE_INPUT);
	gpio_set_pull_mode(GPIO_NUM_36, GPIO_FLOATING);
	gpio_isr_handler_add(GPIO_NUM_36, isr_cf, NULL);
	gpio_set_intr_type(GPIO_NUM_36, GPIO_INTR_POSEDGE);
	gpio_intr_enable(GPIO_NUM_36);

	esp_rom_gpio_pad_select_gpio(PIN_CF1);
	gpio_set_direction(GPIO_NUM_39, GPIO_MODE_INPUT);
	gpio_set_pull_mode(GPIO_NUM_39, GPIO_FLOATING);
	gpio_isr_handler_add(GPIO_NUM_39, isr_cf1, NULL);
	gpio_set_intr_type(GPIO_NUM_39, GPIO_INTR_POSEDGE);
	gpio_intr_enable(GPIO_NUM_39);

	esp_rom_gpio_pad_select_gpio(PIN_SEL);
	gpio_set_direction(GPIO_NUM_34, GPIO_MODE_INPUT);
	gpio_set_pull_mode(GPIO_NUM_34, GPIO_FLOATING);
	// gpio_isr_handler_add(GPIO_NUM_34, isr_sel, NULL);
	// gpio_set_intr_type(GPIO_NUM_34, GPIO_INTR_ANYEDGE);
	// gpio_intr_enable(GPIO_NUM_34);
}

void bl0937_poll(uint32_t diff_ms)
{
	uint64_t t = esp_timer_get_time();
	if(t - prev_tmr_cf > NULL_TIME) period_e = 0;
	if(t - prev_tmr_cf1_i > NULL_TIME && gpio_get_level(GPIO_NUM_34) == 0) period_i = 0;
}

void bl0937_rst_vals(void)
{
	cnt_e = 0;
}

void con_cb_bl0937_damp(print_func_t pf, const char *req, int len, int *ret)
{
	pf("BL0937 cnt: %ld %ld %ld %ld", cnt_e, period_e, period_u, period_i);
}

void con_cb_bl0937_rst_cnt(print_func_t pf, const char *req, int len, int *ret)
{
	bl0937_rst_vals();
	pf("BL0937 cnt reset ok");
}