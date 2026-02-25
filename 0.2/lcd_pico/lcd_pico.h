#ifndef LCD_PICO_H__
#define LCD_PICO_H__

#include "fonts.h"
#include "platform.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define LCD_HEIGHT 240
#define LCD_WIDTH 240

#define LCD_HORIZONTAL 0
#define LCD_VERTICAL 1

void lcd_init(uint8_t scan_dir);
void lcd_fill(uint16_t color);
void lcd_windows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend); 
void lcd_point(uint16_t x, uint16_t y, uint16_t color);
void lcd_point_multi(uint16_t x, uint16_t y, uint16_t color, uint8_t size);
void lcd_rectangle(uint16_t x_s, uint16_t y_s, uint16_t x_e, uint16_t y_e, uint16_t color);
void lcd_circle(uint16_t x_c, uint16_t y_c, uint16_t r, uint16_t color, uint32_t pixel_size, bool fill);
void lcd_char(uint16_t x, uint16_t y, const char c, sFONT *font, uint16_t color_fg, uint16_t color_bg);
void lcd_string(uint16_t x, uint16_t y, sFONT *font, uint16_t color_fg, uint16_t color_bg, const char *format, ...) __attribute__((format(printf, 6, 7)));

#endif // LCD_PICO_H__
