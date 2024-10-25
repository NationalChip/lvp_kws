#
# Voice Signal Preprocess
# Copyright (C) 2001-2022 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_UART_ADPCM_DEMO), y)
app_objs += app/uart_adpcm_demo/lvp_app_uart_adpcm_demo.o
endif

