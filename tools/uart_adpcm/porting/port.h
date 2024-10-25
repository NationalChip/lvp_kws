#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
 #include <unistd.h>
#include <sys/time.h>

#ifdef WINDOWS
#include <windows.h>
#include <time.h>
#include <sys/file.h>
#include <dirent.h>
#else
#include <termios.h>
#endif

/* The following API need to be ported */
/*
 * uart_config :
 *  @com : 串口设备
 *  @baudrate : 波特率
 *  @databits : 数据位
 *  @stopbits : 停止位
 *  @parity : 奇偶校验
 *  return : 配置成功返回 0，配置失败返回负值
 */
int uart_config(char *com, int baudrate, int databits, int stopbits, char parity);


#if 0
/*
 * uart_read_nonblock : 超时从串口中读取数据
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  @timeout_ms : 超时时间设置
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int uart_read_nonblock(uint8_t *buf, int timeout_ms);
/*
 * uart_try_read : 尝试从串口中读取数据（立即返回）
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int uart_try_read(uint8_t *buf, int size);
#endif
/*
 * get_time_ms : 获取当前系统时间
 */
uint64_t get_time_ms(void);


//================================================================================//
int sys_init(void);

/* crc32.c */
uint32_t crc32 (uint32_t crc, const unsigned char/*Bytef*/ *p, unsigned int/*uInt*/ len);

