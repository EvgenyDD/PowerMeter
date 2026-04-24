#ifndef LCD_PICO_H__
#define LCD_PICO_H__

#include "fonts.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define LCD_HEIGHT 240
#define LCD_WIDTH 240

#define RGB565(r, g, b) ((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))
#define RGB565_DIM(color, percent) (((((color >> 11) & 0x1F) * (percent) / 100) << 11) | \
									((((color >> 5) & 0x3F) * (percent) / 100) << 5) |   \
									(((color) & 0x1F) * (percent) / 100))

// Standard colors using RGB565 macro
#define WHITE RGB565(255, 255, 255) // 0xFFFF
#define BLACK RGB565(0, 0, 0)		// 0x0000
#define RED RGB565(255, 0, 0)		// 0xF800
#define GREEN RGB565(0, 255, 0)		// 0x07E0
#define BLUE RGB565(0, 0, 255)		// 0x001F
#define CYAN RGB565(0, 255, 255)	// 0x7FFF
#define MAGENTA RGB565(255, 0, 255) // 0xF81F
#define YELLOW RGB565(255, 255, 0)	// 0xFFE0

// Extended colors - Reds & Pinks
#define DARK_RED RGB565(128, 0, 0)
#define LIGHT_RED RGB565(255, 128, 128)
#define PINK RGB565(255, 192, 203)
#define HOT_PINK RGB565(255, 105, 180)
#define DEEP_PINK RGB565(255, 20, 147)
#define CORAL RGB565(255, 127, 80)
#define TOMATO RGB565(255, 99, 71)
#define SALMON RGB565(250, 128, 114)
#define INDIAN_RED RGB565(205, 92, 92)

// Extended colors - Oranges
#define ORANGE RGB565(255, 165, 0)
#define DARK_ORANGE RGB565(255, 140, 0)
#define ORANGE_RED RGB565(255, 69, 0)
#define GOLDEN_ROD RGB565(218, 165, 32)
#define PEACH RGB565(255, 218, 185)
#define TANGERINE RGB565(242, 133, 0)
#define CORAL_ORANGE RGB565(248, 131, 61)

// Extended colors - Yellows & Golds
#define GOLD RGB565(255, 215, 0)
#define LIGHT_YELLOW RGB565(255, 255, 224)
#define LEMON_CHIFFON RGB565(255, 250, 205)
#define KHAKI RGB565(240, 230, 140)
#define DARK_KHAKI RGB565(189, 183, 107)

// Extended colors - Greens
#define LIME RGB565(50, 205, 50)
#define LIME_GREEN RGB565(50, 205, 50)
#define DARK_GREEN RGB565(0, 128, 0)
#define FOREST_GREEN RGB565(34, 139, 34)
#define SEA_GREEN RGB565(46, 139, 87)
#define LIGHT_GREEN RGB565(144, 238, 144)
#define PALE_GREEN RGB565(152, 251, 152)
#define MINT_CREAM RGB565(245, 255, 250)
#define OLIVE RGB565(128, 128, 0)
#define DARK_OLIVE_GREEN RGB565(85, 107, 47)
#define TEAL RGB565(0, 128, 128)
#define AQUAMARINE RGB565(127, 255, 212)

// Extended colors - Blues
#define LIGHT_BLUE RGB565(173, 216, 230)
#define SKY_BLUE RGB565(135, 206, 235)
#define DEEP_SKY_BLUE RGB565(0, 191, 255)
#define DODGER_BLUE RGB565(30, 144, 255)
#define NAVY RGB565(0, 0, 128)
#define MIDNIGHT_BLUE RGB565(25, 25, 112)
#define CORNFLOWER_BLUE RGB565(100, 149, 237)
#define ROYAL_BLUE RGB565(65, 105, 225)
#define STEEL_BLUE RGB565(70, 130, 180)
#define POWDER_BLUE RGB565(176, 224, 230)

// Extended colors - Purples & Violets
#define PURPLE RGB565(128, 0, 128)
#define DARK_MAGENTA RGB565(139, 0, 139)
#define INDIGO RGB565(75, 0, 130)
#define VIOLET RGB565(238, 130, 238)
#define PLUM RGB565(221, 160, 221)
#define LAVENDER RGB565(230, 230, 250)
#define THISTLE RGB565(216, 191, 216)
#define ORCHID RGB565(218, 112, 214)

// Extended colors - Browns & Tans
#define BROWN RGB565(165, 42, 42)
#define SADDLE_BROWN RGB565(139, 69, 19)
#define SIENNA RGB565(160, 82, 45)
#define CHOCOLATE RGB565(210, 105, 30)
#define PERU RGB565(205, 133, 63)
#define SANDY_BROWN RGB565(244, 164, 96)
#define TAN RGB565(210, 180, 140)
#define ROSY_BROWN RGB565(188, 143, 143)
#define BEIGE RGB565(245, 245, 220)

// Extended colors - Grays
#define GRAY RGB565(128, 128, 128)
#define DARK_GRAY RGB565(64, 64, 64)
#define DIM_GRAY RGB565(105, 105, 105)
#define LIGHT_GRAY RGB565(211, 211, 211)
#define GAINSBORO RGB565(220, 220, 220)
#define SILVER RGB565(192, 192, 192)
#define SLATE_GRAY RGB565(112, 128, 144)

// Pastels
#define PASTEL_RED RGB565(255, 179, 179)
#define PASTEL_GREEN RGB565(179, 255, 179)
#define PASTEL_BLUE RGB565(179, 179, 255)
#define PASTEL_YELLOW RGB565(255, 255, 179)
#define PASTEL_PINK RGB565(255, 179, 204)
#define PASTEL_PURPLE RGB565(204, 179, 255)

// Neons
#define NEON_GREEN RGB565(57, 255, 20)
#define NEON_BLUE RGB565(0, 255, 255)
#define NEON_PINK RGB565(255, 20, 147)
#define NEON_YELLOW RGB565(255, 255, 0)
#define NEON_ORANGE RGB565(255, 95, 0)

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

void lcd_char(uint16_t x, uint16_t y, const char c, const uint8_t *font, uint32_t font_sz, uint16_t color_fg, uint16_t color_bg);
void lcd_string(uint16_t x, uint16_t y, const uint8_t *font, uint32_t font_sz, uint16_t color_fg, uint16_t color_bg, const char *format, ...) __attribute__((format(printf, 7, 8)));

void lcd_dvd(uint16_t color_fg, uint16_t color_bg);

uint16_t lcd_xs(void);
uint16_t lcd_xe(void);
uint16_t lcd_ys(void);
uint16_t lcd_ye(void);

uint16_t lcd_w(void);
uint16_t lcd_h(void);

#endif // LCD_PICO_H__
