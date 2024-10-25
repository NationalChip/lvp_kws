/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * board_config.h
 *
 */
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#include <autoconf.h>
#include <base_addr.h>
#include <soc_config.h>

#ifndef CONFIG_FULL_SPEED_FREQUENCY
# if defined CONFIG_MCU_FREQUENCY_8M
#  define SCPU_CLOCK_SYSTEM   (24576000/3)
# elif defined CONFIG_MCU_FREQUENCY_12M
#  define SCPU_CLOCK_SYSTEM   (24576000/2)
# elif defined CONFIG_MCU_FREQUENCY_24M
#  define SCPU_CLOCK_SYSTEM   (24576000)
# else
#  error "Don't support SCPU_CLOCK_SYSTEM Frequency!!"
# endif
#else
# define SCPU_CLOCK_SYSTEM   (49152000)
#endif

#define SCPU_CLOCK_I2C     12500000
#define CONFIG_CLOCK_GPIO   1000000
#define CONFIG_CLOCK_COUNTER 1000000

#define CONFIG_BOARD_PATH           boards/nationalchip/grus_gx8002_fpga_1v

#define CONFIG_SERIAL_BAUD_RATE 115200
#define CONFIG_CLOCK_UART 25000000
#define CONFIG_SPL_UART_CLKDIV 7  /* (CONFIG_CLOCK_UART) / (16 * baudrate) */
#define CONFIG_SPL_UART_CLKDLF 12  /* (CONFIG_CLOCK_UART % (16 * baudrate) * 1.0) / (16 * baudrate) * (1 << 4) */

/* Environment for SDMMC boot */
#if defined(CONFIG_ENV_IS_IN_MMC) && !defined(CONFIG_ENV_OFFSET)
#define CONFIG_SYS_MMC_ENV_DEV		0	/* device 0 */
#define CONFIG_ENV_OFFSET		512	/* just after the MBR */
#endif

/* Environment for SPI boot */
#if defined(CONFIG_ENV_IS_IN_SPI_FLASH) && !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET		0x00100000
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#endif

/* Environment for nvram boot */
#ifdef CONFIG_ENV_IS_IN_NVRAM
#define CONFIG_ENV_ADDR             (CONFIG_SYS_MALLOC_BASE + CONFIG_SYS_MALLOC_LEN)
#endif

#ifndef CONFIG_BOOT_TOOL
#define CONFIG_BOOTCOMMAND  ""
#endif

/*
 * Designware SPI support
 */
#ifdef CONFIG_CMD_SPI
#define CONFIG_DEFAULT_SPI_BUS		0
#define CONFIG_DEFAULT_SPI_CS		0
#endif

/* SPI FLASH */
#define CONFIG_FLASH_SPI_CLK_SRC        12500000    /* 12.5MHz */
#define CONFIG_SF_DEFAULT_CLKDIV        2            /* 分频数必须为偶数且非0 */
#define CONFIG_SF_DEFAULT_SAMPLE_DELAY  1
#define CONFIG_SF_DEFAULT_SPEED         (CONFIG_FLASH_SPI_CLK_SRC / CONFIG_SF_DEFAULT_CLKDIV)

#define CONFIG_SF_DEFAULT_BUS           0
#define CONFIG_SF_DEFAULT_MODE          0x800
#define CONFIG_SF_DMA_XFER              0x800
#define CONFIG_SF_DEFAULT_CS            0

/*GENERAL_SPI*/
#define CONFIG_GENERAL_SPI_CLK_SRC      (1024000)
#define CONFIG_GENERAL_SPI_BUS_SN       0

/* WDT */
#define CONFIG_WDT_SYS_CLK  12500000

/* load uboot stage1 */
#define CONFIG_UBOOT_STAGE1_OFFS  (0x100000  )
#define CONFIG_UBOOT_STAGE1_SIZE  (0x10000   )
#define CONFIG_UBOOT_STAGE1_SRAM  (0xa0100000)

#endif	/* __BOARD_CONFIG_H__ */
