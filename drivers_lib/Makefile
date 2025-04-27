#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: drivers/ source list
#
#=================================================================================#

# Clock
dev_objs += drivers/clock/v2.0/clock.o

# Pmu
dev_objs += drivers/pmu/pmu_ctrl.o
dev_objs += drivers/pmu/pmu_osc.o

# Uart
CCFLAGS  += -DCONFIG_DW_AHB_DMA -DCONFIG_DW_UART_DMA
dev_objs += drivers/serial/dw_uart_dma.o
dev_objs += drivers/serial/dw_uart.o
dev_objs += drivers/serial/console.o
#dev_objs += drivers/serial/gx_baud_rate_check.o

# Intc
dev_objs += drivers/intc/vic.o

# Dma
dev_objs += drivers/dma/dw_dmac_ahb/dw_dmac_ahb.o

# Cache
dev_objs += drivers/cache/gx_dcache.o
dev_objs += drivers/cache/gx_icache.o

# Counter
ifeq ($(CONFIG_SYSTICK_COUNTER), y)
dev_objs += drivers/counter/systick_counter.o
endif
ifeq ($(CONFIG_GX_COUNTER), y)
dev_objs += drivers/counter/gx_counter.o
endif

# Misc
dev_objs += drivers/misc/grus/misc.o

# Audio In
dev_objs += drivers/audio_in/v2.0/audio_in.o

# Audio Out
dev_objs += drivers/audio_out/v2.0/hw.o
dev_objs += drivers/audio_out/v2.0/hw_config.o
dev_objs += drivers/audio_out/audio_out.o

# I2S
dev_objs += drivers/i2s/v2.0/i2s.o

# Snpu
SNPUDIR=drivers/snpu
ifeq ($(SNPUDIR), $(wildcard $(SNPUDIR)))
dev_objs += drivers/snpu/grus/snpu_hw.o
dev_objs += drivers/snpu/grus/snpu_mcu_ops.o
dev_objs += drivers/snpu/grus/snpu_regs.o
dev_objs += drivers/snpu/grus/snpu.o
endif

# Flash  -DCONFIG_ENABLE_SPI_DMA_FOR_FLASH
CCFLAGS  += -DCONFIG_MTD_FLASH_SPI -DCONFIG_FLASH_SPI_QUAD -DCONFIG_FLASH_SPI_32BITS
ifeq ($(CONFIG_MCU_ENABLE_XIP), y)
CCFLAGS  += -DCONFIG_FLASH_SPI_XIP
endif
ifeq ($(CONFIG_MCU_ENABLE_SPINAND_FLASH), y)
CCFLAGS  += -DCONFIG_MTD_SIMPLE_SPINAND
dev_objs += drivers/mtd/simple-spinand/simple_spi_nand.o
dev_objs += drivers/mtd/simple-spinand/simple_spi.o
endif
dev_objs += drivers/mtd/spinor/flash_spi.o
dev_objs += drivers/mtd/spinor/generic_spi_nor.o
dev_objs += drivers/mtd/spinor/spi_nor_ids.o
dev_objs += drivers/mtd/flash.o

# MMU
dev_objs += drivers/mmu/gx_mmu.o

# Dynamic Program Load
ifeq ($(CONFIG_DL), y)
dev_objs += drivers/dl/dl.o
endif

# Analog
dev_objs += drivers/analog/adc.o
dev_objs += drivers/analog/ldo.o

# Gpio
dev_objs += drivers/gpio/gpio_mini.o

# Spi
dev_objs += drivers/spi/spi.o
dev_objs += drivers/spi/device.o
dev_objs += drivers/spi/dw_spi/spi_master_v3.o
dev_objs += drivers/spi/dw_spi/spi_slave.o

# Padmux
dev_objs += drivers/padmux/grus_padmux.o

# Hw_i2c
dev_objs += drivers/hw_i2c/hw_i2c_ctrl.o

#i2c
CCFLAGS  += -DCONFIG_I2C_STATIC_ALLOC_MEM
dev_objs += drivers/i2c/dw_i2c.o
dev_objs += drivers/i2c/dw_i2c_master.o
dev_objs += drivers/i2c/dw_i2c_slave.o
dev_objs += drivers/i2c/i2c.o

# RTC
dev_objs += drivers/rtc/dw_rtc.o
#watchdog
dev_objs += drivers/watchdog/dw_wdt.o

ifeq ($(CONFIG_GX_COUNTER), y)
lib/libdriver_release_v1.0.6.a: $(dev_objs)
	@rm -f $@
	@echo [$(AR) archiving $@]
	@$(AR) $(ARFLAGS) $@ $^
endif
