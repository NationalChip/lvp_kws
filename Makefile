#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: MCU Top Makefile
#
#=================================================================================#
# Toolchain Configurations
.PHONY: uart defconfig menuconfig post_config autoconf

all: build

# absolute path
CURDIR          := $(shell pwd)
Q               := @
obj             := $(CURDIR)/scripts/kconfig
SUBDIR          := $(obj)
CONFIG_KCONFIG  := Kconfig

ifeq ($(quiet),silent_)
silent := -s
endif

ifneq (,$(wildcard $(CURDIR)/.config))
include $(CURDIR)/.config
endif

uart :
	@make -f uart_boot.mk

defconfig : .config $(obj)/conf
	@echo | $(obj)/conf --oldconfig Kconfig
	@scripts/autoconf.sh > /dev/null
	@scripts/config_parse.sh > /dev/null

novuiconfig : $(obj)/mconf $(obj)/conf
	@cp .config novui.config
	@mv lvp/vui .
	$(Q)$< $(CONFIG_KCONFIG)
	@scripts/autoconf.sh > /dev/null
	@scripts/config_parse.sh > /dev/null
	@test -f lvp/vui/Kconfig || sed -i "3 i#include \"vui/vuiconf.h\"" ./include/autoconf.h
	@mv vui lvp
	@mv novui.config .config

menuconfig : $(obj)/mconf $(obj)/conf
	$(Q)$< $(CONFIG_KCONFIG)
	@scripts/autoconf.sh > /dev/null
	@scripts/config_parse.sh > /dev/null
	@test -f lvp/vui/Kconfig || sed -i "3 i#include \"vui/vuiconf.h\"" ./include/autoconf.h

