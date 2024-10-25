/*
 *  Copyright (C) 2016 NationalChip Corporation <www.nationalchip.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <autoconf.h>
#include <base_addr.h>
#include <board_config.h>
#include <common.h>
#include <spl/spl.h>
#include <base_addr.h>
#include <misc_regs.h>
#include <soc.h>
unsigned int uart_new_baudrate = 115200;
unsigned char stage1_xfer_mode = UNKNOW, stage2_xfer_mode = UNKNOW;

void spl_boot_info_init(void)
{
	volatile leo_boot_mode_t *mode;

	i2c_slave_disable();

	mode = (volatile struct leo_boot_mode *)PMU_CFG_BOOT_MODE;
	switch (mode->boot_medium) {
#ifdef CONFIG_BOOT_BY_UART
		case BOOT_MEDIUM_UART:
			boot_info.boot_device = BOOT_DEVICE_UART;
			boot_info.info.uart.uart_boot_port = mode->boot_info;
#if 0
			serial_init(1, 0);

			serial_put_sync('w'); // wait
			serial_put_sync('f'); // for
			serial_put_sync('b'); // baudrate

			while(1) {
				while (!serial_try_get(&recv_char)) {
					if (recv_char == ok[char_index]) {
						char_index++;
						if (ok[char_index] == '\0')
							break;
					} else
						char_index = 0;
				}

				if (ok[char_index] == '\0')
					break;
			}

			// baudrate is Decimal
			bp = (char *)&uart_new_baudrate;
			*bp++ = serial_get_char();
			*bp++ = serial_get_char();
			*bp++ = serial_get_char();
			*bp   = serial_get_char();

			serial_put_sync('O');
			serial_put_sync('K');
#endif
			break;
#endif
#ifdef CONFIG_SPL_I2C_LOAD
		case BOOT_MEDIUM_I2C:
			boot_info.boot_device = BOOT_DEVICE_I2C;
			boot_info.info.i2c.i2c_info = mode->boot_info;
			break;
#endif
#ifdef CONFIG_BOOT_BY_FLASH
		case BOOT_MEDIUM_NOR_FLASH:
			boot_info.boot_device = BOOT_DEVICE_NOR;
			boot_info.info.norflash.boot_addr = mode->flash_boot_addr << 8;
			break;
		case BOOT_MEDIUM_NAND_FLASH:
			// grus not support nand
#endif
		case BOOT_MEDIUM_USB:
			// grus not support usb
			break;
		default:
			break;
	}

	return ;
}

unsigned int spl_boot_device(void)
{
	return boot_info.boot_device;
}

#ifdef CONFIG_BOOT_BY_UART
static void serial_snpu(void)
{
	serial_put('s');
	serial_put('n');
	serial_put('p');
	serial_put('u');
}

static void serial_snpuv(void)
{
	serial_put('s');
	serial_put('n');
	serial_put('p');
	serial_put('u');
	serial_put('v');
}

static void serial_snpu_ok(void)
{
	serial_put('s');
	serial_put('n');
	serial_put('p');
	serial_put('u');
	serial_put(' ');
	serial_put('o');
	serial_put('k');
}

int spl_uart_load_image(void)
{
	unsigned char *dram_addr = (unsigned char *)CONFIG_STAGE2_DRAM_BASE;
	unsigned int rec_size, crc, newcrc;
	unsigned int size;
	unsigned char *p;
	int i, offset;

	if (stage1_xfer_mode == USBSLAVE)
		serial_init(0, uart_new_baudrate);

	p = (unsigned char *)&crc;
	for(i = 0; i < 4; i++){
		*p++ = serial_get_char();
	}

	p = (unsigned char *)&rec_size;
	for(i = 0; i < 4; i++){
		*p++ = serial_get_char();
	}
	while (1) {
		int i;

		size = rec_size;
		newcrc = 0;


		serial_read_buff(dram_addr, size);
		gx_cache_init();

		for(i = 0; i < size; i++)
			newcrc += dram_addr[i];

		gx_dcache_disable();
		gx_icache_disable();
		if (newcrc != crc)
			serial_put('E');
		else {
			serial_put('O');
			break;
		}
	}

	serial_snpu();
	p = (unsigned char *)&rec_size;
	for(i = 0; i < 4; i++){
		*p++ = serial_get_char();
	}
	if(!rec_size)
		return 0;
	p = (unsigned char *)&size;
	for(i = 0; i < 4; i++){
		*p++ = serial_get_char();
	}
	if(size <= CONFIG_STAGE1_SRAM_SIZE)
	{
		size = CONFIG_STAGE1_SRAM_SIZE;
	}

	serial_snpuv();

#define DMA_MAX_SIZE (60*1024)
	dram_addr = (unsigned char *)(CONFIG_STAGE1_DRAM_BASE + size);
	while(rec_size)
	{
		if(rec_size > DMA_MAX_SIZE)
		{
			serial_read_snpu(dram_addr, DMA_MAX_SIZE);
			rec_size -= DMA_MAX_SIZE;
			dram_addr += DMA_MAX_SIZE;
			serial_snpuv();
		}
		else
		{
			serial_read_snpu(dram_addr, rec_size);
			rec_size = 0;
			serial_snpuv();
			break;
		}
	}
	serial_snpu_ok();

	return 0;
}

int spl_i2c_load_image(void)
{
	return i2c_slave_load_image();
}

#endif
typedef void(*func)(void);
func jump_2_addr;
/* NOR SPL functions */
unsigned int checksum = 0;
int spl_nor_load_image(void)
{
	extern int _stage2_sram_start_text_, _stage2_sram_end_text_;
	extern int _stage2_sram_start_data_, _stage2_sram_end_data_;

	extern int sflash_readdata(unsigned int offset, void *to, unsigned int len);
	u32 flash_addr;
	u32 stage2_sram_text_len = (u32)&_stage2_sram_end_text_ - (u32)&_stage2_sram_start_text_;
	u32 stage2_sram_data_len = (u32)&_stage2_sram_end_data_ - (u32)&_stage2_sram_start_data_;

	flash_addr = boot_info.info.norflash.boot_addr + CONFIG_STAGE1_DRAM_SIZE;
#ifdef CONFIG_BOARD_SUPPORT_MULTIBOOT
	extern unsigned int _multboot_info_addr_;
	volatile unsigned int *boot_mode = (volatile unsigned int*)(u32)&_multboot_info_addr_;
	if(*boot_mode == 0xaabbccdd) //boot from second firmware
	{
		u32 stage2_xip_text_len;
		sflash_readdata(boot_info.info.norflash.boot_addr + CONFIG_SECOND_BIN_OFFSET + CONFIG_STAGE1_DRAM_SIZE, (void *)&stage2_xip_text_len, 4);
		u32 stage2_sram_text_offset = CONFIG_SECOND_BIN_OFFSET + CONFIG_STAGE1_DRAM_SIZE + stage2_xip_text_len + 4;
		sflash_readdata(stage2_sram_text_offset, (void*)(CONFIG_STAGE1_DRAM_BASE + (CONFIG_SECOND_BIN_NPU_SRAM_SIZE_KB)*1024), (CONFIG_SECOND_BIN_STAGE2_BIN_SIZE));
		jump_2_addr = CONFIG_STAGE1_IRAM_BASE + (CONFIG_SECOND_BIN_NPU_SRAM_SIZE_KB)*1024 + 0x100;
		jump_2_addr();
	}
	else //boot from first firmware
	{
		u32 stage2_xip_text_len;
		sflash_readdata(flash_addr, (void *)&stage2_xip_text_len, 4);
		u32 stage2_sram_text_offset = flash_addr + stage2_xip_text_len + 4;
		u32 len  = stage2_sram_text_len + stage2_sram_data_len;
		sflash_readdata(stage2_sram_text_offset, (void *)&_stage2_sram_start_text_, len);
		jump_2_addr = CONFIG_STAGE1_IRAM_BASE + CONFIG_NPU_SRAM_SIZE_KB*1024 + 0x100;
		jump_2_addr();
	}
#else
	u32 stage2_xip_text_len;
	sflash_readdata(flash_addr, (void *)&stage2_xip_text_len, 4);
	u32 stage2_sram_text_offset = flash_addr + stage2_xip_text_len + 4;
	u32 len  = stage2_sram_text_len + stage2_sram_data_len;
	sflash_readdata(stage2_sram_text_offset, (void *)&_stage2_sram_start_text_, len);
	jump_2_addr = (void *)(CONFIG_STAGE1_IRAM_BASE + CONFIG_NPU_SRAM_SIZE_KB*1024 + 0x100);
	jump_2_addr();
#endif

	return 0;
}

