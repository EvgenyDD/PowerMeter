#include "ade7753.h"
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

/*
Calibration:
0. wait stable chip temp
1. disconn inputs physically
2. cal ch1os & ch2os that WF reg is ~0
3. connect inputs
4. calibrate UrmsOS by datasheet at 2 voltages
5. calibrate IrmsOS that ADE7753_VAENERGY change rate ~0
6. calibrate APOS that ADE7753_AENERGY change rate ~0 (AENERGY don't depend on IRMS, VAENERGY does)
7. ... calibrate gains for all values with reference meter
*/

#define CH1OS_VAL 0
#define CH2OS_VAL 4
#define IRMS_OS_VAL -170
#define VRMS_OS_VAL -519
#define APOS_OS_VAL 3350

extern bool flashing;

#define PIN_ZC 26
#define PIN_CF 25

#define PIN_SPI_CLK 14
#define PIN_SPI_MISO 12
#define PIN_SPI_MOSI 13
#define PIN_SPI_CS 27

#define read_S24(reg) ((int32_t)(reg_read_u24(reg) << 8) >> 8)

#define UPD_INTERVAL_MS 100

ade7753_inst_t ade7753_inst = {0};

static volatile uint8_t cnt_zc = 0, cnt_zc_pos = 0, cnt_cf = 0;
static volatile uint32_t period_cf = 0, last_ts_zc = 0;
static volatile bool spi_act = false;

#define WF_REC_U 1
#define WF_REC_I 2

static int wf_rec_type = 0;
static int first_run = 10;

uint32_t ade7753_get_cf(void) { return period_cf; }
int ade7753_is_wf_rec(void) { return wf_rec_type; }

uint8_t ade7753_get_zc_cnt(void) { return cnt_zc; }
uint8_t ade7753_get_zc_cnt_pos(void) { return cnt_zc_pos; }

static uint16_t s12_format(int value)
{
	if(value < -2048) value = -2048;
	if(value > 2047) value = 2047;
	return value < 0
			   ? (uint16_t)(value + 4096) & 0x0FFF
			   : (uint16_t)value & 0x0FFF;
}

static void reg_write_u8(uint8_t reg, uint8_t v)
{
	spi_act = true;
	gpio_set_level(PIN_SPI_CS, 0);
	while(SPI2.cmd.usr)
		;
	SPI2.data_buf[0] = (1 << 7) | reg;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	SPI2.data_buf[0] = v;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	gpio_set_level(PIN_SPI_CS, 1);
	spi_act = false;
}

static void reg_write_u16(uint8_t reg, uint16_t v)
{
	spi_act = true;
	gpio_set_level(PIN_SPI_CS, 0);
	while(SPI2.cmd.usr)
		;
	SPI2.data_buf[0] = (1 << 7) | reg;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	SPI2.data_buf[0] = (v >> 8) & 0xFF;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	SPI2.data_buf[0] = v & 0xFF;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	gpio_set_level(PIN_SPI_CS, 1);
	spi_act = false;
}

static uint8_t reg_read_u8(uint8_t reg)
{
	spi_act = true;
	gpio_set_level(PIN_SPI_CS, 0);
	while(SPI2.cmd.usr)
		;
	SPI2.data_buf[0] = reg;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	SPI2.data_buf[0] = 0;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	uint8_t rx_data = (uint8_t)(SPI2.data_buf[0] & 0xFF);
	gpio_set_level(PIN_SPI_CS, 1);
	spi_act = false;
	return rx_data;
}

static uint16_t reg_read_u16(uint8_t reg)
{
	spi_act = true;
	uint8_t rx[2];
	gpio_set_level(PIN_SPI_CS, 0);
	while(SPI2.cmd.usr)
		;
	SPI2.data_buf[0] = reg;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	SPI2.data_buf[0] = 0;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	rx[0] = (uint8_t)(SPI2.data_buf[0] & 0xFF);
	SPI2.data_buf[0] = 0;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	rx[1] = (uint8_t)(SPI2.data_buf[0] & 0xFF);
	gpio_set_level(PIN_SPI_CS, 1);
	spi_act = false;
	return ((uint16_t)rx[0] << 8) | (uint16_t)rx[1];
}

