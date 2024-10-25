#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_ADPCM_DEMO), y)
app_objs += app/adpcm_demo/lvp_app_adpcm_demo.o
endif

