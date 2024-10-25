#
# Voice Signal Preprocess
# Copyright (C) 2001-2021 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_FFT_RECOVER_DEMO), y)
app_objs += app/fft_recover_demo/lvp_app_fft_recover_demo.o
endif
