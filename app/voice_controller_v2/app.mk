#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_VOICE_CONTROLLER_V2), y)
app_objs += app/voice_controller_v2/lvp_app_voice_controller_v2.o
app_objs += app/voice_controller_v2/vc_message.o
endif

