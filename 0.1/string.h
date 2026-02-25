/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STRING_H
#define STRING_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int strlen(char *);
void itoa_(int, char s[]);
void dtoa_(uint32_t n, char s[]);
void ftoa_(float, char str[], char precision);
void reverse(char s[]);
void strcat_(char first[], char second[]);
float log10_(int v);


#endif //STRING_H
