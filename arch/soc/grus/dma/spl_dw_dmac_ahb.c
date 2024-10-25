#include <common.h>
#include <misc.h>
#include <soc.h>
#include <driver/gx_clock.h>
#include "autoconf.h"
#if defined(CONFIG_ENABLE_SPI_DMA_FOR_FLASH) || defined(CONFIG_DW_UART_DMA)
#include "spl_dw_dmac_ahb.h"

#define DMA_CHANNEL_LLP_SIZE     512
#define DMA_BLOCK_TS             0xFFF

struct dw_dmac_priv{
	dw_dma_regs_t *dw_dmac;
	u8             llp_buf[DMA_CHANNEL_LLP_SIZE];
};
static struct dw_dmac_priv dw_dmac_priv_var={
	.dw_dmac = (dw_dma_regs_t*)GX_REG_BASE_DMA,
};

static struct dw_dmac_priv *priv = &dw_dmac_priv_var;


#define DW_LIST_SET(_lli, _head, _cyc_len, _cnt, _len, _wide) do{           \
	dw_lli_t *plli = _head;                                             \
	uint16_t __index[4] = { _cyc_len, -_cyc_len, 0, 0};                 \
	int32_t sar_add  = __index[0x03 & ((_lli)->ctl[0] >> 9)] * (_wide); \
	int32_t dar_add  = __index[0x03 & ((_lli)->ctl[0] >> 7)] * (_wide); \
	int32_t i = 0;                                                      \
	while(i < _cnt-1){                                                  \
		plli->sar    = (_lli)->sar + sar_add * i;                   \
		plli->dar    = (_lli)->dar + dar_add * i;                   \
		plli->ctl[0] = (_lli)->ctl[0];                              \
		plli->ctl[1] = _cyc_len ;                                   \
		plli->llp    = spl_virt_to_dma(plli + 1);                   \
		++plli; ++i;                                                \
	}                                                                   \
	plli->sar    = (_lli)->sar + sar_add * i;                           \
	plli->dar    = (_lli)->dar + dar_add * i;                           \
	plli->ctl[0] = (_lli)->ctl[0] & (~(0x03 << 27));                    \
	plli->ctl[1] = _len % _cyc_len ? _len % _cyc_len : _cyc_len;        \
	plli->llp    = 0;                                                   \
}while(0)

#define dw_dma_enable(_on) do{ priv->dw_dmac->CFG[0] = !!(_on); }while(0)
#define dw_dma_channel_enable(_ch, _en) do{                                 \
	if(_en)                                                             \
		priv->dw_dmac->CH_EN[0] |=   0x101 << _ch;                  \
	else                                                                \
		priv->dw_dmac->CH_EN[0] &= ~(0x101 << _ch);                 \
}while(0)

#define CHAN_CLEAN(_PTR, _OP, _VAL) do{                                     \
	*((_PTR)->XFER)     _OP _VAL;                                       \
	*((_PTR)->BLOCK)    _OP _VAL;                                       \
	*((_PTR)->SRC_TRAN) _OP _VAL;                                       \
	*((_PTR)->DST_TRAN) _OP _VAL;                                       \
	*((_PTR)->ERROR)    _OP _VAL;                                       \
}while(0)

static unsigned int spl_virt_to_dma(void *addr)
{
	if ((unsigned int)addr >= 0x10000000 && (unsigned int)addr < 0x30000000)
		return (unsigned int)addr & 0x0fffffff;
	else
		return (unsigned int)addr;
}

static void dw_dma_clean_int(int ch)
{
	CHAN_CLEAN(&priv->dw_dmac->CLEAR, |=,  0x01  << ch );
	CHAN_CLEAN(&priv->dw_dmac->RAW,   &=, ~(0x01 << ch));
}

static void dw_dma_channel_config(int *ctl, int *cfg_l, int *cfg_h, int hw_src_per)
{
	//now setting CHXCTL REG
	*ctl = DWC_COM_CTRL0;

	//set msize
	*ctl |= DWC_CTLL_DST_MSIZE(2); // BURST_TRANS_LEN_8
	*ctl |= DWC_CTLL_SRC_MSIZE(2); // BURST_TRANS_LEN_8
	//set tr width
	*ctl |= DWC_CTLL_DST_WIDTH(0); // DMAC_TRANS_WIDTH_8
	*ctl |= DWC_CTLL_SRC_WIDTH(0); // DMAC_TRANS_WIDTH_8

	//set dst/src addr increase
	*ctl |= DWC_CTLL_DST_INC;
	*ctl |= DWC_CTLL_SRC_FIX;

	//AHB master select
	*ctl |= DWC_CTLL_DMS(0x0);
	*ctl |= DWC_CTLL_SMS(0x1);

	//set dir and dataflow
	*ctl |= DWC_CTLL_FC(2); // TT_FC_PER_TO_MEM_DMAC

	//set src/dst handshaking mode.
	//hardware or software
	*cfg_h |= DWC_CFGH_FIFO_MODE;

	//set dst/src per
	*cfg_h |= DWC_CFGH_DEST_PER(0);
	*cfg_h |= DWC_CFGH_SRC_PER(hw_src_per);
}

void spl_clk_gate_enable(GX_CLOCK_MODULE module, unsigned int enable);
void spl_dw_dma_xfer_poll(void *dst, void *src, int len, int hw_src_per)
{
	int lli_cnt;
	dw_lli_t lli;
	int ctl = 0;
	int cfg_l = 0;
	int cfg_h = 0;
	int channel = 0;

	spl_clk_gate_enable(CLOCK_DMA, 1);
	dw_dma_clean_int(channel);
	dw_dma_channel_config(&ctl, &cfg_l, &cfg_h, hw_src_per);

	/* enable dma */
	dw_dma_enable(1);
	priv->dw_dmac->CHAN[channel].SAR[0] = lli.sar    = spl_virt_to_dma(src);
	priv->dw_dmac->CHAN[channel].DAR[0] = lli.dar    = spl_virt_to_dma(dst);
	priv->dw_dmac->CHAN[channel].CTL[0] = lli.ctl[0] = ctl;
	priv->dw_dmac->CHAN[channel].CFG[0] = cfg_l;
	priv->dw_dmac->CHAN[channel].CFG[1] = cfg_h;

	lli_cnt = len / DMA_BLOCK_TS + !!(len % DMA_BLOCK_TS);

	// TODO: max length 71KB, for any length.
	DW_LIST_SET(&lli, (dw_lli_t *)priv->llp_buf, DMA_BLOCK_TS, lli_cnt, len, 1);
	priv->dw_dmac->CHAN[channel].LLP[0]  = spl_virt_to_dma(priv->llp_buf);
	dw_dma_channel_enable(channel, 1);

	// dma wait complete
	while((priv->dw_dmac->RAW.XFER[0] & (0x01 <<channel)) == 0);
	dw_dma_channel_enable(channel, 0);
	dw_dma_enable(0);
	spl_clk_gate_enable(CLOCK_DMA, 0);
}
#endif
