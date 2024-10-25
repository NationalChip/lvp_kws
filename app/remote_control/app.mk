#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_REMOTE_CONTROL), y)
app_objs += app/remote_control/lvp_app_remote_control.o
app_objs += app/remote_control/lvp_button.o
app_objs += app/remote_control/flexible_button.o
app_objs += app/remote_control/led.o
app_objs += app/remote_control/rf_code.o
endif

