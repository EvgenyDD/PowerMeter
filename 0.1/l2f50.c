/* Includes ------------------------------------------------------------------*/
#include "l2f50.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"

#include "string.h"

#include <CoOS.h>


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define L2F50_SPI_MODE //if defined - SPI hardware is used\ else - software SPI


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
extern const char font5x8[];
extern const char font16_26[];


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : L2F50_InitPeripheral
* Description    : Initialize peripherals that drive display
*******************************************************************************/
void L2F50_InitPeripheral()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

#ifdef L2F50_SPI_MODE
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2 | RCC_APB2Periph_AFIO, ENABLE);

	/* Data and Clock pins */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//8
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_Init(SPI2, &SPI_InitStructure);

	SPI_NSSInternalSoftwareConfig(SPI2, SPI_NSSInternalSoft_Set);
	SPI_Cmd(SPI2, ENABLE);
#else
	/* Data and Clock pins */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
}


/*******************************************************************************
* Function Name  : L2F50_Cmd
* Description    : Sent command to display
* Input			 : command
*******************************************************************************/
void L2F50_Cmd(uint8_t data)
{
	//CoSchedLock();
	LCD_RS_0;

#ifdef L2F50_SPI_MODE

	SPI_I2S_SendData(SPI2, data);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	SPI_I2S_SendData(SPI2, 0x00);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

#else

	for(uint8_t i=0; i<8; i++)
	{
		if (data & 0x80)
			LCD_MOSI1;
		else
			LCD_MOSI0;

		data <<= 1;

		LCD_SCK0;
		LCD_SCK1;
	}

	LCD_MOSI0;

	for(uint8_t i=0; i<8; i++)
	{
		LCD_SCK0;
		LCD_SCK1;
	}

#endif

	LCD_RS_1;
	//CoSchedUnlock();
}


/*******************************************************************************
* Function Name  : L2F50_Dat8
* Description    : Send char data to display RAM
* Input			 : Char data
*******************************************************************************/
void L2F50_Dat8(uint8_t data)
{
	//CoSchedLock();
#ifdef L2F50_SPI_MODE

	SPI_I2S_SendData(SPI2, data);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	SPI_I2S_SendData(SPI2, 0x00);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

#else

	for(uint8_t i=0; i<8; i++)
	{
		if (data & 0x80)
			LCD_MOSI1;
		else
			LCD_MOSI0;

		data <<= 1;

		LCD_SCK0;
		LCD_SCK1;
	}

	LCD_MOSI0;

	for(uint8_t i=0; i<8; i++)
	{
		LCD_SCK0;
		LCD_SCK1;
	}

#endif
	//CoSchedUnlock();
}


/*******************************************************************************
* Function Name  : L2F50_Dat16
* Description    : Send int data to display RAM
* Input			 : int data
*******************************************************************************/
void L2F50_Dat16(uint16_t data)
{
	//CoSchedLock();
#ifdef L2F50_SPI_MODE

	SPI_I2S_SendData(SPI2, data>>8);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	SPI_I2S_SendData(SPI2, data);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

#else

	uint16_t m = data >> 8;

	for(uint8_t i=0; i<8; i++)
	{
		if (m & 0x80)
			LCD_MOSI1;
		else
			LCD_MOSI0;

		m <<= 1;

		LCD_SCK0;
		LCD_SCK1;
	}

	for(uint8_t i=0; i<8; i++)
	{
		if (data & 0x80)
			LCD_MOSI1;
		else
			LCD_MOSI0;

		data <<= 1;

		LCD_SCK0;
		LCD_SCK1;
	}

#endif
	//CoSchedUnlock();
}


