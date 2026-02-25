#include "lcd_pico.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "fonts.h"
#include "hal/gpio_hal.h"
#include <stdarg.h>

// ST7789 240x240

#define PIN_H(X) REG_WRITE(GPIO_OUT_W1TS_REG, (1 << X))
#define PIN_L(X) REG_WRITE(GPIO_OUT_W1TC_REG, (1 << X))

#define D_DC 17
#define D_RST 22
#define D_BL 16 // PWM

#define FILL_BUF_SZ /*LCD_WIDTH*/ 512
static uint16_t fill_buf[FILL_BUF_SZ];

static char buf_print[256];

static uint16_t _xs = 0, _xe = 0, _ys = 0, _ye = 0, _xoff = 0, _yoff = 0;
static spi_device_handle_t spi_lcd;

extern const uint8_t font3x5[];
extern const uint8_t font5x8[];
extern const uint8_t font16x24[];
extern const uint8_t fontSTD_swiss721_outline[];
extern const uint8_t font8x12[];
extern const uint8_t font16x16[];
extern const uint8_t font8x16[];

sFONT_t Font3x5 = {font3x5, 0, 0};
sFONT_t Font5x8 = {font5x8, 0, 0};
sFONT_t Font16x24 = {font16x24, 0, 0};
sFONT_t FontSTD_swiss721_outline = {fontSTD_swiss721_outline, 0, 0};
sFONT_t Font8x12 = {font8x12, 0, 0};
sFONT_t Font16x16 = {font16x16, 0, 0};
sFONT_t Font8x16 = {font8x16, 0, 0};

static void lcd_cmd(uint8_t Reg)
{
	PIN_L(D_DC);
	static spi_transaction_t SPITransaction;
	memset(&SPITransaction, 0, sizeof(spi_transaction_t));
	SPITransaction.length = 1 * 8;
	SPITransaction.tx_buffer = &Reg;
	ESP_ERROR_CHECK(spi_device_transmit(spi_lcd, &SPITransaction));
}

static void lcd_data8(uint8_t Data)
{
	PIN_H(D_DC);
	static spi_transaction_t SPITransaction;
	memset(&SPITransaction, 0, sizeof(spi_transaction_t));
	SPITransaction.length = 1 * 8;
	SPITransaction.tx_buffer = &Data;
	ESP_ERROR_CHECK(spi_device_transmit(spi_lcd, &SPITransaction));
}

static void lcd_data16(uint16_t Data)
{
	PIN_H(D_DC);
	uint8_t d[] = {(Data >> 8) & 0xFF, Data & 0xFF};
	static spi_transaction_t SPITransaction;
	memset(&SPITransaction, 0, sizeof(spi_transaction_t));
	SPITransaction.length = 2 * 8;
	SPITransaction.tx_buffer = d;
	ESP_ERROR_CHECK(spi_device_transmit(spi_lcd, &SPITransaction));
}

uint16_t lcd_xs(void) { return _xs; }
uint16_t lcd_xe(void) { return _xe; }
uint16_t lcd_ys(void) { return _ys; }
uint16_t lcd_ye(void) { return _ye; }

uint16_t lcd_w(void) { return _xe - _xs; }
uint16_t lcd_h(void) { return _ye - _ys; }

