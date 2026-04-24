#include "fonts.h"
#include <stdint.h>
#include <stdio.h>

#define FILL_BUF_SZ /*LCD_WIDTH*/ 512
static uint16_t fill_buf[FILL_BUF_SZ];

void p(int row, int col)
{
	printf("=== %dx%d ===\n", row, col);
	for(uint32_t j = 0; j < row + 2; j++)
		printf("-");
	printf("\n");
	for(uint32_t i = 0; i < col; i++)
	{
		printf("|");
		for(uint32_t j = 0; j < row; j++)
		{
			printf("%c", fill_buf[i * row + j] ? '#' : ' ');
		}
		printf("|\n");
	}
	for(uint32_t j = 0; j < row + 2; j++)
		printf("-");
	printf("\n");
}

void lcd_char(const char c, const uint8_t *font, uint32_t font_sz)
{
	uint32_t num_ch = 0;
	if(font[3] == 0 || font[3] == 1) num_ch = (font_sz - 4) / font[0] / (font[1] / 8 + (font[1] % 8 ? 1 : 0));
	if(font[3] == 2 || font[3] == 3) num_ch = (font_sz - 4) / font[1] / (font[0] / 8 + (font[0] % 8 ? 1 : 0));

	if(font[3] == 2)
	{
		if(c >= font[2] + num_ch) return;
		if(c < font[2]) return;
		uint32_t ch_offset = 4 + (c - font[2]) * font[1] * (font[0] / 8 + (font[0] % 8 ? 1 : 0));

		for(uint16_t row = 0; row < font[1]; row++)
		{
			for(uint16_t col = 0; col < font[0]; col++)
			{
				fill_buf[row * font[0] + col] = (font[ch_offset] & (0x80 >> (col % 8)))
													? 1
													: 0;
				if(col % 8 == 7) ch_offset++;
			}
			if(font[0] % 8) ch_offset++;
		}

		p(font[0], font[1]);
	}
	else
	{
		if(c >= font[2] + num_ch) return;
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
						// printf("%x ", frame);
						fill_buf[font[0] * row + k * 8 + 7 - i] = (frame & (1 << i))
																	  ? 1
																	  : 0;
					}
				}
			}
			// printf("\n");
		}
		else
		{
			for(int8_t row = font[1] - 1; row >= 0; row--)
			{
				for(uint16_t col = 0; col < font[0]; col++)
				{
					int a = font[3] == 1 ? row : font[1] - 1 - row;
					fill_buf[font[0] * a + col] = (font[ch_offset + col] & (1 << row))
													  ? 1
													  : 0;
				}
			}
		}

		p(font[0], font[1]);
	}
}

int main(void)
{
	lcd_char('!', F(font5x8_stm));
	lcd_char('!', F(font3x5));
	lcd_char('!', F(font5x8));
	lcd_char('!', F(font16x16_arial_bold));
	lcd_char('!', F(font8x12_tron));
	lcd_char('!', F(font8x16_retro));
	lcd_char('!', F(font24x32_ubuntu));
	lcd_char('0', F(font16x16_arial_normal));
	return 0;
}