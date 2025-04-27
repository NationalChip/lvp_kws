#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: lvp/common source list
#
#=================================================================================#

ifeq ($(CONFIG_LVP_G_SENSOR_VAD_V1), y)
INCLUDE_DIR += lvp/common/G_vad/v1/
gvad_objs += lvp/common/G_vad/v1/lvp_G_vad.o
endif

ifeq ($(CONFIG_LVP_G_SENSOR_VAD_V2), y)
INCLUDE_DIR += lvp/common/G_vad/v2/
gvad_objs += lvp/common/G_vad/v2/lvp_G_vad.o
endif

ifeq ($(CONFIG_LVP_G_SENSOR_VAD_V3), y)
INCLUDE_DIR += lvp/common/G_vad/v3/
gvad_objs += lvp/common/G_vad/v3/lvp_G_vad.o
endif