void lcd_set_bl(uint8_t val)
{
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, val);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void lcd_reinit(uint8_t view)
{
	PIN_H(D_RST);
	vTaskDelay(2);
	PIN_L(D_RST);
	vTaskDelay(2);
	PIN_H(D_RST);
	vTaskDelay(2);

	lcd_cmd(0x36); // MX, MY, RGB mode
	switch(view)
	{
	default:
	case VIEW_12H:
		lcd_data8(VIEW_12H);
		_xoff = 80;
		_yoff = 0;
		_xs = 2;
		_xe = 239;
		_ys = 0;
		_ye = 239;
		break;

	case VIEW_3H:
		lcd_data8(VIEW_3H);
		_xoff = _yoff = 0;
		_xs = 0;
		_xe = 239;
		_ys = 0;
		_ye = 239 - 2;
		break;

	case VIEW_6H:
		lcd_data8(VIEW_6H);
		_xoff = _yoff = 0;
		_xs = 0;
		_xe = 239 - 2;
		_ys = 0;
		_ye = 239;
		break;

	case VIEW_9H:
		lcd_data8(VIEW_9H);
		_xoff = 0;
		_yoff = 80;
		_xs = 0;
		_xe = 239;
		_ys = 2;
		_ye = 239;
		break;
	}

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

void lcd_init(uint8_t view)
{
	spi_bus_config_t bus_conf = {
		.mosi_io_num = 23,
		.miso_io_num = -1,
		.sclk_io_num = 18,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 120000,
	};
	ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &bus_conf, SPI_DMA_CH_AUTO));
	static const spi_device_interface_config_t cfg_lcd = {
		.clock_speed_hz = 60000000,
		.mode = 0,
		.spics_io_num = 5,
		.queue_size = 64,
	};
	ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &cfg_lcd, &spi_lcd));

	// gpio_set_direction(D_BL, GPIO_MODE_OUTPUT);
	gpio_set_direction(D_RST, GPIO_MODE_OUTPUT);
	gpio_set_direction(D_DC, GPIO_MODE_OUTPUT);

	// PIN_H(D_BL);

	ledc_timer_config_t ledc_timer = {
		.speed_mode = LEDC_LOW_SPEED_MODE,
		.duty_resolution = LEDC_TIMER_8_BIT,
		.timer_num = LEDC_TIMER_0,
		.freq_hz = 200,
		.clk_cfg = LEDC_AUTO_CLK};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	ledc_channel_config_t ledc_channel = {
		.speed_mode = LEDC_LOW_SPEED_MODE,
		.channel = LEDC_CHANNEL_0,
		.timer_sel = LEDC_TIMER_0,
		.intr_type = LEDC_INTR_DISABLE,
		.gpio_num = D_BL,
		.duty = 150,
		.hpoint = 0};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

	lcd_reinit(view);
}

void lcd_windows(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
	// if(xe > 240) return;
	// if(ye > 240) return;
	lcd_cmd(0x2A); // x
	lcd_data8((xs + _xs + _xoff) >> 8);
	lcd_data8((xs + _xs + _xoff) & 0xFF);
	lcd_data8(((xe + _xs + _xoff) - 1) >> 8);
	lcd_data8(((xe + _xs + _xoff) - 1) & 0xFF);

	lcd_cmd(0x2B); // y
	lcd_data8((ys + _ys + _yoff) >> 8);
	lcd_data8((ys + _ys + _yoff) & 0xFF);
	lcd_data8(((ye + _ys + _yoff) - 1) >> 8);
	lcd_data8(((ye + _ys + _yoff) - 1) & 0xFF);

	lcd_cmd(0X2C);
}

void lcd_windows_raw(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
	lcd_cmd(0x2A); // x
	lcd_data8((xs + _xoff) >> 8);
	lcd_data8((xs + _xoff) & 0xFF);
	lcd_data8((xe + _xoff - 1) >> 8);
	lcd_data8((xe + _xoff - 1) & 0xFF);

	lcd_cmd(0x2B); // y
	lcd_data8((ys + _yoff) >> 8);
	lcd_data8((ys + _yoff) & 0xFF);
	lcd_data8((ye + _yoff - 1) >> 8);
	lcd_data8((ye + _yoff - 1) & 0xFF);

	lcd_cmd(0X2C);
}

void lcd_fill(uint16_t color)
{
	color = ((color << 8) & 0xff00) | (color >> 8);
	for(uint32_t j = 0; j < LCD_WIDTH; j++)
	{
		fill_buf[j] = color;
	}

	lcd_windows_raw(0, 0, LCD_WIDTH, LCD_HEIGHT);

	PIN_H(D_DC);
	for(uint32_t j = 0; j < LCD_HEIGHT * 2; j++)
	{
		static spi_transaction_t SPITransaction;
		memset(&SPITransaction, 0, sizeof(spi_transaction_t));
		SPITransaction.length = LCD_WIDTH * 8;
		SPITransaction.tx_buffer = fill_buf;
		ESP_ERROR_CHECK(spi_device_transmit(spi_lcd, &SPITransaction));
	}
}

