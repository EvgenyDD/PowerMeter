/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SPI_H
#define SPI_H

/* Includes ------------------------------------------------------------------*/
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_spi.h>


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
// Read registers
#define EM_WAVEform 	(0x01)

#define EM_AEnergy 		(0x02)		//active energy
 #define EM_resAEnergy 	(0x03)
#define EM_LAEnergy 	(0x04)		//line accumulation
#define EM_VAEnergy 	(0x05)		//apparent
 #define EM_resVAEnergy (0x06)
#define EM_LVAEnergy 	(0x07)
#define EM_LVAREnergy 	(0x08)

#define EM_IRMS 		(0x16)		// I rms
#define EM_VRMS 		(0x17)		// U rms
#define EM_LINEcyc 		(0x1C)
#define EM_IPeak 		(0x22)		//peak I
 #define EM_RSTIPeak 	(0x23)
#define EM_VPeak		(0x24)		//peak U
 #define EM_RSTVPeak 	(0x25)
#define EM_Temp 		(0x26)		//temperature
#define EM_Period 		(0x27)		//period
#define EM_CHKSUM 		(0x3E)
#define EM_Revision 	(0x3F)

// Mode registers
#define EM_MODE 		(0x09)
#define EM_IRQEN 		(0x0A)
#define EM_status 		(0x0B)
 #define EM_RSTstatus 	(0x0C)

// Settings registers
#define EM_CH1os 		(0x0D)
#define EM_CH2os 		(0x0E)
#define EM_Gain 		(0x0F)
#define EM_PHcal 		(0x10)
#define EM_APos 		(0x11)
#define EM_Wgain 		(0x12)
#define EM_WDIV 		(0x13)
#define EM_CFnum 		(0x14)
#define EM_CFden 		(0x15)

#define EM_IRMSos 		(0x18)
#define EM_VRMSos 		(0x19)
#define EM_VAgain 		(0x1A)
#define EM_VAdiv 		(0x1B)

#define EM_ZXTOUT 		(0x1D)
#define EM_SAGcyc 		(0x1E)
#define EM_SAGlvl 		(0x1F)
#define EM_IPKlvl 		(0x20)
#define EM_VPKlvl 		(0x21)

#define DISHPF 			(0)
#define DISSAG 			(3)
#define TEMPSEL 		(5)
#define SWRST 			(6)
#define WAVSEL0 		(13)
#define WAVSEL1 		(14)


/* Exported macro ------------------------------------------------------------*/
#define CS_EN	GPIO_ResetBits(GPIOB, GPIO_Pin_0)
#define CS_DIS	GPIO_SetBits(GPIOB, GPIO_Pin_0)


/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint8_t EmRd8(uint8_t);
uint16_t EmRd16(uint8_t);
uint32_t EmRd24(uint8_t);

void EmWr8(uint8_t, uint8_t);
void EmWr16(uint8_t, uint16_t);

int16_t EmRd16S(uint8_t adress);
int32_t EmRd24S(uint8_t adress);


#endif //SPI_H
