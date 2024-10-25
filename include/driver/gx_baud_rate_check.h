/** \addtogroup <scpu>
 *  @{
 */
#ifndef __GX_BAUD_RATE_CHECK_H__
#define __GX_BAUD_RATE_CHECK_H__

/**
 * @brief baud rate check 模块检测结果
 */
typedef struct {
	int port;    ///< 哪路串口的测量结果
	int width;   ///< 接收 5 个 bit 串口数据经历的 cycle 数
	int timeout; ///< 0 : 未超时 1: 超时
} gx_brc_info_t;

/**
 * @brief 主要用来测量串口传输的波特率，依赖于上位机连续发送 0xef，当前接口会计算串口接收 5 个 bit
 * 数据时当前模块所经历的 cycle 数（当前模块跟串口模块同时钟）
 *
 * @param timeout_ms 接口超时配置
 * @param brc_info 统计结果会存放在这个结构体中(其中当 timeout 为 1 时表示统计超时，此时 width 不可用，timeout 为 0 时，width 可用), 详细说明请参考 gxdocref gx_brc_info_t
 *
 * @return int 接口调用结果
 * @retval 0 测量完成,测量结果见传入的结构体成员
 * @retval -1 传参有问题
 */
int gx_brc_start(int timeout_ms, gx_brc_info_t *brc_info);

/**
 * @brief 计算串口模块的输入时钟
 *
 * @param width gx_brc_start 接口调用后统计出来的 5 个 bit 数据所经历的 cycle 数
 * @param baud 串口发送 0xef 时使用的波特率
 *
 * @return int 串口模块的输入时钟
 */
int gx_brc_calculate_uart_freq(int width, int baud);

/**
 * @brief 基于前面调用 gx_brc_start 统计的结果，可以计算出我们 uart 的 div 和 dlf 以达到跟对方匹配的效果
 *
 * @param width gx_brc_start 接口调用后统计出来的 5 个 bit 数据所经历的 cycle 数
 * @param div 储存计算出来的 div
 * @param dlf 储存计算出来的 dlf
 *
 * @return int 返回接口调用结果
 * @retval 0 计算完成
 * @retval -1 计算出错
 */
int gx_brc_calculate_uart_div_dlf(int width, int *div, int *dlf);

#endif

/** @}*/
