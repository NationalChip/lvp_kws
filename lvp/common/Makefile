#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: lvp/common source list
#
#=================================================================================#
-include lvp/common/snpu_engine/Makefile
ifeq ($(CONFIG_LVP_USE_BUFFER_V2), y)
common_objs += lvp/common/lvp_buffer_v2/lvp_base_buffer.o
ifeq ($(CONFIG_LVP_HAS_TWS_MODE), y)
common_objs += lvp/common/lvp_buffer_v2/lvp_tws_buffer.o
endif
ifeq ($(CONFIG_LVP_HAS_DENOISE_MODE), y)
common_objs += lvp/common/lvp_buffer_v2/lvp_denoise_buffer.o
endif
else
common_objs +=lvp/common/lvp_buffer.o
endif

common_objs += lvp/common/lvp_queue.o
common_objs += lvp/common/lvp_audio_in.o
common_objs += lvp/common/lvp_i2c_msg.o
common_objs += lvp/common/lvp_pmu.o
common_objs += lvp/common/lvp_system_init.o
common_objs += lvp/common/lvp_audio_out.o

ifeq ($(CONFIG_LVP_HAS_UART_MESSAGE_2_0), y)
common_objs += lvp/common/uart_message_v2.o
endif

ifeq ($(CONFIG_LVP_HAS_UART_RECORD), y)
common_objs += lvp/common/lvp_uart_record.o
endif
ifeq ($(CONFIG_LVP_HAS_G_SENSOR_DEVICE), y)
-include lvp/common/G_vad/Makefile
common_objs += $(gvad_objs)
endif

ifeq ($(CONFIG_LVP_HAS_VOICE_PLAYER), y)
-include lvp/common/player/voice_player/player.mk
common_objs += $(player_objs)
endif

ifeq ($(CONFIG_LVP_HAS_MP3_PLAYER), y)
-include lvp/common/player/mp3_player/player.mk
common_objs += $(player_objs)
endif

ifeq ($(CONFIG_LVP_FACTORY_MIC_TEST), y)
common_objs += lvp/common/lvp_mic_test.o
endif

ifeq ($(CONFIG_LVP_ENABLE_IDLE_CTRL), y)
common_objs += lvp/common/lvp_idle_ctrl.o
endif

ifeq ($(CONFIG_ENABLE_CACULATE_STANDBY_RATIO), y)
common_objs += lvp/common/lvp_standby_ratio.o
endif

ifeq ($(CONFIG_LVP_HAS_LEO_MINI_UART_UPGRADE),y)
-include lvp/common/uart_upgrade/Makefile
common_objs += $(upgrade_objs)
endif

include lvp/common/peripherals/peripherals.mk