static uint32_t reg_read_u24(uint8_t reg)
{
	spi_act = true;
	uint8_t rx[3];
	gpio_set_level(PIN_SPI_CS, 0);
	while(SPI2.cmd.usr)
		;
	SPI2.data_buf[0] = reg;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	SPI2.data_buf[0] = 0;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	rx[0] = (uint8_t)(SPI2.data_buf[0] & 0xFF);
	SPI2.data_buf[0] = 0;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	rx[1] = (uint8_t)(SPI2.data_buf[0] & 0xFF);
	SPI2.data_buf[0] = 0;
	SPI2.cmd.usr = 1;
	while(SPI2.cmd.usr)
		;
	rx[2] = (uint8_t)(SPI2.data_buf[0] & 0xFF);
	gpio_set_level(PIN_SPI_CS, 1);
	spi_act = false;
	return ((uint32_t)rx[0] << 16) | ((uint32_t)rx[1] << 8) | (uint32_t)rx[2];
}

static void IRAM_ATTR isr_zc(void *arg)
{
	uint64_t t = esp_timer_get_time();
	static volatile uint64_t prev_tmr = 0;
	if(t - prev_tmr > 5000)
	{
		prev_tmr = t;
		cnt_zc++;
		if(gpio_get_level(GPIO_NUM_26) == 0) cnt_zc_pos++;
	}
	if(spi_act == false)
	{
		// update rms only in ZC
		ade7753_inst.raw.urms = reg_read_u24(ADE7753_VRMS);
		ade7753_inst.raw.irms = reg_read_u24(ADE7753_IRMS);
	}
}

static void IRAM_ATTR isr_cf(void *arg)
{
	uint64_t t = esp_timer_get_time();
	cnt_cf++;
	static volatile uint64_t prev_tmr = 0;
	period_cf = t - prev_tmr;
	prev_tmr = t;
}

static int IRAM_ATTR spi_freq_for_pre_n(int fapb, int pre, int n) { return (fapb / (pre * n)); }
static int IRAM_ATTR spi_set_clock(spi_dev_t *hw, int fapb, int hz, int duty_cycle)
{
	int pre, n, h, l, eff_clk;
	if(hz > ((fapb / 4) * 3))
	{
		hw->clock.clkcnt_l = 0;
		hw->clock.clkcnt_h = 0;
		hw->clock.clkcnt_n = 0;
		hw->clock.clkdiv_pre = 0;
		hw->clock.clk_equ_sysclk = 1;
		eff_clk = fapb;
	}
	else
	{
		int bestn = -1;
		int bestpre = -1;
		int besterr = 0;
		int errval;
		for(n = 1; n <= 64; n++)
		{
			pre = ((fapb / n) + (hz / 2)) / hz;
			if(pre <= 0) pre = 1;
			if(pre > 8192) pre = 8192;
			errval = abs(spi_freq_for_pre_n(fapb, pre, n) - hz);
			if(bestn == -1 || errval <= besterr)
			{
				besterr = errval;
				bestn = n;
				bestpre = pre;
			}
		}

		n = bestn;
		pre = bestpre;
		l = n;
		h = (duty_cycle * n + 127) / 256;
		if(h <= 0) h = 1;

		hw->clock.clk_equ_sysclk = 0;
		hw->clock.clkcnt_n = n - 1;
		hw->clock.clkdiv_pre = pre - 1;
		hw->clock.clkcnt_h = h - 1;
		hw->clock.clkcnt_l = l - 1;
		eff_clk = spi_freq_for_pre_n(fapb, pre, n);
	}
	return eff_clk;
}

