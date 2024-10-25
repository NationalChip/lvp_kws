#ifndef __SCPU_MISC__
#define __SCPU_MISC__

#include <spl/spl.h>

/*
 *	return value : 0 success, -1 failed
 */
int get_dsk(unsigned char *buf, int len);

/*
 *  let cache work
 */
void gx_cache_init(void);

/*
 * load uboot stage1
 */
int load_uboot_stage1(void);

#define GX8008 0x8008
#define GX8009 0x8009
#define GX8010 0x8010
#define GX8012 0x8012

/*
 * get chipname
 */
int get_chipname(void);

#define NNNA 'A'
#define NNNB 'B'
#define NNNC 'C'

/*
 * get chipversion
 */
int get_chipversion(void);

/*
 * Simultaneous output to uart and usb
 */

int logger(const char *fmt, ...);

/*
 * get a char from uart or usb, auto select
 */
int auto_getchar(void);

/*
 * send a char to uart or usb, auto select
 */
int auto_putchar(char value);

/*
 * Virtual address to dma address
 */
unsigned int virt_to_dma(void *addr);

/*
 * reinit boot info
 */
extern struct boot_info boot_info_s2;
void bootinfo_init(void);

/*
 * Rom 从串口启动时，该接口用于获取 stage2 应该配置的波特率
 * return value : 波特率的值，0 表示不修改串口波特率的配置(沿用之前的波特率)
 */
unsigned int get_baudrate_in_boot_mode(void);

/**
 * @brief 使能模拟参数更新功能.
 * 要想配置模拟参数，需要先调用该函数
 */
void gx_analog_config_update_enable(void);

/**
 * @brief MCU唤醒源
 */
typedef enum {
	WAKEUP_SOURCE_SYSTICK     = 0,
	WAKEUP_SOURCE_GPIO        = 1,
	WAKEUP_SOURCE_AUDIO_IN    = 2,
	WAKEUP_SOURCE_PMU         = 3,
	WAKEUP_SOURCE_PMU_RTC     = 4,
	WAKEUP_SOURCE_PMU_OSC     = 5,
	WAKEUP_SOURCE_DW_UART1    = 6,
	WAKEUP_SOURCE_DW_UART2    = 7,
	WAKEUP_SOURCE_DW_I2C1     = 8,
	WAKEUP_SOURCE_DW_I2C2     = 9,
	WAKEUP_SOURCE_DMA         = 10,
	WAKEUP_SOURCE_WDT         = 11,
	WAKEUP_SOURCE_NPU         = 12,
	WAKEUP_SOURCE_AUDIO_PLAY  = 13,
	WAKEUP_SOURCE_TIMER       = 14,
	WAKEUP_SOURCE_DW_SPI      = 15,
	WAKEUP_SOURCE_GENERAL_SPI = 16,
	WAKEUP_SOURCE_HW_I2C      = 17,
} GX_MCU_WAKEUP_SOURCE;

/**
 * @brief 设置mcu idle状态下的唤醒源
 */
void gx_mcu_set_wakeup_source(GX_MCU_WAKEUP_SOURCE wakeup_source);

/**
 * @brief 清除mcu idle状态下的唤醒源
 */
void gx_mcu_clr_wakeup_source(GX_MCU_WAKEUP_SOURCE wakeup_source);

/**
 * @brief mcu 进入idle状态
 */
void gx_mcu_idle(void);

#endif
