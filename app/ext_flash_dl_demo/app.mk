#
# Voice Signal Preprocess
# Copyright (C) 2001-2021 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_DL_DEMO), y)
app_objs += app/ext_flash_dl_demo/lvp_app_dl_demo.o
endif

