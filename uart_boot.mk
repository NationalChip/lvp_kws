#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: MCU Top Makefile
#
#=================================================================================#
# Toolchain Configurations
.PHONY: defconfig menuconfig post_config autoconf

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

defconfig : .config
	@scripts/autoconf.sh > /dev/null
	@scripts/config_parse.sh > /dev/null

menuconfig : $(obj)/mconf $(obj)/conf
	$(Q)$< $(CONFIG_KCONFIG)
	@scripts/autoconf.sh > /dev/null
	@scripts/config_parse.sh > /dev/null

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
#ifeq ($(CONFIG_MCU_ENABLE_JTAG_DEBUG), y)
    CCFLAGS    += -g
#endif

VERSION=0x42555858
CCFLAGS += -DRELEASE_VERSION=$(VERSION)

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
MCU_VERSION    := "$(shell git describe --dirty --always --tags)"
CCFLAGS        += -DMCU_VERSION=\"$(MCU_VERSION)\"
ASFLAGS        += -DMCU_VERSION=\"$(MCU_VERSION)\"

# The build time of mcu
BUILD_TIME      = $(shell date +"%Y-%m-%d,\ %H:%M:%S")
CCFLAGS        += -DBUILD_TIME=\"$(BUILD_TIME)\"

# The options used in linking as well as in any direct use of ld.
LDFLAGS         = -EL

# The options used in archive as well as in any direct use of ar.
ARFLAGS         = -rcs

CCFLAGS        += -ffunction-sections -nostdlib
LDFLAGS        += --gc-sections

CCFLAGS        += -DCONFIG_ARCH_GRUS
#---------------------------------------------------------------------------------#

INCLUDE_DIR    += arch/soc/$(CONFIG_SOC_MODEL)/include
INCLUDE_DIR    += boards/$(CONFIG_BOARD_VENDOR)/$(CONFIG_BOARD_MODEL)/include
INCLUDE_DIR    += include/utility/libdsp
INCLUDE_DIR    += include/utility/libc
INCLUDE_DIR    += include/utility
INCLUDE_DIR    += include
INCLUDE_DIR    += lvp/common
#INCLUDE_DIR    += lvp/kws
INCLUDE_DIR    += base/include
INCLUDE_DIR    += lvp/vui/kws
INCLUDE_DIR    += lvp
INCLUDE_DIR    += lvp/vui/denoise

ifeq ($(CONFIG_DENOISE_ENABLE_GSC), y)
LIBS           += -lgsc_earphone
endif
LIBS           += -lm -lc
LIBS           += -lgcc
LIBS           += -lcsky_CK804ef_dsp2_nn -lcsky_dsp -lvma -lcsky_dsp

ifeq ($(CONFIG_ENABLE_NPU_DEBUG), y)
LIBS           += -lsnpu_debug
else
LIBS           += -lsnpu
endif
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
-include drivers/Makefile
-include boards/Makefile
-include lvp/Makefile
-include 3rdparty/Makefile
-include base/Makefile
-include app/Makefile

ifdef CONFIG_MCU_COMPILE_MATH_LIB_FROM_SOURCE_CODE
$(ckdsp_objs):EXTRA_FLAGS := -mhard-float -D__FPU_PRESENT -fdata-sections -Wl,--gc-sections -DUNALIGNED_SUPPORT_DISABLE -DCSKY_SIMID
endif

lib_objs        = $(boot_objs) $(dev_objs) $(utility_objs) $(common_objs) $(lvp_objs) $(kws_objs) $(denoise_objs) $(ctc_decoder_objs) $(vpr_objs) $(app_objs)

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

