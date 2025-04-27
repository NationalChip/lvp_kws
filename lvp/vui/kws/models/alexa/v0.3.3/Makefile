#
# Voice Signal Preprocess
# Copyright (C) 2001-2022 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list in dsp/vpa/arach_lib/src/ctc/decoder
#
#=================================================================================#

ifeq ($(CONFIG_LVP_KWS_ALEXA_V0DOT3DOT3_2022_1129), y)

MODEL_DIR_PRE  = lvp/vui/kws/models/alexa/v0.3.3

INCLUDE_DIR    += $(MODEL_DIR_PRE)/
INCLUDE_DIR    += $(MODEL_DIR_PRE)/../
kws_objs       += $(MODEL_DIR_PRE)/ctc_model.o

endif