#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "uart_sendboot.h"


#include <driver/gx_uart.h>
#include <driver/gx_timer.h>
#include <driver/gx_delay.h>
#include <driver/gx_gpio.h>

/*
 * xx_uart_init : uart 初始化
 *  return : 配置成功返回 0，配置失败返回负值
 */
int xx_uart_init(int baudrate)
{
    gx_uart_init(UPGRADE_PORT, baudrate);
    return 0;
}

/*
 * xx_uart_done : uart 资源释放
 *  return : 配置成功返回 0，配置失败返回负值
 */
int xx_uart_done(void)
{
    gx_uart_exit(UPGRADE_PORT);
    return 0;
}

/*
 * xx_uart_config :
 *  @baudrate : 波特率
 *  @databits : 数据位
 *  @stopbits : 停止位
 *  @parity : 奇偶校验
 *  return : 配置成功返回 0，配置失败返回负值
 */
int xx_uart_config(int baudrate)
{
    return gx_uart_init(UPGRADE_PORT, baudrate);
}

/*
 * xx_uart_read : 阻塞接收串口数据
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int xx_uart_read(uint8_t *buf, int len)
{
    return gx_uart_read(UPGRADE_PORT, buf, len);
}

/*
 * xx_uart_try_read : 尝试从串口中读取数据（立即返回）
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int xx_uart_try_read(uint8_t *buf, int len)
{
    return gx_uart_try_getc(UPGRADE_PORT, (char*)buf);
}

/*
 * xx_uart_read_nonblock : 超时从串口中读取数据
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  @timeout_ms : 超时时间设置
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int xx_uart_read_nonblock(uint8_t *buf, int len, int timeout_ms)
{
    return gx_uart_read_non_block(UPGRADE_PORT, buf, len, timeout_ms);
}

/*
 * xx_uart_write : 串口写数据
 *  @buf : 串口数据发送缓冲
 *  @len : 待发送数据的长度
 *  return : 返回串口实际发送的数据长度，出错则返回负值
 */
int xx_uart_write(const uint8_t *buf, int len)
{
    return gx_uart_write(UPGRADE_PORT, buf, len);;
}

/*
 * xx_msleep : 睡眠
 */
void xx_msleep(int ms)
{
    gx_mdelay(ms);
    return ;
}

/*
 * xx_get_time_ms : 获取当前系统时间
 */
uint64_t xx_get_time_ms(void)
{
    return gx_get_time_ms();;
}

/*
 * xx_reboot_chip : 重启 leo/leo mini
 */
void xx_reboot_chip(void)
{
    printf ("wait reboot the board...\n");
    gx_gpio_set_level(7,GX_GPIO_LEVEL_LOW);
    gx_mdelay(200);
    gx_gpio_set_level(7,GX_GPIO_LEVEL_HIGH);
    return ;
}
