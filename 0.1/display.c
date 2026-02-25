/* Includes ------------------------------------------------------------------*/
#include "global.h"
#include "l2f50.h"
#include "menu.h"
#include "spi.h"
#include "timework.h"
#include <CoOS.h>
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// ���    ����   �����  ���2   �����2  border
const uint16_t colorScheme[3][6] = {
	{BLACK, GREEN, WHITE, GREEN, BLUE, GREEN},
	{WHITE, RED, BLACK, BLUE, ORANGE, BLUE},
	{BLACK, BLUE, GREEN, BLUE, YELLOW, ORANGE}};

/* Private macro -------------------------------------------------------------*/
#define triangleDraw(pos, color)                             \
	L2F50_Pixel(pos, Yst - 2, color);                        \
	L2F50_Line(pos - 1, Yst - 3, pos + 1, Yst - 3, color);   \
	L2F50_Line(pos - 2, Yst - 4, pos + 2, Yst - 4, color);   \
	L2F50_Line(pos - 3, Yst - 5, pos + 3, Yst - 5, color);   \
	L2F50_Pixel(pos, Yst + 20, color);                       \
	L2F50_Line(pos - 1, Yst + 21, pos + 1, Yst + 21, color); \
	L2F50_Line(pos - 2, Yst + 22, pos + 2, Yst + 22, color); \
	L2F50_Line(pos - 3, Yst + 23, pos + 3, Yst + 23, color);

// Menus:
// Name	//Next		//Prev 		//Parent, 	//Child 	//SelectF 	//EnterF 	//Text
M_M(MMain, MGraph, MDebug, NULL_MENU, S1, NULL_FUNC, NULL_FUNC, "*"); // basic
M_M(MGraph, MInfo, MMain, NULL_MENU, S1, NULL_FUNC, NULL_FUNC, "*");
M_M(MInfo, MDebug, MGraph, NULL_MENU, S1, NULL_FUNC, NULL_FUNC, "*");
M_M(MDebug, MMain, MInfo, NULL_MENU, S1, NULL_FUNC, NULL_FUNC, "*");

M_M(S1, I1, NULL_MENU, MMain, S2a, NULL_FUNC, NULL_FUNC, "Settings"); // 1st
M_M(I1, NULL_MENU, S1, MMain, I2, NULL_FUNC, NULL_FUNC, "Info    ");

M_M(S2a, S2b, NULL_MENU, S1, STD, NULL_FUNC, NULL_FUNC, "TimeDate"); // 2nd
M_M(S2b, NULL_MENU, S2a, S1, NULL_MENU, NULL_FUNC, NULL_FUNC, "Constant");

M_M(STD, NULL_MENU, NULL_MENU, S2a, NULL_MENU, TimeDateChange, NULL_FUNC, "*"); // 3rd
M_M(I2, NULL_MENU, NULL_MENU, MMain, NULL_MENU, NULL_FUNC, NULL_FUNC, "Undo    ");

/* Private variables ---------------------------------------------------------*/
uint8_t scheme = 0;

/* Extern variables ----------------------------------------------------------*/
extern OS_MutexID displayDataMutex;
extern DisplayDataStr Display;

extern Menu_Item *CurrMenuItem;

/* Private function prototypes -----------------------------------------------*/
void MainDisplay(void);
void GraphDisplay(void);
void InfoDisplay(void);

void DumbDebug(void);

uint16_t e = 1;
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  : TimeDateChange
 * Description    :
 *******************************************************************************/