void ade7753_init(void)
{
	gpio_set_direction(PIN_SPI_CS, GPIO_MODE_OUTPUT);
	gpio_set_level(PIN_SPI_CS, 1);

	PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[PIN_SPI_MOSI], PIN_FUNC_GPIO);
	gpio_ll_output_enable(&GPIO, PIN_SPI_MOSI);
	gpio_matrix_out(PIN_SPI_MOSI, HSPID_OUT_IDX, false, false);
	gpio_matrix_in(PIN_SPI_MOSI, HSPID_IN_IDX, false);

	PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[PIN_SPI_MISO], PIN_FUNC_GPIO);
	gpio_ll_input_enable(&GPIO, PIN_SPI_MISO);
	gpio_matrix_in(PIN_SPI_MISO, HSPIQ_IN_IDX, false);

	PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[PIN_SPI_CLK], PIN_FUNC_GPIO);
	gpio_ll_output_enable(&GPIO, PIN_SPI_CLK);
	gpio_matrix_out(PIN_SPI_CLK, HSPICLK_OUT_IDX, false, false);

	periph_module_enable(PERIPH_HSPI_MODULE);
	esp_intr_alloc(ETS_SPI2_INTR_SOURCE, ESP_INTR_FLAG_INTRDISABLED, NULL, NULL, NULL);
	/*int effclk = */ spi_set_clock(&SPI2, APB_CLK_FREQ, 1000000, 128);
	SPI2.dma_conf.val |= SPI_OUT_RST | SPI_AHBM_RST | SPI_AHBM_FIFO_RST;
	SPI2.dma_out_link.start = 0;
	SPI2.dma_in_link.start = 0;
	SPI2.dma_conf.val &= ~(SPI_OUT_RST | SPI_AHBM_RST | SPI_AHBM_FIFO_RST);
	SPI2.slave.rd_buf_done = 0;
	SPI2.slave.wr_buf_done = 0;
	SPI2.slave.rd_sta_done = 0;
	SPI2.slave.wr_sta_done = 0;
	SPI2.slave.rd_buf_inten = 0;
	SPI2.slave.wr_buf_inten = 0;
	SPI2.slave.rd_sta_inten = 0;
	SPI2.slave.wr_sta_inten = 0;
	SPI2.slave.trans_inten = 0;
	SPI2.slave.trans_done = 0;
	SPI2.pin.master_ck_sel &= (1 << 0);
	SPI2.pin.master_cs_pol &= (1 << 0);
	SPI2.pin.ck_idle_edge = 0;
	SPI2.pin.cs0_dis = 0;
	SPI2.pin.cs1_dis = 0;
	SPI2.pin.cs2_dis = 0;
	SPI2.user.ck_out_edge = 1;
	SPI2.user.usr_dummy = 0;
	SPI2.user.usr_addr = 0;
	SPI2.user.usr_command = 0;
	SPI2.user.doutdin = 1;
	SPI2.user.sio = 0;
	SPI2.user.cs_setup = 0;
	SPI2.user.cs_hold = 0;
	SPI2.user.usr_mosi_highpart = 0;
	SPI2.user.usr_mosi = 1;
	SPI2.user.usr_miso = 1;
	SPI2.user1.usr_addr_bitlen = 0 - 1;
	SPI2.user1.usr_dummy_cyclelen = 0 - 1;
	SPI2.user2.usr_command_bitlen = 0 - 1;
	SPI2.ctrl.rd_bit_order = 0;
	SPI2.ctrl.wr_bit_order = 0;
	SPI2.ctrl2.val = 0;
	SPI2.ctrl2.miso_delay_mode = 0;
	SPI2.ctrl2.setup_time = 0 - 1;
	SPI2.ctrl2.hold_time = 0 - 1;
	SPI2.mosi_dlen.usr_mosi_dbitlen = 8 - 1;
	SPI2.miso_dlen.usr_miso_dbitlen = 8 - 1;

	esp_rom_gpio_pad_select_gpio(PIN_ZC);
	gpio_set_direction(PIN_ZC, GPIO_MODE_INPUT);

	ade7753_read_sts(false);

	ade7753_inst.mode = 1 << MODE_SWRST;
	reg_write_u16(ADE7753_MODE, ade7753_inst.mode);
	esp_rom_delay_us(50);

	uint8_t sts;
	while(!((sts = ade7753_read_sts(false)) & (1 << STS_RESET)))
	{
		esp_rom_delay_us(10);
	}

	uint8_t v = 0x8;
	reg_write_u16(ADE7753_IRQEN, v);

	reg_write_u8(ADE7753_CH1OS, CH1OS_VAL);
	reg_write_u8(ADE7753_CH2OS, CH2OS_VAL);
	reg_write_u16(ADE7753_IRMSOS, s12_format(IRMS_OS_VAL));
	reg_write_u16(ADE7753_VRMSOS, s12_format(VRMS_OS_VAL));
	reg_write_u16(ADE7753_APOS, APOS_OS_VAL);
	reg_write_u8(ADE7753_GAIN, (1 << 3) | 3); // CH1 FULL SCALE 0.25V, Gain 8

	ade7753_inst.mode = (1 << MODE_DTRT0);
	reg_write_u16(ADE7753_MODE, ade7753_inst.mode);

	esp_rom_gpio_pad_select_gpio(PIN_ZC);
	gpio_set_direction(GPIO_NUM_26, GPIO_MODE_INPUT);
	gpio_set_pull_mode(GPIO_NUM_26, GPIO_FLOATING);
	gpio_isr_handler_add(GPIO_NUM_26, isr_zc, NULL);
	gpio_set_intr_type(GPIO_NUM_26, GPIO_INTR_ANYEDGE);
	gpio_intr_enable(GPIO_NUM_26);

	esp_rom_gpio_pad_select_gpio(PIN_CF);
	gpio_set_direction(GPIO_NUM_25, GPIO_MODE_INPUT);
	gpio_set_pull_mode(GPIO_NUM_25, GPIO_FLOATING);
	gpio_isr_handler_add(GPIO_NUM_25, isr_cf, NULL);
	gpio_set_intr_type(GPIO_NUM_25, GPIO_INTR_ANYEDGE);
	gpio_intr_enable(GPIO_NUM_25);
}

