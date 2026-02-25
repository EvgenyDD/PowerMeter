#ifndef BL0937_H__
#define BL0937_H__

#include "console.h"

void bl0937_init(void);
void bl0937_poll(uint32_t diff_ms);
void bl0937_rst_vals(void);

float bl0937_get_cf_e(void);
float bl0937_get_cf_p(void);
float bl0937_get_cf_u(void);
float bl0937_get_cf_i(void);

void con_cb_bl0937_damp(print_func_t pf, const char *req, int len, int *ret);
void con_cb_bl0937_rst_cnt(print_func_t pf, const char *req, int len, int *ret);

// clang-format off

#define BLE0937_CONSOLE_ITEMS \
	{"em_dump2", con_cb_bl0937_damp}, \
	{"em_rst2", con_cb_bl0937_rst_cnt}

// clang-format on

#endif // BL0937_H__