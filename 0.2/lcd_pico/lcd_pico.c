#include "lcd_pico.h"
#include "fonts.h"
#include <stdarg.h>

// ST7789 240x240

extern SPI_HandleTypeDef hspi2;

#define FILL_BUF_SZ /*LCD_WIDTH*/ 512
static uint16_t fill_buf[FILL_BUF_SZ];

static uint8_t buf_print[256];

static void lcd_cmd(uint8_t Reg)
{
	PIN_CLR(D_DC);
	PIN_CLR(D_CS);
	HAL_SPI_Transmit(&hspi2, &Reg, 1, 200);
	PIN_SET(D_CS);
}

static void lcd_data8(uint8_t Data)
{
	PIN_SET(D_DC);
	PIN_CLR(D_CS);
	HAL_SPI_Transmit(&hspi2, &Data, 1, 200);
	PIN_SET(D_CS);
}

static void lcd_data16(uint16_t Data)
{
	PIN_SET(D_DC);
	PIN_CLR(D_CS);
	uint8_t d[] = {(Data >> 8) & 0xFF, Data & 0xFF};
	HAL_SPI_Transmit(&hspi2, d, 2, 200);
	PIN_SET(D_CS);
}

void lcd_init(uint8_t scan_dir)
{
	PIN_SET(D_BL);

	PIN_SET(D_RST);
	HAL_Delay(100);
	PIN_CLR(D_RST);
	HAL_Delay(100);
	PIN_SET(D_RST);
	HAL_Delay(100);

	lcd_cmd(0x36);									  // MX, MY, RGB mode
	lcd_data8(scan_dir == LCD_HORIZONTAL ? 0x70 : 0); // 0x08 set RGB

	lcd_cmd(0x3A);
	lcd_data8(0x05);

	lcd_cmd(0xB2);
	lcd_data8(0x0C);
	lcd_data8(0x0C);
	lcd_data8(0x00);
	lcd_data8(0x33);
	lcd_data8(0x33);

	lcd_cmd(0xB7); // Gate Control
	lcd_data8(0x35);

	lcd_cmd(0xBB); // VCOM Setting
	lcd_data8(0x19);

	lcd_cmd(0xC0); // LCM Control
	lcd_data8(0x2C);

	lcd_cmd(0xC2); // VDV and VRH Command Enable
	lcd_data8(0x01);
	lcd_cmd(0xC3); // VRH Set
	lcd_data8(0x12);
	lcd_cmd(0xC4); // VDV Set
	lcd_data8(0x20);

	lcd_cmd(0xC6); // Frame Rate Control in Normal Mode
	lcd_data8(0x03);

	lcd_cmd(0xD0); // Power Control 1
	lcd_data8(0xA4);
	lcd_data8(0xA1);

	lcd_cmd(0xE0); // Positive Voltage Gamma Control
	lcd_data8(0xD0);
	lcd_data8(0x04);
	lcd_data8(0x0D);
	lcd_data8(0x11);
	lcd_data8(0x13);
	lcd_data8(0x2B);
	lcd_data8(0x3F);
	lcd_data8(0x54);
	lcd_data8(0x4C);
	lcd_data8(0x18);
	lcd_data8(0x0D);
	lcd_data8(0x0B);
	lcd_data8(0x1F);
	lcd_data8(0x23);

	lcd_cmd(0xE1); // Negative Voltage Gamma Control
	lcd_data8(0xD0);
	lcd_data8(0x04);
	lcd_data8(0x0C);
	lcd_data8(0x11);
	lcd_data8(0x13);
	lcd_data8(0x2C);
	lcd_data8(0x3F);
	lcd_data8(0x44);
	lcd_data8(0x51);
	lcd_data8(0x2F);
	lcd_data8(0x1F);
	lcd_data8(0x1F);
	lcd_data8(0x20);
	lcd_data8(0x23);

	lcd_cmd(0x21); // Display Inversion On

	lcd_cmd(0x11); // Sleep Out

	lcd_cmd(0x29); // Display On
}

void lcd_windows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend)
{
	lcd_cmd(0x2A); // x
	lcd_data8(0x00);
	lcd_data8(Xstart);
	lcd_data8(0x00);
	lcd_data8(Xend - 1);
	lcd_cmd(0x2B); // y
	lcd_data8(0x00);
	lcd_data8(Ystart);
	lcd_data8(0x00);
	lcd_data8(Yend - 1);
	lcd_cmd(0X2C);
}

void lcd_fill(uint16_t color)
{
	color = ((color << 8) & 0xff00) | (color >> 8);
	for(uint32_t j = 0; j < LCD_WIDTH; j++)
	{
		fill_buf[j] = color;
	}

	lcd_windows(0, 0, LCD_WIDTH, LCD_HEIGHT);

	PIN_SET(D_DC);
	PIN_CLR(D_CS);
	for(uint32_t j = 0; j < LCD_HEIGHT * 2; j++)
	{
		HAL_SPI_Transmit(&hspi2, (uint8_t *)&fill_buf, LCD_WIDTH, 200);
	}
	PIN_SET(D_CS);
}

