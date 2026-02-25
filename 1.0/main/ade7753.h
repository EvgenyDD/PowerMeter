#ifndef ADE7753_H__
#define ADE7753_H__

#include "console.h"
#include <stdbool.h>
#include <stdint.h>

#define ADE7753_WAVEFORM 0x01
#define ADE7753_AENERGY 0x02
#define ADE7753_RAENERGY 0x03
#define ADE7753_LAENERGY 0x04
#define ADE7753_VAENERGY 0x05
#define ADE7753_RVAENERGY 0x06
#define ADE7753_LVAENERGY 0x07
#define ADE7753_LVARENERGY 0x08

#define ADE7753_STATUS 0x0B
#define ADE7753_RSTATUS 0x0C

#define ADE7753_IRMS 0x16
#define ADE7753_VRMS 0x17

#define ADE7753_IPEAK 0x22
#define ADE7753_RSTIPEAK 0x23
#define ADE7753_VPEAK 0x24
#define ADE7753_RSTVPEAK 0x25
#define ADE7753_TEMP 0x26
#define ADE7753_PERIOD 0x27

#define ADE7753_CHKSUM 0x3E
#define ADE7753_DIEREV 0x3F

// =========================

#define ADE7753_MODE 0x09
#define ADE7753_IRQEN 0x0A

#define ADE7753_CH1OS 0x0D // -1 = 31, -2 = 30...
#define ADE7753_CH2OS 0x0E // -1 = 31, -2 = 30...
#define ADE7753_GAIN 0x0F
#define ADE7753_PHCAL 0x10
#define ADE7753_APOS 0x11
#define ADE7753_WGAIN 0x12
#define ADE7753_WDIV 0x13
#define ADE7753_CFNUM 0x14
#define ADE7753_CFDEN 0x15
#define ADE7753_IRMSOS 0x18
#define ADE7753_VRMSOS 0x19
#define ADE7753_VGAIN 0x1A
#define ADE7753_VADIV 0x1B
#define ADE7753_LINECYC 0x1C
#define ADE7753_ZXTOUT 0x1D
#define ADE7753_SAGCYC 0x1E
#define ADE7753_SAGLVL 0x1F
#define ADE7753_IPKLVL 0x20
#define ADE7753_VPKLVL 0x21

#define STS_AEHF 0	 // interrupt occurred because the active energy register, AENERGY, is more than half full
#define STS_SAG 1	 // interrupt was caused by a SAG on the line voltage
#define STS_CYCEND 2 // end of energy accumulation over an integer number of half line cycles as defined by the content of the LINECYC register
#define STS_WSMP 3	 // new data is present in the waveform register
#define STS_ZX 4	 // is set to Logic 0 on the rising and falling edge of the the voltage waveform
#define STS_TEMPC 5	 // temperature conversion result is available in the temperature register
#define STS_RESET 6	 // end of a reset (for both software or hardware reset)
#define STS_AEOF 7	 // active energy register has overflowed
#define STS_PKV 8	 // waveform sample from Channel 2 has exceeded the VPKLVL value
#define STS_PKI 9	 // waveform sample from Channel 1 has exceeded the IPKLVL value
#define STS_VAEHF 10 // interrupt occurred because the active energy register, VAENERGY, is more than half full
#define STS_VAEOF 11 // apparent energy register has overflowed.
#define STS_ZXTO 12	 // interrupt was caused by a missing zero crossing on the line voltage for the specified number of line cycles
#define STS_PPOS 13	 // power has gone from negative to positive
#define STS_PNEG 14	 // power has gone from positive to negative

