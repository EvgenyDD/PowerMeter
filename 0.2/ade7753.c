#include "ade7753.h"
#include "platform.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;

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

#define ADE7753_CH1OS 0x0D
#define ADE7753_CH2OS 0x0E
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

#define STS_MSK_AEHF 0x0001
#define STS_MSK_SAG 0x0002
#define STS_MSK_CYCEND 0x0004
#define STS_MSK_WSMP 0x0008
#define STS_MSK_ZX 0x0010
#define STS_MSK_TEMPC 0x0020
#define STS_MSK_RESET 0x0040
#define STS_MSK_AEOF 0x0080
#define STS_MSK_PKV 0x0100
#define STS_MSK_PKI 0x0200
#define STS_MSK_VAEHF 0x0400
#define STS_MSK_VAEOF 0x0800
#define STS_MSK_ZXTO 0x1000
#define STS_MSK_PPOS 0x2000
#define STS_MSK_PNEG 0x4000

#define CHNG_MODE SPI1->CR1 |= (1 << 0)

#define CONV_24S(v) (((v) & 0x800000) ? (int32_t)(-1 * (((uint32_t)((v) ^ 0xFFFFFF))) + 1) : (int32_t)(v))
// #define CONV_16S(v) (((v) & 0x8000) ? (int16_t)(-1 * (((uint16_t)((v) ^ 0xFFFF))) + 1) : (int16_t)(v))

ade7753_inst_t ade7753_inst = {0};

#define DLY()   \
	asm("nop"); \
	asm("nop"); \
	asm("nop"); \
	asm("nop"); \
	asm("nop");

#define DLY_IT()                                    \
	for(volatile uint32_t _it = 0; _it < 30; _it++) \
		asm("nop");

#define S_WR                           \
	while(!(SPI1->SR & SPI_FLAG_RXNE)) \
		asm("nop");

#define S_WT                          \
	while(!(SPI1->SR & SPI_FLAG_TXE)) \
		asm("nop");

#define S_DT(x)                         \
	asm("nop");                         \
	*(volatile uint8_t *)&SPI1->DR = x; \
	asm("nop");

#define S_DR(x) \
	x = (uint8_t)SPI1->DR;

#define S_WAIT_BSY()               \
	while(SPI1->SR & SPI_FLAG_BSY) \
		;

static int32_t EmRd24S(uint32_t value)
{
	uint32_t result = value;
	if(result & 0x800000)
	{
		uint32_t temp = result ^ 0xFFFFFF;
		return (int32_t)(-1 * (temp + 1));
	}
	else
		return (int32_t)result;
}

// int16_t EmRd16S(uint8_t adress)
// {
// 	uint16_t result = EmRd16(adress);
// 	if(result & 0x8000)
// 	{
// 		uint16_t temp = result ^ 0xFFFF;
// 		return (int16_t)(-1 * (temp + 1));
// 	}
// 	else
// 		return (int16_t)result;
// }

#if 0
L2F50_FloatWDoubleString(1, 122 - 15, Display.voltage, 1, "V=", "B", FONT5x8, YELLOW, colorScheme[scheme][0]);
L2F50_FloatWDoubleString(40, 122, Display.current, 3, "I=", "A  ", FONT5x8, RED, colorScheme[scheme][0]);
L2F50_FloatWDoubleString(100, 121, Display.frequency, 1, "f=", "Hz    ", FONT5x8, BLUE, colorScheme[scheme][0]);
L2F50_FloatWDoubleString(116, 89, Display.pApp, 1, "AE=", "VA   ", FONT5x8, colorScheme[scheme][2], colorScheme[scheme][0]);
L2F50_FloatWDoubleString(1, 60, Display.pAct, (Display.pAct / 100 < 100) ? 2 : 0, "P=", "W   ", FONT16x26, colorScheme[scheme][2], colorScheme[scheme][0]);
L2F50_FloatWDoubleString(1, 40, Display.energy, (Display.energy / 100 < 100) ? 2 : 0, "E=", "kWh   ", FONT10x16, colorScheme[scheme][2], colorScheme[scheme][0]);
L2F50_FloatWDoubleString(1, 21, Display.costAmount, (Display.costAmount / 100 < 100) ? 2 : 0, "Cost=", "Rub    ", FONT10x16, colorScheme[scheme][2], colorScheme[scheme][0]);
#endif

