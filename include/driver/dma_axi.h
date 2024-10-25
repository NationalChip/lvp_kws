/*!
@file dmac_axi.h
@brief Describes dma api
*/
#ifndef __DMA_AXI_H__
#define __DMA_AXI_H__

enum {
	DWAXIDMAC_ARWLEN_1      = 0,
	DWAXIDMAC_ARWLEN_2      = 1,
	DWAXIDMAC_ARWLEN_4      = 3,
	DWAXIDMAC_ARWLEN_8      = 7,
	DWAXIDMAC_ARWLEN_16     = 15,
	DWAXIDMAC_ARWLEN_32     = 31,
	DWAXIDMAC_ARWLEN_64     = 63,
	DWAXIDMAC_ARWLEN_128    = 127,
	DWAXIDMAC_ARWLEN_256    = 255,
	DWAXIDMAC_ARWLEN_MIN    = DWAXIDMAC_ARWLEN_1,
	DWAXIDMAC_ARWLEN_MAX    = DWAXIDMAC_ARWLEN_256
};

/* CH_CTL_L */
#define CH_CTL_L_LAST_WRITE_EN      (DMA_BIT(30))

#define CH_CTL_L_DST_MSIZE_POS      18
#define CH_CTL_L_SRC_MSIZE_POS      14
#define CH_CTL_L_DST_MSIZE_MASK        (GENMASK(21,18))
#define CH_CTL_L_SRC_MSIZE_MASK        (GENMASK(17,14))

typedef enum {
	DWAXIDMAC_BURST_TRANS_LEN_1 = 0,
	DWAXIDMAC_BURST_TRANS_LEN_4,
	DWAXIDMAC_BURST_TRANS_LEN_8,
	DWAXIDMAC_BURST_TRANS_LEN_16,
	DWAXIDMAC_BURST_TRANS_LEN_32,
	DWAXIDMAC_BURST_TRANS_LEN_64,
	DWAXIDMAC_BURST_TRANS_LEN_128,
	DWAXIDMAC_BURST_TRANS_LEN_256,
	DWAXIDMAC_BURST_TRANS_LEN_512,
	DWAXIDMAC_BURST_TRANS_LEN_1024
}BURST_SIZE;

#define CH_CTL_L_DST_WIDTH_POS      11
#define CH_CTL_L_SRC_WIDTH_POS      8
#define CH_CTL_L_DST_WIDTH_MASK     (GENMASK(13,11))
#define CH_CTL_L_SRC_WIDTH_MASK     (GENMASK(10,8))

#define CH_CTL_L_DST_INC_POS        6
#define CH_CTL_L_SRC_INC_POS        4
#define CH_CTL_L_DST_INC_POS_MASK   (DMA_BIT(6))
#define CH_CTL_L_SRC_INC_POS_MASK   (DMA_BIT(4))

typedef enum {
	DWAXIDMAC_CH_CTL_L_INC   = 0,
	DWAXIDMAC_CH_CTL_L_NOINC
}ADDRESS_UPDATE;

#define CH_CTL_L_DST                     2
#define CH_CTL_L_SRC                     0
#define CH_CTL_L_DST_MASK       (DMA_BIT(2))
#define CH_CTL_L_SRC_MASK       (DMA_BIT(0))

/* CH_CFG_H */
#define CH_CFG_L_PRIORITY_POS             47
#define CH_CFG_L_PRIORITY_POS_MASK        (DMA_BIT(47))
#define CH_CFG_L_HS_SEL_DST_POS           36
#define CH_CFG_L_HS_SEL_DST_POS_MASK      (DMA_BIT(36))
#define CH_CFG_L_HS_SEL_SRC_POS           35
#define CH_CFG_L_HS_SEL_SRC_POS_MASK      (DMA_BIT(35))

typedef enum {
	DWAXIDMAC_HS_SEL_HW = 0,
	DWAXIDMAC_HS_SEL_SW
}HANDSHAKE_SELECT;

