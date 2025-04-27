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
endif

ifeq ($(CONFIG_LVP_ENABLE_USER_DECODER), y)
kws_objs       += $(KWS_DIR_PRE)/user_decoder.o
else
ifeq ($(CONFIG_LVP_ENABLE_CTC_DECODER), y)
ifeq ($(CONFIG_ENABLE_CTC_KWS_AND_BUN_KWS_CASCADE), y)
kws_objs       += $(KWS_DIR_PRE)/max_decoder.o
endif
kws_objs       += $(KWS_DIR_PRE)/ctc_decoder.o
endif
ifeq ($(CONFIG_LVP_ENABLE_CTC_GX_DECODER), y)
kws_objs       += $(KWS_DIR_PRE)/ctc_gxdecoder.o
-include $(KWS_DIR_PRE)/gxdecoder/Makefile
endif
ifeq ($(CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER), y)
kws_objs       += $(KWS_DIR_PRE)/beamsearch_ctc_decoder.o
kws_objs       += $(KWS_DIR_PRE)/tiny-regex-c/re.o
kws_objs       += $(KWS_DIR_PRE)/levenshtein/levenshtein.o
kws_objs       += $(KWS_DIR_PRE)/dimsim/dimsim.o
kws_objs       += $(KWS_DIR_PRE)/dimsim/dimsim_map.o
INCLUDE_DIR    += $(KWS_DIR_PRE)/dimsim
endif
ifeq ($(CONFIG_LVP_ENABLE_BEAMSEARCH_DECODER), y)
kws_objs       += $(KWS_DIR_PRE)/beamsearch_decoder.o
kws_objs       += $(KWS_DIR_PRE)/tiny-regex-c/re.o
endif

endif

ifeq ($(CONFIG_LVP_ADVANCE_HUMAN_VAD_ENABLE), y)
kws_objs       += $(KWS_DIR_PRE)/human_vad.o
endif

# decoder
ifneq ($(CONFIG_LVP_ENABLE_USER_DECODER), y)
-include $(KWS_DIR_PRE)/decoder/Makefile
kws_objs       += $(KWS_DIR_PRE)/kws_strategy.o
kws_objs       += $(KWS_DIR_PRE)/kws_level_decoder.o
endif

# models
-include $(KWS_DIR_PRE)/models/*/*/Makefile
-include $(KWS_DIR_PRE)/models/voice_print/Makefile
-include $(KWS_DIR_PRE)/lm_models/enable_lm/*/*/Makefile
-include $(KWS_DIR_PRE)/lm_models/disable_lm/Makefile


ifeq ($(CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER)$(CONFIG_LVP_ENABLE_BEAMSEARCH_DECODER), y)
LIBS           += -lbeamsearch_ctc_decoder -lm
else
LIBS           += -lctc_decoder
endif

ifeq ($(CONFIG_ENABLE_USED_FOR_ALL_GRUS_FAMILY), y)
ifeq ($(CONFIG_MCU_ENABLE_PRINTF), y)
LIBS           += -lgxdecoder_all_release_v1.0.3
else
LIBS           += -lgxdecoder_all_release_v1.0.3
endif
else
ifeq ($(CONFIG_MCU_ENABLE_PRINTF), y)
LIBS           += -lgxdecoder_release_v1.0.3
else
LIBS           += -lgxdecoder_release_v1.0.3
endif
endif
