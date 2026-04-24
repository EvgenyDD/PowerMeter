#include "fonts.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

uint16_t fill_buf[10240];
char buf_print[2048];
uint16_t *fb = NULL;
uint16_t xs = 0, ys = 0;

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

static void lcd_char(uint16_t x, uint16_t y, const char c, const uint8_t *font, uint32_t font_sz, uint16_t color_fg, uint16_t color_bg)
{
	uint32_t num_ch = 0;
	if(font[3] == 0 || font[3] == 1) num_ch = (font_sz - 4) / font[0] / (font[1] / 8 + (font[1] % 8 ? 1 : 0));
	if(font[3] == 2 || font[3] == 3) num_ch = (font_sz - 4) / font[1] / (font[0] / 8 + (font[0] % 8 ? 1 : 0));

	if(font[3] == 2)
	{
		if(x + font[0] >= xs || y + font[1] >= ys) return;
		if((uint32_t)c >= font[2] + num_ch) return;
		if(c < font[2]) return;
		uint32_t ch_offset = 4 + (c - font[2]) * font[1] * (font[0] / 8 + (font[0] % 8 ? 1 : 0));

		for(uint16_t row = 0; row < font[1]; row++)
		{
			for(uint16_t col = 0; col < font[0]; col++)
			{
				fill_buf[row * font[0] + col] = (font[ch_offset] & (0x80 >> (col % 8)))
													? color_fg
													: color_bg;
				if(col % 8 == 7) ch_offset++;
			}
			if(font[0] % 8) ch_offset++;
		}

		for(uint16_t _y = 0; _y < font[1]; _y++)
		{
			for(uint16_t _x = 0; _x < font[0]; _x++)
			{
				fb[(y + _y) * xs + _x + x] = fill_buf[_y * font[0] + _x];
			}
		}
	}
	else
	{
		if(x + font[0] >= xs || y + font[1] >= ys) return;

		if((uint32_t)c >= font[2] + num_ch) return;
		if(c < font[2]) return;
		uint32_t ch_offset = 4 + (uint32_t)((c - font[2]) * ((uint32_t)((font[0]) < 8
																			? font[0] * ((font[1] + 7) / 8)
																			: (font[0] * font[1] + 7) / 8)));

		if(font[3] == 3)
		{
			for(uint16_t row = 0; row < font[1]; row++)
			{
				for(uint8_t k = 0; k < font[0] / 8; k++)
				{
					uint8_t frame = font[ch_offset++];
					for(uint8_t i = 0; i < 8; i++)
					{
						fill_buf[font[0] * row + k * 8 + 7 - i] = (frame & (1 << i))
																	  ? color_fg
																	  : color_bg;
					}
				}
			}
		}
		else
		{
			for(int8_t row = font[1] - 1; row >= 0; row--)
			{
				for(uint16_t col = 0; col < font[0]; col++)
				{
					int a = font[3] == 1 ? row : font[1] - 1 - row;
					fill_buf[font[0] * a + col] = (font[ch_offset + col] & (1 << row))
													  ? color_fg
													  : color_bg;
				}
			}
		}

		for(uint16_t _y = 0; _y < font[1]; _y++)
		{
			for(uint16_t _x = 0; _x < font[0]; _x++)
			{
				fb[(y + _y) * xs + _x + x] = fill_buf[_y * font[0] + _x];
			}
		}
	}
}

static void lcd_string(uint16_t x, uint16_t y, const uint8_t *font, uint32_t font_sz, uint16_t color_fg, uint16_t color_bg, const char *format, ...) __attribute__((format(gnu_printf, 7, 8)));
static void lcd_string(uint16_t x, uint16_t y, const uint8_t *font, uint32_t font_sz, uint16_t color_fg, uint16_t color_bg, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(buf_print, sizeof(buf_print), format, args);
	va_end(args);

	for(char *it = buf_print; *it; it++)
	{
		lcd_char(x, y, *it, font, font_sz, color_fg, color_bg);
		x += font[0] + 1;
	}
}

static void lcd_fill(uint16_t color)
{
	for(uint32_t i = 0; i < xs * ys; i++)
		fb[i] = color;
}

static void lcd_fill_bounds(uint16_t x, uint16_t y, uint16_t x_len, uint16_t y_len, uint16_t color)
{
	for(uint16_t _y = 0; _y < y_len; _y++)
	{
		for(uint16_t _x = 0; _x < x_len; _x++)
		{
			fb[(y + _y) * xs + _x + x] = color;
		}
	}
}

static void lcd_pixel(uint16_t x, uint16_t y, uint16_t color)
{
	if(x >= xs || y >= ys) return;
	fb[y * xs + x] = color;
}

