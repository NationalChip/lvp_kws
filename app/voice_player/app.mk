#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_VOICE_PLAYER), y)
app_objs += app/voice_player/lvp_app_voice_player.o
#app_objs += $(patsubst %.c, %.o, $(wildcard app/voice_player/resource/*.c))
endif

