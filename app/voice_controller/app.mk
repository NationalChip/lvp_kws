#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_VOICE_CONTROLLER), y)
app_objs += app/voice_controller/lvp_app_voice_controller.o
app_objs += app/voice_controller/vc_led.o
app_objs += app/voice_controller/vc_message.o
endif

