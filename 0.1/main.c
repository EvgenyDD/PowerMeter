#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_rtc.h>
#include <stm32f10x_pwr.h>
#include <CoOS.h>
#include "misc.h"
#include "time.h"

#include "l2f50.h"
#include "spi.h"
#include "timework.h"
#include "global.h"
//#include "menu.h"
#include "string.h"
#include "printf_.h"




void DumbDebug(void);

/*---------------------------- Variable Define -------------------------------*/
OS_STK     taskA_stk[128];	  /*!< Define "taskA" task stack */
OS_STK     taskB_stk[128];	  /*!< Define "taskB" task stack */
OS_STK     taskC_stk[128];	  /*!< Define "led" task stack   */
OS_STK     DisplayTask_stk[128];	  /*!< Define "led" task stack   */


RTC_TimeDateTypeDef TimeDate;
DisplayDataStr	Display;

//OS_MutexID L2F50Mutex;
OS_MutexID displayDataMutex;


void MainDisplay(void);





void taskA (void* pdata)
{

	for (;;)
	{
		CoTickDelay (1);

		CoTimeDelay(0,0,0,200);
	}
}




void taskB (void* pdata)
{

	for(;;)
	{
		//CoTickDelay(1);
		//GPIOB->ODR ^= GPIO_Pin_2;

#ifdef GRAPHmode
		static uint8_t iks = 1;
		iks++;
		if(iks>= 170)
		{
			iks = 1;
			CoSchedLock();
			L2F50_Draw_Rect_Fill(0,0,170,90,BLACK,BLACK);
			L2F50_Draw_Rect(0,0,170,90,WHITE);
			L2F50_Draw_Line(175,20*2,165,20*2,CYAN);
			L2F50_Draw_Line(175,25*2,170,25*2,CYAN);
			L2F50_Draw_Line(175,30*2,165,30*2,CYAN);
			uint8_t x=0, y=0;

			for(x=10;x<170; x+=10)
				for(y=10;y<90;y+=10)
				{
					L2F50_Put_Pixel(x,y,WHITE);
				}
			CoSchedUnlock();
		}
		L2F50_Put_Pixel(iks,(TX-200)*2,GREEN);
#endif

static uint8_t laststate= 0;

		if(GPIOB->IDR & GPIO_Pin_11)
		{
			if(laststate)
				laststate = 0;
		}
		else
		{
			if(!laststate)
			{
				laststate = 1;
				GPIOB->ODR ^= GPIO_Pin_2;
			}
		}
	}
}



void DataTask(void* pdata)
{
	for(;;)
	{
		static uint32_t oldTimeCnt;

		/* Wait until new second appear */
		while(oldTimeCnt == RTC_GetCounter());
		oldTimeCnt = RTC_GetCounter();

		/* Translate counter value to time and date */
		RTC_CntToTimeDate(RTC_GetCounter(), &TimeDate);

		CoEnterMutexSection(displayDataMutex);
		{
			TimeToString(Display.time, &TimeDate);
			DateToString(Display.date, &TimeDate);
		}
		CoLeaveMutexSection(displayDataMutex);
	}
}



int main(void)
{
	PeriphInit();
	L2F50_InitPeripheral();
	L2F50_Init();

	GPIO_SetBits(GPIOA, GPIO_Pin_11);
	L2F50_FillScreen(BLACK);

	CoInitOS();

	uint16_t ddd = EmRd16(EM_MODE);
			BitReset(ddd, 2);
			BitSet(ddd, 14);
			BitSet(ddd, 13);
			EmWr16(EM_MODE, ddd);

	//init_printf(NULL,L2F50_Char);
	//printf("FUCK", fd);

//	L2F50Mutex = CoCreateMutex();
	displayDataMutex = CoCreateMutex();

	//!< Create three tasks
	 CoCreateTask (taskA,0,0,&taskA_stk[128-1],128);
	 CoCreateTask (taskB,0,0,&taskB_stk[128-1],128);
	 CoCreateTask (DataTask,0,0,&taskC_stk[128-1],128);
	 CoCreateTask (DisplayTask,0,0,&DisplayTask_stk[128-1],128);
	//  CoCreateTask (taskC,0,2,&taskC_stk[128-1],128);
	CoStartOS();
	GPIO_SetBits(GPIOA, GPIO_Pin_11);
    //GPIOB->ODR ^= GPIO_Pin_2;
	for(;;);

}


void NMI_Handler(void){}