#define CH_CFG_L_TT_FC_POS         32
#define CH_CFG_L_TT_FC_MASK        (GENMASK(34,32))

#define CH_CFG_L_SRC_PER_POS       4
#define CH_CFG_L_DST_PER_POS       11
#define CH_CFG_L_SRC_PER_MASK            (GENMASK(8,4))
#define CH_CFG_L_DST_PER_MASK            (GENMASK(15,11))


#define DMA_BIT(n)                  (1ULL << n)

#define GENMASK(h, l)       ((((1ULL) << ((h) - (l) + 1)) - 1) << (l))

typedef enum {
	DWAXIDMAC_TT_FC_MEM_TO_MEM_DMAC = 0,
	DWAXIDMAC_TT_FC_MEM_TO_PER_DMAC,
	DWAXIDMAC_TT_FC_PER_TO_MEM_DMAC,
	DWAXIDMAC_TT_FC_PER_TO_PER_DMAC,
	DWAXIDMAC_TT_FC_PER_TO_MEM_SRC,
	DWAXIDMAC_TT_FC_PER_TO_PER_SRC,
	DWAXIDMAC_TT_FC_MEM_TO_PER_DST,
	DWAXIDMAC_TT_FC_PER_TO_PER_DST
}FLOW_CONTROL;

/* CH_CFG_L */
#define CH_CFG_L_DST_MULTBLK_TYPE_POS   2
#define CH_CFG_L_SRC_MULTBLK_TYPE_POS   0
enum {
	DWAXIDMAC_MBLK_TYPE_CONTIGUOUS = 0,
	DWAXIDMAC_MBLK_TYPE_RELOAD,
	DWAXIDMAC_MBLK_TYPE_SHADOW_REG,
	DWAXIDMAC_MBLK_TYPE_LL
};

/**
 * DW AXI DMA channel interrupts
 *
 * @DWAXIDMAC_IRQ_NONE: Bitmask of no one interrupt
 * @DWAXIDMAC_IRQ_BLOCK_TRF: Block transfer complete
 * @DWAXIDMAC_IRQ_DMA_TRF: Dma transfer complete
 * @DWAXIDMAC_IRQ_SRC_TRAN: Source transaction complete
 * @DWAXIDMAC_IRQ_DST_TRAN: Destination transaction complete
 * @DWAXIDMAC_IRQ_SRC_DEC_ERR: Source decode error
 * @DWAXIDMAC_IRQ_DST_DEC_ERR: Destination decode error
 * @DWAXIDMAC_IRQ_SRC_SLV_ERR: Source slave error
 * @DWAXIDMAC_IRQ_DST_SLV_ERR: Destination slave error
 * @DWAXIDMAC_IRQ_LLI_RD_DEC_ERR: LLI read decode error
 * @DWAXIDMAC_IRQ_LLI_WR_DEC_ERR: LLI write decode error
 * @DWAXIDMAC_IRQ_LLI_RD_SLV_ERR: LLI read slave error
 * @DWAXIDMAC_IRQ_LLI_WR_SLV_ERR: LLI write slave error
 * @DWAXIDMAC_IRQ_INVALID_ERR: LLI invalid error or Shadow register error
 * @DWAXIDMAC_IRQ_MULTIBLKTYPE_ERR: Slave Interface Multiblock type error
 * @DWAXIDMAC_IRQ_DEC_ERR: Slave Interface decode error
 * @DWAXIDMAC_IRQ_WR2RO_ERR: Slave Interface write to read only error
 * @DWAXIDMAC_IRQ_RD2RWO_ERR: Slave Interface read to write only error
 * @DWAXIDMAC_IRQ_WRONCHEN_ERR: Slave Interface write to channel error
 * @DWAXIDMAC_IRQ_SHADOWREG_ERR: Slave Interface shadow reg error
 * @DWAXIDMAC_IRQ_WRONHOLD_ERR: Slave Interface hold error
 * @DWAXIDMAC_IRQ_LOCK_CLEARED: Lock Cleared Status
 * @DWAXIDMAC_IRQ_SRC_SUSPENDED: Source Suspended Status
 * @DWAXIDMAC_IRQ_SUSPENDED: Channel Suspended Status
 * @DWAXIDMAC_IRQ_DISABLED: Channel Disabled Status
 * @DWAXIDMAC_IRQ_ABORTED: Channel Aborted Status
 * @DWAXIDMAC_IRQ_ALL_ERR: Bitmask of all error interrupts
 * @DWAXIDMAC_IRQ_ALL: Bitmask of all interrupts
 */

