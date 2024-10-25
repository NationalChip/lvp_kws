3rdparty_mkfile := $(abspath $(lastword $(MAKEFILE_LIST)))
3rdparty_path := $(patsubst %/, %, $(dir $(3rdparty_mkfile)))

ifeq ($(CONFIG_USE_OPUS), y)
-include $(3rdparty_path)/opus/Makefile
endif

ifeq ($(CONFIG_USE_AMR), y)
-include $(3rdparty_path)/opencore-amr/Makefile
endif

ifeq ($(CONFIG_USE_MP3), y)
-include $(3rdparty_path)/helix_mp3/helix_mp3.mk
endif

-include $(3rdparty_path)/adpcm/Makefile

