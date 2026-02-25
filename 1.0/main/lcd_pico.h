#ifndef LCD_PICO_H__
#define LCD_PICO_H__

#include "fonts.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define LCD_HEIGHT 240
#define LCD_WIDTH 240

#define MADCTL_MY 0x80
#define MADCTL_MX 0x40
#define MADCTL_MV 0x20

#define VIEW_12H (MADCTL_MY | MADCTL_MV)
#define VIEW_3H 0
#define VIEW_6H (MADCTL_MX | MADCTL_MV)
#define VIEW_9H (MADCTL_MY | MADCTL_MX)

void lcd_init(uint8_t view);
void lcd_reinit(uint8_t view);

void lcd_set_bl(uint8_t val);
void lcd_fill(uint16_t color);
void lcd_fill_bounds(uint16_t xs, uint16_t ys, uint16_t x_len, uint16_t y_len, uint16_t color);
void lcd_windows(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);
void lcd_windows_raw(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);
void lcd_pixel(uint16_t x, uint16_t y, uint16_t color);
void lcd_line_h(int16_t x, int16_t y, int16_t l, uint16_t color);
void lcd_line_v(int16_t x, int16_t y, int16_t l, uint16_t color);
void lcd_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool fill);
void lcd_rectangle(uint16_t x_s, uint16_t y_s, uint16_t x_e, uint16_t y_e, uint16_t color);
void lcd_circle(uint16_t x_c, uint16_t y_c, uint16_t r, uint16_t color, uint32_t pixel_size, bool fill);
void lcd_char(uint16_t x, uint16_t y, const char c, sFONT_t *font, uint16_t color_fg, uint16_t color_bg);
void lcd_string(uint16_t x, uint16_t y, sFONT_t *font, uint16_t color_fg, uint16_t color_bg, const char *format, ...) __attribute__((format(printf, 6, 7)));

uint16_t lcd_xs(void);
uint16_t lcd_xe(void);
uint16_t lcd_ys(void);
uint16_t lcd_ye(void);

uint16_t lcd_w(void);
uint16_t lcd_h(void);

extern sFONT_t Font3x5;
extern sFONT_t Font5x8;
// extern sFONT_t Font16x24;
// extern sFONT_t Font8x12;
extern sFONT_t Font16x16;
// extern sFONT_t Font8x16;
extern sFONT_t FontSTD_swiss721_outline;

#endif // LCD_PICO_H__
