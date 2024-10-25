#
# LVP
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# peripheral.mk: source list of peripherals
#
#=================================================================================#

INCLUDE_DIR += lvp/common/peripherals/
INCLUDE_DIR += lvp/common/peripherals/gpio_led/
INCLUDE_DIR += lvp/common/peripherals/pwm_led/
INCLUDE_DIR += lvp/common/peripherals/slight_led/

dev_objs += lvp/common/peripherals/gpio_led/gpio_led.o
dev_objs += lvp/common/peripherals/pwm_led/pwm_led.o
dev_objs += lvp/common/peripherals/pwm_motor/pwm_motor.o
dev_objs += lvp/common/peripherals/slight_led/slight_led.o

INCLUDE_DIR += lvp/common/peripherals/multi_button/src/
dev_objs += lvp/common/peripherals/multi_button/src/multi_button.o
dev_objs += lvp/common/peripherals/multi_button/src/button_simulate.o


INCLUDE_DIR += lvp/common/peripherals/gsensor/

dev_objs += lvp/common/peripherals/gsensor/gsensor.o

#gsensor use lis2dw12
dev_objs += lvp/common/peripherals/gsensor/lis2dw12/driver/lis2dw12_reg.o
dev_objs += lvp/common/peripherals/gsensor/lis2dw12/src/lis2dw12.o
# dev_objs += lvp/common/peripherals/gsensor/lis2dw12/example/lis2dw12_orientation_6d.o
# dev_objs += lvp/common/peripherals/gsensor/lis2dw12/example/lis2dw12_read_data_simple.o
# dev_objs += lvp/common/peripherals/gsensor/lis2dw12/example/lis2dw12_read_data_single.o
# dev_objs += lvp/common/peripherals/gsensor/lis2dw12/example/lis2dw12_self_test.o
# dev_objs += lvp/common/peripherals/gsensor/lis2dw12/example/lis2dw12_tap_single.o
# dev_objs += lvp/common/peripherals/gsensor/lis2dw12/example/lis2dw12_tap_double.o
# dev_objs += lvp/common/peripherals/gsensor/lis2dw12/example/lis2dw12_wake_up.o
# dev_objs += lvp/common/peripherals/gsensor/lis2dw12/example/lis2dw12_activity.o
# dev_objs += lvp/common/peripherals/gsensor/lis2dw12/example/lis2dw12_free_fall.o
# dev_objs += lvp/common/peripherals/gsensor/lis2dw12/example/lis2dw12_fifo.o

#gsensor use ds_da662
dev_objs += lvp/common/peripherals/gsensor/ds_da662/src/ds_da662.o