/*******************************************************************************
* Function Name  : L2F50_Init
* Description    : Sequence of commands to initialize the display
*******************************************************************************/
void L2F50_Init()
{
	uint16_t i;
	static const int disctl[9] = {0x4C, 0x01, 0x53, 0x00, 0x02, 0xB4, 0xB0, 0x02, 0x00};
	static const int gcp64_0[29] = {0x11,0x27,0x3C,0x4C,0x5D,0x6C,0x78,0x84,0x90,0x99,0xA2,0xAA,0xB2,0xBA,
	0xC0,0xC7,0xCC,0xD2,0xD7,0xDC,0xE0,0xE4,0xE8,0xED,0xF0,0xF4,0xF7,0xFB,
	0xFE};
	static const int gcp64_1[34] = {0x01,0x03,0x06,0x09,0x0B,0x0E,0x10,0x13,0x15,0x17,0x19,0x1C,0x1E,0x20,
	0x22,0x24,0x26,0x28,0x2A,0x2C,0x2D,0x2F,0x31,0x33,0x35,0x37,0x39,0x3B,
	0x3D,0x3F,0x42,0x44,0x47,0x5E};
	static const int gcp16[15] = {0x13,0x23,0x2D,0x33,0x38,0x3C,0x40,0x43,0x46,0x48,0x4A,0x4C,0x4E,0x50,0x64};

	LCD_RST_0;
	LCD_CS_1;
	LCD_RS_1;
		asm("nop");	asm("nop");	asm("nop");	asm("nop");	asm("nop");
	LCD_RST_1;
		asm("nop");	asm("nop");	asm("nop");	asm("nop");	asm("nop");
	LCD_CS_0;

	L2F50_Cmd(DATCTL);
	L2F50_Dat8(0x2A); // 0x2A=565 mode, 0x0A=666mode, 0x3A=444mode

	LCD_CS_1;
	asm("nop"); asm("nop");
	LCD_CS_0;

	L2F50_Cmd(DISCTL);

	for (i=0; i<9; i++)
		L2F50_Dat8(disctl[i]);

	L2F50_Cmd(GCP64);

	for (i=0; i<29; i++)
	{
		L2F50_Dat8(gcp64_0[i]);
		L2F50_Dat8(0x00);
	}

	for (i=0; i<34; i++)
	{
		L2F50_Dat8(gcp64_1[i]);
		L2F50_Dat8(0x01);
	}

	L2F50_Cmd(GCP16);

	for (i=0; i<15; i++)
		L2F50_Dat8(gcp16[i]);

	L2F50_Cmd(GSSET);
	L2F50_Dat8(0x00);

	L2F50_Cmd(OSSEL);
	L2F50_Dat8(0x00);

	L2F50_Cmd(SLPOUT);

	L2F50_Cmd(SD_CSET);
	L2F50_Dat8(0x08);
	L2F50_Dat8(0x01);
	L2F50_Dat8(0x8B);
	L2F50_Dat8(0x01);

	L2F50_Cmd(SD_PSET);
	L2F50_Dat8(0x00);
	L2F50_Dat8(0x8F);


	L2F50_Cmd(ASCSET);
	L2F50_Dat8(0x00);
	L2F50_Dat8(0xAF);
	L2F50_Dat8(0xAF);
	L2F50_Dat8(0x03);

	L2F50_Cmd(SCSTART);
	L2F50_Dat8(0x00);

	LCD_RS_0;
	L2F50_Dat8(DISON);

	LCD_CS_1;
}


/*******************************************************************************
* Function Name  : L2F50_SetWindow
* Description    : Set window in display RAM to write data in it
* Input			 : start corner {x1, y1}, end corner {x2, y2}
*******************************************************************************/
void L2F50_SetWindow (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	L2F50_Cmd(SD_CSET);
	L2F50_Dat8(8+x1);
	L2F50_Dat8(0x01);
	L2F50_Dat8(8+x2);
	L2F50_Dat8(0x01);

	L2F50_Cmd(SD_PSET);
	L2F50_Dat8(y1);
	L2F50_Dat8(y2);
}


/*******************************************************************************
* Function Name  : L2F50_FillScreen
* Description    : Fill all screen with color
* Input			 : color
*******************************************************************************/
void L2F50_FillScreen(uint16_t color)
{
	CoSchedLock();
	LCD_CS_0;

	L2F50_SetWindow(0,0,DISP_H-1,DISP_W-1);
	L2F50_Cmd(RAMWR);

	for (uint16_t i=0; i<DISP_W*DISP_H; i++)
		L2F50_Dat16(color);

	LCD_CS_1;
	CoSchedUnlock();
}