#if 0
	if(!flag)
	{
		// EmWr16(EM_MODE, 808);
	}

	/*uint16_t ddd = EmRd16(EM_MODE);
	BitSet(ddd, 5);
	BitReset(ddd, 2);*/

	L2F50_DblWDoubleString(1, 123, EmRd24S(2), "#AE=", "     ", FONT5x8, BLUE, BLACK);
	L2F50_DblWDoubleString(88, 123, EmRd24(5), "VAE=", "     ", FONT5x8, BLUE, BLACK);

	L2F50_DblWDoubleString(1, 123 - 1 * 9, EmRd24(7), "LVA=", "     ", FONT5x8, BLUE, BLACK);
	L2F50_DblWDoubleString(88, 123 - 1 * 9, EmRd24S(4), "#LAE=", "     ", FONT5x8, BLUE, BLACK);

	L2F50_DblWDoubleString(1, 123 - 2 * 9, EmRd24S(8), "#LVAR", "     ", FONT5x8, BLUE, BLACK);
	L2F50_DblWDoubleString(88, 123 - 2 * 9, EmRd24S(1), "#WaFm", "     ", FONT5x8, WHITE, BLACK); // waveform

	L2F50_DblWDoubleString(1, 123 - 3 * 9, EmRd24(0x16), "Irms", "     ", FONT5x8, RED, BLACK);
	L2F50_DblWDoubleString(88, 123 - 3 * 9, EmRd24(0x17), "Vrms", "     ", FONT5x8, RED, BLACK);

	L2F50_DblWDoubleString(1, 123 - 4 * 9, EmRd24(0x22), "Ipk=", "     ", FONT5x8, RED, BLACK);
	L2F50_DblWDoubleString(88, 123 - 4 * 9, EmRd24(0x24), "Vpk=", "     ", FONT5x8, RED, BLACK);

	L2F50_IntWDoubleString(1, 123 - 5 * 9, EmRd16(0x27), "Per=", "     ", FONT5x8, BLUE, BLACK);
	L2F50_IntWDoubleString(58, 123 - 5 * 9, EmRd16(0x1C), "LCyc", "     ", FONT5x8, WHITE, BLACK);
	L2F50_IntWDoubleString(116, 123 - 5 * 9, EmRd8(0x26), "#Temp=", "     ", FONT5x8, YELLOW, BLACK);

	L2F50_IntWDoubleString(58, 123 - 6 * 9, EmRd16(0x09), "Mod=", "     ", FONT5x8, WHITE, BLACK);
	L2F50_IntWDoubleString(116, 123 - 6 * 9, EmRd16(0x0B), "Irq=", "     ", FONT5x8, WHITE, BLACK);

	//	L2F50_IntWDoubleString(58,123-9*9, EmRd16(0x14),"=", "     ", FONT5x8, GREEN, BLACK);
	//	L2F50_IntWDoubleString(116,123-9*9, EmRd16(0x15),"=", "     ", FONT5x8, GREEN, BLACK);


	if(f % 10 == 0)
	{
		EmRd24(0x23);
		EmRd24(0x25);
		/*if(f%20==0)
		{
		uint16_t ddd = EmRd16(EM_MODE);
			//BitSet(ddd, 5);
			//BitSet(ddd, 4);
		BitSet(ddd, 8);
		BitSet(ddd, 9);
			EmWr16(EM_MODE, ddd);
			EmRd24(0x23);
					EmRd24(0x25);
		}*/
	}

	if(f % 8 == 0)
	{
		uint16_t ddd = EmRd16(EM_MODE);
		BitSet(ddd, 5);
		EmWr16(EM_MODE, ddd);
	}