multiboot:
	@echo -e "\033[36m  _____________________________/Note\___________________________________  \033[0m"
	@echo -e "\033[36m /                                                                      \ \033[0m"
	@echo -e "\033[36m   Multi-boot supports running two firmwares on one flash ! \033[0m"
	@echo -e "\033[36m   Need prepare first.config and second.config of the two firmwares \033[0m"
	@echo -e "\033[36m \______________________________________________________________________/ \033[0m"
	@
	@rm ./_output/ -rf
	@mkdir ./_output/
	@test -f first.config || echo -e "\033[31m  "Cannot find ./first.config, need first.config and second.config!" \033[0m"
	@test -f second.config || echo -e "\033[31m  "Cannot find ./second.config, need first.config and second.config!" \033[0m"
	@cp first.config ./_output/
	@cp second.config ./_output/
	@sed -i '/CONFIG_BOARD_SUPPORT_MULTIBOOT */d' 			./_output/first.config
	@sed -i '/CONFIG_MULTIBOOT_FIRST_BIN */d' 			./_output/first.config
	@sed -i '/CONFIG_MULTIBOOT_SECOND_BIN */d' 			./_output/first.config
	@sed -i '/CONFIG_SECOND_BIN_OFFSET */d' 			./_output/first.config
	@sed -i '/CONFIG_SECOND_BIN_STAGE2_BIN_SIZE */d' 	./_output/first.config
	@sed -i '/CONFIG_SECOND_BIN_NPU_SRAM_SIZE_KB */d' 	./_output/first.config
	@sed -i "5 iCONFIG_BOARD_SUPPORT_MULTIBOOT=y" 			./_output/first.config
	@sed -i "5 iCONFIG_MULTIBOOT_FIRST_BIN=y" 			./_output/first.config
	@sed -i "5 iCONFIG_SECOND_BIN_OFFSET=0x0" 			./_output/first.config
	@sed -i "5 iCONFIG_SECOND_BIN_STAGE2_BIN_SIZE=0x0" 	./_output/first.config
	@sed -i "5 iCONFIG_SECOND_BIN_NPU_SRAM_SIZE_KB=0" 	./_output/first.config
	@
	@
	@sed -i '/CONFIG_BOARD_SUPPORT_MULTIBOOT */d' 			./_output/second.config
	@sed -i '/CONFIG_MULTIBOOT_FIRST_BIN */d' 			./_output/second.config
	@sed -i '/CONFIG_MULTIBOOT_SECOND_BIN */d' 			./_output/second.config
	@sed -i '/CONFIG_SECOND_BIN_OFFSET */d' 			./_output/second.config
	@sed -i '/CONFIG_SECOND_BIN_STAGE2_BIN_SIZE */d' 	./_output/second.config
	@sed -i '/CONFIG_SECOND_BIN_NPU_SRAM_SIZE_KB */d' 	./_output/second.config
	@
	@sed -i "5 iCONFIG_BOARD_SUPPORT_MULTIBOOT=y" 			./_output/second.config
	@sed -i "5 iCONFIG_MULTIBOOT_SECOND_BIN=y" 			./_output/second.config
	@sed -i "5 iCONFIG_SECOND_BIN_OFFSET=0x0" 			./_output/second.config
	@sed -i "5 iCONFIG_SECOND_BIN_STAGE2_BIN_SIZE=0x0" 	./_output/second.config
	@sed -i "5 iCONFIG_SECOND_BIN_NPU_SRAM_SIZE_KB=0" 	./_output/second.config
	@
	@echo -e "\033[36m  _____________________________/Note\___________________________________  \033[0m"
	@echo -e "\033[36m /                                                                      \ \033[0m"
	@echo -e "\033[36m               Start build first part! \033[0m"
	@echo -e "\033[36m \______________________________________________________________________/ \033[0m"
	@cp ./_output/first.config .config
	@echo | $(obj)/conf --oldconfig Kconfig
	@scripts/autoconf.sh > /dev/null
	@scripts/config_parse.sh > /dev/null
	@make clean;make
	@echo -e "\033[36m  _____________________________/Note\___________________________________  \033[0m"
	@echo -e "\033[36m /                                                                      \ \033[0m"
	@echo -e "\033[36m               Build first part done! \033[0m"
	@echo -e "\033[36m \______________________________________________________________________/ \033[0m"
	@
	@cp output/mcu_nor.bin ./_output/first_mcu_nor.bin
	@
	@sed -i '/CONFIG_SECOND_BIN_OFFSET */d' 			./_output/second.config
	@sed -i "5 iCONFIG_SECOND_BIN_OFFSET=0x$$(echo "obase=16;ibase=10;$$(stat -c %s _output/first_mcu_nor.bin | tr -d '\n')" | bc)" 			./_output/second.config
	@
	@echo -e "\033[36m  _____________________________/Note\___________________________________  \033[0m"
	@echo -e "\033[36m /                                                                      \ \033[0m"
	@echo -e "\033[36m               Start build second part! \033[0m"
	@echo -e "\033[36m \______________________________________________________________________/ \033[0m"
	@cp ./_output/first.config .config
	@cp ./_output/second.config .config
	@echo | $(obj)/conf --oldconfig Kconfig
	@scripts/autoconf.sh > /dev/null
	@scripts/config_parse.sh > /dev/null
	@make clean;make
	@echo -e "\033[36m  _____________________________/Note\___________________________________  \033[0m"
	@echo -e "\033[36m /                                                                      \ \033[0m"
	@echo -e "\033[36m               Build second part done! \033[0m"
	@echo -e "\033[36m \______________________________________________________________________/ \033[0m"
	@
	@
	@cp output/mcu.elf ./_output/second_mcu_nor.elf
	@cp output/mcu_nor.bin ./_output/second_mcu_nor.bin
	@
	@sed -i '/CONFIG_SECOND_BIN_NPU_SRAM_SIZE_KB */d' 	./_output/first.config
	@sed -i "5 iCONFIG_SECOND_BIN_NPU_SRAM_SIZE_KB=$$(grep CONFIG_NPU_SRAM_SIZE_KB ./second.config | cut -d = -f 2)" 	./_output/first.config
	@
	@sed -i '/CONFIG_SECOND_BIN_STAGE2_BIN_SIZE */d' 	./_output/first.config
	@sed -i "5 iCONFIG_SECOND_BIN_STAGE2_BIN_SIZE=0x$$(echo "obase=16;ibase=10;$$(echo "\
	$$(echo "obase=10;ibase=16; $$(readelf -s ./_output/second_mcu_nor.elf |grep -E "_stage2_sram_end_text_" | awk '{str = $$2};NR==1{str1 = str};NR==1{printf str};' | xargs echo | tr [:lower:] [:upper:])" | bc) \
	- $$(echo "obase=10;ibase=16; $$(readelf -s ./_output/second_mcu_nor.elf |grep -E "_stage2_sram_start_text_" | awk '{str = $$2};NR==1{str1 = str};NR==1{printf str};' | xargs echo | tr [:lower:] [:upper:])" | bc) \
	+ $$(echo "obase=10;ibase=16; $$(readelf -s ./_output/second_mcu_nor.elf |grep -E "_stage2_sram_end_data_" | awk '{str = $$2};NR==1{str1 = str};NR==1{printf str};' | xargs echo | tr [:lower:] [:upper:])" | bc) \
	- $$(echo "obase=10;ibase=16; $$(readelf -s ./_output/second_mcu_nor.elf |grep -E "_stage2_sram_start_data_" | awk '{str = $$2};NR==1{str1 = str};NR==1{printf str};' | xargs echo | tr [:lower:] [:upper:])" | bc)"\
	| bc)" | bc)" 	./_output/first.config
