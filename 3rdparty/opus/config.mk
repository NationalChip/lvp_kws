OPUS_CFLAGS+=   -DELF   -msoft-float -mcpu=ck803efr1 -DCPU_CSKY

ifndef CROSS_COMPILE
CROSS_COMPILE=csky-abiv2-elf-
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
LD=$(CROSS_COMPILE)ld
OBJDUMP=$(CROSS_COMPILE)objdump
OBJCOPY=$(CROSS_COMPILE)objcopy
AR=$(CROSS_COMPILE)ar
endif
RANLIB=$(CROSS_COMPILE)ranlib

ifdef SEMI
OPUS_LDFLAGS+=-EL  -mcpu=ck803er1 -Wl,--whole-archive -lsemi -Wl,-no-whole-archive
OPUS_OPUS_CFLAGS+= -DSEMI
else
OPUS_LDFLAGS+=-EL  -mcpu=ck803er1
endif
OPUS_CXXFLAGS+=   -DELF   -msoft-float -mcpu=ck803efr1 -O2 -DCPU_CSKY
opus_soc = sl03
opus_board = sl03_evb
opus_cpu = ck803S
OPUS_CFLAGS+= -O2