static void lcd_pixel_multi(uint16_t x, uint16_t y, uint16_t color, uint8_t size)
{
	for(int16_t XDir_Num = 0; XDir_Num < 2 * size - 1; XDir_Num++)
	{
		for(int16_t YDir_Num = 0; YDir_Num < 2 * size - 1; YDir_Num++)
		{
			if(x + XDir_Num - size < 0 || y + YDir_Num - size < 0) break;
			lcd_pixel(x + XDir_Num - size, y + YDir_Num - size, color);
		}
	}
}

static void lcd_line_h(int16_t x, int16_t y, int16_t l, uint16_t color)
{
	for(uint16_t _x = 0; _x < l; _x++)
		lcd_pixel(x + _x, y, color);
}

static void lcd_line_v(int16_t x, int16_t y, int16_t l, uint16_t color)
{
	for(uint16_t _y = 0; _y < l; _y++)
		lcd_pixel(x, y + _y, color);
}

static void lcd_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool fill)
{
	if(fill)
	{
		lcd_fill_bounds(x, y, w, h, color);
	}
	else
	{
		lcd_line_h(x, y, w, color);
		lcd_line_h(x, y + h - 1, w, color);
		lcd_line_v(x, y, h, color);
		lcd_line_v(x + w - 1, y, h, color);
	}
}

static void lcd_circle(uint16_t x_c, uint16_t y_c, uint16_t r, uint16_t color, uint32_t pixel_size, bool fill)
{
	int16_t x = 0, y = r;		// draw a circle from(0, R) as a starting point
	int16_t esp = 3 - (r << 1); // cumulative error

	if(fill)
	{
		while(x <= y)
		{
			for(int16_t sCountY = x; sCountY <= y; sCountY++)
			{
				lcd_pixel_multi(x_c + x, y_c + sCountY, color, 1); // 1
				lcd_pixel_multi(x_c - x, y_c + sCountY, color, 1); // 2
				lcd_pixel_multi(x_c - sCountY, y_c + x, color, 1); // 3
				lcd_pixel_multi(x_c - sCountY, y_c - x, color, 1); // 4
				lcd_pixel_multi(x_c - x, y_c - sCountY, color, 1); // 5
				lcd_pixel_multi(x_c + x, y_c - sCountY, color, 1); // 6
				lcd_pixel_multi(x_c + sCountY, y_c - x, color, 1); // 7
				lcd_pixel_multi(x_c + sCountY, y_c + x, color, 1);
			}
			if(esp < 0)
			{
				esp += 4 * x + 6;
			}
			else
			{
				esp += 10 + 4 * (x - y);
				y--;
			}
			x++;
		}
	}
	else // hollow circle
	{
		while(x <= y)
		{
			lcd_pixel_multi(x_c + x, y_c + y, color, pixel_size); // 1
			lcd_pixel_multi(x_c - x, y_c + y, color, pixel_size); // 2
			lcd_pixel_multi(x_c - y, y_c + x, color, pixel_size); // 3
			lcd_pixel_multi(x_c - y, y_c - x, color, pixel_size); // 4
			lcd_pixel_multi(x_c - x, y_c - y, color, pixel_size); // 5
			lcd_pixel_multi(x_c + x, y_c - y, color, pixel_size); // 6
			lcd_pixel_multi(x_c + y, y_c - x, color, pixel_size); // 7
			lcd_pixel_multi(x_c + y, y_c + x, color, pixel_size); // 0

			if(esp < 0)
			{
				esp += 4 * x + 6;
			}
			else
			{
				esp += 10 + 4 * (x - y);
				y--;
			}
			x++;
		}
	}
}

