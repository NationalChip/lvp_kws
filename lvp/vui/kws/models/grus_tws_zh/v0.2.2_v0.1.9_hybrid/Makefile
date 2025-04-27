#
# Voice Signal Preprocess
# Copyright (C) 2001-2021 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list in dsp/vpa/arach_lib/src/ctc/decoder
#
#=================================================================================#

ifeq ($(CONFIG_LVP_KWS_GRUS_TWS_ZH_V0DOT2DOT2_V0DOT1DOT9_2021_0728), y)

MODEL_DIR_PRE  = lvp/vui/kws/models/grus_tws_zh/v0.2.2_v0.1.9_hybrid

INCLUDE_DIR    += $(MODEL_DIR_PRE)/
INCLUDE_DIR    += $(MODEL_DIR_PRE)/../
kws_objs       += $(MODEL_DIR_PRE)/ctc_model.o

endif