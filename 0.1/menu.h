/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MENU_H
#define MENU_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef void (*FuncPtr)(void);
typedef void (*WriteFuncPtr)(const char*);

typedef struct {
	void       *Next;
	void       *Previous;
	void       *Parent;
	void       *Child;
	FuncPtr     SelectFunc;
	FuncPtr     EnterFunc;
	const char  Text[];
}Menu_Item;


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define M_M(Name,   Next, Previous, Parent, Child,    SelectFunc, EnterFunc, Text) \
    extern Menu_Item Next;     \
	extern Menu_Item Previous; \
	extern Menu_Item Parent;   \
	extern Menu_Item Child;  \
	Menu_Item Name = {(void*)&Next, (void*)&Previous, (void*)&Parent, (void*)&Child, (FuncPtr)SelectFunc, (FuncPtr)EnterFunc, Text}


/* Exported define -----------------------------------------------------------*/
#define NULL_MENU 	Null_Menu
#define NULL_FUNC  (void*)0

#define PREVIOUS   ( CurrMenuItem->Previous )
#define NEXT       ( CurrMenuItem->Next )
#define PARENT     ( CurrMenuItem->Parent )
#define CHILD      ( CurrMenuItem->Child )
#define ENTERFUNC  ( CurrMenuItem->EnterFunc )
#define SELECTFUNC ( CurrMenuItem->SelectFunc )


/* Exported functions ------------------------------------------------------- */
void MenuChange(Menu_Item* NewMenu);
void MenuDraw(void);


#endif //MENU_H
