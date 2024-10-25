/*****************************************
  Copyright (c) 2003-2019
  Nationalchip Science & Technology Co., Ltd. All Rights Reserved
  Proprietary and Confidential
 *****************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <util.h>
#include <stdarg.h>
#include <driver/gx_flash.h>
#include <misc.h>
#include <soc.h>
#include <spl/spl.h>
#include <driver/gx_clock.h>
#include "spl_spinor.h"
#include "autoconf.h"

#define SPI_CS_HIGH()  do {writel(0x03, FLASH_SPI_CS_REG);} while(0);
#define SPI_CS_LOW()   do {writel(0x02, FLASH_SPI_CS_REG);} while(0);
#define SPI_CS_HW()    do {writel(0x01, FLASH_SPI_CS_REG);} while(0);

#ifdef CONFIG_FLASH_SPI_XIP
static int spl_xip_init(unsigned int cmd,  unsigned int cmd_len,  unsigned int cmd_mode,
			unsigned int addr_len, unsigned int addr_mode,
			unsigned int mode_code, unsigned int mode_code_enable,
			unsigned int wait_cycles, unsigned int spi_mode);
#endif

static inline int wait_bus_ready(void)
{
	uint32_t val;

	do{
		val = readl(SPIM_SR);
	} while (val & SR_BUSY);

	return 0;
}

static inline int wait_rx_done(void)
{
	uint32_t val;

	do{
		val = readl(SPIM_RXFLR);
	} while (val != 0);

	wait_bus_ready();

	return 0;
}

static inline int wait_tx_done(void)
{
	uint32_t val;

	do{
		val = readl(SPIM_TXFLR);
	} while (val != 0);

	wait_bus_ready();

	return 0;
}

static int sflash_read_reg(uint8_t cmd, void* reg_val, uint32_t len)
{
	uint32_t i, val;
	uint8_t *p;

	p = (uint8_t*)reg_val;
	wait_bus_ready();
	writel(0x00, SPIM_ENR);
	writel(0x00, SPIM_DMACR);
	writel(EEPROM_8BITS_MODE, SPIM_CTRLR0);
	writel(len - 1, SPIM_CTRLR1);
	writel(0x01, SPIM_SER);
	writel(0 << 16, SPIM_TXFTLR);
	writel(0x00, SPIM_SPI_CTRL0);
	writel(0x01, SPIM_ENR);

	writel(cmd, SPIM_TXDR_LE);

	for (i = 0; i < len; i++) {
		do{
			val = readl(SPIM_SR);
		} while (!(val & SR_RF_NOT_EMPT));
		p[i] = readl(SPIM_RXDR_LE);
	}

	wait_rx_done();

	return 0;
}


static int sflash_write_reg(uint8_t cmd, const void* reg_val, uint32_t len)
{
	uint32_t i, val;
	const uint8_t *p;

	p = (const uint8_t*)reg_val;
	wait_bus_ready();
	writel(0x00, SPIM_ENR);
	writel(0x00, SPIM_DMACR);
	writel(STANDARD_TO_8BITS_MODE, SPIM_CTRLR0);
	writel(len, SPIM_CTRLR1);
	writel(0x01, SPIM_SER);
	writel(len << 16, SPIM_TXFTLR);
	writel(0x00, SPIM_SPI_CTRL0);
	writel(0x01, SPIM_ENR);

	writel(cmd, SPIM_TXDR_LE);

	for (i = 0; i < len; i++) {
		do{
			val = readl(SPIM_SR);
		} while (!(val & SR_TF_NOT_FULL));
		writel(p[i], SPIM_TXDR_LE);
	}

	wait_tx_done();

	return 0;
}
/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static uint8_t read_sr1(void)
{
	uint8_t status;

	sflash_read_reg(GX_CMD_RDSR1, &status, sizeof(status));

	return status;
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int write_enable(void)
{
	sflash_write_reg(GX_CMD_WREN, NULL, 0);

	return 0;
}

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns **** if error.
 */
static int wait_till_ready(void)
{
	uint8_t sr;

	do{
		sr = read_sr1();
	}while (sr & GX_STAT_WIP);

	return 1;
}

static int sflash_readid(int * jedec)
{
	int              tmp;
	unsigned char    id[3];
	tmp = sflash_read_reg(GX_CMD_READID, id, sizeof(id));
	if (tmp !=0) {
		return -2;
	}

	*jedec = id[0];
	*jedec = *jedec << 8;
	*jedec |= id[1];
	*jedec = *jedec << 8;
	*jedec |= id[2];

	return 0;
}