mcu_objs        = $(subst ",,$(lib_objs))
all_objs        = $(mcu_objs) $(lvp_objs) $(base_objs)
all_deps        = $(subst .o,.d,$(all_objs))

#=================================================================================#
# Build Rules

.PHONY: clean export help

ifeq (,$(wildcard $(CURDIR)/.config))# check .config
# Build .config
build: .config

else # .config is exit
# Build objects
build: prepare output/libmcu.a lib/libsnpu.a lib/libcsky_dsp.a lvp/vma/gsc/libgsc_earphone.a output/mcu.elf output/mcu.info.txt output/mcu.dump.txt
ifneq ($(CONFIG_MCU_ENABLE_JTAG_DEBUG), y)
build: output/mcu.bin checkbin
endif # endif ifneq ($(CONFIG_MCU_ENABLE_JTAG_DEBUG), y)

endif # endif ifeq (,$(wildcard $(CURDIR)/.config))

.config:
	@make menuconfig

prepare:
	@echo [Preparing lvp ...]
	@mkdir -p output
	@$(CC) $(CCFLAGS) $(EXTRA_FLAGS) -E -x assembler-with-cpp -P -o output/mcu.ld arch/soc/$(CONFIG_SOC_MODEL)/link.ld

ifdef snpu_objs
lib/libsnpu.a: $(snpu_objs)
	@rm -f $@
	@echo [$(AR) archiving $@]
	@$(AR) $(ARFLAGS) $@ $^
endif

ifdef ckdsp_objs
lib/libcsky_dsp.a: $(ckdsp_objs)
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
	@$(LD) -o $@ -T $^ $(LDFLAGS)
	@size $@

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
	@rm -f $(all_objs) $(all_deps) $(snpu_objs) $(ckdsp_objs) $(gsc_objs)
	@rm -rf output
	@echo [Clean all]

distclean: clean
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
#  * | bin_header |   stage1_info   |          |
#  * |            |                 |          |
#  * -------------------------------------------
#
# #define BIN_HEADER_MAGIC 0x484E4942
#  struct bin_header {
#      unsigned int magic;                // 'B' 'I' 'N' 'H'
#      unsigned int medium_magic;         // 0xaa55aa55 for nor; 0xbb55bb55 for nand
#      unsigned int version;              // 0x00000001
#  };
#
#  struct stage1_info_v1 {
#      unsigned int stage1_addr;
#      unsigned int stage1_size;
#      unsigned int stage1_load_addr;
#  };
#
#  bin body : stage1 + stage2
#
BOOTLOADER_MAGIC_NOR='AA55AA55'
BIN_HEADER_MEDIUM_MAGIC=$(BOOTLOADER_MAGIC_NOR)
BIN_HEADER_MAGIC_PRE='0x484E4942'
BIN_HEADER_MAGIC=$(shell ./scripts/switch_endian.sh $(shell printf "%08x" $(BIN_HEADER_MAGIC_PRE)))
BIN_HEADER_VERSION_PRE='0x00000001'
BIN_HEADER_VERSION=$(shell ./scripts/switch_endian.sh $(shell printf "%08x" $(BIN_HEADER_VERSION_PRE)))

STAGE1_ADDR_PRE='0x00000000'
STAGE1_ADDR=$(shell ./scripts/switch_endian.sh $(shell printf "%08x" $(STAGE1_ADDR_PRE)))
STAGE1_SIZE_PRE=$(shell printf "0x%08x" $(shell grep "CONFIG_STAGE1_SRAM_SIZE" arch/soc/$(CONFIG_SOC_MODEL)/include/soc_config.h | head -n 1 | awk '{print $$3}'))
STAGE1_SIZE=$(shell ./scripts/switch_endian.sh $(shell printf "%08x" $(STAGE1_SIZE_PRE)))
STAGE1_LOAD_ADDR_PRE=$(shell printf "0x%08x" $(shell grep "CONFIG_STAGE1_DRAM_BASE" arch/soc/$(CONFIG_SOC_MODEL)/include/soc_config.h | head -n 1 | awk '{print $$3}'))
STAGE1_LOAD_ADDR=$(shell ./scripts/switch_endian.sh $(shell printf "%08x" $(STAGE1_LOAD_ADDR_PRE)))

MAGIC_NUM := 4
#SUFFIX = ".bin"
BOOT_HEAD_NOR=$(BIN_HEADER_MAGIC)$(BIN_HEADER_MEDIUM_MAGIC)$(BIN_HEADER_VERSION)$(STAGE1_ADDR)$(STAGE1_SIZE)$(STAGE1_LOAD_ADDR)
BOOT_HEAD_LEN=24


BOOTLOADER_MAGIC_UART='02800101'
ORIGIN_FILED="gxscpu"
LONG=$(INT)$(INT)
INT='00000000'
CHAR='00'
CONFIG_BOOT_DELAY=0
CONFIG_BOOT_HEADER_SERIAL_BAUD_RATE=7

BOOTLOADER_MAGIC=$(BOOTLOADER_MAGIC_UART)
TMP=$(shell printf "%04x" $(CONFIG_BOOT_DELAY))
BOOT_DELAY=$(shell echo $(TMP) | cut -b 3-4)$(shell echo $(TMP) | cut -b 1-2)
BOOT_HEAD_SERIAL_BAUD_RATE=$(shell printf "%02x" $(CONFIG_BOOT_HEADER_SERIAL_BAUD_RATE))
MAGIC_NUM := 40
SUFFIX = ".boot"
BOOT_HEAD_UART=$(BOOTLOADER_MAGIC)$(BOOT_DELAY)$(BOOT_HEAD_SERIAL_BAUD_RATE)$(CHAR)$(LONG)$(LONG)$(LONG)$(LONG)
BOOT_HEAD_LEN=40

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
	@./loader_write tmp.bin output/$5 $3 $(SOC_STAGE1_SIZE) $4
	@rm -rf loader_write tmp.bin
endef


# 生成经过CRC校验的boot文件
# $1 name (boot)
# $2 boot_head
# S3 header_len
# $4 stage1备份的数量
# $5 目标文件的名称
define CREATE_BOOT
    @gcc scripts/loader_write.c -o loader_write
	@xxd -r -c4 -p output/TEMP_FILE1 > tmp.bin
	@./loader_write tmp.bin TEMP_FILE2 $3 $(SOC_STAGE1_SIZE) $4
	@xxd -c4 -p TEMP_FILE2 > TEMP_FILE3
	@sed '1i$2' TEMP_FILE3 > TEMP_FILE4
	@xxd -r -c4 -p TEMP_FILE4 > output/$5
	@rm -f loader_write tmp.bin TEMP_FILE3 TEMP_FILE4
endef

define CLEAN_TMP_FILE
	@rm -rf output/TEMP_FILE1 TEMP_FILE2 $@
endef

output/mcu.bin: output/mcu.elf
	@echo [$(OBJCOPY) dump $@]
	@$(OBJCOPY) -O binary --remove-section=.dummy_for_stage1_text_place_hold --remove-section=.dummy_for_stage2_text_place_hold $< $@
	@xxd -c4 -p $@ > output/TEMP_FILE1
	@$(call CREATE_BOOT,"mcu_uart",$(BOOT_HEAD_UART),0,1,mcu_uart.bin)
	@csky-abiv2-elf-readelf -S output/mcu.elf |grep -E "\<stage2_sram_text\>|\<stage2_sram_data\>"| awk '{str = "0x"$$7 ;print str}' > stage2_size.txt
ifeq ($(CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION), y)
	@csky-abiv2-elf-readelf -s output/mcu.elf |grep -E "\<_start_npu_section_\>|\<_end_npu_section_\>"| awk '{str = "0x"$$2 ;print str}' > npu_cmd_weight_size.txt
else ifeq ($(CONFIG_LVP_ENABLE_NN_DENOISE), y)
	@csky-abiv2-elf-readelf -s output/mcu.elf |grep -E "\<_start_nc_npu_section_\>|\<_end_nc_npu_section_\>"| awk '{str = "0x"$$2 ;print str}' > npu_cmd_weight_size.txt
else
	@csky-abiv2-elf-readelf -s output/mcu.elf |grep -E "\<_start_npu_section_\>|\<_end_npu_section_\>"| awk '{str = "0x"$$2 ;print str}' > npu_cmd_weight_size.txt
endif
	@csky-abiv2-elf-readelf -s output/mcu.elf |grep -E "\<LD_NPU_IMAGE_OFFSET\>"| awk '{str ="0x"$$2 ;print str}' > npu_offset.txt
	@gcc scripts/patch_boot.c -o patch_boot_size
	@./patch_boot_size output/mcu_uart.bin
	@rm stage2_size.txt npu_cmd_weight_size.txt npu_offset.txt
	@$(CLEAN_TMP_FILE)
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