#define MODE_DISHPF 0	// HPF (high-pass filter) in Channel 1 is disabled when this bit is set
#define MODE_DISLPF2 1	// LPF (low-pass filter) after the multiplier (LPF2) is disabled when this bit is set
#define MODE_DISCF 2	// frequency output CF is disabled when this bit is set
#define MODE_DISSAG 3	// line voltage sag detection is disabled when this bit is set
#define MODE_ASUSPEND 4 // both ADE7753 A/D converters can be turned off
#define MODE_TEMPSEL 5	// Temperature conversion starts when this bit is set to 1
#define MODE_SWRST 6	// software Chip Reset
#define MODE_CYCMODE 7	// places the chip into line cycle energy accumulation mode
#define MODE_DISCH1 8	// ADC 1 (Channel 1) inputs are internally shorted together
#define MODE_DISCH2 9	// ADC 2 (Channel 2) inputs are internally shorted together
#define MODE_SWAP 10	// analog inputs V2P and V2N are connected to ADC 1 and the analog inputs V1P and V1N are connected to ADC 2
#define MODE_DTRT0 11	// select the waveform register update rate
#define MODE_DTRT1 12	// select the waveform register update rate
#define MODE_WAVSEL0 13 // select the source of the sampled data for the waveform register
#define MODE_WAVSEL1 14 // select the source of the sampled data for the waveform register
#define MODE_POAM 15	// only positive active power to be accumulated in the ADE7753

typedef struct
{
	struct
	{
		uint32_t e_app;
		int32_t e_act;
		int32_t wf;
		int32_t irms;
		int32_t urms;
		uint16_t period;
		int8_t temp;
	} raw;

	struct
	{
		uint32_t e_app;
		int32_t e_act;
		int32_t e_react;
		bool en;
		uint64_t time;
		uint64_t time2;
	} acc;

	float e_app;
	float e_act;
	float wf;
	float irms;
	float urms;
	float freq;
	float temp;

	uint16_t mode;
	uint16_t status;
} ade7753_inst_t;

void ade7753_init(void);
void ade7753_poll(uint32_t diff_ms);

uint16_t ade7753_read_sts(bool clear);
int32_t ade7753_read_wf(void);

uint32_t ade7753_get_cf(void);
uint8_t ade7753_get_zc_cnt(void);
uint8_t ade7753_get_zc_cnt_pos(void);

void ade7753_set_wf_e(void);
void ade7753_set_wf_i(void);
void ade7753_set_wf_u(void);

int ade7753_is_wf_rec(void);

void ade7753_rst_vals(void);

void con_cb_ade7753_damp(print_func_t pf, const char *req, int len, int *ret);
void con_cb_ade7753_wf1(print_func_t pf, const char *req, int len, int *ret);
void con_cb_ade7753_wf2(print_func_t pf, const char *req, int len, int *ret);
void con_cb_ade7753_roff1(print_func_t pf, const char *req, int len, int *ret);
void con_cb_ade7753_roff2(print_func_t pf, const char *req, int len, int *ret);
void con_cb_ade7753_off0(print_func_t pf, const char *req, int len, int *ret);
void con_cb_ade7753_off1(print_func_t pf, const char *req, int len, int *ret);
void con_cb_ade7753_off2(print_func_t pf, const char *req, int len, int *ret);
void con_cb_ade7753_acc(print_func_t pf, const char *req, int len, int *ret);
void con_cb_ade7753_ch1_en(print_func_t pf, const char *req, int len, int *ret);
void con_cb_ade7753_ch2_en(print_func_t pf, const char *req, int len, int *ret);
void con_cb_ade7753_rst_vals(print_func_t pf, const char *req, int len, int *ret);

// clang-format off

#define ADE7753_CONSOLE_ITEMS           \
	{"em_dump", con_cb_ade7753_damp},   \
	{"em_wf1", con_cb_ade7753_wf1},     \
	{"em_wf2", con_cb_ade7753_wf2},     \
	{"em_roff1", con_cb_ade7753_roff1}, \
	{"em_roff2", con_cb_ade7753_roff2}, \
	{"em_off0", con_cb_ade7753_off0},   \
	{"em_off1", con_cb_ade7753_off1},   \
	{"em_off2", con_cb_ade7753_off2},   \
	{"em_acc", con_cb_ade7753_acc},     \
	{"em_en1", con_cb_ade7753_ch1_en},  \
	{"em_en2", con_cb_ade7753_ch2_en},  \
	{"em_rst", con_cb_ade7753_rst_vals}

// clang-format on

extern ade7753_inst_t ade7753_inst;

#endif // ADE7753_H__