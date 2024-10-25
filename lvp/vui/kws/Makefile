#
# Voice Signal Preprocess
# Copyright (C) 2001-2019 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list in dsp/vpa/arach_lib/src/ctc/decoder
#
#=================================================================================#

KWS_DIR_PRE  = lvp/vui/kws

ifeq ($(CONFIG_LVP_ENABLE_MAX_DECODER), y)
kws_objs       += $(KWS_DIR_PRE)/max_decoder.o
kws_objs       += $(KWS_DIR_PRE)/kws_strategy.o
endif
ifeq ($(CONFIG_LVP_ENABLE_USER_DECODER), y)
kws_objs       += $(KWS_DIR_PRE)/user_decoder.o
else
ifeq ($(CONFIG_LVP_ENABLE_CTC_DECODER), y)
kws_objs       += $(KWS_DIR_PRE)/ctc_decoder.o
kws_objs       += $(KWS_DIR_PRE)/kws_strategy.o
endif
ifeq ($(CONFIG_LVP_ENABLE_CTC_GX_DECODER), y)
kws_objs       += $(KWS_DIR_PRE)/ctc_gxdecoder.o
-include $(KWS_DIR_PRE)/gxdecoder/Makefile
endif
endif
# decoder
ifeq ($(CONFIG_KWS_COMPILE_CTC_SCORE), y)
-include $(KWS_DIR_PRE)/decoder/Makefile
endif

# models
-include $(KWS_DIR_PRE)/models/*/*/Makefile
-include $(KWS_DIR_PRE)/models/voice_print/Makefile

LIBS           += -lctc_decoder
ifeq ($(CONFIG_MCU_ENABLE_PRINTF), y)
LIBS           += -lgxdecoder_debug_v1.0.3
else
LIBS           += -lgxdecoder_release_v1.0.3
endif