#endif

static void reg_write1(uint8_t reg, uint8_t v)
{
	uint8_t rx;

	CHNG_MODE;
	DLY();
	PIN_CLR(CS_EM);
	DLY();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	S_DT((1 << 7) | reg);
	S_WT;
	S_DT(v);
	S_WR;
	S_DR(rx);
	S_WR;
	S_DR(rx);
#pragma GCC diagnostic pop
	S_WAIT_BSY();

	DLY();
	PIN_SET(CS_EM);
	DLY_IT();
}

static void reg_write2(uint8_t reg, uint16_t v)
{
	uint8_t rx;

	CHNG_MODE;
	DLY();
	PIN_CLR(CS_EM);
	DLY();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	S_DT((1 << 7) | reg);
	S_WT;
	S_DT(v >> 8);
	S_WR;
	S_DR(rx);
	S_WT;
	S_DT(v & 0xFF);
	S_WR;
	S_DR(rx);
	S_WR;
	S_DR(rx);
#pragma GCC diagnostic pop
	S_WAIT_BSY();

	DLY();
	PIN_SET(CS_EM);
	DLY_IT();
}

static void reg_read1(uint8_t reg, uint8_t *v)
{
	uint8_t rx[2];

	CHNG_MODE;
	DLY();
	PIN_CLR(CS_EM);
	DLY();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	S_DT(reg);
	S_WT;
	S_DT(0);
	S_WR;
	S_DR(rx[0]);
	S_WR;
	S_DR(rx[1]);
#pragma GCC diagnostic pop
	S_WAIT_BSY();

	DLY();
	PIN_SET(CS_EM);
	DLY_IT();
	*v = rx[1];
}

static void reg_read2(uint8_t reg, uint16_t *v)
{
	uint8_t rx[3];

	CHNG_MODE;
	DLY();
	PIN_CLR(CS_EM);
	DLY();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	S_DT(reg);
	S_WT;
	S_DT(0);
	S_WR;
	S_DR(rx[0]);
	S_WT;
	S_DT(0);
	S_WR;
	S_DR(rx[1]);
	S_WR;
	S_DR(rx[2]);
#pragma GCC diagnostic pop
	S_WAIT_BSY();

	DLY();
	PIN_SET(CS_EM);
	DLY_IT();
	*v = ((uint16_t)rx[1] << 8) | (uint16_t)rx[2];
}

static void reg_read3(uint8_t reg, uint32_t *v)
{
	uint8_t rx[4];

	CHNG_MODE;
	DLY();
	PIN_CLR(CS_EM);
	DLY();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	S_DT(reg);
	S_WT;
	S_DT(0);
	S_WR;
	S_DR(rx[0]);
	S_WT;
	S_DT(0);
	S_WR;
	S_DR(rx[1]);
	S_WT;
	S_DT(0);
	S_WR;
	S_DR(rx[2]);
	S_WR;
	S_DR(rx[3]);
#pragma GCC diagnostic pop
	S_WAIT_BSY();

	DLY();
	PIN_SET(CS_EM);
	DLY_IT();
	*v = ((uint32_t)rx[1] << 16) | ((uint32_t)rx[2] << 8) | (uint32_t)rx[3];
}

#include "GUI_Paint.h"
#include "lcd_pico.h"

