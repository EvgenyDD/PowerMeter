/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef L2F50_H
#define L2F50_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DISP_W 176
#define DISP_H 132

enum {FONT5x8, FONT10x16, FONT15x24, FONT16x26} FONT_TYPES;

//*  Colors  *//
#define BLACK 0x0000
#define WHITE 0xFFFF

#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0

#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define ORANGE 0xFD20

#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Purple          0x780F      /* 128,   0, 128 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define GreenYellow     0xAFE5      /* 173, 255,  47 */
#define Pink            0xF81F

/*  Display Registers  */
#define DATCTL 	0xBC 	// Data Control (data handling in RAM)
#define DISCTL 	0xCA 	// Display Control

#define GCP64 	0xCB 	// pulse set for 64 gray scale
#define GCP16 	0xCC 	// pulse set for 16 gray scale
#define GSSET 	0xCD 	// set for gray scales

#define OSSEL 	0xD0 	// Oscillator select
#define ASCSET 	0xAA 	// area scroll setting
#define SCSTART 0xAB 	// scroll start setting

#define DISON 	0xAF 	// Display ON (no parameter)
#define DISOFF 	0xAE 	// Display OFF (no parameter)
#define DISINV 	0xA7 	// Display Invert (no parameter)
#define DISNOR 	0xA6 	// Display Normal (no parameter)

#define SLPIN 	0x95 	// Display Sleep (no parameter)
#define SLPOUT 	0x94 	// Display out of sleep (no parameter)
#define RAMWR 	0x5C 	// Display Memory write
#define PTLIN 	0xA8 	// partial screen write

#define SD_CSET 0x15 	// column address setting
#define SD_PSET 0x75 	// page address setting

/* Exported macro ------------------------------------------------------------*/
//#define Delay _delay_loops(1)

#define LCD_CS_1 	GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define LCD_CS_0 	GPIO_ResetBits(GPIOB, GPIO_Pin_12)

#define LCD_RS_1 	GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define LCD_RS_0 	GPIO_ResetBits(GPIOA, GPIO_Pin_8)

#define LCD_RST_1 	GPIO_SetBits(GPIOB, GPIO_Pin_14)
#define LCD_RST_0 	GPIO_ResetBits(GPIOB, GPIO_Pin_14)

#define LCD_MOSI1 	GPIO_SetBits(GPIOB, GPIO_Pin_15)
#define LCD_MOSI0 	GPIO_ResetBits(GPIOB, GPIO_Pin_15)
#define LCD_SCK1 	GPIO_SetBits(GPIOB, GPIO_Pin_13)
#define LCD_SCK0 	GPIO_ResetBits(GPIOB, GPIO_Pin_13)


/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void L2F50_InitPeripheral();
void L2F50_Init();

void L2F50_Cmd(uint8_t);
void L2F50_Dat8(uint8_t);
void L2F50_Dat16(uint16_t);

void L2F50_SetWindow (uint16_t, uint16_t, uint16_t, uint16_t);

void L2F50_FillScreen(uint16_t);
void L2F50_Pixel(uint16_t, uint16_t, int);

//Text
uint8_t L2F50_Char(uint8_t, uint8_t, char, uint8_t, uint16_t, uint16_t);
void L2F50_String(uint8_t, uint8_t, char*, uint8_t, uint16_t, uint16_t);

void L2F50_Int(uint8_t, uint8_t, int, uint8_t, uint16_t, uint16_t);
void L2F50_Float(uint8_t, uint8_t, float, uint8_t, uint8_t, uint16_t, uint16_t);
void L2F50_IntWString(uint8_t, uint8_t, int, char*, uint8_t, uint16_t, uint16_t);
void L2F50_IntWDoubleString(uint8_t, uint8_t, int, char*, char*, uint8_t, uint16_t, uint16_t);
void L2F50_DblWString(uint8_t, uint8_t, uint32_t, char*, uint8_t, uint16_t, uint16_t);
void L2F50_DblWDoubleString(uint8_t, uint8_t, uint32_t, char*, char*, uint8_t, uint16_t, uint16_t);
void L2F50_FloatWString(uint8_t, uint8_t, float, uint8_t, char*, uint8_t, uint16_t, uint16_t);
void L2F50_FloatWDoubleString(uint8_t, uint8_t, float, uint8_t, char*, char*, uint8_t, uint16_t, uint16_t);

//Graphics
void L2F50_Line(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
void L2F50_Rect(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
void L2F50_RectFill(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
void L2F50_Circle(uint16_t, uint16_t, uint16_t, uint16_t);
void L2F50_CircleFill(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);

#endif //L2F50_H