void ade7753_set_wf_e(void)
{
	ade7753_inst.mode &= ~((1 << MODE_WAVSEL1) | (1 << MODE_WAVSEL0));
	reg_write_u16(ADE7753_MODE, ade7753_inst.mode);
}

void ade7753_set_wf_i(void)
{
	ade7753_inst.mode &= ~((1 << MODE_WAVSEL1) | (1 << MODE_WAVSEL0));
	ade7753_inst.mode |= (1 << MODE_WAVSEL1);
	reg_write_u16(ADE7753_MODE, ade7753_inst.mode);
}

void ade7753_set_wf_u(void)
{
	ade7753_inst.mode &= ~((1 << MODE_WAVSEL1) | (1 << MODE_WAVSEL0));
	ade7753_inst.mode |= (1 << MODE_WAVSEL1) | (1 << MODE_WAVSEL0);
	reg_write_u16(ADE7753_MODE, ade7753_inst.mode);
}

uint16_t ade7753_read_sts(bool clear)
{
	ade7753_inst.status = reg_read_u16(clear ? ADE7753_RSTATUS : ADE7753_STATUS);
	if(ade7753_inst.acc.en && (ade7753_inst.status & (1 << STS_CYCEND)))
	{
		ade7753_inst.acc.time = esp_timer_get_time() - ade7753_inst.acc.time;
		ade7753_inst.mode &= ~(1 << MODE_CYCMODE);
		reg_write_u16(ADE7753_MODE, ade7753_inst.mode);
		ade7753_inst.acc.en = false;
		reg_read_u16(ADE7753_RSTATUS);
		flashing = false;
	}
	return ade7753_inst.status;
}

int32_t ade7753_read_wf(void)
{
	while(!(ade7753_read_sts(true) & (1 << STS_WSMP)))
		asm("nop");
	ade7753_inst.raw.wf = read_S24(ADE7753_WAVEFORM);
	return ade7753_inst.raw.wf;
}

void ade7753_rst_vals(void)
{
	ade7753_read_sts(true);
	reg_read_u24(ADE7753_RVAENERGY);
	reg_read_u24(ADE7753_RAENERGY);
	reg_read_u24(ADE7753_RSTIPEAK);
	reg_read_u24(ADE7753_RSTVPEAK);
}

void ade7753_poll(uint32_t diff_ms)
{
	static uint32_t tmr = 0;
	tmr += diff_ms;
	if(tmr > UPD_INTERVAL_MS)
	{
		tmr = 0;
		if(first_run)
		{
			first_run--;
			ade7753_rst_vals();
		}

		ade7753_inst.raw.period = reg_read_u16(ADE7753_PERIOD);
		ade7753_inst.raw.e_app = reg_read_u24(ADE7753_VAENERGY); // apparent energy, ADE7753_RVAENERGY to reset
		ade7753_inst.raw.e_act = read_S24(ADE7753_AENERGY);		 // active energy, ADE7753_RAENERGY to reset
		ade7753_inst.raw.wf = ade7753_read_wf();

		ade7753_inst.acc.e_app = reg_read_u24(ADE7753_LVAENERGY); // L acc: apparent energy
		ade7753_inst.acc.e_act = read_S24(ADE7753_LAENERGY);	  // L acc: active energy
		ade7753_inst.acc.e_react = read_S24(ADE7753_LVARENERGY);  // L acc: reactive energy

		ade7753_inst.urms = (float)ade7753_inst.raw.urms * 0.000169072785f + 0.3849661702f;
		ade7753_inst.irms = (float)ade7753_inst.raw.irms * 0.000012498129061f;
		ade7753_inst.freq = (3579545.0f / 4.0f / 32.0f * 16.0f) / (float)ade7753_inst.raw.period;
		ade7753_inst.wf = (float)ade7753_inst.raw.wf * 0.007539173036473f;
		ade7753_inst.e_act = (float)ade7753_inst.raw.e_act * 0.000078494950491f;
		ade7753_inst.e_app = (float)ade7753_inst.raw.e_app * 0.000078494950491f;
		ade7753_read_sts(false);

		static uint32_t cnt_temp = 0;
		if(++cnt_temp >= 10)
		{
			cnt_temp = 0;
			ade7753_inst.raw.temp = (int8_t)reg_read_u8(ADE7753_TEMP);
			ade7753_inst.temp = 1.5f * (float)ade7753_inst.raw.temp - 25.0f;
			ade7753_inst.mode |= 1 << 5; // TEMP_SEL
			reg_write_u16(ADE7753_MODE, ade7753_inst.mode);
		}
	}
	else if(ade7753_inst.acc.en)
	{
		ade7753_read_sts(false);
	}
}