struct boot_info boot_info = {
	.boot_device = BOOT_DEVICE_NONE,
};

__attribute__((unused)) static int spl_load_image(unsigned int boot_device)
{
	boot_info.boot_device = boot_device;

	switch (boot_device) {
#if defined (CONFIG_BOOT_BY_FLASH)
	case BOOT_DEVICE_NOR:
		return spl_nor_load_image();
#elif defined (CONFIG_BOOT_BY_UART)
	case BOOT_DEVICE_UART:
			stage1_xfer_mode = SERIAL;
			serial_hello();
			char boot_mode_select;
			while (1) {
				boot_mode_select = serial_get_char();
				if (boot_mode_select == 'S') {
					stage2_xfer_mode = SERIAL;
					return spl_uart_load_image();
				} else
					return -1;
			}
#else
	case BOOT_DEVICE_I2C:
			stage1_xfer_mode = I2C;
			spl_delay_init();
			i2c_slave_init(boot_info.info.i2c.i2c_info);
			i2c_slave_wait_connect();
			return spl_i2c_load_image();
#endif
	default:
		return -1;
	}
	return -1;
}

extern void spl_clear_bss(void);
void spl_board_init_r(void)
{
	spl_boot_info_init();
	spl_board_init();
	spl_clk_init();
	spl_clear_bss();
	serial_init(1, 0);
#ifndef CONFIG_MCU_ENABLE_JTAG_DEBUG
	spl_load_image(spl_boot_device());
#endif
}

