#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list of VSP other hook
#
#=================================================================================#

ifeq ($(CONFIG_LVP_APP_GPIO_PWM), y)
app_objs += app/gpio_pwm_demo/lvp_app_gpio_pwm_demo.o
endif

