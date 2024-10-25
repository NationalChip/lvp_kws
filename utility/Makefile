#
# Voice Signal Preprocess
# Copyright (C) 2001-2018 NationalChip Co., Ltd
#
# Makefile: utility/ source list
#
#=================================================================================#
ifeq ($(CONFIG_MCU_COMPILE_MATH_LIB_FROM_SOURCE_CODE), y)
-include utility/libdsp/Makefile
endif

ifeq ($(CONFIG_MCU_ENABLE_FLOAT_PRINTF), y)
CCFLAGS  += -DPRINTF_DISABLE_SUPPORT_EXPONENTIAL
utility_objs += utility/libc/printf.o
else
utility_objs += utility/libc/tinyprintf.o
utility_objs += utility/libc/vprintf.o
endif
utility_objs += utility/libc/memmove.o
utility_objs += utility/libc/vsprintf.o
#utility_objs += utility/libc/memcpy.o
#utility_objs += utility/libc/memset.o
utility_objs += utility/libc/vsscanf.o
utility_objs += utility/libc/libc_port.o
utility_objs += utility/libc/atoi.o
utility_objs += utility/libc/strcpy.o
utility_objs += utility/libc/strcmp.o
utility_objs += utility/libc/strncmp.o
utility_objs += utility/libc/strlen.o
utility_objs += utility/libc/memfile.o

# tmp malloc
# utility_objs += utility/tmp_malloc/q_malloc.o
# utility_objs += utility/tmp_malloc/fakelibc.o
#NCLUDE_DIR  += utility/tmp_malloc
# utility_objs += utility/test/malloc_test.o

#rt_malloc
utility_objs += utility/rt_malloc/rt_mem.o
INCLUDE_DIR  += utility/rt_malloc/
