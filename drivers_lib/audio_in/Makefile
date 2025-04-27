AUDIO_IN_VERSION = $(patsubst "%",%,$(CONFIG_GX_AUDIO_IN_VERSION))

ifeq ($(CONFIG_GX_AUDIO_IN_VERSION), "v1.0")
	cobj-$(CONFIG_GX_AUDIO_IN_TEST) += drivers/audio_in/$(AUDIO_IN_VERSION)/adc/gx_ck20810.o
endif

ifeq ($(CONFIG_GX_AUDIO_IN_VERSION), "v2.0")
	cobj-$(CONFIG_GX_AUDIO_IN_TEST) += drivers/audio_in/$(AUDIO_IN_VERSION)/audio_in_uart.o
endif

ifeq ($(CONFIG_GX_AUDIO_IN_VERSION), "v3.0")
	AUDIO_IN_VERSION = $(patsubst "%",%,$(CONFIG_GX_AUDIO_IN_VERSION))/audio_in
	AUDIO_IN_LITE_VERSION = $(patsubst "%",%,$(CONFIG_GX_AUDIO_IN_VERSION))/audio_in_lite
	cobj-$(CONFIG_GX_AUDIO_IN_LITE) += drivers/audio_in/$(AUDIO_IN_LITE_VERSION)/audio_in_lite.o
	cobj-$(CONFIG_GX_AUDIO_IN_LITE_TEST) += drivers/audio_in/$(AUDIO_IN_LITE_VERSION)/audio_in_lite_test.o
endif

cobj-$(CONFIG_GX_AUDIO_IN) += drivers/audio_in/$(AUDIO_IN_VERSION)/audio_in.o
cobj-$(CONFIG_GX_AUDIO_IN_TEST) += drivers/audio_in/$(AUDIO_IN_VERSION)/audio_in_test.o
