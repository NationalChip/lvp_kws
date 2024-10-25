#
# Voice Signal Preprocess
# Copyright (C) 2001-2019 NationalChip Co., Ltd
#
# Makefile: boot/ source list
#
#=================================================================================#

BOOT_PREFIX_PATH = arch/soc/$(strip $(subst ", ,$(CONFIG_SOC_MODEL)))
#boot_objs += $(BOOT_PREFIX_PATH)/padmux.o
boot_objs += $(BOOT_PREFIX_PATH)/spl/spl_clk.o
boot_objs += $(BOOT_PREFIX_PATH)/spl/spl.o
boot_objs += $(BOOT_PREFIX_PATH)/system.o
boot_objs += $(BOOT_PREFIX_PATH)/board_osc.o
boot_objs += $(BOOT_PREFIX_PATH)/trap_c.o
boot_objs += $(BOOT_PREFIX_PATH)/vectors.o
boot_objs += $(BOOT_PREFIX_PATH)/spl/spl_spinor.o
boot_objs += $(BOOT_PREFIX_PATH)/spl/spl_dw_dmac_ahb.o
boot_objs += $(BOOT_PREFIX_PATH)/spl/spl_i2c.o
boot_objs += $(BOOT_PREFIX_PATH)/spl/spl_counter.o
boot_objs += $(BOOT_PREFIX_PATH)/spl/spl_uart.o
boot_objs += $(BOOT_PREFIX_PATH)/spl/spl_trim.o
boot_objs += $(BOOT_PREFIX_PATH)/spl/spl_osc.o
boot_objs += arch/cpu/csky/ck804/spl_start.o
boot_objs += arch/cpu/csky/ck804/start.o
