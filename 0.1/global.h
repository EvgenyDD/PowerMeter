/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef GLOBAL_H
#define GLOBAL_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_rtc.h"


/* Exported types ------------------------------------------------------------*/
typedef struct
{
	//time
	char date[11];
	char time[10];

	//measure1
	float 	voltage;
	float 	current;
	float	frequency;

	//measure calculations
	float 	pf;
	float 	pApp;
	float 	pAct;
	float 	energy;

	//cost
	float 	costAmount;
}DisplayDataStr;


/* Exported constants --------------------------------------------------------*/
#define LEFT	1
#define RIGHT	2
#define UP		3
#define DOWN	4
#define OK		5


/* Exported macro ------------------------------------------------------------*/
#define BitSet(p,m) ((p) |= (1<<(m)))
#define BitReset(p,m) ((p) &= ~(1<<(m)))
#define BitFlip(p,m) ((p) ^= (m))
#define BitWrite(c,p,m) ((c) ? BitSet(p,m) : BitReset(p,m))
#define BitIsSet(reg, bit) (((reg) & (1<<(bit))) != 0)
#define BitIsReset(reg, bit) (((reg) & (1<<(bit))) == 0)


/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void _delay_loops(uint32_t loops);

void PeriphInit(void);
char KeyboardRead(void);
void DisplayTask (void* pdata);
void TimeDateChange(void);

void FUCk(uint8_t f, ...);

#endif //GLOBAL_H

/*#define delay_us( US ) _delay_loops( (uint32_t)((double)US * 64000000 / 3000000.0) )
#define delay_ms( MS ) _delay_loops( (uint32_t)((double)MS * 64000000 / 3000.0) )
#define delay_s( S )   _delay_loops( (uint32_t)((double)S  * 64000000 / 3.0) )*/
