/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_UART_H__
#define __GX_UART_H__

#include <common.h>

#if defined(CONFIG_ARCH_LEO) || defined(CONFIG_ARCH_LEO_MINI) || defined(CONFIG_ARCH_GRUS)
# define UART_PORT_MAX 2
#else
# define UART_PORT_MAX CONFIG_UART_PORT_NUMBER
#endif

#define GX_UART_PORT0 0
#define GX_UART_PORT1 1
#define GX_UART_PORT2 2

/* Generic uart API */
/**
 * @brief 串口初始化
 *   只需要调用一次，且串口在时候过程中不能调用
 *
 * @param port 端口
 * @param baudrate 波特率, 支持标准波特率， 一般为 115200
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_init(int port, uint32_t baudrate);

/**
 * @brief 串口退出
 *
 * @param port 端口
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_exit(int port);

/**
 * @brief 输出一个字符
 *
 * @param port 端口
 * @param ch 要输出的字符
 */
void gx_uart_putc(int port, int ch);

/**
 * @brief 输出一个字符, 如果是换行,转换成回车输出
 *
 * @param port 端口
 * @param ch 要输出的字符
 */
void gx_uart_compatible_putc(int port, int ch);

/**
 * @brief 清空串口 fifo
 *
 * @param port 端口
 */
void gx_uart_empty_fifo(int port);

/**
 * @brief 获取串口 fifo 深度
 *
 * @param port 端口
 */
int gx_uart_get_fifo_depth(int port);

/**
 * @brief 获取串口 dlf size
 *
 * @param port 端口
 */
int gx_uart_get_dlf_size(int port);

/**
 * @brief 获取串口 IP 版本
 *
 * @param port 端口
 */
int gx_uart_get_ip_version(int port);

/**
 * @brief 获取一个字符
 *
 * @param port 端口
 *
 * @return int 获取的字符
 */
int gx_uart_getc(int port);

/**
 * @brief 尝试获取一个字符，如果没有获取到字符，立马返回
 *
 * @param port 端口
 * @param c 获取到的字符
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_try_getc(int port, char *c);

/**
 * @brief 判断 fifo 中是否有数据
 *
 * @param port 端口
 * @return int 结果
 * @retval 0 没有数据
 * @retval 1 有数据
 */
int gx_uart_can_read(int port);

/**
 * @brief 同步读取指定长度的数据
 *        如果没有读到指定的数据，会一直等待
 *
 * @param port 端口
 * @param buf 读取到的数据保存buf
 * @param len 数据长度
 * @return int 实际读取到的数据长度
 */
int gx_uart_read(int port, unsigned char *buf, int len);

/**
 * @brief 读取指定长度的数据, 指定时间内没有读到指定长度的数据，立即返回
 *
 * @param port 端口
 * @param buf 读取到的数据保存buf
 * @param len 数据长度
 * @param timeout_ms 超时时间
 * @return int 实际读取到的数据长度
 */
int gx_uart_read_non_block(int port, unsigned char *buf, int len, unsigned int timeout_ms);


/**
 * @brief 同步写指定长度的数据
 *        以阻塞的方式写入 len
 *
 * @param port 端口
 * @param buf 需要写出的数据buf
 * @param len 数据长度
 * @return int 实际写出数据长度
 */
int gx_uart_write(int port, const unsigned char *buf, int len);

/**
 * @brief 串口暂停
 *
 * @param port 端口
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_suspend(int port);

/**
 * @brief 串口恢复
 *
 * @param port 端口
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_resume(int port);

// FLOW Related

/**
 * @brief 开启串口流控
 *
 * @param port 端口
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_flow_on(int port);

/**
 * @brief 关闭串口流控
 *
 * @param port 端口
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_flow_off(int port);

/**
 * @brief 设置串口工作频率
 *   设置串口的输入频率，调用该接口后要调用gx_uart_init重新初始化串口
 *
 * @param port 端口
 * @param freq 频率
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_set_freq(int port, int freq);

//-------------------------------------------------------------------------------------------------
// FIFO Related

/**
 * @brief 串口发送 fifo 门限值
 */
typedef enum {
    GX_UART_FIFO_SEND_THRESHOLD_EMPTY,      ///< empty　
    GX_UART_FIFO_SEND_THRESHOLD_2,          ///< 2 byte　
    GX_UART_FIFO_SEND_THRESHOLD_QUARTER,    ///< 25%　
    GX_UART_FIFO_SEND_THRESHOLD_HALF,       ///< 50%　
} GX_UART_FIFO_SEND_THRESHOLD;

/**
 * @brief 串口接收 fifo 门限值
 */
typedef enum {
    GX_UART_FIFO_RECV_THRESHOLD_ONE_BYTE,   ///< 1 byte
    GX_UART_FIFO_RECV_THRESHOLD_QUARTER,    ///< 25%
    GX_UART_FIFO_RECV_THRESHOLD_HALF,       ///< 50%
    GX_UART_FIFO_RECV_THRESHOLD_FULL,       ///< Almost Full
} GX_UART_FIFO_RECV_THRESHOLD;

