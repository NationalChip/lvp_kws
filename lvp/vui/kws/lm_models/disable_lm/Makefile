#
# Grus
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile:
#
#=================================================================================#

ifeq ($(CONFIG_LVP_ENABLE_BEAMSEARCH_DECODER)$(CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER)$(CONFIG_ENABLE_LANGUAGE_MODEL), y)

LM_MODEL_DIR_PRE  = lvp/vui/kws/lm_models/disable_lm

INCLUDE_DIR    += $(LM_MODEL_DIR_PRE)/

kws_objs       += $(LM_MODEL_DIR_PRE)/lm_decoder.o
endif
