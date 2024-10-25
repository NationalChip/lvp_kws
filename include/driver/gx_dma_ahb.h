/** \addtogroup <scpu>
 *  @{
 */
#ifndef __GX_DMA_AHB_H__
#define __GX_DMA_AHB_H__

/**
 * @brief dma一次传输(burst)多少个位宽单元
 */
typedef enum {
	GX_DMA_AHB_BURST_TRANS_LEN_1 = 0, ///< burst为1
	GX_DMA_AHB_BURST_TRANS_LEN_4,     ///< burst为4
	GX_DMA_AHB_BURST_TRANS_LEN_8,     ///< burst为8
	GX_DMA_AHB_BURST_TRANS_LEN_16,    ///< burst为16
	GX_DMA_AHB_BURST_TRANS_LEN_32,    ///< burst为32
	GX_DMA_AHB_BURST_TRANS_LEN_64,    ///< burst为64
	GX_DMA_AHB_BURST_TRANS_LEN_128,   ///< burst为128
	GX_DMA_AHB_BURST_TRANS_LEN_256,   ///< burst为256
	GX_DMA_AHB_BURST_TRANS_LEN_512,   ///< burst为512
	GX_DMA_AHB_BURST_TRANS_LEN_1024   ///< burst为1024
}GX_DMA_AHB_BURST_SIZE;

/**
 * @brief dma地址更新
 */
typedef enum {
	GX_DMA_AHB_CH_CTL_L_INC   = 0, ///< 地址递增
	GX_DMA_AHB_CH_CTL_L_NOINC      ///< 地址不变
}GX_DMA_AHB_ADDRESS_UPDATE;

/**
 * @brief dma选择握手信号
 */
typedef enum {
	GX_DMA_AHB_HS_SEL_HW = 0, ///< 硬件握手信号
	GX_DMA_AHB_HS_SEL_SW      ///< 软件握手信号
}GX_AHB_DMA_HANDSHAKE_SELECT;

/**
 * @brief dma选择流控方式
 */
typedef enum {
	GX_DMA_AHB_TT_FC_MEM_TO_MEM_DMAC = 0, ///< 内存到内存，dma流控
	GX_DMA_AHB_TT_FC_MEM_TO_PER_DMAC,     ///< 内存到外设，dma流控
	GX_DMA_AHB_TT_FC_PER_TO_MEM_DMAC,     ///< 外设到内存，dma流控
	GX_DMA_AHB_TT_FC_PER_TO_PER_DMAC,     ///< 外设到外设，dma流控
	GX_DMA_AHB_TT_FC_PER_TO_MEM_SRC,      ///< 外设到内存，外设流控
	GX_DMA_AHB_TT_FC_PER_TO_PER_SRC,      ///< 外设到外设，外设流控
	GX_DMA_AHB_TT_FC_MEM_TO_PER_DST,      ///< 内存到外设，外设流控
	GX_DMA_AHB_TT_FC_PER_TO_PER_DST       ///< 外设到外设，外设流控
}GX_DMA_AHB_FLOW_CONTROL;

/**
 * @brief dma操作位宽
 */
typedef enum {
	GX_DMA_AHB_TRANS_WIDTH_8 = 0, ///< 位宽8bit
	GX_DMA_AHB_TRANS_WIDTH_16,    ///< 位宽16bit
	GX_DMA_AHB_TRANS_WIDTH_32,    ///< 位宽32bit
	GX_DMA_AHB_TRANS_WIDTH_64,    ///< 位宽64bit
	GX_DMA_AHB_TRANS_WIDTH_128,   ///< 位宽128bit
	GX_DMA_AHB_TRANS_WIDTH_256,   ///< 位宽256bit
	GX_DMA_AHB_TRANS_WIDTH_512,   ///< 位宽512bit
	GX_DMA_AHB_TRANS_WIDTH_MAX = GX_DMA_AHB_TRANS_WIDTH_512 ///< 位宽512bit
}GX_DMA_AHB_TRANSFRE_WIDTH;

/**
 * @brief dma master
 */
typedef enum {
	GX_DMA_AHB_MASTER_1, ///< master 1
	GX_DMA_AHB_MASTER_2, ///< master 2
	GX_DMA_AHB_MASTER_3, ///< master 3
	GX_DMA_AHB_MASTER_4  ///< master 4
}GX_DMA_AHB_MASTER_SELECT;

/**
 * @brief dma 通道配置
 */
