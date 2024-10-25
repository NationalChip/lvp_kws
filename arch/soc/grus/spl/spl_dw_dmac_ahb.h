#ifndef __SPL_DW_DMAC_AHB_H__
#define __SPL_DW_DMAC_AHB_H__

#define DW_REG(_name)   volatile unsigned int _name[2]

/* Hardware register definitions. */
typedef struct dw_dma_chan_regs {
	DW_REG(SAR);             /* Source Address Register */
	DW_REG(DAR);             /* Destination Address Register */
	DW_REG(LLP);             /* Linked List Pointer */
	DW_REG(CTL);             /* Channel Control Register */
	DW_REG(SSTAT);           /* Channel Source Status Register */
	DW_REG(DSTAT);           /* Channel Destination Status Register */
	DW_REG(SSTATAR);         /* Channel Source Status Address Register */
	DW_REG(DSTATAR);         /* Channel Destination Status Address Register */
	DW_REG(CFG);             /* Configuration Register */
	DW_REG(SGR);             /* Channel Source Gather Register */
	DW_REG(DSR);             /* Channel Destination Scatter Register */
} dw_dma_ch_regs_t;

typedef struct dw_dma_irq_regs {
	DW_REG(XFER);
	DW_REG(BLOCK);
	DW_REG(SRC_TRAN);
	DW_REG(DST_TRAN);
	DW_REG(ERROR);
} dw_dma_irq_regs_t;

typedef struct dw_dma_regs {
	dw_dma_ch_regs_t   CHAN[8];

	dw_dma_irq_regs_t  RAW;          /* Interrupt Raw Status Registers */
	dw_dma_irq_regs_t  STATUS;       /* Interrupt Status Registers */
	dw_dma_irq_regs_t  MASK;         /* Interrupt Mask Registers */
	dw_dma_irq_regs_t  CLEAR;        /* Interrupt Clear Registers */
	DW_REG(STATUS_INT);              /* Combined Interrupt Status Register */

	/* software handshaking */
	DW_REG(REQ_SRC);                 /* Source Software Transaction Request Register */
	DW_REG(REQ_DST);                 /* Destination Software Transaction Request Register */
	DW_REG(SGL_REQ_SRC);             /* Single Source Transaction Request Register */
	DW_REG(SGL_REQ_DST);             /* Single Destination Transaction Request Register */
	DW_REG(LAST_SRC);                /* Last Source Transaction Request Register */
	DW_REG(LAST_DST);                /* Last Destination Transaction Request Register */

	/* miscellaneous */
	DW_REG(CFG);                     /* DW_ahb_dmac Configuration Register */
	DW_REG(CH_EN);                   /* DW_ahb_dmac Channel Enable Register */
	DW_REG(ID);                      /* DW_ahb_dmac ID Register */
	DW_REG(TEST);                    /* DW_ahb_dmac Test Register */
	DW_REG(Reserved_1);
	DW_REG(Reserved_2);
	DW_REG(DMA_COMP_PARAMS_6);
	DW_REG(DMA_COMP_PARAMS_5);
	DW_REG(DMA_COMP_PARAMS_4);
	DW_REG(DMA_COMP_PARAMS_3);
	DW_REG(DMA_COMP_PARAMS_2);
	DW_REG(DMA_COMP_PARAMS_1);
	DW_REG(DMA_COMP_ID);
} dw_dma_regs_t;

typedef struct dw_lli {
	/* values that are not changed by hardware */
	uint32_t  sar;
	uint32_t  dar;
	uint32_t  llp;      // chain to next lli
	uint32_t  ctl[2];
	uint32_t  dstat;
} dw_lli_t;