#	@echo "obase=16;ibase=10;$$(echo "\
	$$(echo "obase=10;ibase=16; $$(readelf -s output/mcu.elf |grep -E "_stage2_sram_end_text_" | awk '{str = $$2};NR==1{str1 = str};NR==1{printf str};' | xargs echo | tr [:lower:] [:upper:])" | bc) \
	- $$(echo "obase=10;ibase=16; $$(readelf -s output/mcu.elf |grep -E "_stage2_sram_start_text_" | awk '{str = $$2};NR==1{str1 = str};NR==1{printf str};' | xargs echo | tr [:lower:] [:upper:])" | bc) \
	+ $$(echo "obase=10;ibase=16; $$(readelf -s output/mcu.elf |grep -E "_stage2_sram_end_data_" | awk '{str = $$2};NR==1{str1 = str};NR==1{printf str};' | xargs echo | tr [:lower:] [:upper:])" | bc) \
	- $$(echo "obase=10;ibase=16; $$(readelf -s output/mcu.elf |grep -E "_stage2_sram_start_data_" | awk '{str = $$2};NR==1{str1 = str};NR==1{printf str};' | xargs echo | tr [:lower:] [:upper:])" | bc)"\
	| bc)" | bc
	@
	@sed -i '/CONFIG_SECOND_BIN_OFFSET */d' 			./_output/first.config
	@sed -i "5 iCONFIG_SECOND_BIN_OFFSET=0x$$(echo "obase=16;ibase=10;$$(stat -c %s _output/first_mcu_nor.bin | tr -d '\n')" | bc)" 			./_output/first.config
	@
	@cp ./_output/first.config .config
	@echo | $(obj)/conf --oldconfig Kconfig
	@scripts/autoconf.sh > /dev/null
	@scripts/config_parse.sh > /dev/null
	@make clean;make
	@
	@cp output/mcu_nor.bin ./_output/first_mcu_nor.bin
	@
	@echo $$(stat -c %s _output/first_mcu_nor.bin | tr -d '\n')
	@dd if=./_output/first_mcu_nor.bin bs=1 count=$$(stat -c %s _output/first_mcu_nor.bin | tr -d '\n') skip=0 seek=0 of=./_output/mcu_nor.bin
	@dd if=./_output/second_mcu_nor.bin bs=1 count=$$(stat -c %s _output/second_mcu_nor.bin | tr -d '\n') skip=0 seek=$$(stat -c %s _output/first_mcu_nor.bin | tr -d '\n') of=./_output/mcu_nor.bin
	@cp ./_output/mcu_nor.bin ./output/
	@
	@
	@echo -e "\033[36m  _____________________________/Note\___________________________________  \033[0m"
	@echo -e "\033[36m /                                                                      \ \033[0m"
	@echo -e "\033[36m               Multboot firmware:    \033[0m"
	@echo -e "\033[36m  $$(ls -l ./output/mcu_nor.bin)   \033[0m"
	@echo -e "\033[31m    Please make sure the flash size is larger than the firmware size ! \033[0m"
	@echo -e "\033[36m \______________________________________________________________________/ \033[0m"




$(obj)/mconf:
	$(Q)$(MAKE) -C $(SUBDIR) prepare
	$(Q)$(MAKE) -C $(SUBDIR)

$(obj)/conf:
	$(Q)$(MAKE) -C $(SUBDIR) prepare
	$(Q)$(MAKE) -C $(SUBDIR)

autoconf:
	@scripts/autoconf.sh > /dev/null

post_config:
	@scripts/config_parse.sh > /dev/null

#==================================================================================#

CROSS_COMPILE   = csky-abiv2-elf-
AR              = $(CROSS_COMPILE)ar
AS              = $(CROSS_COMPILE)as
CC              = $(CROSS_COMPILE)gcc
LD              = $(CROSS_COMPILE)ld
OBJCOPY         = $(CROSS_COMPILE)objcopy
OBJDUMP         = $(CROSS_COMPILE)objdump
NM              = $(CROSS_COMPILE)nm
STRIP           = $(CROSS_COMPILE)strip
READELF         = $(CROSS_COMPILE)readelf -a

SHELL           = /bin/bash

#==================================================================================#

#=================================================================================#
# Flags Configurations
#---------------------------------------------------------------------------------#
#       Optimization options (add to CCFLAGS ):
#       -O0 -g  = Level 0       no optimization, debug info is enabled
#       -O1     = Level 1       (incompatible with -g option (gebug info))
#       -O2     = Level 2       (incompatible with -g option)
#       -O3     = Level 3       (incompatible with -g option)
#---------------------------------------------------------------------------------#
# The pre-processor and compiler options.

# Schema Flags
ifeq ($(CONFIG_MCU_BUILD_SCHEMA_DEBUG), y)
    CCFLAGS    += -O0
endif
ifeq ($(CONFIG_MCU_BUILD_SCHEMA_RELEASE), y)
    CCFLAGS    += -O2