void lcd_point(uint16_t x, uint16_t y, uint16_t color)
{
	if(x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
	lcd_windows(x, y, x, y);
	lcd_data16(color);
}

void lcd_point_multi(uint16_t x, uint16_t y, uint16_t color, uint8_t size)
{
	for(int16_t XDir_Num = 0; XDir_Num < 2 * size - 1; XDir_Num++)
	{
		for(int16_t YDir_Num = 0; YDir_Num < 2 * size - 1; YDir_Num++)
		{
			if(x + XDir_Num - size < 0 || y + YDir_Num - size < 0) break;
			lcd_point(x + XDir_Num - size, y + YDir_Num - size, color);
		}
	}
}

void lcd_rectangle(uint16_t x_s, uint16_t y_s, uint16_t x_len, uint16_t y_len, uint16_t color)
{
	color = ((color << 8) & 0xff00) | (color >> 8);
	for(uint32_t j = 0; j < LCD_WIDTH; j++)
	{
		fill_buf[j] = color;
	}
	lcd_windows(x_s, y_s, x_s + x_len, y_s + y_len);
	PIN_SET(D_DC);
	PIN_CLR(D_CS);
	uint32_t total_pixels = x_len * y_len;
	for(uint32_t j = 0; total_pixels > j; j += FILL_BUF_SZ)
	{
		HAL_SPI_Transmit(&hspi2, (uint8_t *)&fill_buf, total_pixels - j >= FILL_BUF_SZ ? FILL_BUF_SZ : total_pixels - j, 200);
	}
	PIN_SET(D_CS);
}

void lcd_circle(uint16_t x_c, uint16_t y_c, uint16_t r, uint16_t color, uint32_t pixel_size, bool fill)
{
	int16_t x = 0, y = r;		// draw a circle from(0, R) as a starting point
	int16_t esp = 3 - (r << 1); // cumulative error

	if(fill)
	{
		while(x <= y)
		{
			for(int16_t sCountY = x; sCountY <= y; sCountY++)
			{
				lcd_point_multi(x_c + x, y_c + sCountY, color, 1); // 1
				lcd_point_multi(x_c - x, y_c + sCountY, color, 1); // 2
				lcd_point_multi(x_c - sCountY, y_c + x, color, 1); // 3
				lcd_point_multi(x_c - sCountY, y_c - x, color, 1); // 4
				lcd_point_multi(x_c - x, y_c - sCountY, color, 1); // 5
				lcd_point_multi(x_c + x, y_c - sCountY, color, 1); // 6
				lcd_point_multi(x_c + sCountY, y_c - x, color, 1); // 7
				lcd_point_multi(x_c + sCountY, y_c + x, color, 1);
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
			lcd_point_multi(x_c + x, y_c + y, color, pixel_size); // 1
			lcd_point_multi(x_c - x, y_c + y, color, pixel_size); // 2
			lcd_point_multi(x_c - y, y_c + x, color, pixel_size); // 3
			lcd_point_multi(x_c - y, y_c - x, color, pixel_size); // 4
			lcd_point_multi(x_c - x, y_c - y, color, pixel_size); // 5
			lcd_point_multi(x_c + x, y_c - y, color, pixel_size); // 6
			lcd_point_multi(x_c + y, y_c - x, color, pixel_size); // 7
			lcd_point_multi(x_c + y, y_c + x, color, pixel_size); // 0

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

void lcd_char(uint16_t x, uint16_t y, const char c, sFONT *font, uint16_t color_fg, uint16_t color_bg)
{
	if(x + font->Width >= LCD_WIDTH || y + font->Height >= LCD_HEIGHT) return;
	color_fg = ((color_fg << 8) & 0xff00) | (color_fg >> 8);
	color_bg = ((color_bg << 8) & 0xff00) | (color_bg >> 8);
	uint32_t Char_Offset = (c - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
	const unsigned char *ptr = &font->table[Char_Offset];

	for(uint16_t row = 0; row < font->Height; row++)
	{
		for(uint16_t col = 0; col < font->Width; col++)
		{
			fill_buf[row * font->Width + col] = (*ptr & (0x80 >> (col % 8))) ? color_fg : color_bg;
			if(col % 8 == 7) ptr++;
		}
		if(font->Width % 8) ptr++;
	}

	lcd_windows(x, y, x + font->Width, y + font->Height);
	PIN_SET(D_DC);
	PIN_CLR(D_CS);
	for(uint32_t j = 0; j < font->Height; j++)
	{
		HAL_SPI_Transmit(&hspi2, (uint8_t *)&fill_buf, font->Width * font->Height * 2, 200);
	}
	PIN_SET(D_CS);
}

void lcd_string(uint16_t x, uint16_t y, sFONT *font, uint16_t color_fg, uint16_t color_bg, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(buf_print, sizeof(buf_print), format, args);
	va_end(args);

	for(char *it = buf_print; *it; it++)
	{
		lcd_char(x, y, *it, font, color_fg, color_bg);
		x += font->Width + 1 * 0;
	}
}