static unsigned int uint32_divmodsi4(unsigned int num, unsigned int den, int modwanted)
{
	unsigned int bit = 1;
	unsigned int res = 0;

	while (den < num && bit && !(den & (1L<<31))) {
		den <<= 1;
		bit <<= 1;
	}

	while (bit) {
		if (num >= den) {
			num -= den;
			res |= bit;
		}
		bit >>= 1;
		den >>= 1;
	}

	if (modwanted)
		return num;

	return res;
}

unsigned int uint32_div(unsigned int dividend, unsigned int divisor)
{
	return uint32_divmodsi4(dividend, divisor, 0);
}

unsigned int uint32_mod(unsigned int dividend, unsigned int divisor)
{
	return uint32_divmodsi4(dividend, divisor, 1);
}

void spl_osc_set_all_trim_state(int trim_done)
{
	*(volatile unsigned int*)PMU_CFG_POWER_ON_RESET_REG1 &= ~(1 << 0);
	*(volatile unsigned int*)PMU_CFG_POWER_ON_RESET_REG1 |=  ((trim_done & 0x1) << 0);
}

void spl_osc_set_32k_trim_state(int trim_done)
{
	*(volatile unsigned int*)PMU_CFG_POWER_ON_RESET_REG1 &= ~(1 << 1);
	*(volatile unsigned int*)PMU_CFG_POWER_ON_RESET_REG1 |=  ((trim_done & 0x1) << 1);
}

int spl_osc_get_all_trim_state(void)
{
	return ((*(volatile unsigned int*)PMU_CFG_POWER_ON_RESET_REG1 >> 0) & 0x1);
}

int spl_osc_get_32k_trim_state(void)
{
	return ((*(volatile unsigned int*)PMU_CFG_POWER_ON_RESET_REG1 >> 1) & 0x1);
}
