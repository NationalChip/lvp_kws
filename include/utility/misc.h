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
void cache_init(void);

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

#endif
