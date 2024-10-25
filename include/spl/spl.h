/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef	_ASM_SPL_H_
#define	_ASM_SPL_H_

#include <stdint.h>

enum boot_device_type {
	BOOT_DEVICE_UART,
	BOOT_DEVICE_RAM,
	BOOT_DEVICE_NOR,
	BOOT_DEVICE_MMC,
	BOOT_DEVICE_SPINAND,
	BOOT_DEVICE_USB_HIGH,
	BOOT_DEVICE_USB_FULL,
	BOOT_DEVICE_I2C,
	BOOT_DEVICE_NONE
};

#define MAGIC_NUM_EXTRA        (28)

enum transfer_mode {
	UNKNOW = 1,
	SERIAL,
	USBSLAVE,
	I2C,
};

extern unsigned char stage1_xfer_mode, stage2_xfer_mode;
extern unsigned int uart_new_baudrate;


struct boot_info {
	enum boot_device_type boot_device;
	union {
		unsigned int value;

		struct {
			int uart_boot_port;
		} uart;

		struct {
			unsigned int boot_addr;
		} norflash;

		struct {
			unsigned int i2c_info;
		} i2c;
	} info;
};

extern struct boot_info boot_info;

/* SPL clock functions */
void spl_clk_init(void);
int spl_clk_trim(void);

/* SPL OSC functions*/
void spl_osc_set_32k_trim_state(int trim_done);
void spl_osc_set_all_trim_state(int trim_done);
int spl_osc_get_32k_trim_state(void);
int spl_osc_get_all_trim_state(void);

/* SPL board init */
void spl_board_init(void);
void spl_board_clk_init(void);

void spl_boot_info_init(void);
unsigned int spl_boot_device(void); // 调用该函数之前需要先调用 spl_boot_info_init 进行初始化

/* UART SPL functions */
int spl_uart_load_image(void);

/* USB SLAVE SPL functions */
int  spl_usbslave_load_image(void);
void spl_usbslave_init(void);
void spl_usbslave_exit(void);

/* NOR SPL functions */
int spl_nor_load_image(void);

/* nand flash SPL functions */
int spl_spinand_load_image(void);

/* ININ UART */
void serial_init(int use_rom_baud, int baudrate);

/* PUT CHAR FROM UART*/
void serial_put(int ch);
int  serial_get_char(void);
int serial_try_get(unsigned char *c);
void serial_hello(void);
int serial_read_buff(unsigned char *buff, unsigned int len);
void serial_put_sync(int ch);

/* I2C SLAVE SPL functions */
void i2c_slave_disable(void);
void i2c_slave_init(unsigned int addr);
void i2c_slave_wait_connect(void);
int i2c_slave_load_image(void);
int spl_i2c_load_image(void);

/* SPI SLAVE SPL functions */
int spi_read_buff(unsigned char *buff, unsigned int len);

/* dma 接口 */
void spl_dw_dma_config(void *dst, void *src, int len, int hw_src_per);
void spl_dw_dma_start(void);

/* spl中除法接口 */
unsigned int uint32_div(unsigned int dividend, unsigned int divisor);

/* spl中取余接口 */
unsigned int uint32_mod(unsigned int dividend, unsigned int divisor);


/* spl 延时接口 */
void spl_delay_init(void);
void spl_udelay(unsigned int usec);
void spl_mdelay(unsigned int msec);
unsigned int spl_get_time_ms(void);

#endif
