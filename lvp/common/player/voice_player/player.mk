INCLUDE_DIR += lvp/common/player/voice_player/
INCLUDE_DIR += lvp/common/player/

player_objs += lvp/common/player/voice_player/lvp_voice_player.o

ifeq ($(CONFIG_USE_OPUS), y)
player_objs += lvp/common/player/decoder/opus_decoder.o
endif

ifeq ($(CONFIG_USE_AMR), y)
player_objs += lvp/common/player/decoder/amr_decoder.o
endif

ifeq ($(CONFIG_USE_ADPCM), y)
player_objs += lvp/common/player/decoder/adpcm_decoder.o
endif