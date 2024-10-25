#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_SAMPLE), y)
app_objs += app/sample/lvp_app_sample.o
endif

