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
-include lvp/vma/Makefile

lvp_objs += lvp/lvp_mode.o
lvp_objs += lvp/main.o
lvp_objs += lvp/lvp_mode_idle.o

ifeq ($(CONFIG_LVP_HAS_TWS_MODE), y)
lvp_objs += lvp/lvp_mode_tws.o
lvp_objs += lvp/lvp_vpr.o
endif

ifeq ($(CONFIG_LVP_HAS_FEED_MODE), y)
lvp_objs += lvp/lvp_mode_feed.o
endif

ifeq ($(CONFIG_LVP_HAS_RECORD_MODE), y)
lvp_objs += lvp/lvp_mode_record.o
endif

ifeq ($(CONFIG_LVP_HAS_DENOISE_MODE), y)
INCLUDE_DIR += lvp/vma/
lvp_objs += lvp/lvp_mode_denoise.o

ifeq ($(CONFIG_DENOISE_ENABLE_WEBRTC), y)
-include lvp/vma/webrtc/webrtc.mk
lvp_objs += $(webrtc_objs)
endif

ifeq ($(CONFIG_COMPILE_GSC_LIB_FROM_SOURCE_CODE), y)
-include lvp/vma/gsc/gsc.mk
lvp_objs += $(gsc_objs)
endif

endif

ifeq ($(CONFIG_LVP_HAS_SED_MODE), y)
INCLUDE_DIR += lvp/vui/sed
lvp_objs += lvp/lvp_mode_sed.o
endif

ifeq ($(CONFIG_LVP_HAS_NN_DENOISE_MODE), y)
ifeq ($(CONFIG_NN_DENOISE_SUPER_FAST), y)
lvp_objs += lvp/lvp_mode_nn_denoise_super_fast.o
else
ifeq ($(CONFIG_NN_DENOISE_LOW_LATENCY), y)
lvp_objs += lvp/lvp_mode_nn_denoise_fast.o
else
ifeq ($(CONFIG_LVP_ENABLE_CNN_DENOISE),y)
lvp_objs += lvp/lvp_mode_nn_denoise.o
else
lvp_objs += lvp/lvp_mode_rnn_denoise.o
endif
endif
endif
endif

ifeq ($(CONFIG_LVP_HAS_FFT_RECOVER_MODE), y)
lvp_objs += lvp/lvp_mode_fft_recover.o
endif

ifeq ($(CONFIG_LVP_HAS_PITCH_SHIFT_MODE), y)
INCLUDE_DIR += lvp/vma/pitchshift
lvp_objs += lvp/lvp_mode_pitch_shift.o
endif