/* Bitfields in CTL_LO */
#define DWC_CTLL_INT_EN         (1 << 0)    /* irqs enabled? */
#define DWC_CTLL_DST_WIDTH(n)   ((n)<<1)    /* bytes per element */
#define DWC_CTLL_SRC_WIDTH(n)   ((n)<<4)
#define DWC_CTLL_DST_INC        (0<<7)      /* DAR update/not */
#define DWC_CTLL_DST_DEC        (1<<7)
#define DWC_CTLL_DST_FIX        (2<<7)
#define DWC_CTLL_SRC_INC        (0<<9)      /* SAR update/not */
#define DWC_CTLL_SRC_DEC        (1<<9)
#define DWC_CTLL_SRC_FIX        (2<<9)
#define DWC_CTLL_DST_MSIZE(n)   ((n)<<11)   /* burst, #elements */
#define DWC_CTLL_SRC_MSIZE(n)   ((n)<<14)
#define DWC_CTLL_S_GATH_EN      (1 << 17)   /* src gather, !FIX */
#define DWC_CTLL_D_SCAT_EN      (1 << 18)   /* dst scatter, !FIX */
#define DWC_CTLL_FC(type)       ((type) << 20)/* flow controller */
#define DWC_CTLL_FC_M2M_DMA     (0 << 20)   /* mem-to-mem */
#define DWC_CTLL_FC_M2P_DMA     (1 << 20)   /* mem-to-periph */
#define DWC_CTLL_FC_P2M_DMA     (2 << 20)   /* periph-to-mem */
#define DWC_CTLL_FC_P2P_DMA     (3 << 20)   /* periph-to-periph */
#define DWC_CTLL_FC_P2M_P       (4 << 20)   /* periph-to-mem peripheral-as-flow-controller */
#define DWC_CTLL_FC_P2P_P       (5 << 20)   /* periph-to-periph peripheral-as-flow-controller */
#define DWC_CTLL_FC_M2P_P       (6 << 20)   /* mem-to-periph peripheral-as-flow-controller */
#define DWC_CTLL_DMS(n)         ((n)<<23)   /* dst master select */
#define DWC_CTLL_SMS(n)         ((n)<<25)   /* src master select */
#define DWC_CTLL_LLP_D_EN       (1 << 27)   /* dest block chain */
#define DWC_CTLL_LLP_S_EN       (1 << 28)   /* src block chain */

/* Bitfields in CTL_HI */
#define DWC_CTLH_DONE           0x00001000
#define DWC_CTLH_BLOCK_TS_MASK  0x00000fff

/* Bitfields in CFG_LO. Platform-configurable bits are in <dw_dmac.h> */
#define DWC_CFGL_CH_SUSP        (1 << 8)    /* pause xfer */
#define DWC_CFGL_FIFO_EMPTY     (1 << 9)    /* pause xfer */
#define DWC_CFGL_HS_DST         (1 << 10)   /* handshake w/dst */
#define DWC_CFGL_HS_SRC         (1 << 11)   /* handshake w/src */
#define DWC_CFGL_MAX_BURST(x)   ((x) << 20)
#define DWC_CFGL_RELOAD_SAR     (1 << 30)
#define DWC_CFGL_RELOAD_DAR     (1 << 31)

/* Bitfields in CFG_HI. Platform-configurable bits are in <dw_dmac.h> */
#define DWC_CFGH_FIFO_MODE      (1 << 1)
#define DWC_CFGH_DS_UPD_EN      (1 << 5)
#define DWC_CFGH_SS_UPD_EN      (1 << 6)
#define DWC_CFGH_SRC_PER(n)     ((n) << 7)
#define DWC_CFGH_DEST_PER(n)    ((n) << 11)


/* Bitfields in SGR */
#define DWC_SGR_SGI(x)          ((x) << 0)
#define DWC_SGR_SGC(x)          ((x) << 20)

/* Bitfields in DSR */
#define DWC_DSR_DSI(x)          ((x) << 0)
#define DWC_DSR_DSC(x)          ((x) << 20)

/* Bitfields in CFG */
#define DW_CFG_DMA_EN           (1 << 0)
#define DW_REGLEN               0x400


#define DWC_COM_CTRL0   DWC_CTLL_LLP_D_EN | DWC_CTLL_LLP_S_EN            |\
			DWC_CTLL_INT_EN

#endif