endif
ifeq ($(CONFIG_MCU_BUILD_SCHEMA_OPTIMIZED_SPEED), y)
    CCFLAGS    += -O3
endif
ifeq ($(CONFIG_MCU_BUILD_SCHEMA_OPTIMIZED_SIZE), y)
    CCFLAGS    += -Os
endif

GCCVERSION = $(shell $(CC) --version | grep csky-abiv2-elf-gcc)
ifeq "$(GCCVERSION)" "csky-abiv2-elf-gcc (C-SKY Tools V3.10.15 Minilibc abiv2 B20190929) 6.3.0"
else
	_VERSION=$(GCCVERSION)
	_ERR_FLAG="Version error!"
endif

CCFLAGS        += -mcpu=ck804ef -Wpointer-arith -Wundef -Wall -Wundef \
                  -Wstrict-prototypes -pipe -fno-builtin \
                  -fstrict-volatile-bitfields -mhard-float

ASFLAGS         = -mcpu=ck804ef -Wa,--defsym=ck804=1 -EL -mhard-float

# The git version of mcu
#MCU_VERSION    := "$(shell git describe --dirty --always --tags)"
MCU_VERSION    := ""
CCFLAGS        += -DMCU_VERSION=\"$(MCU_VERSION)\"
ASFLAGS        += -DMCU_VERSION=\"$(MCU_VERSION)\"

# The build time of mcu
BUILD_TIME      = $(shell date +"%Y-%m-%d,\ %H:%M:%S")
CCFLAGS        += -DBUILD_TIME=\"$(BUILD_TIME)\"

# The options used in linking as well as in any direct use of ld.
LDFLAGS         = -EL

# The options used in archive as well as in any direct use of ar.
ARFLAGS         = -rcsD

CCFLAGS        += -ffunction-sections -nostdlib #-fdata-sections
LDFLAGS        += --gc-sections

CCFLAGS        += -DCONFIG_ARCH_GRUS
#---------------------------------------------------------------------------------#

INCLUDE_DIR    += arch/soc/$(CONFIG_SOC_MODEL)/include
INCLUDE_DIR    += boards/$(CONFIG_BOARD_VENDOR)/$(CONFIG_BOARD_MODEL)/include
INCLUDE_DIR    += include/utility/libdsp
INCLUDE_DIR    += include/utility/libc
INCLUDE_DIR    += include/utility
INCLUDE_DIR    += include/vui
INCLUDE_DIR    += include
INCLUDE_DIR    += lvp/common
#INCLUDE_DIR    += lvp/kws
INCLUDE_DIR    += base/include
INCLUDE_DIR    += lvp/vui/kws
INCLUDE_DIR    += lvp
INCLUDE_DIR    += lvp/vui/denoise

LIBS           += -ldriver_release_v1.0.5

LIBS           += -lvui

ifeq ($(CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION), y)
ifeq ($(CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER)$(CONFIG_LVP_ENABLE_BEAMSEARCH_DECODER), y)
LIBS           += -lbeamsearch_ctc_decoder -lm
else
LIBS           += -lctc_decoder
endif
endif


ifeq ($(CONFIG_DENOISE_ENABLE_GSC), y)
LIBS           += -lgsc_earphone
endif
LIBS           += -lcsky_CK804ef_dsp2_nn -lcsky_dsp -lvma -lcsky_dsp
LIBS           += -lm -lc

LIB_DIR         = lib
ifeq ($(CONFIG_DENOISE_ENABLE_GSC), y)
LIB_DIR        += lvp/vma/gsc
endif
LIB_DIR        += $(shell $(CC) -print-file-name=ck803)/hard-fp
LIB_DIR        += $(shell $(CC) -print-file-name=ck803)/../../../../../csky-elfabiv2/lib/ck803/hard-fp/


# The directories in which header files reside.
# If not specified, only the current directory will be serached.
INCLUDE_FLAGS   = $(addprefix -I , $(INCLUDE_DIR))
LIB_FLAGS       = $(addprefix -L , $(LIB_DIR))

#---------------------------------------------------------------------------------#

ASFLAGS        += $(INCLUDE_FLAGS)
CCFLAGS        += $(INCLUDE_FLAGS)
LDFLAGS        += $(LIB_FLAGS) $(LIBS)

#=================================================================================#
# Source List

-include arch/Makefile
-include utility/Makefile
# -include drivers/Makefile
-include boards/Makefile
-include lvp/Makefile
-include 3rdparty/Makefile
-include base/Makefile
-include app/Makefile

LIBS           += -lgcc

ifdef CONFIG_MCU_COMPILE_MATH_LIB_FROM_SOURCE_CODE
$(ckdsp_objs):EXTRA_FLAGS := -mhard-float -D__FPU_PRESENT -fdata-sections -Wl,--gc-sections -DUNALIGNED_SUPPORT_DISABLE -DCSKY_SIMID
endif