/*******************************************************************************
* Function Name  : L2F50_Pixel
* Description    : Draw one pixel on the screen
* Input			 : x position, y position, color
*******************************************************************************/
void L2F50_Pixel(uint16_t x, uint16_t y, int color)
{
	LCD_CS_0;

	L2F50_Cmd(SD_CSET);
	L2F50_Dat8(0x08+y);  // start is 8, not 0
	L2F50_Dat8(0x01);
	L2F50_Cmd(SD_PSET);
	L2F50_Dat8(x);
	L2F50_Cmd(RAMWR);
	L2F50_Dat16(color);

	LCD_CS_1;
}


////////////////////////////////////////////////////////////////////////////////
/////////////////////////// GRAPHICS ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
* Function Name  : L2F50_Line
* Description    : Draw line
* Input			 : start point {x1, y1}, end point {x2, y2}, line color
*******************************************************************************/
void L2F50_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	CoSchedLock();
	LCD_CS_0;
	L2F50_SetWindow(0,0,131,175);
	LCD_CS_1;

	int dX = /*L2F50_abs(x2 - x1);*/(x2-x1>0)?x2-x1:x1-x2;
	int dY = /*L2F50_abs(y2 - y1);*/(y2-y1>0)?y2-y1:y1-y2;
	int signX = /*L2F50_sign(x2 - x1)*/(x2-x1>0)?1:-1;
	int signY = /*L2F50_sign(y2 - y1)*/(y2-y1>0)?1:-1;
	int err=dY-dX, err2;

	while(1)
	{
		L2F50_Pixel(x1, y1, color);

		if(y1 == y2 && x1 == x2) break;
		err2 = err * 2;

		if(err2 > -dX)
		{
			err -= dX;
			y1 += signY;
		}
		if(err2 < dY)
		{
			err += dY;
			x1 += signX;
		}
	}
	CoSchedUnlock();
}


/*******************************************************************************
* Function Name  : L2F50_Rect
* Description    : Draw rectangular (outline)
* Input			 : start corner {x1, y1}, end corner {x2, y2}, rectangular color
*******************************************************************************/
void L2F50_Rect(uint16_t x, uint16_t y, uint16_t width, uint16_t heigh, uint16_t color)
{
	L2F50_Line(x, y, x, y + heigh, color);
	L2F50_Line(x, y + heigh, x + width, y + heigh, color);
	L2F50_Line(x + width, y + heigh, x + width, y, color);
	L2F50_Line(x + width, y, x, y, color);
}


/*******************************************************************************
* Function Name  : L2F50_RectFill
* Description    : Draw rectangular (outline+inside)
* Input			 : start corner {x1, y1}, end corner {x2, y2},
* 				 : rectangular outline color, inside color
*******************************************************************************/
void L2F50_RectFill(uint16_t x, uint16_t y, uint16_t width, uint16_t heigh,
							uint16_t outlineColor, uint16_t fillColor)
{
	L2F50_Line(x, y, x, y + heigh, outlineColor);
	L2F50_Line(x, y + heigh, x + width, y + heigh, outlineColor);
	L2F50_Line(x + width, y + heigh, x + width, y, outlineColor);
	L2F50_Line(x + width, y, x, y, outlineColor);

	CoSchedLock();
	LCD_CS_0;

	L2F50_SetWindow(y+1, x+1, y+heigh-1, x+width-1);
	L2F50_Cmd(RAMWR);

	for(uint16_t i=0; i<(width-1)*(heigh-1); i++)
		L2F50_Dat16(fillColor);

	LCD_CS_1;
	CoSchedUnlock();
}


/*******************************************************************************
* Function Name  : L2F50_Circle
* Description    : Draw circle (outline)
* Input			 : center point {x, y}, radius, circle outline color
*******************************************************************************/
void L2F50_Circle(uint16_t xPos, uint16_t yPos, uint16_t radius, uint16_t color)
{
	if(xPos >= DISP_W) xPos = DISP_W;
	if(yPos >= DISP_H) yPos = DISP_H;

	int32_t D = 3 - (radius << 1);
	uint32_t curX = 0;
	uint32_t curY = radius;

	while(curX <= curY)
	{
		L2F50_Pixel(xPos + curX, yPos + curY, color);
		L2F50_Pixel(xPos + curX, yPos - curY, color);
		L2F50_Pixel(xPos - curX, yPos + curY, color);
		L2F50_Pixel(xPos - curX, yPos - curY, color);

		L2F50_Pixel(xPos + curY, yPos + curX, color);
		L2F50_Pixel(xPos + curY, yPos - curX, color);
		L2F50_Pixel(xPos - curY, yPos + curX, color);
		L2F50_Pixel(xPos - curY, yPos - curX, color);

		if(D < 0)
			D += (curX << 2) + 6;
		else
			D += ((curX - curY--) << 2) + 10;

		curX++;
	}
}