#define DWAXIDMAC_IRQ_NONE             0
#define DWAXIDMAC_IRQ_BLOCK_TRF        DMA_BIT(0)
#define DWAXIDMAC_IRQ_DMA_TRF          DMA_BIT(1)
#define DWAXIDMAC_IRQ_SRC_TRAN         DMA_BIT(3)
#define DWAXIDMAC_IRQ_DST_TRAN         DMA_BIT(4)
#define DWAXIDMAC_IRQ_SRC_DEC_ERR      DMA_BIT(5)
#define DWAXIDMAC_IRQ_DST_DEC_ERR      DMA_BIT(6)
#define DWAXIDMAC_IRQ_SRC_SLV_ERR      DMA_BIT(7)
#define DWAXIDMAC_IRQ_DST_SLV_ERR      DMA_BIT(8)
#define DWAXIDMAC_IRQ_LLI_RD_DEC_ERR   DMA_BIT(9)
#define DWAXIDMAC_IRQ_LLI_WR_DEC_ERR   DMA_BIT(10)
#define DWAXIDMAC_IRQ_LLI_RD_SLV_ERR   DMA_BIT(11)
#define DWAXIDMAC_IRQ_LLI_WR_SLV_ERR   DMA_BIT(12)
#define DWAXIDMAC_IRQ_INVALID_ERR      DMA_BIT(13)
#define DWAXIDMAC_IRQ_MULTIBLKTYPE_ERR DMA_BIT(14)
#define DWAXIDMAC_IRQ_DEC_ERR          DMA_BIT(16)
#define DWAXIDMAC_IRQ_WR2RO_ERR        DMA_BIT(17)
#define DWAXIDMAC_IRQ_RD2RWO_ERR       DMA_BIT(18)
#define DWAXIDMAC_IRQ_WRONCHEN_ERR     DMA_BIT(19)
#define DWAXIDMAC_IRQ_SHADOWREG_ERR    DMA_BIT(20)
#define DWAXIDMAC_IRQ_WRONHOLD_ERR     DMA_BIT(21)
#define DWAXIDMAC_IRQ_LOCK_CLEARED     DMA_BIT(27)
#define DWAXIDMAC_IRQ_SRC_SUSPENDED    DMA_BIT(28)
#define DWAXIDMAC_IRQ_SUSPENDED        DMA_BIT(29)
#define DWAXIDMAC_IRQ_DISABLED         DMA_BIT(30)
#define DWAXIDMAC_IRQ_ABORTED          DMA_BIT(31)
#define DWAXIDMAC_IRQ_ALL_ERR          (GENMASK(21, 16) | GENMASK(14, 5))
#define DWAXIDMAC_IRQ_ALL              GENMASK(31, 0)

typedef enum {
	DWAXIDMAC_TRANS_WIDTH_8     = 0,
	DWAXIDMAC_TRANS_WIDTH_16,
	DWAXIDMAC_TRANS_WIDTH_32,
	DWAXIDMAC_TRANS_WIDTH_64,
	DWAXIDMAC_TRANS_WIDTH_128,
	DWAXIDMAC_TRANS_WIDTH_256,
	DWAXIDMAC_TRANS_WIDTH_512,
	DWAXIDMAC_TRANS_WIDTH_MAX = DWAXIDMAC_TRANS_WIDTH_512
}TRANSFRE_WIDTH;

typedef enum {
	AXI_MASTER_1,
	AXI_MASTER_2
}AXI_MASTER_SELECT;

