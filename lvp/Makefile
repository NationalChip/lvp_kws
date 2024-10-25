#
# Voice Signal Preprocess
# Copyright (C) 2001-2019 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: VSP source list
#
#=================================================================================#

-include lvp/common/Makefile
-include lvp/vui/Makefile
-include lvp/app_core/Makefile

lvp_objs += lvp/lvp_mode.o
lvp_objs += lvp/main.o
lvp_objs += lvp/lvp_mode_idle.o

ifeq ($(CONFIG_LVP_HAS_TWS_MODE), y)
lvp_objs += lvp/lvp_mode_tws.o
endif

ifeq ($(CONFIG_LVP_HAS_FEED_MODE), y)
lvp_objs += lvp/lvp_mode_feed.o
endif

ifeq ($(CONFIG_LVP_HAS_RECORD_MODE), y)
lvp_objs += lvp/lvp_mode_record.o
endif

ifeq ($(CONFIG_LVP_HAS_SED_MODE), y)
INCLUDE_DIR += lvp/vui/sed
lvp_objs += lvp/lvp_mode_sed.o
endif
ifeq ($(CONFIG_LVP_HAS_FFT_RECOVER_MODE), y)
lvp_objs += lvp/lvp_mode_fft_recover.o
endif