/*******************************************************************************
* Function Name  : L2F50_CircleFill
* Description    : Draw circle (outline + inside)
* Input			 : center point {x, y}, radius, outline color, fill color
*******************************************************************************/
void L2F50_CircleFill(uint16_t xPos, uint16_t yPos,
		uint16_t Radius, uint16_t outlineColor, uint16_t fillColor)
{
	if(xPos >= DISP_W) xPos = DISP_W;
	if(yPos >= DISP_H) yPos = DISP_H;

	int32_t D = 3 - (Radius << 1);
	uint32_t curX = 0;
	uint32_t curY = Radius;

	while (curX <= curY)
	{
		if(curY > 0)
		{
			L2F50_Line(xPos - curX, yPos + curY, xPos - curX, yPos + curY - 2*curY, fillColor);
			L2F50_Line(xPos + curX, yPos + curY, xPos + curX, yPos + curY - 2*curY, fillColor);
		}
		if(curX > 0)
		{
			L2F50_Line(xPos - curY, yPos + curX, xPos - curY, yPos + curX - 2*curX, fillColor);
			L2F50_Line(xPos + curY, yPos + curX, xPos + curY, yPos + curX - 2*curX, fillColor);
		}

		if (D < 0)
		{
			D += (curX << 2) + 6;
		}
		else
		{
			D += ((curX - curY) << 2) + 10;
			curY--;
		}
		curX++;
	}

	L2F50_Circle(xPos, yPos, Radius, outlineColor);
}


////////////////////////////////////////////////////////////////////////////////
///////////////////////////// TEXT /////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
* Function Name  : L2F50_Char
* Description    : Display char on display
* Input			 : start point {x, y}, char to display, font,
* 				 : colors {text, back color}
*******************************************************************************/
uint8_t L2F50_Char(uint8_t x, uint8_t y, char data, uint8_t font, uint16_t textColor, uint16_t fillColor)
{
	CoSchedLock();
	LCD_CS_0;
	uint8_t CHAR_POS;

	if(font == 0 || font == 1 || font == 2) //5x8
	{
		uint8_t fontH = 8;
		uint8_t fontW = 5;

		uint16_t pointer = (data-32)*(fontW);
		uint16_t real;

		for(uint16_t row=0; row<fontH+1; row++)
		{
			for(uint8_t i=0; i<font+1; i++)
			{
				L2F50_SetWindow(y + (font+1)*row+i, x, y + (font+1)*row+i, x + (font+1)*fontW);
				L2F50_Cmd(RAMWR);

				real = pointer;

				for(uint8_t col=0; col<fontW; col++)
				{
					for(uint8_t i=0; i<font+1; i++)
					{
						if( (font5x8[real] << row) & 0x80 )
							L2F50_Dat16(textColor);
						else
							L2F50_Dat16(fillColor);
					}
					real++;
				}
			}
		}
		CHAR_POS = (font+1)*(fontW)+1;
	}

	else if(font == 3) //16x26
	{
		uint8_t fontW = 16;
		uint8_t fontH = 26;

		uint16_t pointer = 2*(data - 32)*(fontH);
		uint16_t shift = 0;

		for(uint16_t col=fontH+1; col>0; col--)
		{
			L2F50_SetWindow(y + col-1, x, y + col-1, x + fontW);
			L2F50_Cmd(RAMWR);

			pointer++;

			for(uint16_t row=0; row<fontW; row++)
			{
				if( ((font16_26[pointer]) >> (7 - shift)) & 1 )
					L2F50_Dat16(textColor);
				else
					L2F50_Dat16(fillColor);

				if(++shift >= 8)
				{
					shift = 0;
					pointer--;
				}
			}
			pointer += 3;
			shift = 0;
		}
		CHAR_POS = fontW + 1;
	}

	LCD_CS_1;
	CoSchedUnlock();
	return CHAR_POS;
}