typedef enum {
	FLASH_SPI_RX   = 0,
	FLASH_SPI_TX,
	SPI_MASTER_RX,
	SPI_MASTER_TX,
	UART1_RX,
	UART1_TX,
	UART0_RX,
	UART0_TX
}HS_PER;

/**
@addtogroup dma axi api
@{
@brief AXI DMA API description
*/

typedef struct dw_dmac_ch_config {
	TRANSFRE_WIDTH src_trans_width;
	BURST_SIZE src_msize;
	ADDRESS_UPDATE src_addr_update;
	HS_PER src_per;
	AXI_MASTER_SELECT src_master_select;
	HANDSHAKE_SELECT src_hs_select;

	TRANSFRE_WIDTH dst_trans_width;
	BURST_SIZE dst_msize;
	ADDRESS_UPDATE dst_addr_update;
	HS_PER dst_per;
	AXI_MASTER_SELECT dst_master_select;
	HANDSHAKE_SELECT dst_hs_select;

	FLOW_CONTROL flow_ctrl;
}DW_DMAC_CH_CONFIG;

/**
@brief select unused dma channel
@return -1 for no available channel \n
        0-5 for available channel
*/
int dw_dma_select_channel(void);

/**
@brief release used dma channel
@param dma_channel: release dma channel number
@return return void
*/
void dw_dma_release_channel(int channel);

/**
@brief setup dma configs before dma transfer occur
@param dma_channel: selected dma channel for this dma transfer
@param config: dma configs, gxdocref DW_DMAC_CH_CONFIG
@return 0 for no error \n
        nagative value for error code
*/
int dw_dma_channel_config(int dma_channel, DW_DMAC_CH_CONFIG config);

/**
@brief start a dma tranfer without poll wait
@param dst: destination address
@param src: source address
@param len: total number of src_tr_width, for example: src_tr_width=32bit, len=4, 32*4=128 bytes will be transfered
@param dma_channel: selected dma channel for this dma transfer
@return 0 for success
*/
int dw_dma_xfer(void *dst, void *src, int len, int dma_channel);

/**
@brief wait a dma tranfer complete
@param dma_channel: selected dma channel for this dma transfer
@return 0 for success
*/
int dw_dma_wait_complete(int dma_channel);

/**
@brief wait a dma tranfer complete with timeout
@param dma_channel: selected dma channel for this dma transfer
@param timeout_ms: timeout in ms
@return 0 for success
       -1 for timeout
*/
int dw_dma_wait_complete_timeout(int dma_channel, unsigned int timeout_ms);

/**
@brief start a dma tranfer using poll mode, dw_dma_xfer_poll = dw_dma_xfer + dw_dma_wait_complete
@param dst: destination address
@param src: source address
@param len: total number of src_tr_width, for example: src_tr_width=32bit, len=4, 32*4=128 bytes will be transfered
@param dma_channel: selected dma channel for this dma transfer
@return 0 for success
*/
int dw_dma_xfer_poll(void *dst, void *src, int len, int dma_channel);

/**
@brief start a dma tranfer using poll mode
@param dst: destination address
@param src: source address
@param len: total number of src_tr_width, for example: src_tr_width=32bit, len=4, 32*4=128 bytes will be transfered
@param dma_channel: selected dma channel for this dma transfer
@return 0 for success
*/
int dw_dma_xfer_int(void *dst, void *src, int len, int dma_channel);

/**
@brief app can register a function to know when the dma transfer finishes
@param dma_channel: selected dma channel for this dma transfer
@param func: function pointer to the callback function
@param para: the param of callback function
@return return void
*/
void dw_dma_register_complete_callback(int dma_channel, void *func, void *para);

/**
@brief enable/disable debug mode
@param debug: 0 disable printf,  1  enable printf
@return return void
*/
void dw_dmac_debug(int debug);

/**
@brief initial or reset dma module
@return return void
*/
void dw_dmac_init(void);

#endif
