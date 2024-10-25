#ifndef __DSP_H__
#define __DSP_H__

typedef struct {
	void* addr;
	unsigned int size;
} gx_firmware;

typedef struct {
	gx_firmware iram;
	gx_firmware dram;
	gx_firmware sram;
	gx_firmware xip;
	gx_firmware ddr;
} GxDspConfig;

typedef struct {
	unsigned int addr;
	unsigned int size;
} GxDspAddrMsg;

void gx_dsp_send_msg(GxDspAddrMsg *msg);
int  gx_dsp_init(void);
void gx_dsp_deinit(void);
int  gx_dsp_start(void);
int  gx_dsp_config(GxDspConfig *property);
int gx_dsp_ddr_test(void);
int gx_dsp_simple_test(void);
int gx_dsp_interrupt_test(void);
int gx_dsp_wait_mode_test(void);
int gx_dsp_key_test(void);
int gx_dsp_fft_test(void);
int gx_dsp_bbt_test(void);
int gx_dsp_bbt_audio_test(unsigned int *snr, unsigned int *thd);

#endif
