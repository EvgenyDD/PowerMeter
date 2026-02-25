#ifndef FONTS_H__
#define FONTS_H__

#include <stdint.h>

#define MAX_HEIGHT_FONT 41
#define MAX_WIDTH_FONT 32
#define OFFSET_BITMAP

typedef struct
{
	const uint8_t *table;
	uint16_t Width;
	uint16_t Height;
} sFONT_t;

extern sFONT_t Font8;
extern sFONT_t Font12;
extern sFONT_t Font16;
extern sFONT_t Font20;
extern sFONT_t Font24;

#endif // FONTS_H__
