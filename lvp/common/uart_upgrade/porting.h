#ifndef PORTING_H
#define PORTING_H

#include <stdint.h>


#define UPGRADE_PORT 1
/* The following API need to be ported */

/*
 * xx_uart_init : uart 初始化
 *  return : 配置成功返回 0，配置失败返回负值
 */
int32_t xx_uart_init(int baudrate);

/*
 * xx_uart_done : uart 资源释放
 *  return : 配置成功返回 0，配置失败返回负值
 */
int32_t xx_uart_done(void);

/*
 * xx_uart_config : 配置 uart 的波特率等信息
 *  @baudrate : 波特率
 *  @databits : 数据位
 *  @stopbits : 停止位
 *  @parity : 奇偶校验
 *  return : 配置成功返回 0，配置失败返回负值
 */
int32_t xx_uart_config(int32_t baudrate);

/*
 * xx_uart_read : 阻塞接收串口数据
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int32_t xx_uart_read(uint8_t *buf, int32_t len);

/*
 * xx_uart_try_read : 尝试从串口中读取数据（立即返回）
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int32_t xx_uart_try_read(uint8_t *buf, int32_t len);

/*
 * xx_uart_read_nonblock : 超时从串口中读取数据
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  @timeout_ms : 超时时间设置
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int32_t xx_uart_read_nonblock(uint8_t *buf, int32_t len, int32_t timeout_ms);

/*
 * xx_uart_write : 串口写数据
 *  @buf : 串口数据发送缓冲
 *  @len : 待发送数据的长度
 *  return : 返回串口实际发送的数据长度，出错则返回负值
 */
int32_t xx_uart_write(const uint8_t *buf, int32_t len);

/*
 * xx_msleep : 睡眠
 */
void xx_msleep(int32_t ms);

/*
 * xx_get_time_ms : 获取当前系统时间
 */
uint64_t xx_get_time_ms(void);

/*
 * xx_reboot_chip : 重启 leo/leo mini
 */
void xx_reboot_chip(void);

#endif