/**
 * @brief 设置串口发送 fifo 中断触发门限
 *
 * @param port 端口
 * @param threshold 门限值, 详细说明请参考 gxdocref GX_UART_FIFO_SEND_THRESHOLD
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_set_send_fifo_threshold(int port, GX_UART_FIFO_SEND_THRESHOLD threshold);

/**
 * @brief 设置接收 fio 中断触发门限
 *
 * @param port 端口
 * @param threshold 门限值, 详细说明请参考 gxdocref GX_UART_FIFO_RECV_THRESHOLD
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_set_recv_fifo_threshold(int port, GX_UART_FIFO_RECV_THRESHOLD threshold);

/**
 * @brief 清空发送 fifo
 *
 * @param port 端口
 */
void gx_uart_flush_send_fifo(int port);

/**
 * @brief 清空接收 fifo
 *
 * @param port 端口
 */
void gx_uart_flush_recv_fifo(int port);


//-------------------------------------------------------------------------------------------------
// Async API Mode 1 (Interrupt Mode)

typedef int (*UART_CAN_SEND_CALLBACK)(int port, int length, void *priv);
typedef int (*UART_CAN_RECV_CALLBACK)(int port, int length, void *priv);

/**
 * @brief 开始异步接收串口数据
 *
 * @param port 端口
 * @param callback 串口有数据可以接收回调函数
 * @param priv 回调函数参数
 * @return int 函数是否执行成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_start_async_send(int port, UART_CAN_SEND_CALLBACK callback, void *priv);

/**
 * @brief 停止异步接收串口数据
 *
 * @param port 端口
 * @return int 函数是否执行成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_stop_async_send(int port);

/**
 * @brief 开始异步发送串口数据
 *
 * @param port 端口
 * @param callback 可以发送串口数据回调函数
 * @param priv 回调函数参数
 * @return int 函数是否执行成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_start_async_recv(int port, UART_CAN_RECV_CALLBACK callback, void *priv);

/**
 * @brief 停止异步发送串口数据
 *
 * @param port 端口
 * @return int 函数是否执行成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_stop_async_recv(int port);

// No check FIFO status in these function
/**
 * @brief 发送串口数据， 但是不检查 fifo 是否满，
 * 　　　　和　gx_uart_start_async_send 配合使用
 *
 * @param port 端口
 * @param buffer 要发送的数据
 * @param length 数据长度
 * @return int 实际发送数据长度
 */
int gx_uart_send_buffer(int port, unsigned char *buffer, int length);

/**
 * @brief 接收串口数据，　但是不检查 fifo 中是否有数据
 * 　　　　和 gx_uart_start_async_recv 函数配合使用
 *
 * @param port 端口
 * @param buffer 接收数据 buffer
 * @param length 数据长度
 * @return int 实际接收数据长度
 */
int gx_uart_recv_buffer(int port, unsigned char *buffer, int length);

//-------------------------------------------------------------------------------------------------
// Async API Mode 2 (DMA Mode)
//   In Leo, it is implemented by interrupt mode

typedef int (*UART_SEND_DONE_CALLBACK)(int port, void *priv);
typedef int (*UART_RECV_DONE_CALLBACK)(int port, void *priv);

/**
 * @brief 异步发送串口数据
 *
 * @param port 端口
 * @param buffer 需要发送的数据
 * @param length 数据长度
 * @param callback 数据发送完成回调
 * @param priv 回调参数
 * @return int 函数是否执行成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_async_send_buffer(int port, unsigned char *buffer, int length, UART_SEND_DONE_CALLBACK callback, void *priv);

/**
 * @brief 异步接收串口数据
 *
 * @param port 端口
 * @param buffer 接收数据 buffer
 * @param length 接收数据的长度
 * @param callback 数据接收完成回调
 * @param priv 回调参数
 * @return int 函数是否执行成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_async_recv_buffer(int port, unsigned char *buffer, int length, UART_RECV_DONE_CALLBACK callback, void *priv);

/**
 * @brief 停止串口异步发送接口
 *
 * @param port 端口
 * @return int 函数是否执行成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_async_send_buffer_stop(int port);

/**
 * @brief 停止串口异步接收接口
 *
 * @param port 端口
 * @return int 函数是否执行成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_uart_async_recv_buffer_stop(int port);

/* Console API */

/**
 * @brief 控制台初始化
 *
 * @param port 端口
 * @param baudrate 波特率
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_console_init(int port, uint32_t baudrate);

/**
 * @brief 控制台退出
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_console_exit(void);

/**
 * @brief 从控制台输出一个字符
 *
 * @param ch 要输出的字符
 */
void gx_console_putc(int ch);

/**
 * @brief 从控制台输出一个字符, 如果是换行,转换成回车输出
 *
 * @param ch 要输出的字符
 */
void gx_console_compatible_putc(int ch);

/**
 * @brief 清空控制台 fifo
 */
void gx_console_empty_fifo(void);

/**
 * @brief 从控制台获取一个字符
 *
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_console_getc(void);

/**
 * @brief 从控制台尝试获取一个字符，如果没有获取到字符，立马返回
 *
 * @param c 获取到的字符
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_console_try_getc(char *c);

/**
 * @brief 判断控制台 fifo 中是否有数据
 *
 * @return int 结果
 * @retval 0 没有数据
 * @retval 1 有数据
 */
int gx_console_can_read(void);

/**
 * @brief 控制台暂停
 *    需要在串口空闲时调用
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_console_suspend(void);

/**
 * @brief 控制台恢复
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_console_resume(void);

#endif

/** @}*/
