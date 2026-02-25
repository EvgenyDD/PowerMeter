/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_gpio.h"
#include "global.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define Btn_PORT	GPIOB->IDR

#define Btn_LEFT 	GPIO_Pin_9
#define Btn_RIGHT	GPIO_Pin_5
#define Btn_UP		GPIO_Pin_7
#define Btn_DOWN	GPIO_Pin_8
#define Btn_OK		GPIO_Pin_6


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : KeyboardRead
* Description    : Read keyboard
* Output 		 : button code
*******************************************************************************/
char KeyboardRead(void)
{
	static unsigned char isPressed = 0;

//LEFT Button
	if( !(Btn_PORT & Btn_LEFT) && BitIsReset(isPressed, LEFT) )
	{
		BitSet(isPressed, LEFT);
		return LEFT;
	}
	else
		if(Btn_PORT & Btn_LEFT) BitReset(isPressed, LEFT);


//RIGHT Button
	if( !(Btn_PORT & Btn_RIGHT) && BitIsReset(isPressed, RIGHT) )
	{
		BitSet(isPressed, RIGHT);
		return RIGHT;
	}
	else
		if(Btn_PORT & Btn_RIGHT) BitReset(isPressed, RIGHT);

//UP Button
	if( !(Btn_PORT & Btn_UP) && BitIsReset(isPressed, UP) )
	{
		BitSet(isPressed, UP);
		return UP;
	}
	else
		if(Btn_PORT & Btn_UP) BitReset(isPressed, UP);

//DOWN Button
	if( !(Btn_PORT & Btn_DOWN) && BitIsReset(isPressed, DOWN) )
	{
		BitSet(isPressed, DOWN);
		return DOWN;
	}
	else
		if(Btn_PORT & Btn_DOWN) BitReset(isPressed, DOWN);

//OK Button
	if( !(Btn_PORT & Btn_OK) && BitIsReset(isPressed, OK) )
	{
		BitSet(isPressed, OK);
		return OK;
	}
	else
		if(Btn_PORT & Btn_OK) BitReset(isPressed, OK);

	return 0;
}