void lcd_fill_bounds(uint16_t xs, uint16_t ys, uint16_t x_len, uint16_t y_len, uint16_t color)
{
	color = ((color << 8) & 0xff00) | (color >> 8);
	for(uint32_t j = 0; j < LCD_WIDTH; j++)
	{
		fill_buf[j] = color;
	}

	lcd_windows_raw(xs, ys, xs + x_len, ys + y_len);

	PIN_H(D_DC);
	for(uint32_t j = 0; j < y_len * 2; j++)
	{
		static spi_transaction_t SPITransaction;
		memset(&SPITransaction, 0, sizeof(spi_transaction_t));
		SPITransaction.length = x_len * 2 * 8;
		SPITransaction.tx_buffer = fill_buf;
		ESP_ERROR_CHECK(spi_device_transmit(spi_lcd, &SPITransaction));
	}
}

void lcd_pixel(uint16_t x, uint16_t y, uint16_t color)
{
	if(x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
	lcd_windows(x, y, x + 1, y + 1);
	lcd_data16(color);
}

void lcd_pixel_multi(uint16_t x, uint16_t y, uint16_t color, uint8_t size)
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

void lcd_rectangle(uint16_t x_s, uint16_t y_s, uint16_t x_len, uint16_t y_len, uint16_t color)
{
	color = ((color << 8) & 0xff00) | (color >> 8);
	for(uint32_t j = 0; j < LCD_WIDTH; j++)
	{
		fill_buf[j] = color;
	}
	lcd_windows(x_s, y_s, x_s + x_len, y_s + y_len);
	PIN_H(D_DC);
	uint32_t total_pixels = x_len * y_len;
	for(uint32_t j = 0; total_pixels > j; j += FILL_BUF_SZ)
	{
		static spi_transaction_t SPITransaction;
		memset(&SPITransaction, 0, sizeof(spi_transaction_t));
		SPITransaction.length = (total_pixels - j >= FILL_BUF_SZ ? FILL_BUF_SZ : total_pixels - j) * 8;
		SPITransaction.tx_buffer = fill_buf;
		ESP_ERROR_CHECK(spi_device_transmit(spi_lcd, &SPITransaction));
	}
}

static void writeRepeat(uint16_t color, uint32_t len)
{
	len *= 2;
	color = ((color << 8) & 0xff00) | (color >> 8);
	for(uint32_t j = 0; j < (len < FILL_BUF_SZ ? len : FILL_BUF_SZ); j++)
	{
		fill_buf[j] = color;
	}
	PIN_H(D_DC);
	for(uint32_t j = 0; len > j; j += FILL_BUF_SZ)
	{
		static spi_transaction_t SPITransaction;
		memset(&SPITransaction, 0, sizeof(spi_transaction_t));
		SPITransaction.length = (len - j >= FILL_BUF_SZ ? FILL_BUF_SZ : len - j) * 8;
		SPITransaction.tx_buffer = fill_buf;
		ESP_ERROR_CHECK(spi_device_transmit(spi_lcd, &SPITransaction));
	}
}

void lcd_line_h(int16_t x, int16_t y, int16_t l, uint16_t color)
{
	lcd_windows(x, y, x + l, y + 1);
	writeRepeat(color, l);
}

void lcd_line_v(int16_t x, int16_t y, int16_t l, uint16_t color)
{
	lcd_windows(x, y, x + 1, y + l);
	writeRepeat(color, l);
}

void lcd_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool fill)
{
	if(fill)
	{
		lcd_windows(x, y, w, h);
		writeRepeat(color, (uint32_t)w * h);
	}
	else
	{
		lcd_line_h(x, y, w, color);
		lcd_line_h(x, y + h - 1, w, color);
		lcd_line_v(x, y, h, color);
		lcd_line_v(x + w - 1, y, h, color);
	}
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

extern uint8_t yy[8];
void lcd_char(uint16_t x, uint16_t y, const char c, sFONT_t *font, uint16_t color_fg, uint16_t color_bg)
{
	color_fg = ((color_fg << 8) & 0xff00) | (color_fg >> 8);
	color_bg = ((color_bg << 8) & 0xff00) | (color_bg >> 8);

	if(font->Height && font->Width)
	{
		if(x + font->Width >= LCD_WIDTH || y + font->Height >= LCD_HEIGHT) return;
		uint32_t Char_Offset = (c - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));

		const uint8_t *ptr = &font->table[Char_Offset];
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
		PIN_H(D_DC);
		for(uint32_t j = 0; j < font->Height; j++)
		{
			static spi_transaction_t SPITransaction;
			memset(&SPITransaction, 0, sizeof(spi_transaction_t));
			SPITransaction.length = (font->Width * font->Height * 2) * 8;
			SPITransaction.tx_buffer = fill_buf;
			ESP_ERROR_CHECK(spi_device_transmit(spi_lcd, &SPITransaction));
		}
	}
	else
	{
		struct
		{
			const uint8_t *font;
			uint8_t x_size;
			uint8_t y_size;
			uint8_t offset;
			uint8_t numchars;
			uint8_t inverse;
		} cfont;
		cfont.font = font->table;
		cfont.x_size = cfont.font[0];
		cfont.y_size = cfont.font[1];
		cfont.offset = cfont.font[2];
		cfont.numchars = cfont.font[3];
		cfont.inverse = cfont.font[4];

#define FONT_WIDTH cfont.x_size
#define FONT_HEIGHT cfont.y_size

#define BitIsSet(x, y) ((x) & (1 << (y)))

		if(c >= cfont.offset + cfont.numchars) return;
		uint32_t temp = (uint32_t)((c - cfont.offset) * (FONT_WIDTH * ((FONT_HEIGHT + 7) / 8))) + 5;

		if(BitIsSet(cfont.inverse, 7))
		{
			for(int16_t j = 0; j < FONT_HEIGHT; j++)
			{
				for(int8_t k = 0; k < FONT_WIDTH / 8; k++)
				{
					char frame = cfont.font[temp++];
					for(uint8_t i = 0; i < 8; i++)
					{
						if(BitIsSet(frame, cfont.inverse & 0x01 ? (7 - i) : (i)))
						{
							fill_buf[(k * 8 + 7 - i) + cfont.y_size * (j)] = color_fg;
						}
						else
						{
							fill_buf[(k * 8 + 7 - i) + cfont.y_size * (j)] = color_bg;
						}
					}
				}
			}
		}
		else
		{
			for(uint16_t col = 0; col < FONT_WIDTH; col++)
			{
				const uint8_t frame = cfont.font[temp++];
				for(uint8_t row = 0; row < FONT_HEIGHT; row++)
				{
					if(BitIsSet(frame, cfont.inverse & 0x01 ? (7 - row) : (row)))
					{
						fill_buf[(col) + cfont.x_size * (FONT_HEIGHT - 1 - row)] = color_fg;
					}
					else
					{
						fill_buf[(col) + cfont.x_size * (FONT_HEIGHT - 1 - row)] = color_bg;
					}
				}
			}
		}

		lcd_windows(x, y, x + cfont.x_size, y + cfont.y_size);
		PIN_H(D_DC);
		for(uint32_t j = 0; j < 8 + 0 * cfont.y_size; j++)
		{
			static spi_transaction_t SPITransaction;
			memset(&SPITransaction, 0, sizeof(spi_transaction_t));
			SPITransaction.length = (cfont.x_size * cfont.y_size * 2) * 8;
			SPITransaction.tx_buffer = fill_buf;
			ESP_ERROR_CHECK(spi_device_transmit(spi_lcd, &SPITransaction));
		}
	}
}

void lcd_string(uint16_t x, uint16_t y, sFONT_t *font, uint16_t color_fg, uint16_t color_bg, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(buf_print, sizeof(buf_print), format, args);
	va_end(args);

	for(char *it = buf_print; *it; it++)
	{
		lcd_char(x, y, *it, font, color_fg, color_bg);
		if(font->Height && font->Width)
			x += font->Width + 1 * 0;
		else
			x += font->table[0] + 1;
	}
}