void con_cb_ade7753_damp(print_func_t pf, const char *req, int len, int *ret)
{
	pf("ADE7753 Register dump");
	pf("===== R/W =====");
	pf("Mode:     x%x", reg_read_u16(ADE7753_MODE));
	pf("IRQEN:    x%x", reg_read_u16(ADE7753_IRQEN));
	pf("CH OS 1/2:     x%x x%x", reg_read_u8(ADE7753_CH1OS), reg_read_u8(ADE7753_CH2OS));
	pf("Gain:          %d", reg_read_u8(ADE7753_GAIN));
	pf("Ph Cal:        x%x", reg_read_u8(ADE7753_PHCAL));
	pf("APOS:          %d", (int16_t)reg_read_u16(ADE7753_APOS));
	pf("Wgain Wdiv:    x%x x%x", reg_read_u16(ADE7753_WGAIN), reg_read_u8(ADE7753_WDIV));
	pf("CFnum CFden:   x%x x%x", reg_read_u16(ADE7753_CFNUM), reg_read_u16(ADE7753_CFDEN));
	pf("I/Vrms os:     x%x x%x", reg_read_u16(ADE7753_IRMSOS), reg_read_u16(ADE7753_VRMSOS));
	pf("VAgain VAdiv:  x%x x%x", reg_read_u16(ADE7753_VGAIN), reg_read_u8(ADE7753_VADIV));
	pf("LineCyc:       %d", reg_read_u16(ADE7753_LINECYC));
	pf("ZXTout:        %d", reg_read_u16(ADE7753_ZXTOUT));
	pf("SAGcyc SAGlvl: %d %d", reg_read_u8(ADE7753_SAGCYC), reg_read_u8(ADE7753_SAGLVL));
	pf("I/Vpk lvl:     %d", reg_read_u8(ADE7753_IPKLVL), reg_read_u8(ADE7753_VPKLVL));
	pf("===== R =====");
	pf("WF:    %d", read_S24(ADE7753_WAVEFORM));
	pf("AE:    %d", read_S24(ADE7753_AENERGY));
	pf("VAE:   %d", reg_read_u24(ADE7753_VAENERGY));
	pf("LAE:   %d", read_S24(ADE7753_LAENERGY));
	pf("LVAE:  %d", reg_read_u24(ADE7753_LVAENERGY));
	pf("LVARE: %d", read_S24(ADE7753_LVARENERGY));
	pf("Irms:  %d", reg_read_u24(ADE7753_IRMS));
	pf("Vrms:  %d", reg_read_u24(ADE7753_VRMS));
	pf("Ipeak: %d", reg_read_u24(ADE7753_IPEAK));
	pf("Vpeak: %d", reg_read_u24(ADE7753_VPEAK));
	pf("Temp:  %d", (int8_t)reg_read_u8(ADE7753_TEMP));
	pf("Status: x%x", reg_read_u16(ADE7753_RSTATUS));
	pf("Period: %d", reg_read_u16(ADE7753_PERIOD));
	pf("CS:  x%x", reg_read_u8(ADE7753_CHKSUM));
	pf("Die: %d", reg_read_u8(ADE7753_DIEREV));
	pf("Die: %d", reg_read_u8(ADE7753_DIEREV));
	ade7753_inst.acc.en
		? pf("L ACC rec")
		: pf("L ACC %d us", (uint32_t)ade7753_inst.acc.time);
	pf("==========================================");
}