void TimeDateChange(void)
{
#define Yst 55		  // ������ ������ ������� � ����
#define Xst 15		  // ������ ������ ����
#define Xst2 Xst + 92 // ������ ������ �����

	uint8_t n = 0;
	RTC_TimeDateTypeDef TimeDate;

	const uint8_t trianglePos[] = {Xst + 10, Xst + 41, Xst + 70, Xst2 + 5, Xst2 + 21, Xst2 + 43};

	L2F50_FillScreen(BLACK);

	L2F50_String(15, 100, "TIME SETTINGS", FONT5x8, GREEN, BLACK);
	triangleDraw(trianglePos[n], GREEN);

	for(;;)
	{
		/* Translate counter walue to time and date */
		RTC_CntToTimeDate(RTC_GetCounter(), &TimeDate);
		uint16_t TD[6] = {TimeDate.hour, TimeDate.minute, TimeDate.second, TimeDate.date, TimeDate.month, TimeDate.year};
		/*uint32_t datetime = int_to_time(RTC_GetCounter(), &T[0], &T[1], &T[2]);
		int_to_date(datetime, &T[3], &T[4], &T[5]);*/

		switch(KeyboardRead())
		{
		case LEFT:
			triangleDraw(trianglePos[n], BLACK);
			if(n == 0)
				n = 5;
			else
				n--;
			triangleDraw(trianglePos[n], GREEN);
			break;

		case RIGHT:
			triangleDraw(trianglePos[n], BLACK);
			if(++n > 5) n = 0;
			triangleDraw(trianglePos[n], GREEN);
			break;

		case UP:
			TD[n]++;
			TimeDate.hour = TD[0];
			TimeDate.minute = TD[1];
			TimeDate.second = TD[2];
			TimeDate.date = TD[3];
			TimeDate.month = TD[4];
			TimeDate.year = TD[5];
			RTC_SetCounter(RTC_TimeDateToCnt(&TimeDate));
			break;

		case DOWN:
			TD[n]--;
			TimeDate.hour = TD[0];
			TimeDate.minute = TD[1];
			TimeDate.second = TD[2];
			TimeDate.date = TD[3];
			TimeDate.month = TD[4];
			TimeDate.year = TD[5];
			RTC_SetCounter(RTC_TimeDateToCnt(&TimeDate));
			break;

		case OK:
			L2F50_FillScreen(BLACK);
			MenuChange(CurrMenuItem->Parent);
			MenuDraw();
			return;
			break;
		}

		char s[20];
		TimeToString(s, &TimeDate);
		L2F50_String(Xst, Yst, s, FONT10x16, WHITE, BLACK);
		DateToString(s, &TimeDate);
		L2F50_String(Xst2, Yst + 4, s, FONT5x8, WHITE, BLACK);

		CoTimeDelay(0, 0, 0, 50);

		// Time
		/*//L2F50_Put_Num2x(T[0],2,0, Xst,Yst,  WHITE, BLACK);
		L2F50_Put_Char2x(Xst+21,Yst, ':', WHITE, BLACK);
		L2F50_Put_Num2x(T[1],2,0, Xst+31, Yst, WHITE, BLACK);
		L2F50_Put_Char2x(Xst+52,Yst, ':', WHITE, BLACK);
		L2F50_Put_Num2x(T[2],2,0, Xst+61, Yst, WHITE, BLACK);*/

		// Date
		/*//L2F50_Put_Num(T[3], 2,0, Xst2, Yst+4, WHITE, BLACK);
		L2F50_Put_Char58(Xst2+11,Yst+4, '/', RED, BLACK);
		L2F50_Put_Num(T[4], 2,0, Xst2+16, Yst+4, WHITE, BLACK);
		L2F50_Put_Char58(Xst2+27,Yst+4, '/', RED, BLACK);
		L2F50_Put_Num(T[5], 4,0, Xst2+32, Yst+4, WHITE, BLACK);*/
		////
	}
}

/*******************************************************************************
 * Function Name  : TimeDateChange
 * Description    :
 *******************************************************************************/
void DisplayTask(void *pdata)
{
	MenuChange((Menu_Item *)&MDebug);

	/* forever task loop */
	for(;;)
	{
		switch(KeyboardRead())
		{
		case LEFT:
			MenuChange(CurrMenuItem->Parent);
			break;

		case RIGHT:
			MenuChange(CurrMenuItem->Child);
			break;

		case UP:
			MenuChange(CurrMenuItem->Previous);
			break;

		case DOWN:
			MenuChange(CurrMenuItem->Next);
			break;

		case OK:
			// e=1;
			// EmRd16(EM_MODE);
			if(CurrMenuItem->EnterFunc != NULL_FUNC)
				(CurrMenuItem->EnterFunc)();
			break;

		case 0:
			break;
		}

		if(CurrMenuItem == &MMain) MainDisplay();
		if(CurrMenuItem == &MGraph) GraphDisplay();
		if(CurrMenuItem == &MInfo) InfoDisplay();
		if(CurrMenuItem == &MDebug) DumbDebug();

		CoTimeDelay(0, 0, 0, 50);

		// GPIOB->ODR ^= GPIO_Pin_2;
	}
}

/*******************************************************************************
 * Function Name  : MainDisplay
 * Description    :
 *******************************************************************************/
