#ifndef __MISC_REGS_H__
#define __MISC_REGS_H__

#include <types.h>
#include "base_addr.h"


/*
    该寄存器记录了启动的方式及该方式的相关信息
    bit [3 ~ 0] : boot_medium
        0 : uart 启动
        1 : i2c  启动
        2 : nor flash 启动
        3 : nand flash 启动
        4 : usb 启动

    bit [7 ~ 4]（根据前面的启动方式，该区域会有不同的含义）: boot_info
        从 uart 启动时，表示这片区域表示从哪一路 uart 启动
        从 i2c  启动时，表示这片区域表示从哪一路 uart 启动
        从 nor flash 启动时，3 表示从 3 字节 spi nor 启动，4 表示从 4 字节 spi nor 启动

    bit [31 ~ 8]：
        从 Flash 启动时的 Flash 启动地址（256 字节对齐）
*/

enum {
    BOOT_MEDIUM_UART = 0,
    BOOT_MEDIUM_I2C,
    BOOT_MEDIUM_NOR_FLASH,
    BOOT_MEDIUM_NAND_FLASH,
    BOOT_MEDIUM_USB,
};

typedef struct leo_boot_mode {
    unsigned int boot_medium:4;
    unsigned int boot_info:4;
    unsigned int flash_boot_addr:24; // 包括低 8 位，默认低 8 位为 0，使用时需要左移 8 位
} leo_boot_mode_t;

#endif
