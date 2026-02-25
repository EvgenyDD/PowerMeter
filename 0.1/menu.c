/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "Menu.h"
#include "l2f50.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SPACING 12		//расстояние между строками меню
#define MAXMENU 120		//начало отрисовки меню сверху
#define NROWS	7		//количество пунктов меню на уровень
#define XSTART	5


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
Menu_Item 	Null_Menu = {&NULL_MENU, &NULL_MENU, &NULL_MENU, &NULL_MENU, NULL_FUNC, NULL_FUNC, "*"};
Menu_Item*	CurrMenuItem;


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MenuChange
* Description    : pointer to new menu
*******************************************************************************/
void MenuChange(Menu_Item* NewMenu)
{
	if(NewMenu == &NULL_MENU) return;
	L2F50_FillScreen(BLACK);

	CurrMenuItem = NewMenu;

	if(CurrMenuItem->SelectFunc != NULL_FUNC)
		(CurrMenuItem->SelectFunc)();

	if(CurrMenuItem->Parent != &NULL_MENU)
		MenuDraw();
}


/*******************************************************************************
* Function Name  : MenuDraw
* Description    : drawing menu on display
*******************************************************************************/
void MenuDraw(void)
{
	Menu_Item*	SeekMenuItem = CurrMenuItem;
	uint8_t height = 0;

	/* find the head */
	while( SeekMenuItem->Previous != &NULL_MENU)
		SeekMenuItem = SeekMenuItem->Previous;

	/* draw menu positions */
#define SEL_B	BLUE
#define SEL_F	WHITE
#define DESEL_B	WHITE
#define DESEL_F	BLACK
#define EMPTY_F	DarkCyan

	L2F50_String(XSTART, MAXMENU-SPACING*height++, (char*)SeekMenuItem->Text, FONT5x8,
			(SeekMenuItem!=CurrMenuItem)?SEL_F:SEL_B, (SeekMenuItem!=CurrMenuItem)?DESEL_F:DESEL_B);

	while(SeekMenuItem->Next != &NULL_MENU)
	{
		SeekMenuItem = SeekMenuItem->Next;

		L2F50_String(XSTART, MAXMENU-SPACING*height++, (char*)SeekMenuItem->Text, FONT5x8,
			(SeekMenuItem!=CurrMenuItem)?SEL_F:SEL_B, (SeekMenuItem!=CurrMenuItem)?DESEL_F:DESEL_B);
	}

	/* Fill empty fields */
	while(height < NROWS)
		L2F50_String(XSTART, MAXMENU-SPACING*height++, "+", FONT5x8, GREEN, EMPTY_F);
}
