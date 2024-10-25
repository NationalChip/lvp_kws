#include <autoconf.h>
#include <util.h>
#include <soc.h>
#include <driver/gx_clock.h>
#include "spl_spi.h"

void spl_clk_set_gate_enable(GX_CLOCK_MODULE module, unsigned int enable);
void spl_dw_dma_start(void);
void spl_dw_dma_config(void *dst, void *src, int len, int hw_src_per);
void serial_put(int ch);

#define SERIAL_RECV_SYNC()   do { \
	serial_put('r');              \
	serial_put('e');              \
	serial_put('a');              \
	serial_put('d');              \
	serial_put('y');              \
} while (0)

int spi_read_buff(unsigned char *buff, unsigned int len)
{
	spl_clk_set_gate_enable(CLOCK_MODULE_GENERAL_SPI, 1);
	writel(0x0, SPIM_ENR);
	writel(STANDARD_SLAVE_RO_8BITS_MODE, SPIM_CTRLR0);
	writel(0x1, SPIM_ENR);
	writel(0x0, SPIM_SER);
	writel(7, SPIM_DMARDLR);
	writel(0x01, SPIM_SER);
#ifdef CONFIG_SPL_DMA_LOAD
	writel(0x01, SPIM_DMACR);
	spl_dw_dma_config(buff, (void*)SPIM_RXDR_LE, len, DMA_HS_PRE_SPI0_RX);
	SERIAL_RECV_SYNC();
	spl_dw_dma_start();
	writel(0x00, SPIM_DMACR);
#else
	SERIAL_RECV_SYNC();
	int i;
	for (i = 0; i < len; i++) {
		while(!(readl(SPIM_SR) & SR_RF_NOT_EMPT));
		((char *)buff)[i] = 0xff & readl(SPIM_RXDR_LE);
	}
#endif
	return 0;
}

