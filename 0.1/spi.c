/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include <CoOS.h>

#include "global.h"
#include "l2f50.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define Delay _delay_loops(640)


/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : _delay_loops
* Description    : Software delay in loops
* Input			 : num of loops
*******************************************************************************/
void _delay_loops(uint32_t loops)
{
	asm volatile (
		"1: SUBS %[loops], %[loops], #1 \n"
		"   BNE 1b \n"
		: [loops] "+r"(loops)
	);
}


/*******************************************************************************
* Function Name  : EmRd8
* Description    :
* Input			 : device SPI adress
*******************************************************************************/
uint8_t EmRd8(uint8_t adress)
{
	CS_EN;

	SPI_I2S_SendData(SPI1, adress);//first
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, 0x00);//second

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI1); //dummy

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	uint8_t result = SPI_I2S_ReceiveData(SPI1);//data

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);//end
	CS_DIS;
	return result;
}


/*******************************************************************************
* Function Name  : EmRd16
* Description    :
* Input			 : device SPI adress
*******************************************************************************/
uint16_t EmRd16(uint8_t adress)
{
	uint16_t result = 0;
	CS_EN;

	SPI_I2S_SendData(SPI1, adress);//first
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, 0x00);//second

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI1); //dummy

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, 0x00);//third

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	result |= (uint16_t)(SPI_I2S_ReceiveData(SPI1) << 8);

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	result |= (uint16_t)SPI_I2S_ReceiveData(SPI1);

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);//end
	CS_DIS;
	return result;
}


/*******************************************************************************
* Function Name  : EmRd24
* Description    :
* Input			 : device SPI adress
*******************************************************************************/
uint32_t EmRd24(uint8_t adress)
{
	uint32_t result = 0;
	CS_EN;

	SPI_I2S_SendData(SPI1, adress);//first
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, 0x00);//second

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI1); //dummy

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, 0x00);//third

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	result |= SPI_I2S_ReceiveData(SPI1) << 16;

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, 0x00);//fourth

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	result |= (uint32_t)(SPI_I2S_ReceiveData(SPI1) << 8);

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	result |= (uint32_t)SPI_I2S_ReceiveData(SPI1);

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);//end
	CS_DIS;
	return result;
}


/*******************************************************************************
* Function Name  : EmWr8
* Description    :
* Input			 : device SPI adress, register data
*******************************************************************************/
void EmWr8(uint8_t adress, uint8_t value)
{
	CS_EN;

	SPI_I2S_SendData(SPI1, adress | 0x80);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, value);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI1);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI1);

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	CS_DIS;
}


/*******************************************************************************
* Function Name  : EmWr16
* Description    :
* Input			 : device SPI adress, register data
*******************************************************************************/
void EmWr16(uint8_t adress, uint16_t value)
{
	CS_EN;

	SPI_I2S_SendData(SPI1, adress | 0x80);//first
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, (uint8_t)(value>>8));//second

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI1); //dummy 1

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, (uint8_t)value);//third

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI1); //dummy 2

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI1); //dummy 3

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);//end
	CS_DIS;
}


/*******************************************************************************
* Function Name  : EmRd24S
* Description    : read signed two's complement 24-bit value
* Input			 : register adress
*******************************************************************************/
int32_t EmRd24S(uint8_t adress)
{
	uint32_t result = EmRd24(adress);
	if(result & 0x800000)
	{
		uint32_t temp = result ^ 0xFFFFFF;
		return (int32_t)(-1*(temp+1));
	}
	else
		return (int32_t)result;
}

/*******************************************************************************
* Function Name  : EmRd16S
* Description    : read signed two's complement 16-bit value
* Input			 : register adress
*******************************************************************************/
int16_t EmRd16S(uint8_t adress)
{
	uint16_t result = EmRd16(adress);
	if(result & 0x8000)
	{
		uint16_t temp = result ^ 0xFFFF;
		return (int16_t)(-1*(temp+1));
	}
	else
		return (int16_t)result;
}


