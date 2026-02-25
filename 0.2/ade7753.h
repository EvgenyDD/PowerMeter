#ifndef ADE7753_H__
#define ADE7753_H__

#include <stdint.h>

typedef struct
{
	struct
	{
		int8_t temp;
		int32_t irms;
		int32_t irms2;
		int32_t urms;
		int32_t urms2;
		uint32_t vae;
		int32_t wf;
		uint16_t period;
	} r;
	uint16_t irqen;
	uint16_t mode;
	uint16_t status;
	uint8_t die_rev;
	uint8_t checksum;
} ade7753_inst_t;

void ade7753_init(void);

uint16_t ade7753_read_sts(void);
uint16_t ade7753_read_sts_clr(void);
int32_t ade7753_read_wf(void);
uint32_t ade7753_read_i_rms(void);
uint32_t ade7753_read_u_rms(void);
int8_t ade7753_read_temp(void);

void ade7753_set_wf_i(void);
void ade7753_set_wf_u(void);

extern ade7753_inst_t ade7753_inst;

#endif // ADE7753_H__