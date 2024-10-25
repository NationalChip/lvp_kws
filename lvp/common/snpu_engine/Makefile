#
# Voice Signal Preprocess
# Copyright (C) 2001-2021 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: lvp/common/snpu_engine
#
#=================================================================================#
INCLUDE_DIR += lvp/common/snpu_engine/

ifeq ($(CONFIG_LVP_ENABLE_RNN_SED), y)
common_objs += lvp/common/snpu_engine/lvp_rnn_sed.o
endif

ifeq ($(CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION), y)
common_objs += lvp/common/snpu_engine/lvp_kws.o
endif