void MainDisplay(void)
{
	CoEnterMutexSection(displayDataMutex);

	GPIOB->BSRR |= GPIO_Pin_2;
	L2F50_RectFill(1, 18, 175, 2, colorScheme[scheme][5], colorScheme[scheme][5]);
	L2F50_RectFill(90, 0, 2, 18, colorScheme[scheme][5], colorScheme[scheme][5]);

	L2F50_String(1, 0, Display.time, FONT10x16, colorScheme[scheme][1], colorScheme[scheme][0]);
	L2F50_String(105, 5, Display.date, FONT5x8, colorScheme[scheme][2], colorScheme[scheme][0]);

	L2F50_FloatWDoubleString(1, 122 - 15, Display.voltage, 1, "V=", "B", FONT5x8, YELLOW, colorScheme[scheme][0]);
	L2F50_FloatWDoubleString(40, 122, Display.current, 3, "I=", "A  ", FONT5x8, RED, colorScheme[scheme][0]);
	L2F50_FloatWDoubleString(100, 121, Display.frequency, 1, "f=", "Hz    ", FONT5x8, BLUE, colorScheme[scheme][0]);
	L2F50_FloatWDoubleString(116, 89, Display.pApp, 1, "AE=", "VA   ", FONT5x8, colorScheme[scheme][2], colorScheme[scheme][0]);
	L2F50_FloatWDoubleString(1, 60, Display.pAct, (Display.pAct / 100 < 100) ? 2 : 0, "P=", "W   ", FONT16x26, colorScheme[scheme][2], colorScheme[scheme][0]);
	L2F50_FloatWDoubleString(1, 40, Display.energy, (Display.energy / 100 < 100) ? 2 : 0, "E=", "kWh   ", FONT10x16, colorScheme[scheme][2], colorScheme[scheme][0]);
	L2F50_FloatWDoubleString(1, 21, Display.costAmount, (Display.costAmount / 100 < 100) ? 2 : 0, "Cost=", "Rub    ", FONT10x16, colorScheme[scheme][2], colorScheme[scheme][0]);
	GPIOB->BRR |= GPIO_Pin_2;

	CoLeaveMutexSection(displayDataMutex);
}

/*******************************************************************************
 * Function Name  : GraphDisplay
 * Description    :
 *******************************************************************************/
void GraphDisplay(void)
{
	L2F50_FillScreen(WHITE);
}

/*******************************************************************************
 * Function Name  : InfoDisplay
 * Description    :
 *******************************************************************************/
void InfoDisplay(void)
{
	L2F50_FillScreen(RED);
}

/*******************************************************************************
 * Function Name  : DumbDebug
 * Description    :
 *******************************************************************************/
void DumbDebug(void)
{
	static uint8_t flag = 0;
	if(!flag)
	{

		CoTickDelay(200);
		flag = 1;
		// EmWr16(EM_MODE, 808);
	}

	/*uint16_t ddd = EmRd16(EM_MODE);
	BitSet(ddd, 5);
	BitReset(ddd, 2);*/
	static uint8_t f = 1;
	L2F50_RectFill(0, 60, 175, 131, BLACK, BLACK);

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

	EmWr16(0x0A, (1 << 3));

	/*
		L2F50_IntWString(1,123-7*9, 0,"=", FONT5x8, GREEN, BLACK);
		L2F50_IntWString(58,123-7*9, 0,"=", FONT5x8, GREEN, BLACK);
		L2F50_IntWString(116,123-7*9, 0,"=", FONT5x8, GREEN, BLACK);

		L2F50_IntWString(1,123-8*9, f++,"=", FONT5x8, GREEN, BLACK);
		L2F50_IntWString(58,123-8*9, 0,"=", FONT5x8, GREEN, BLACK);
		L2F50_IntWString(116,123-8*9, e,"=", FONT5x8, BLUE, BLACK);

		L2F50_IntWDoubleString(1,123-9*9, 0,"=", "     ", FONT5x8, GREEN, BLACK);
		L2F50_IntWDoubleString(58,123-9*9, EmRd16(0x14),"=", "     ", FONT5x8, GREEN, BLACK);
		L2F50_IntWDoubleString(116,123-9*9, EmRd16(0x15),"=", "     ", FONT5x8, GREEN, BLACK);*/

	L2F50_IntWString(1, 123 - 8 * 9, f++, "=", FONT5x8, GREEN, BLACK);

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

	CoTimeDelay(0, 0, 0, 400);
}
