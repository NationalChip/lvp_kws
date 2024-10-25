#
# Voice Signal Preprocess
# Copyright (C) 2001-2021 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_KWS_STATE_DEMO), y)
app_objs += app/lvp_app_kws_state_demo/lvp_app_kws_state_demo.o
app_objs += app/lvp_app_kws_state_demo/vc_led.o
endif

