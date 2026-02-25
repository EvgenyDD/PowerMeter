#include "GUI_Paint.h"
#include "ade7753.h"
#include "lcd_pico.h"
#include "mcp3561.h"
#include "platform.h"

// https://github.com/Bobeye/cta2045-wifi-modules/blob/3646496d0187bd21bb205171ff542e122f3039f7/Aztec/ADE7753.h#L42
// https://github.com/Bobeye/cta2045-wifi-modules/blob/master/Aztec/ADE7753.c
// https://github.com/teslamotors/linux/blob/f1fb5b640056e29852f3d9345ac0a381e04cedbe/drivers/staging/iio/meter/ade7753.c#L166
// https://github.com/tinyos/tinyos-main/blob/c4fcab72d062e1dc6a294b8b2a64792e647528b6/tos/chips/ade7753/ADE7753P.nc
// https://github.com/fngstudios/ADE7753/blob/master/ADE7753.h
// https://github.com/fngstudios/ADE7753/blob/master/ADE7753.cpp#L427
// https://github.com/fngstudios/ADE7753/blob/master/ADE7753.h#L137
// https://github.com/Ideely/Metering-Project/blob/63e96721f0f4587e117e938400b36fbf9ae52bdb/Main/ADE7753.cpp#L361
// https://github.com/SEL-Columbia/telduino/blob/383989a7c0d6f376a28fa36898909f99881edf66/core/ADE7753/ADE7753.cpp#L6


extern SPI_HandleTypeDef hspi1;

void config(void)
{
	__HAL_SPI_ENABLE(&hspi1);

	lcd_init(LCD_VERTICAL);
	lcd_init(LCD_VERTICAL);
	lcd_fill(BLACK);

	// mcp3561_init();
	ade7753_init();
}

volatile uint8_t cnt_zx = 0, prev_lvl = 0;
volatile uint32_t last_smpl_cnt = 0;

int32_t min[2] = {0}, max[2] = {0};

int32_t wf[1024] = {0};

static int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
	if(x < in_min) x = in_min;
	if(x > in_max) x = in_max;
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

__attribute__((noreturn)) void loop(void)
{
	static uint32_t off = 0;

	lcd_string(0, 140, &Font12, BLUE, WHITE, "CHK:%d DIE:%x TEMP:%d sts:x%x  ",
			   ade7753_inst.checksum,
			   ade7753_inst.die_rev,
			   ade7753_inst.r.temp,
			   ade7753_inst.status);

	while(1)
	{
		lcd_string(0, 80, &Font12, WHITE, BLACK, "VRMS: %ld     ", ade7753_read_u_rms());
		lcd_string(0, 97, &Font12, WHITE, BLACK, "IRMS: %ld      ", ade7753_read_i_rms());
		lcd_string(0, 200, &Font12, BLUE, BLACK, "%ld %ld     ", min[0], max[0]);
		lcd_string(0, 220, &Font12, RED, BLACK, "%ld %ld      ", min[1], max[1]);
		lcd_string(0, 0, &Font16, YELLOW, BLACK, "x%x %ld %ld     ", ade7753_read_sts(), 454545454 / (uint32_t)ade7753_inst.r.period, /*cnt_zx*/ last_smpl_cnt);
		lcd_string(0, 17, &Font16, WHITE, BLACK, "x%08lx    ", ade7753_inst.r.vae);

		// lcd_string(0, 215, &Font24, GREEN, RED, "%ld   ", off);
		// lcd_fill(BLACK);
		// lcd_rectangle(off, off * 2, 80, 40, GREEN);
		// lcd_circle(200, 200, off % 20, BLUE, 2, false);
		// lcd_circle(100, 200, 5, RED, 1, true);
		// for(uint32_t i = 0; i < 10; i++)
		// {
		// lcd_char(i * 2, 24 * i, (rand() % 20) + 'a', &Font24, GREEN, GRAY);
		// },
		// HAL_Delay(10);
		// lcd_rectangle(off, off * 2, 80, 40, BLACK);

		off += 1;
		if(off == 10)
		{
			off = 0;
			// lcd_rectangle(0, S, W, S + H, BLACK);
			lcd_fill(BLACK);

			for(uint32_t m = 0; m < 2; m++)
			{
				m == 0 ? ade7753_set_wf_u() : ade7753_set_wf_i();

				volatile uint8_t cur = cnt_zx;
				last_smpl_cnt = 0;
				while(cur == cnt_zx)
					asm("nop");
				cur = cnt_zx;
				ade7753_read_wf();
				ade7753_read_wf();
				while(cur == cnt_zx)
				{
					wf[last_smpl_cnt] = ade7753_read_wf();
					last_smpl_cnt++;
					if(last_smpl_cnt >= 1023) last_smpl_cnt = 1023;
				}
				cur = cnt_zx;
				while(cur == cnt_zx)
				{
					wf[last_smpl_cnt] = ade7753_read_wf();
					last_smpl_cnt++;
					if(last_smpl_cnt >= 1023) last_smpl_cnt = 1023;
				}
				min[m] = INT32_MAX;
				max[m] = INT32_MIN;
				for(uint32_t i = 0; i < last_smpl_cnt; i++)
				{
					if(wf[i] < min[m]) min[m] = wf[i];
					if(wf[i] > max[m]) max[m] = wf[i];
				}

#define S 70
#define H 160
#define W 240

				for(uint32_t i = 0; i < W; i++)
				{
					int32_t xt = (int32_t)i * (int32_t)last_smpl_cnt / W;
					int32_t v = map(wf[xt], min[m], max[m], S, S + H - 2);
					lcd_point(i, v + 0, m == 0 ? BLUE : RED);
					lcd_point(i, v + 1, m == 0 ? BLUE : RED);
					lcd_point(i, v + 2, m == 0 ? BLUE : RED);
				}
			}
		}
	}
}

void systick_irq(void);
void systick_irq(void)
{
	if(PIN_GET(ZC) != prev_lvl)
	{
		prev_lvl = PIN_GET(ZC);
		cnt_zx++;
	}
}