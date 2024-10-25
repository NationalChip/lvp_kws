#
# Voice Signal Preprocess
# Copyright (C) 2001-2022 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_SPI_DEMO), y)
app_objs += app/spi_demo/lvp_app_spi_demo.o
endif

