#include "ade7753.h"
#include "bl0937.h"
#include "console.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lcd_pico.h"
#include "web/time_wrapper.h"
#include "web/web_common.h"
#include "web/wifi.h"
#include "web/ws.h"

#if !CONFIG_IDF_TARGET_ESP32
#error "Wrong target"
#endif

// 16MB

uint32_t restart_timer = 0;
bool flashing = false;

#include "GUI_Paint.h"

extern volatile uint8_t cnt_zx, cnt_cf;

uint8_t yy[8] = {0};

volatile uint32_t last_smpl_cnt = 0;
int32_t min[2] = {0}, max[2] = {0};
#define LIM 4096
int32_t wf[4096] = {0};
uint8_t last_smpl_cnt_[2];
uint16_t last_ts[3];

static int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
	if(x < in_min) x = in_min;
	if(x > in_max) x = in_max;
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void app_main(void)
{
	lcd_init(VIEW_12H);
	// lcd_init(0);

	console_init();
	web_common_init();
	wifi_init();
	ws_init();
	time_wrapper_init(NULL);

	lcd_fill(BLACK);
	lcd_rect(0, 0, lcd_w(), lcd_h(), GREEN, false);

	gpio_install_isr_service(0);
	ade7753_init();
	bl0937_init();

	uint32_t prev_systick = esp_log_timestamp();

	while(1)
	{
		const uint32_t systick_now = esp_log_timestamp();
		uint32_t diff_ms = systick_now - prev_systick;
		prev_systick = systick_now;

		if(restart_timer)
		{
			if(restart_timer <= diff_ms) esp_restart();
			restart_timer -= diff_ms;
		}

		wifi_poll(diff_ms);
		bl0937_poll(diff_ms);
		ade7753_poll(diff_ms);

		if(flashing == false && 0)
		{
			static uint32_t upd_tmr_ms = 0;
			upd_tmr_ms += diff_ms;
			if(upd_tmr_ms >= 200)
			{
				if(ade7753_is_wf_rec() == false)
				{
					upd_tmr_ms = 0;

					lcd_string(3, 3, &Font12, WHITE, BLACK, "VRMS: %.2f V  ", ade7753_inst.urms);
					lcd_string(120, 3, &Font12, WHITE, BLACK, "IRMS: %.3f A  ", ade7753_inst.irms);

					lcd_string(3, 16, &Font12, WHITE, BLACK, "%.2f Hz | %.2f C  ", ade7753_inst.freq, ade7753_inst.temp);
					lcd_string(3, 30, &Font12, WHITE, BLACK, "%.1f %.1f %.1f | %.1f   ",
							   bl0937_get_cf_e(), bl0937_get_cf_u(), bl0937_get_cf_i(),
							   (float)ade7753_get_cf());
					lcd_string(3, 44, &Font12, WHITE, BLACK, "%.3f W %.2f V %.3f A    ",
							   1000000 / bl0937_get_cf_e() * 1.37226f, 1000000 / bl0937_get_cf_u() * 0.1196f, 1000000 / bl0937_get_cf_i() * 0.011456f);
					lcd_string(3, 58, &Font12, WHITE, BLACK, "%.3f W  ", ade7753_read_wf() / 133.643617f);
					lcd_string(160, 44, &Font12, GREEN, BLACK, "%ld %d %d  ", last_smpl_cnt, last_smpl_cnt_[0], last_smpl_cnt_[1]);
					lcd_string(110, 58, &Font12, YELLOW, BLACK, "%d %d  ", last_ts[1], last_ts[2]);

					// lcd_string(3, 3, &Font8, GREEN, BLACK, "ABCDEF0123456789");
					// lcd_string(3, 12, &Font12, GREEN, BLACK, "ABCDEF0123456789");
					// lcd_string(3, 25, &Font16, GREEN, BLACK, "ABCDEF0123456789");
					// lcd_string(3, 42, &Font20, GREEN, BLACK, "ABCDEF0123456789");
					// lcd_string(3, 65, &Font24, GREEN, BLACK, "ABCDEF0123456789");

					// lcd_string(3, 90, &Font3x5, GREEN, BLACK, "ABCDEF0123456789");
					// lcd_string(3, 98, &Font5x8, GREEN, BLACK, "ABCDEF0123456789");
					// lcd_string(3, 145, &Font16x16, GREEN, BLACK, "ABCDEF0123456789");
					// lcd_string(3, 188, &FontSTD_swiss721_outline, GREEN, BLACK, "ABCDEF0123456789");

					static uint32_t c = 0;
					if(++c >= 10 && !flashing)
					{
						c = 0;
#define XS (3)
#define YS (80)
#define H (lcd_h() - 3 - YS)
#define W (lcd_w() - XS)
#define LINE_H 2
						lcd_fill_bounds(XS, YS, W, H, GRAY);

						int32_t max_r[2] = {0};
						for(uint32_t m = 0; m < 2; m++)
						{
							m == 0 ? ade7753_set_wf_u() : ade7753_set_wf_i();

							last_smpl_cnt = 0;
							volatile uint8_t curp = ade7753_get_zc_cnt_pos();
							asm("nop");
							while(curp == ade7753_get_zc_cnt_pos())
							{
								asm("nop");
							}

							uint64_t ts = esp_timer_get_time();
							volatile uint8_t cur = ade7753_get_zc_cnt();
							while(cur == ade7753_get_zc_cnt())
							{
								wf[last_smpl_cnt] = ade7753_read_wf();
								if(++last_smpl_cnt >= 4096) last_smpl_cnt = 4096;
							}
							last_smpl_cnt_[0] = last_smpl_cnt;
							last_ts[1] = esp_timer_get_time() - ts;

							cur = ade7753_get_zc_cnt();
							asm("nop");
							while(cur == ade7753_get_zc_cnt())
							{
								wf[last_smpl_cnt] = ade7753_read_wf();
								if(++last_smpl_cnt >= 4096) last_smpl_cnt = 4096;
							}
							last_smpl_cnt_[1] = last_smpl_cnt - last_smpl_cnt_[0];
							last_ts[2] = esp_timer_get_time() - ts;

							min[m] = INT32_MAX;
							max[m] = INT32_MIN;

#define NS 5
							for(uint32_t i = 0; i < last_smpl_cnt; i++)
							{
								if(i > NS)
								{
									int32_t avg = 0;
									for(uint32_t k = 0; k < NS; k++)
										avg += wf[i - k];
									avg /= NS;
									if(avg < min[m]) min[m] = avg;
									if(avg > max[m]) max[m] = avg;
								}
								if(abs(wf[i]) > max_r[m]) max_r[m] = abs(wf[i]);
							}
							if(abs(min[m]) < max[m]) min[m] = -max[m];
							if(abs(min[m]) > max[m]) max[m] = -min[m];

#define CUR_MIN_LIMIT 1
							if(m == 1)
							{
								if(max[m] < CUR_MIN_LIMIT)
								{
									max[m] = CUR_MIN_LIMIT;
									min[m] = -CUR_MIN_LIMIT;
								}
							}

							for(uint32_t i = XS; i < W; i++)
							{
								int32_t xt = (int32_t)i * (int32_t)last_smpl_cnt / (W - XS);
								int32_t v = map(wf[xt], min[m], max[m], YS, YS + H - 2);
								lcd_line_v(i, v, LINE_H, m == 0 ? BLUE : RED);
							}
							lcd_string(XS + W / 5, YS + 3 * H / 4 - 9, &Font5x8, BLUE, BLACK, "%ld %ld", max[0], max_r[0]);
							lcd_string(XS + W / 5, YS + 3 * H / 4, &Font5x8, RED, BLACK, "%ld %ld", max[1], max_r[1]);
						}

						ade7753_set_wf_e();
					}
				}
			}
		}

		vTaskDelay(1);
	}
}
