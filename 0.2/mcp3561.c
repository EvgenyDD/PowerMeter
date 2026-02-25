#include "mcp3561.h"
#include "platform.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;

#define CHNG_MODE SPI1->CR1 &= ~(1 << 0)

static void reg_write(uint8_t reg, uint8_t *buf, size_t size)
{
	uint8_t tx[1 + size], rx[1 + size];
	tx[0] = (1 << 7) | reg;
	memcpy(&tx[1], buf, size);

	CHNG_MODE;
	PIN_CLR(CS_ADC);
	HAL_SPI_TransmitReceive(&hspi1, tx, rx, 1 + size, 200);
	PIN_SET(CS_ADC);
}

static void reg_read(uint8_t reg, uint8_t *buf, size_t size)
{
	uint8_t tx[1 + size], rx[1 + size];
	tx[0] = reg;

	CHNG_MODE;
	PIN_CLR(CS_ADC);
	HAL_SPI_TransmitReceive(&hspi1, tx, rx, 1 + size, 200);
	PIN_SET(CS_ADC);
	memcpy(buf, &rx[1], size);
}

void mcp3561_init(void)
{
	uint8_t cmd[3] = {(1 << 6) | (0xe << 2) | 1, 0, 0}, ret[3];

	CHNG_MODE;
	PIN_CLR(CS_ADC);
	HAL_SPI_TransmitReceive(&hspi1, cmd, ret, 3, 200);
	PIN_SET(CS_ADC);
	// fuck[0] = ret[0];
	// fuck[1] = ret[1];
	// fuck[2] = ret[2];
}