void ade7753_init(void)
{
	PIN_CLR(RST_EM);
	HAL_Delay(1);
	PIN_SET(RST_EM);
	HAL_Delay(1);
	uint8_t a = 0;

	while(!(ade7753_read_sts() & STS_MSK_RESET))
	{
		lcd_string(0, 80, &Font16, WHITE, BLACK, "STATUS: x%x        ", ade7753_inst.status);
	}

	ade7753_inst.irqen = 0x8;
	reg_write2(ADE7753_IRQEN, ade7753_inst.irqen);

	// uint32_t r = 0;
	// while(++r < 100000)
	// {
	// 	ade7753_inst.irqen++;
	// 	reg_write2(ADE7753_IRQEN, ade7753_inst.irqen);
	// 	uint16_t iqqe = 0;
	// 	reg_read2(ADE7753_IRQEN, &iqqe);
	// 	if(iqqe != (ade7753_inst.irqen | 0x40))
	// 	{
	// 		lcd_string(0, 80, &Font16, WHITE, BLACK, "IQQE: x%x x%x        ", ade7753_inst.irqen, iqqe);
	// 		while(1)
	// 			;
	// 	}
	// 	if((ade7753_inst.irqen % 100) == 0)
	// 	{
	// 		lcd_string(0, 80, &Font16, WHITE, BLACK, "IQQE: x%x x%x        ", ade7753_inst.irqen, iqqe);
	// 	}
	// }

	reg_write1(ADE7753_CH1OS, 0x0);
	reg_write1(ADE7753_CH2OS, 0x0);
	reg_write1(ADE7753_GAIN, (1 << 3) | (1 << 1) | (1 << 0));
	ade7753_inst.mode = 1 << 11;
	reg_write2(ADE7753_MODE, ade7753_inst.mode);

	reg_read1(ADE7753_CHKSUM, &ade7753_inst.checksum);
	reg_read1(ADE7753_DIEREV, &ade7753_inst.die_rev);
}

void ade7753_set_wf_i(void)
{
	ade7753_inst.mode &= ~((1 << 14) | (1 << 13));
	ade7753_inst.mode |= (1 << 14);
	reg_write2(ADE7753_MODE, ade7753_inst.mode);
}

void ade7753_set_wf_u(void)
{
	ade7753_inst.mode &= ~((1 << 14) | (1 << 13));
	ade7753_inst.mode |= (1 << 14) | (1 << 13);
	reg_write2(ADE7753_MODE, ade7753_inst.mode);
}

int8_t ade7753_read_temp(void)
{
	reg_read1(ADE7753_TEMP, &ade7753_inst.r.temp);
	return ade7753_inst.r.temp;
}

uint16_t ade7753_read_sts(void)
{
	reg_read2(ADE7753_STATUS, &ade7753_inst.status);
	return ade7753_inst.status;
}

uint16_t ade7753_read_sts_clr(void)
{
	reg_read2(ADE7753_RSTATUS, &ade7753_inst.status);
	return ade7753_inst.status;
}

uint32_t ade7753_read_i_rms(void)
{
	reg_read3(ADE7753_IRMS, &ade7753_inst.r.irms);
	reg_read2(ADE7753_PERIOD, &ade7753_inst.r.period);
	reg_read3(ADE7753_VAENERGY, &ade7753_inst.r.vae);
	return ade7753_inst.r.irms;
}

int32_t ade7753_read_wf(void)
{
	uint32_t wf = 0;
	while(!(ade7753_read_sts_clr() & STS_MSK_WSMP))
		asm("nop");
	reg_read3(ADE7753_WAVEFORM, &wf);
	// ade7753_inst.r.wf = /*EmRd24S*/ (int32_t)(wf);
	// uint8_t nBytes = 3;
	// Make signed
	int32_t wfs = wf;

	const int MODULO = 1 << 24;
	const int MAX_VALUE = (1 << 23) - 1;

	if(wfs > MAX_VALUE)
	{
		wfs -= MODULO;
	}

	// // Use signed shift for 8 byte alignment, then to move LSB to 0 byte
	// wfs >>= 8;
	ade7753_inst.r.wf = wfs;

	return ade7753_inst.r.wf;
}

uint32_t ade7753_read_u_rms(void)
{
	reg_read3(ADE7753_VRMS, &ade7753_inst.r.urms);
	return ade7753_inst.r.urms;
}