#ifndef FONTS_H__
#define FONTS_H__

#include <stdint.h>

#define MAX_HEIGHT_FONT 41
#define MAX_WIDTH_FONT 32
#define OFFSET_BITMAP

typedef struct _tFont
{
	const uint8_t *table;
	uint16_t Width;
	uint16_t Height;
} sFONT;

extern sFONT Font24;
extern sFONT Font20;
extern sFONT Font16;
extern sFONT Font12;
extern sFONT Font8;

#endif // FONTS_H__