$(vma_objs):EXTRA_FLAGS := -mhard-float -D__FPU_PRESENT -fdata-sections -Wl,--gc-sections

lib_objs        = $(boot_objs) $(dev_objs) $(utility_objs) $(common_objs) $(lvp_objs) $(kws_objs) $(vpr_objs) $(app_objs) $(denoise_objs)

ifeq ($(CONFIG_USE_OPUS), y)
lib_objs += $(opus_objs)
INCLUDE_DIR += $(OPUS_CINCLUDES)
CCFLAGS += $(OPUS_CFLAGS)
CCFLAGS += -DUSE_OPUS
endif
ifeq ($(CONFIG_USE_AMR), y)
lib_objs += $(amr_objs)
CCFLAGS += $(AMR_CFLAGS)
CCFLAGS += -DUSE_AMR
endif
ifeq ($(CONFIG_USE_MP3), y)
lib_objs += $(mp3_objs)
CCFLAGS += -DUSE_MP3
endif
lib_objs += $(adpcm_objs)
CCFLAGS += -DUSE_ADPCM
INCLUDE_DIR += $(ADPCM_CINCLUDES)

mcu_objs        = $(subst ",,$(lib_objs))
all_objs        = $(mcu_objs) $(lvp_objs) $(base_objs)
all_deps        = $(subst .o,.d,$(all_objs))

VERSION=0x42555858
CCFLAGS += -DRELEASE_VERSION=$(VERSION)

CCFLAGS  += -DCONFIG_MTD_FLASH_SPI -DCONFIG_FLASH_SPI_QUAD -DCONFIG_FLASH_SPI_32BITS
CCFLAGS  += -DCONFIG_FLASH_SPI_XIP
CCFLAGS  += -DCONFIG_DW_AHB_DMA -DCONFIG_DW_UART_DMA
CCFLAGS  += -DCONFIG_I2C_STATIC_ALLOC_MEM

$(adpcm_objs):CCFLAGS := $(CCFLAGS) -O2

#=================================================================================#
# Build Rules

.PHONY: clean export help

ifeq (,$(wildcard $(CURDIR)/.config))# check .config
# Build .config
build: .config

else # .config is exit
# Build objects
ifdef gsc_objs
build: prepare output/libmcu.a lib/libcsky_dsp.a lvp/vma/gsc/libgsc_earphone.a lib/libvma.a output/mcu.elf output/mcu.info.txt output/mcu.dump.txt
else
build: prepare output/libmcu.a lib/libcsky_dsp.a output/mcu.elf output/mcu.info.txt output/mcu.dump.txt
endif
ifneq ($(CONFIG_MCU_ENABLE_JTAG_DEBUG), y)
build: output/mcu.bin checkbin
endif # endif ifneq ($(CONFIG_MCU_ENABLE_JTAG_DEBUG), y)

endif # endif ifeq (,$(wildcard $(CURDIR)/.config))

.config:
	@make menuconfig

ifeq ($(CONFIG_LVP_USE_BUFFER_V2), y)
prepare:
	@echo [Preparing lvp ...]
	@mkdir -p output
	@$(CC) $(CCFLAGS) $(EXTRA_FLAGS) -E -x assembler-with-cpp -P -o output/mcu.ld arch/soc/$(CONFIG_SOC_MODEL)/link_buffer_v2.ld
else
prepare:
	@echo [Preparing lvp ...]
	@mkdir -p output
	@mkdir -p tmp_objs
	@$(AR) -x lib/libdriver_release_v1.0.5.a
	@mv *.o tmp_objs/
	@$(CC) $(CCFLAGS) $(EXTRA_FLAGS) -E -x assembler-with-cpp -P -o output/mcu.ld arch/soc/$(CONFIG_SOC_MODEL)/link.ld
endif


ifdef ckdsp_objs
lib/libcsky_dsp.a: $(ckdsp_objs)
	@rm -f $@
	@echo [$(AR) archiving $@]
	@$(AR) $(ARFLAGS) $@ $^
endif

ifdef vma_objs
lib/libvma.a: $(vma_objs)
	@rm -f $@
	@echo [$(AR) archiving $@]
	@$(AR) $(ARFLAGS) $@ $^
endif

ifdef gsc_objs
lvp/vma/gsc/libgsc_earphone.a: $(gsc_objs)
	@rm -f $@
	@echo [$(AR) archiving $@]
	@$(AR) $(ARFLAGS) $@ $^
endif

output/libmcu.a: $(mcu_objs)
	@rm -f $@
	@echo [$(AR) archiving $@]
	@$(AR) $(ARFLAGS) $@ $^

output/mcu.elf: output/mcu.ld $(all_objs)
	@echo [$(LD) linking $@]
	$(LD) -o $@ -T $^ $(LDFLAGS)
	@size $@
ifeq ($(VERSION), 0x42555858)
	@echo -e "\033[33m  ____________________________/Warning\_________________________________  \033[0m"
	@echo -e "\033[33m /                                                                      \ \033[0m"
	@echo -e "\033[33m             Here used defult Version $(VERSION)                          \033[0m"
	@echo -e "\033[33m         if you want to change it to 0x1234: $$ make VERSION=0x1234       \033[0m"
	@echo -e "\033[33m \______________________________________________________________________/ \033[0m"
else
	@echo -e "\033[36m  _____________________________/Note\___________________________________  \033[0m"
	@echo -e "\033[36m /                                                                      \ \033[0m"
	@echo -e "\033[36m                     Here set Version : $(VERSION)                        \033[0m"
	@echo -e "\033[36m         if you want to change it to 0x1234: $$ make VERSION=0x1234       \033[0m"
	@echo -e "\033[36m \______________________________________________________________________/ \033[0m"
endif


#---------------------------------------------------------------------------------#

output/mcu.dump.txt: output/mcu.elf
	@echo [$(OBJDUMP) disassemble $^]
	@$(OBJDUMP) -Sa $^ > $@

output/mcu.info.txt: output/mcu.elf
	@echo [$(READELF) dump sections $^]
	@$(READELF) $^ > $@

ifeq ($(CONFIG_MCU_ENABLE_JTAG_DEBUG), y)
export: output/mcu.elf
else
export: output/mcu.elf output/mcu_nor.bin output/mcu_main.bin output/mcu_main.h
endif
	@echo "[Exporting $^]"
	@cp -f $^ ../output

checkbin:
	@echo
	@-ls -l output/* | awk '{ print $$9 ":\t" $$5 " Byte" }'

#---------------------------------------------------------------------------------#

ifneq ($(MAKECMDGOALS), )
    NOT_INCLUDE_DEP = true
endif
ifneq ($(NOT_INCLUDE_DEP), true)
    -include $(all_deps)
endif

#---------------------------------------------------------------------------------#
# Common Build Rules
ifneq (,$(wildcard $(CURDIR)/.config))
%.d:%.c
    ifeq (${_ERR_FLAG},"Version error!")
	@echo
	@echo "________________________________/ERROR\\___________________________________"
	@echo "|Now compiler version :                                                  |"
	@echo "|${GCCVERSION}"
	@echo "|                                                                        |"
	@echo "|Version error!                                                          |"
	@echo "|                                                                        |"
	@echo "|The right version :                                                     |"
	@echo "|csky-abiv2-elf-gcc (C-SKY Tools V3.10.15 Minilibc abiv2 B20190929) 6.3.0 |"
	@echo "|________________________________________________________________________|"
	@echo
	@killall make
    endif
	@echo [$(CC) creating $@]
	@$(CC) $(CCFLAGS) -M $< > $@.$$$$;\
	sed 's,\(.*\)\.o[ :]*,$(shell dirname $@)/\1.o $@ : ,g' <$@.$$$$ > $@;\
	$(RM) $@.$$$$

%.d:%.S
	@echo [$(CC) creating $@]
	@$(CC) $(CCFLAGS) -M $< > $@.$$$$;\
	sed 's,\(.*\)\.o[ :]*,$(shell dirname $@)/\1.o $@ : ,g' <$@.$$$$ > $@;\
	$(RM) $@.$$$$

%.o:%.c
	@echo [$(CC) compiling $@]
	@$(CC) $(CCFLAGS) $(EXTRA_FLAGS) -c $< -o $@

%.o:%.S
	@echo [$(CC) assembling $@]
	@$(CC) $(ASFLAGS) -c $< -o $@
endif


clean:
	@find .  -regex ".*[.][1-9][0-9][0-9][0-9][0-9]$$" | xargs rm -f
	@find .  -regex ".*[.][1-9][0-9][0-9][0-9]$$" | xargs rm -f
	@rm -f $(all_objs) $(all_deps) $(snpu_objs) $(ckdsp_objs) $(gsc_objs) $(vma_objs) $(vui_objs)
	@rm -rf output
	@echo [Clean all]

distclean: clean
	@find .  -regex ".*[.][1-9][0-9][0-9][0-9][0-9]$$" | xargs rm -f
	@find .  -regex ".*[.][1-9][0-9][0-9][0-9]$$" | xargs rm -f
	@-make -C $(SUBDIR) clean
	@-find . \( -path "./utility/libc/csky" -o -path "./utility/libm/csky" \) -prune -o  -name *.o | xargs rm
	@-find . -name *.d | xargs rm
	@-rm -f .config .config.old
	@-rm -f include/autogenerated_config.h
	@-rm -f include/autoconf.h

help:
	@echo 'Generic Makefile for C/C++ Programs (GNU/linux makefile) version 0.1'
	@echo 'Copyright (C) 2001-2021 NationalChip Co., Ltd'
	@echo 'ALL RIGHTS RESERVED!'
	@echo
	@echo 'Usage: make [TARGET]'
	@echo 'TARGETS:'
	@echo '  all       compile and create elf files(.elf)'
	@echo '  clean     clean all objects and the executable file.'
	@echo '  help      print this message.'
	@echo


#---------------------------------------------------------------------------------#
#   Grus flash bin 文件组成
#
#  * -------------------------------------------
#  * |            |                 |          |
#  * |   struct   |      struct     | bin body |
#  * | bin_header |   stage_info    |          |
#  * |            |                 |          |
#  * -------------------------------------------
#
# #define BIN_HEADER_MAGIC 0x484E4942
#  struct bin_header {
#      unsigned int magic;                // 'B' 'I' 'N' 'H'
#      unsigned int medium_magic;         // 0xaa55aa55 for nor; 0xbb55bb55 for nand
#      unsigned int version;              // CONFIG_SOFT_VERSTION
#  };
#
#  struct stage_info {
#      unsigned int stage2_size;
#      unsigned int stage1_size;
#      unsigned int stage1_load_addr;
#  };
#
#  bin body : stage1 + stage2 header +stage2
#
BOOTLOADER_MAGIC_NOR='AA55AA55'
BIN_HEADER_MEDIUM_MAGIC=$(BOOTLOADER_MAGIC_NOR)
BIN_HEADER_MAGIC_PRE='0x484E4942'
BIN_HEADER_MAGIC=$(shell ./scripts/switch_endian.sh $(shell printf "%08x" $(BIN_HEADER_MAGIC_PRE)))
BIN_SOFT_VERSION=$(shell IFS=. read V1 V2 V3 V4 <<< $(CONFIG_SOFT_VERSTION);printf "%02x%02x%02x%02x" $$V4 $$V3 $$V2 $$V1)

STAGE2_XIP_TEXT_SIZE=$(shell csky-abiv2-elf-readelf -S output/mcu.elf |grep -E ".stage2_xip_text"| awk '{print "0x"$$7}' | xargs printf %d $$1)
STAGE2_SRAM_TEXT_SIZE=$(shell csky-abiv2-elf-readelf -S output/mcu.elf |grep -E ".stage2_sram_text"| awk '{print "0x"$$7}' | xargs printf %d $$1)
STAGE2_SRAM_DATA_SIZE=$(shell csky-abiv2-elf-readelf -S output/mcu.elf |grep -E ".stage2_sram_data"| awk '{print "0x"$$7}' | xargs printf %d $$1)
STAGE2_SIZE=$(shell ./scripts/switch_endian.sh $(shell printf %08x `expr ${STAGE2_XIP_TEXT_SIZE} + ${STAGE2_SRAM_TEXT_SIZE} + ${STAGE2_SRAM_DATA_SIZE}`))

STAGE1_SIZE_PRE=$(shell printf "0x%08x" $(shell grep "CONFIG_STAGE1_SRAM_SIZE" arch/soc/$(CONFIG_SOC_MODEL)/include/soc_config.h | head -n 1 | awk '{print $$3}'))
STAGE1_SIZE=$(shell ./scripts/switch_endian.sh $(shell printf "%08x" $(STAGE1_SIZE_PRE)))
STAGE1_LOAD_ADDR_PRE=$(shell printf "0x%08x" $(shell grep "CONFIG_STAGE1_DRAM_BASE" arch/soc/$(CONFIG_SOC_MODEL)/include/soc_config.h | head -n 1 | awk '{print $$3}'))
STAGE1_LOAD_ADDR=$(shell ./scripts/switch_endian.sh $(shell printf "%08x" $(STAGE1_LOAD_ADDR_PRE)))

MAGIC_NUM := 4
SUFFIX = ".bin"
BOOT_HEAD_NOR=$(BIN_HEADER_MAGIC)$(BIN_HEADER_MEDIUM_MAGIC)$(BIN_SOFT_VERSION)$(STAGE2_SIZE)$(STAGE1_SIZE)$(STAGE1_LOAD_ADDR)
BOOT_HEAD_LEN=24

ORIGIN_FILE = "$(ORIGIN_FILED)$(SUFFIX)"

SOC_STAGE1_SIZE=$(STAGE1_SIZE_PRE)
# 生成经过CRC校验的bin文件
# $1 name (spi_nor/spi_nand/......)
# $2 boot_head
# S3 header_len
# $4 stage1备份的数量，spi_nand备份4份
# $5 目标文件的名称
define CREATE_BIN
	@gcc scripts/loader_write.c -o loader_write
	@mkdir -p output
	@sed '1i$2' output/TEMP_FILE1 > TEMP_FILE2
	@xxd -r -c4 -p TEMP_FILE2 > tmp.bin
	@./loader_write tmp.bin output/$5 $3 $(SOC_STAGE1_SIZE) $4 $6
	@rm -rf loader_write tmp.bin
endef

define CLEAN_TMP_FILE
	@rm -rf output/TEMP_FILE1 TEMP_FILE2 $@
endef

BOARD_NAME = $(shell cat include/autoconf.h | grep -E 'CONFIG_BOARD_MODEL' | awk '{str = $$3 ;print str}')
PROJECT_NAME = $(shell cat include/autoconf.h | grep -E 'CONFIG_PROJECT_NAME' | awk '{str = $$3 ;print str}')
SOFT_VERSTION = $(shell cat include/autoconf.h | grep -E 'CONFIG_SOFT_VERSTION' | awk '{str = $$3 ;print str}')
ZIP_TIME   = $(shell date +"%Y%m%d_%H%M%S")

output/mcu.bin: output/mcu.elf
	@echo [$(OBJCOPY) dump $@]
	@$(OBJCOPY) -O binary $< $@
	@xxd -c4 -p $@ > output/TEMP_FILE1
ifeq ($(CONFIG_MCU_ENABLE_XIP), y)
	@$(call CREATE_BIN,"spi_nor",$(BOOT_HEAD_NOR),$(BOOT_HEAD_LEN),1,"mcu_nor.bin", $(STAGE2_XIP_TEXT_SIZE))
	@$(call CREATE_BIN,"main",$(BOOT_HEAD_NOR),$(BOOT_HEAD_LEN),0,"mcu_main.bin", $(STAGE2_XIP_TEXT_SIZE))
else
	@$(call CREATE_BIN,"spi_nor",$(BOOT_HEAD_NOR),$(BOOT_HEAD_LEN),1,"mcu_nor.bin", 0)
	@$(call CREATE_BIN,"main",$(BOOT_HEAD_NOR),$(BOOT_HEAD_LEN),0,"mcu_main.bin", 0)
endif
	@cd output;xxd -i mcu_main.bin > mcu_main.h;cd -
ifeq ($(CONFIG_DL), y)
	@echo
	@readelf -s output/mcu.elf |grep -E "\<LD_MCU_END_IMAGE_OFFSET\>"|awk '{str = " %d 0x"$$2;print str}' | xargs printf |awk '{str = "if=output/mcu_nor.bin bs=1 count=" $$1 " skip=0 of=output/mcu_flash.bin" ;print str}' | xargs dd
	@readelf -s output/mcu.elf |grep -E "\<LD_EXT_FLASH_IMAGE_SIZE\>"|awk '{str = " %d 0x"$$2;print str}' | xargs printf |awk '{str = "if=output/mcu_nor.bin bs=1 count=" $$1 " skip=524288 of=output/ext_flash.bin" ;print str}' | xargs dd
	@echo
endif

	@rm -f ./output/lvp.zip
	@cp ./docs/lvp_zip_readme.md output/README.md
	@cp ./scripts/download.sh output/download.sh
	#@git rev-parse HEAD >> output/README.md
	@echo "branch info:" >> output/README.md
	#@git branch -v | awk '/\*/' >> output/README.md
	@echo "pc info:" >> output/README.md
	@echo $$USER >> output/README.md
	@pwd >> output/README.md
	@echo "time:" >> output/README.md
	@date >>  output/README.md
	@echo >>  output/README.md
	@md5sum output/mcu_nor.bin > output/mcu_nor.bin.md5
	@zip -q -j ./output/lvp.zip ./output/mcu_nor.bin ./output/mcu.dump.txt ./output/mcu.info.txt ./output/mcu.ld ./.config ./output/README.md ./output/mcu_nor.bin.md5 ./output/download.sh
	@zip -q -j ./output/[$(PROJECT_NAME)]$(BOARD_NAME)-$(ZIP_TIME)-V$(SOFT_VERSTION)-lvp.zip ./output/mcu_nor.bin ./output/mcu.dump.txt ./output/mcu.info.txt ./output/mcu.ld ./.config ./output/README.md ./output/mcu_nor.bin.md5 ./output/download.sh
	@$(CLEAN_TMP_FILE)

output/mcu_win.bin: $(all_objs)
	@echo [Preparing lvp ...]
	@mkdir -p output
	@$(CC) $(CCFLAGS) -E -x assembler-with-cpp -P -o output/mcu.ld arch/soc/$(CONFIG_SOC_MODEL)/link.ld
	@echo [$(LD) linking $@ $(CONFIG_SOC_MODEL)]
	@$(LD) -o output/mcu.elf -T output/mcu.ld $^ $(LDFLAGS)
	create_bin.exe
	@rm -rf output/mcu.bin output/tmp.bin