typedef struct {
	GX_DMA_AHB_TRANSFRE_WIDTH trans_width;       ///< dma操作位宽

	GX_DMA_AHB_BURST_SIZE src_msize;             ///< dma 源通道burst
	GX_DMA_AHB_ADDRESS_UPDATE src_addr_update;   ///< dma 源通道地址更新方式
	unsigned int src_hs_per;                     ///< dma 源通道握手信号值
	GX_DMA_AHB_MASTER_SELECT src_master_select;  ///< dma 源通道master选择
	GX_AHB_DMA_HANDSHAKE_SELECT src_hs_select;   ///< dma 源通道握手信号选择

	GX_DMA_AHB_BURST_SIZE dst_msize;             ///< dma 目的通道burst
	GX_DMA_AHB_ADDRESS_UPDATE dst_addr_update;   ///< dma 目的通道地址更新方式
	unsigned int dst_hs_per;                     ///< dma 目的通道握手信号值
	GX_DMA_AHB_MASTER_SELECT dst_master_select;  ///< dma 目的通道master选择
	GX_AHB_DMA_HANDSHAKE_SELECT dst_hs_select;   ///< dma 目的通道握手信号选择

	GX_DMA_AHB_FLOW_CONTROL flow_ctrl;           ///< dma 流控方式
}GX_DMA_AHB_CH_CONFIG;

/**
 * @brief dma初始化
 */
void gx_dma_init(void);

/**
 * @brief 获取dma空闲通道
 *
 * @return int 空闲的dma通道
 * @retval -1 无空闲dma通道
 * @retval 非-1 空闲dma通道值
 */
int gx_dma_select_channel(void);

/**
 * @brief 开始一次dma传输，使用poll模式, 不阻塞等待完成
 * @param dst 目的地址
 * @param src 源地址
 * @param len 传输dma位宽单元数量，例如:width=32bit, len=5, 4*5=20 bytes 将被传输
 * @param channel dma空闲通道
 * @param config  dma通道配置, 详细说明请参考 gxdocref GX_DMA_AHB_CH_CONFIG
 *
 * @return int dma启动传输是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_dma_xfer(void *dst, void *src, int len, int channel, GX_DMA_AHB_CH_CONFIG *config);

/**
 * @brief 等待一次dma传输完成
 *
 * @param channel dma通道
 *
 * @return int dma传输是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_dma_wait_complete(int channel);

/**
 * @brief 等待一次dma传输完成,不关闭dma
 *
 * @param channel dma通道
 *
 * @return int dma传输是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_dma_wait_complete_norelease(int channel);

/**
 * @brief 关闭dma
 *
 * @param channel dma通道
 */
void gx_dma_release_channel(int channel);

/**
 * @brief 异常停止一次dma传输
 *
 * @param channel dma通道
 *
 * @return int dma是否停止成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_dma_channel_abort(int channel);

/**
 * @brief 超时等待一次dma传输完成
 *
 * @param channel dma通道
 * @param timeout_ms 超时毫秒
 *
 * @return int dma传输是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_dma_wait_complete_timeout(int channel, unsigned int timeout_ms);

/**
 * @brief 开始一次dma传输，使用poll模式,并阻塞等待完成
 *
 * @param dst 目的地址
 * @param src 源地址
 * @param len 传输dma位宽单元数量，例如:width=32bit, len=5, 4*5=20 bytes 将被传输
 * @param channel dma空闲通道
 * @param config  dma通道配置,详细说明请参考 gxdocref GX_DMA_AHB_CH_CONFIG
 *
 * @return int dma传输是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_dma_xfer_poll(void *dst, void *src, int len, int channel, GX_DMA_AHB_CH_CONFIG *config);

/**
 * @brief 注册dma 通道回调函数，使用gx_dma_xfer_int()接口完成传输时，会调用回调函数
 *
 * @param channel dma空闲通道
 * @param func dma 回调函数,类型为 void func(void *param)
 * @param param dma 回调函数参数
 */
void gx_dma_register_complete_callback(int channel, void *func, void *param);

/**
 * @brief 开始一次dma传输，使用中断模式,不阻塞等待完成,完成以后调用通道完成回调回调函数
 *
 * @param dst 目的地址
 * @param src 源地址
 * @param len 传输dma位宽单元数量，例如:width=32bit, len=5, 4*5=20 bytes 将被传输
 * @param channel dma空闲通道
 * @param config  dma通道配置, 详细说明请参考 gxdocref GX_DMA_AHB_CH_CONFIG
 *
 * @return int dma传输是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_dma_xfer_int(void *dst, void *src, int len, int channel, GX_DMA_AHB_CH_CONFIG *config);


#endif

/** @}*/