void make_ui_array(uint16_t *array, uint16_t lcd_w, uint16_t lcd_h, uint32_t diff_ms)
{
	fb = array;
	xs = lcd_w;
	ys = lcd_h;
	static uint8_t iii = 0;

	if(0)
	{
		lcd_fill(BLACK);
		lcd_fill_bounds(20, 80, 200, 80, RED);
		lcd_string(48, 96, F(font24x48_grotesk_bold), WHITE, RED, "ONLINE");
		return;
	}

	lcd_fill(BLACK);
	lcd_rect(0, 0, xs, ys, GREEN, false);

	float v1 = 88888.888888;
	float v0 = 1.888888;

#define OFF 100
#define DIM1 90
#define DIM2 80

	lcd_string(2, 2 + 15 * 0, F(font8x16_retro), ROYAL_BLUE, BLACK, "Eapp");									   // 0
	lcd_string(2, 2 + 15 * 1, F(font8x16_retro), CORAL, BLACK, "Eact");											   // 1
	lcd_string(2, 2 + 15 * 2, F(font8x16_retro), LIME, BLACK, "Erea");											   // 2
	lcd_string(2, 2 + 15 * 3, F(font8x16_retro), ORANGE, BLACK, "PWR");											   // 3
	lcd_string(2, 2 + 15 * 4, F(font8x16_retro), GREEN, BLACK, "U");											   // 4
	lcd_string(2, 2 + 15 * 5, F(font8x16_retro), RED, BLACK, "I ");												   // 5
	lcd_string(2, 2 + 15 * 6, F(font8x8_sinclair_s), WHITE, BLACK, "Freq  %.0f        t% d C", v0 * 50, (int)105); // 6
	lcd_string(2 + 72, 2 + 15 * 6, F(font5x8), RGB565_DIM(WHITE, DIM2), BLACK, "%3d", (int)(v0 * 50 * 1000) % 1000);

	char buf[32];
	int int_part, f02, f35, pl = 0;
	uint16_t clr;

#define M(val)                               \
	snprintf(buf, sizeof(buf), "%.6f", val); \
	sscanf(buf, "%d.%3d%3d", &int_part, &f02, &f35);

	// #0
	M(v0);
	clr = ROYAL_BLUE;
	lcd_string(2 + 36, 2 + 15 * pl, F(font8x12_small), clr, BLACK, "% 6d", int_part);
	lcd_string(2 + 88, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM1), BLACK, "%3d", f02);
	lcd_string(2 + 110, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM2), BLACK, "%3d", f35);

	M(v1);
	lcd_string(2 + 36 + OFF, 2 + 15 * pl, F(font8x12_small), clr, BLACK, "% 6d", int_part);
	lcd_string(2 + 88 + OFF, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM1), BLACK, "%3d", f02);
	lcd_string(2 + 110 + OFF, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM2), BLACK, "%3d", f35);

	// #1
	pl++;
	M(v1);
	clr = CORAL;
	lcd_string(2 + 36, 2 + 15 * pl, F(font8x12_small), clr, BLACK, "% 6d", int_part);
	lcd_string(2 + 88, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM1), BLACK, "%3d", f02);
	lcd_string(2 + 110, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM2), BLACK, "%3d", f35);

	M(v1);
	lcd_string(2 + 36 + OFF, 2 + 15 * pl, F(font8x12_small), clr, BLACK, "% 6d", int_part);
	lcd_string(2 + 88 + OFF, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM1), BLACK, "%3d", f02);
	lcd_string(2 + 110 + OFF, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM2), BLACK, "%3d", f35);

	// #2
	pl++;
	M(v1);
	clr = LIME;
	lcd_string(2 + 36, 2 + 15 * pl, F(font8x12_small), clr, BLACK, "% 6d", int_part);
	lcd_string(2 + 88, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM1), BLACK, "%3d", f02);
	lcd_string(2 + 110, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM2), BLACK, "%3d", f35);

	// #3
	pl++;
	M(v1);
	clr = ORANGE;
	lcd_string(2 + 36, 2 + 15 * pl, F(font8x12_small), clr, BLACK, "% 6d", int_part);
	lcd_string(2 + 88, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM1), BLACK, "%3d", f02);

	M(v1);
	lcd_string(2 + 36 + OFF, 2 + 15 * pl, F(font8x12_small), clr, BLACK, "% 6d", int_part);
	lcd_string(2 + 88 + OFF, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM1), BLACK, "%3d", f02);

	// #4
	pl++;
	M(v1);
	clr = GREEN;
	lcd_string(2 + 36, 2 + 15 * pl, F(font8x12_small), clr, BLACK, "% 6d", int_part);
	lcd_string(2 + 88, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM1), BLACK, "%3d", f02);

	M(v1);
	lcd_string(2 + 36 + OFF, 2 + 15 * pl, F(font8x12_small), clr, BLACK, "% 6d", int_part);
	lcd_string(2 + 88 + OFF, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM1), BLACK, "%3d", f02);

	// #5
	pl++;
	M(v1);
	clr = RED;
	lcd_string(2 + 36, 2 + 15 * pl, F(font8x12_small), clr, BLACK, "% 6d", int_part);
	lcd_string(2 + 88, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM1), BLACK, "%3d", f02);
	lcd_string(2 + 110, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM2), BLACK, "%3d", f35);

	M(v1);
	lcd_string(2 + 36 + OFF, 2 + 15 * pl, F(font8x12_small), clr, BLACK, "% 6d", int_part);
	lcd_string(2 + 88 + OFF, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM1), BLACK, "%3d", f02);
	lcd_string(2 + 110 + OFF, 2 + 15 * pl + 2, F(font5x8), RGB565_DIM(clr, DIM2), BLACK, "%3d", f35);

#define XS 3
#define YS 104
#define H (xs - 3 - YS)
#define W (ys - XS * 2)
	lcd_fill_bounds(XS, YS, W, H, GRAY);
}