/*******************************************************************************
* Function Name  : L2F50_String
* Description    : Display string on display
* Input			 : start point {x, y}, pointer to text, font,
* 				 : colors {text, back color}
*******************************************************************************/
void L2F50_String(uint8_t x, uint8_t y, char s[], uint8_t font, uint16_t textColor, uint16_t fillColor)
{
	const char mass[] = {8,17,26,26};
	uint16_t k = 0;

	while (*s != '\0')
	{
		uint16_t res = L2F50_Char(x+k, y, *s, font, textColor, fillColor);
		k += res;
		L2F50_Line(x+k-1, y, x+k-1, y+mass[font], fillColor);
		s++;
	}
}


void L2F50_Int(uint8_t x, uint8_t y,
		int val, uint8_t font,
		uint16_t textColor, uint16_t fillColor)
{
	char s[15];
	itoa_(val, s);
	L2F50_String(x, y, s, font, textColor, fillColor);
}

void L2F50_Float(uint8_t x, uint8_t y,
		float val, uint8_t precision, uint8_t font,
		uint16_t textColor, uint16_t fillColor)
{
	char s[35];
	ftoa_(val, s, precision);
	L2F50_String(x, y, s, font, textColor, fillColor);
}


void L2F50_IntWString(uint8_t x, uint8_t y,
		int val, char s[], uint8_t font,
		uint16_t textColor, uint16_t fillColor)
{
	char out[strlen(s)+12];
	out[0] = '\0';
	strcat_(out, s);
	char ss[15];
	itoa_(val, ss);
	strcat_(out, ss);
	L2F50_String(x, y, out, font, textColor, fillColor);
}

void L2F50_DblWString(uint8_t x, uint8_t y,
		uint32_t val, char s[], uint8_t font,
		uint16_t textColor, uint16_t fillColor)
{
	char out[strlen(s)+12];
	out[0] = '\0';
	strcat_(out, s);
	char ss[15];
	dtoa_(val, ss);
	strcat_(out, ss);
	L2F50_String(x, y, out, font, textColor, fillColor);
}

void L2F50_IntWDoubleString(uint8_t x, uint8_t y,
		int val, char front[], char back[], uint8_t font,
		uint16_t textColor, uint16_t fillColor)
{
	char out[strlen(front)+strlen(back)+12];
	out[0] = '\0';
	strcat_(out, front);
	char ss[15];
	itoa_(val, ss);
	strcat_(out, ss);
	strcat_(out, back);
	L2F50_String(x, y, out, font, textColor, fillColor);
}

void L2F50_DblWDoubleString(uint8_t x, uint8_t y,
		uint32_t val, char front[], char back[], uint8_t font,
		uint16_t textColor, uint16_t fillColor)
{
	char out[strlen(front)+strlen(back)+12];
	out[0] = '\0';
	strcat_(out, front);
	char ss[15];
	dtoa_(val, ss);
	strcat_(out, ss);
	strcat_(out, back);
	L2F50_String(x, y, out, font, textColor, fillColor);
}

void L2F50_FloatWString(uint8_t x, uint8_t y,
		float val, uint8_t precision, char s[], uint8_t font,
		uint16_t textColor, uint16_t fillColor)
{
	char out[strlen(s)+18];
	out[0] = '\0';
	strcat_(out, s);
	char ss[18];
	ftoa_(val, ss, precision);
	strcat_(out, ss);
	L2F50_String(x, y, out, font, textColor, fillColor);
}

void L2F50_FloatWDoubleString(uint8_t x, uint8_t y,
		float val, uint8_t precision, char front[], char back[], uint8_t font,
		uint16_t textColor, uint16_t fillColor)
{
	char out[strlen(front)+strlen(back)+18];
	out[0] = '\0';
	strcat_(out, front);
	char ss[18];
	ftoa_(val, ss, precision);
	strcat_(out, ss);
	strcat_(out, back);
	L2F50_String(x, y, out, font, textColor, fillColor);
}