#ifdef CONFIG_FLASH_SPI_QUAD
static uint8_t read_sr2(void)
{
	uint8_t status;

	sflash_read_reg(GX_CMD_RDSR2, &status, sizeof(status));

	return status;
}

// Write 8bit or 16bit status register
static int write_sr(uint8_t *buf, uint32_t len, uint32_t reg_order)
{
	uint8_t cmd;

	if(len > 2)
		return -1;

	switch(reg_order)
	{
		case 1:
			cmd = GX_CMD_WRSR1;
			break;
		case 2:
			cmd = GX_CMD_WRSR2;
			break;
		case 3:
			cmd = GX_CMD_WRSR3;
			break;
		default:
			return -1;
	}

	wait_till_ready();
	write_enable();
	sflash_write_reg(cmd, buf, len);

	return 0;
}

/* 将status2 寄存器QE位置1 */
static int sflash_enable_quad(void)
{
	uint8_t status = read_sr2();
	if (status & 0x02)
		return 0;
	status |= 0x02;
	write_sr(&status, 1, 2);
	wait_till_ready();

	return 0;
}

static int sflash_enable_quad_1(void)
{
	uint8_t status[2];
	status[0] = read_sr1();
	status[1] = read_sr2();
	if (status[1] & 0x02)
		return 0;
	status[1] |= 0x02;
	write_sr(status, 2, 1);
	wait_till_ready();

	return 0;
}

#endif

static int flash_jedec = 0xffffff;

