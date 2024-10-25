#ifndef UART_SENDBOOT_H
#define UART_SENDBOOT_H

#include <stdint.h>
#include "porting.h"


/*
*
*download_init：下载固件初始化
*@pkg_size:     每个包的大小，4096的倍数
*@flash_addr:   flash地址
*@pkg_buff:     固件buff
*
*/
int DownloadInit(int32_t pkg_size);                                     // 下载初始化

/**
 * @brief 下载数据到目标芯片
 *
 * @param flash_addr    目标flash地址
 * @param pkg_buff      待下载数据地址
 *
 * @return uint32_t 中断状态
 */
int DownloadFirmware(uint32_t flash_addr, unsigned char *pkg_buff);     // 下载固件

/**
 * @brief 重启目标芯片
 */
int DownloadReboot(void);                                               // 下载完成

#endif