extern bool flashing;

void con_cb_ade7753_wf1(print_func_t pf, const char *req, int len, int *ret)
{
	flashing = true;
	ade7753_set_wf_i();
}

void con_cb_ade7753_wf2(print_func_t pf, const char *req, int len, int *ret)
{
	flashing = true;
	ade7753_set_wf_u();
}

void con_cb_ade7753_roff1(print_func_t pf, const char *req, int len, int *ret)
{
	while(*req == ' ' || *req == '+')
		req++;
	uint8_t value = atoi(req) & (32 - 1);
	pf("wr i raw off: %d", value);
	reg_write_u8(ADE7753_CH1OS, value);
}

void con_cb_ade7753_roff2(print_func_t pf, const char *req, int len, int *ret)
{
	while(*req == ' ' || *req == '+')
		req++;
	uint8_t value = atoi(req) & (32 - 1);
	pf("wr u raw off: %d", value);
	reg_write_u8(ADE7753_CH2OS, value);
}

void con_cb_ade7753_off1(print_func_t pf, const char *req, int len, int *ret)
{
	while(*req == ' ' || *req == '+')
		req++;
	int value = atoi(req);
	uint16_t v = s12_format(value);
	pf("wr i_off: %d->%d", value, v);
	reg_write_u16(ADE7753_IRMSOS, v);
}

void con_cb_ade7753_off2(print_func_t pf, const char *req, int len, int *ret)
{
	while(*req == ' ' || *req == '+')
		req++;
	int value = atoi(req);
	uint16_t v = s12_format(value);
	pf("wr u_off: %d->%d", value, v);
	reg_write_u16(ADE7753_VRMSOS, v);
}

void con_cb_ade7753_off0(print_func_t pf, const char *req, int len, int *ret)
{
	while(*req == ' ' || *req == '+')
		req++;
	int16_t v = atoi(req);
	pf("wr apos_off: %d", v);
	reg_write_u16(ADE7753_APOS, v);
}

void con_cb_ade7753_acc(print_func_t pf, const char *req, int len, int *ret)
{
	ade7753_inst.mode &= ~(1 << MODE_CYCMODE);
	reg_write_u16(ADE7753_MODE, ade7753_inst.mode);

	while(*req == ' ' || *req == '+')
		req++;
	int value = atoi(req);

	if(value * 50 * 2 > 65535) value = 65535 / 100;
	pf("wr line cyc: %d sec.", value);

	ade7753_read_sts(true);
	reg_write_u16(ADE7753_LINECYC, value * 100);

	uint8_t v = 0x8 + 0x4;
	reg_write_u16(ADE7753_IRQEN, v);

	ade7753_inst.mode |= 1 << MODE_CYCMODE;
	reg_write_u16(ADE7753_MODE, ade7753_inst.mode);

	ade7753_inst.acc.en = true;
	ade7753_inst.acc.time = esp_timer_get_time();
	flashing = true;
}

void con_cb_ade7753_ch1_en(print_func_t pf, const char *req, int len, int *ret)
{
	flashing = true;
	while(*req == ' ' || *req == '+')
		req++;
	int value = atoi(req);
	if(value == 0)
		ade7753_inst.mode |= (1 << MODE_DISCH1);
	else
		ade7753_inst.mode &= ~(1 << MODE_DISCH1);
	pf("ch1 short circuit: %s", value == 0 ? "en" : "dis");
	reg_write_u16(ADE7753_MODE, ade7753_inst.mode);
}

void con_cb_ade7753_ch2_en(print_func_t pf, const char *req, int len, int *ret)
{
	flashing = true;
	while(*req == ' ' || *req == '+')
		req++;
	int value = atoi(req);
	if(value == 0)
		ade7753_inst.mode |= (1 << MODE_DISCH2);
	else
		ade7753_inst.mode &= ~(1 << MODE_DISCH2);
	pf("ch2 short circuit: %s", value == 0 ? "en" : "dis");
	reg_write_u16(ADE7753_MODE, ade7753_inst.mode);
}

void con_cb_ade7753_rst_vals(print_func_t pf, const char *req, int len, int *ret)
{
	ade7753_rst_vals();
	pf("Reset values OK");
}