static int sflash_init(void)
{
	int  jedec    = 0;

	if (flash_jedec != 0xffffff)
		return 0;
	SPI_CS_HW();
	writel(0x0, SPIM_ENR);
	writel(0, SPIM_IMR);
	writel(CONFIG_SF_DEFAULT_SAMPLE_DELAY, SPIM_SAMPLE_DLY);
	writel(CONFIG_SF_DEFAULT_CLKDIV, SPIM_BAUDR);
	writel(0x1f, SPIM_RXFTLR);
	writel(0x1, SPIM_ENR);

	if (sflash_readid(&jedec) != 0)
		return -1;

	flash_jedec = jedec;

#ifdef CONFIG_FLASH_SPI_QUAD
	switch(jedec) {
	case 0x001c3812: /* en25s20a */
	case 0x001c3813: /* en25s40a */
		break;
	case 0x00854012: /* P25Q21L */
	case 0x00856013: /* P25Q40L */
    case 0x00856014: /* P25Q80L */
		sflash_enable_quad_1();
		break;
	default:
		sflash_enable_quad();
		break;
	}
#ifdef CONFIG_FLASH_SPI_XIP
	spl_xip_init(0xeb, 8, 1,
			24, 4,
			0x00, 1,
			4, 4);
#endif
#else
#ifdef CONFIG_FLASH_SPI_XIP
	spl_xip_init(0xbb, 8, 1,
			24, 2,
			0x00, 1,
			0, 2);
#endif
#endif

	return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int sflash_fread(unsigned int from, void *buf,unsigned int len)
{
	/* sanity checks */
	if (!len)
		return 0;

	wait_bus_ready();
	writel(0x00, SPIM_DMACR);
	writel(0x00, SPIM_ENR);
	writel(0x00, SPIM_SER);

#ifdef CONFIG_FLASH_SPI_QUAD
	writel(QUAD_RO_8BITS_MODE, SPIM_CTRLR0);
#else
	writel(DUAL_RO_8BIT_MODE, SPIM_CTRLR0);
#endif
	writel(len-1, SPIM_CTRLR1);
	writel(0 << 16, SPIM_TXFTLR);
	writel(7, SPIM_DMARDLR);
#ifdef CONFIG_FLASH_SPI_QUAD
	switch(flash_jedec) {
	case 0x001c3812: /* en25s20a */
	case 0x001c3813: /* en25s40a */
		writel(STRETCH_WAIT4_INST8_ADDR24, SPIM_SPI_CTRL0);
		writel(0x01, SPIM_ENR);
		writel(GX_CMD_QREAD2, SPIM_TXDR_BE);
		break;
	default:
		writel(STRETCH_WAIT8_INST8_ADDR24, SPIM_SPI_CTRL0);
		writel(0x01, SPIM_ENR);
		writel(GX_CMD_QREAD, SPIM_TXDR_BE);
		break;
	}
#else
	writel(STRETCH_WAIT8_INST8_ADDR24, SPIM_SPI_CTRL0);
	writel(0x01, SPIM_ENR);
	writel(0x3b, SPIM_TXDR_BE);
#endif
	writel(from, SPIM_TXDR_BE);

	writel(0x01, SPIM_SER);
#ifdef CONFIG_ENABLE_SPI_DMA_FOR_FLASH
	writel(0x01, SPIM_DMACR);

	spl_dw_dma_config(buf, (void*)SPIM_RXDR_LE, len, DMA_HS_PRE_FLASH_SPI_RX);
	spl_dw_dma_start();
	writel(0x00, SPIM_DMACR);
#else
	uint32_t i;
	for (i = 0; i < len; i++) {
		while(!(readl(SPIM_SR) & SR_RF_NOT_EMPT));
		((uint8_t*)buf)[i] = 0xff & readl(SPIM_RXDR_LE);
	}
#endif
	wait_rx_done();

	return 0;
}

void spl_clk_set_gate_enable(GX_CLOCK_MODULE module, unsigned int enable);
int sflash_readdata(unsigned int offset, void *to, unsigned int len)
{
	unsigned int size;
	unsigned char* p = to;

	spl_clk_set_gate_enable(CLOCK_MODULE_FLASH_SPI, 1);
	sflash_init();
	wait_till_ready();
	while(len != 0) {
		size = min(0x10000, len);
		sflash_fread(offset, p, size);
		len -= size;
		offset += size;
		p += size;
	}
#ifndef CONFIG_FLASH_SPI_XIP
	spl_clk_set_gate_enable(CLOCK_MODULE_FLASH_SPI, 0);
#endif

	return 0;
}

//=======================================================================================

#ifdef CONFIG_FLASH_SPI_XIP
static int spl_xip_init(unsigned int cmd,  unsigned int cmd_len,  unsigned int cmd_mode,
			unsigned int addr_len, unsigned int addr_mode,
			unsigned int mode_code, unsigned int mode_code_enable,
			unsigned int wait_cycles, unsigned int spi_mode)
{
	unsigned int cmd_addr_mode = XIP_STANDARD;
	unsigned int reg_data      = 0;

	writel(0x00, SPIM_ENR);

	cmd_mode  = cmd_mode  >> 1;
	addr_mode = addr_mode >> 1;
	spi_mode  = spi_mode  >> 1;

	if (cmd_len == 0)
		cmd_len = XIP_INST_L0;
	else if (cmd_len == 4)
		cmd_len = XIP_INST_L4;
	else if (cmd_len == 8)
		cmd_len = XIP_INST_L8;
	else if (cmd_len == 16)
		cmd_len = XIP_INST_L16;
	else {
		printf("Spi does not support this cmd len\n");
		return -1;
	}

	if ((cmd_mode == XIP_STANDARD) && (addr_mode == XIP_STANDARD))
		cmd_addr_mode = XIP_TRANS_TYPE_TT0;
	else if ((cmd_mode == XIP_STANDARD) && (addr_mode == spi_mode))
		cmd_addr_mode = XIP_TRANS_TYPE_TT1;
	else if ((cmd_mode == spi_mode) && (addr_mode == spi_mode))
		cmd_addr_mode = XIP_TRANS_TYPE_TT2;
	else{
		printf("Spi does not support this configuration\n");
		return -1;
	}

	if (addr_len % 4) {
		printf("addr_len error\n");
		return -1;
	}

	reg_data |= 1 << XIP_CONT_XFER_EN;
	reg_data |= 1 << XIP_INST_EN;
	reg_data |= wait_cycles << XIP_WAIT_CYCLES;
	reg_data |= cmd_len << XIP_INST_L;
	reg_data |= uint32_div(addr_len, 4) << XIP_ADDR_L;
	reg_data |= spi_mode << XIP_FRF;
	reg_data |= cmd_addr_mode << XIP_TRANS_TYPE;

	// 模式位设置
	if (mode_code_enable == 1) {
		reg_data |= MBL_8 << XIP_MBL;
		reg_data |= 1 << XIP_MD_BITS_EN;
	}

	writel(255,      XIP_CNT_TIME_OUT);
	writel(reg_data, XIP_CTRL);
	writel(1,        XIP_SER);
	writel(cmd,      XIP_INCR_INST);
	writel(cmd,      XIP_WRAP_INST);
	writel(0,        FLASH_SPI_CS_REG);
	writel(0x01,     SPIM_ENR);
	return 0;
}
#endif
