#
# Voice Signal Preprocess
# Copyright (C) 2001-2022 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_LEO_MINI_UART_UPGRADE), y)
app_objs += app/leo_mini_uart_upgrade_demo/lvp_app_leo_mini_uart_upgrade.o
endif

