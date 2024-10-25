#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_SPI_NAND_FLASH_SAMPLE_APP), y)
app_objs += app/spi_nand_flash_sample/spi_nand_flash_sample.o
endif

