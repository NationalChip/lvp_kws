/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * audio_board.c
 *
 */

#include <autoconf.h>
#include <board_config.h>
#include <common.h>

#include <driver/gx_audio_in.h>
#include <lvp_param.h>

//=================================================================================================

static LVP_AUDIO_IN_PARAM_CTRL g_audio_in_param_ctrl = {
    .input_source       = 0
#ifdef CONFIG_TYPE_AMIC
                         | AUDIO_IN_SADC
#endif
#ifdef CONFIG_TYPE_DMIC
                         | AUDIO_IN_PDM
#endif
#if (defined CONFIG_TYPE_I2S_MASTER || defined CONFIG_TYPE_I2S_SLAVE)
                         | AUDIO_IN_I2S
#endif
                         ,

    .input_source_config = {
        {
            .pga_gain           = 30,
            .audio_in_gain      = AUDIO_IN_GAIN_0dB,

            .dc_enable          = 1,
#ifdef CONFIG_TYPE_AMIC
            .pcm_output_track   = (CONFIG_TYPE_AMIC_TRACK == 1) ? TRACK_MONO : TRACK_STEREO,
#else
            .pcm_output_track   = TRACK_MONO,
#endif
            .evad = {
                .evad_enable = 0,
                .zcr_enable  = 0,
                .state_valid_enable  = 1,
            },
            .evad_threshold = {
                .evad_low_threshold  = 0,
                .evad_high_threshold = 0,
                .evad_zcr_threshold  = 0,
            },
        },
        {
            .pga_gain           = 0,
            .audio_in_gain      = AUDIO_IN_GAIN_18dB,

            .dc_enable          = 1,
#ifdef CONFIG_TYPE_DMIC
            .pcm_output_track   = (CONFIG_TYPE_DMIC_TRACK == 1) ? TRACK_MONO : TRACK_STEREO,
#else
            .pcm_output_track   = TRACK_MONO,
#endif
            .evad = {
                .evad_enable = 0,
                .zcr_enable  = 0,
                .state_valid_enable  = 1,
            },
            .evad_threshold = {
                .evad_low_threshold  = 0,
                .evad_high_threshold = 0,
                .evad_zcr_threshold  = 0,
            },
        },
        {
            .pga_gain           = 0,
            .audio_in_gain      = AUDIO_IN_GAIN_0dB,

            .dc_enable          = 1,

#if (defined CONFIG_TYPE_I2S_MASTER || defined CONFIG_TYPE_I2S_SLAVE)
            .pcm_output_track   = (CONFIG_TYPE_I2S_TRACK == 1) ? TRACK_MONO : TRACK_STEREO,
#else
            .pcm_output_track   = TRACK_MONO,

#endif
            .evad = {
                .evad_enable = 0,
                .zcr_enable  = 0,
                .state_valid_enable  = 1,
            },
            .evad_threshold = {
                .evad_low_threshold  = 0,
                .evad_high_threshold = 0,
                .evad_zcr_threshold  = 0,
            },
        },


    },


    .sadc = {
        .pga_enable = 1,
    },
    .pdm = {
        .clk = AUDIO_IN_PDM_1M,
    },
    .i2s_in = {
        .fsync_mode = FSYNC_MODE_SHORT,
        .i2s.pcm_length  = PCM_LENGTH_16BIT,
        .i2s.data_format = DATA_FORMAT_I2S,
        .i2s.bclk_sel    = BCLK_MODE_64FS,
        .i2s.i2s_fs      = SAMPLE_RATE_16K,
    },


    .output_channel  = {
#if (defined CONFIG_TYPE_AMIC || (defined CONFIG_TYPE_DMIC && (defined CONFIG_TYPE_I2S_MASTER || defined CONFIG_TYPE_I2S_SLAVE)))
        .pcm0 = 1,
#else
        .pcm0 = 0,
#endif
#if (defined CONFIG_TYPE_DMIC || defined CONFIG_TYPE_I2S_MASTER || defined CONFIG_TYPE_I2S_SLAVE)
        .pcm1 = 1,
#else
        .pcm1 = 0,
#endif
        .logfbank = 1,
        .i2s = 0,
        .reserve = 0,
    },
    .pcm0 = {
        .endian = 0,
        .source = PCM_SOURCE_SADC,
    },
    .pcm1 = {
        .endian = 0,
        .source = PCM_SOURCE_PDM,
    },
    .logfbank = {
        .endian = 0,
#if (defined CONFIG_TYPE_AMIC)
        .source = LOGFBANK_SOURCE_SADC,
#elif (defined CONFIG_TYPE_DMIC)
        .source = LOGFBANK_SOURCE_PDM_LEFT,
#elif (defined CONFIG_TYPE_I2S_MASTER || defined CONFIG_TYPE_I2S_SLAVE)
        .source = LOGFBANK_SOURCE_I2SIN_LEFT,
#endif
    },
    .i2s_out = {
        .i2s.pcm_length  = PCM_LENGTH_16BIT,
        .i2s.data_format = DATA_FORMAT_I2S,
        .i2s.bclk_sel    = BCLK_MODE_64FS,
        .i2s.i2s_fs      = SAMPLE_RATE_16K,

        .left_source     = I2S_SOURCE_I2SIN_LEFT,
        .right_source    = I2S_SOURCE_I2SIN_RIGHT,
    }
};

LVP_AUDIO_IN_PARAM_CTRL *AudioInBoardGetParamCtrl(void)
{
	return &g_audio_in_param_ctrl;
}

int _AudioInGainMap(int gain)
{
    switch (gain) {
        case AUDIO_IN_GAIN_0dB:
            return 0;
        case AUDIO_IN_GAIN_6dB:
            return 6;
        case AUDIO_IN_GAIN_12dB:
            return 12;
        case AUDIO_IN_GAIN_18dB:
            return 18;
        case AUDIO_IN_GAIN_24dB:
            return 24;
        case AUDIO_IN_GAIN_30dB:
            return 30;
        case AUDIO_IN_GAIN_36dB:
            return 36;
        case AUDIO_IN_GAIN_42dB:
            return 42;
        case AUDIO_IN_GAIN_48dB:
            return 48;
        case AUDIO_IN_GAIN_54dB:
            return 54;
    }

    return 0;
}

void AudioInBoardInit(void)
{
#ifdef CONFIG_TYPE_AMIC
    printf (LOG_TAG"amic          [1 Channel]\n");
    printf (LOG_TAG"amic pga_gain:[%d dB]\n", g_audio_in_param_ctrl.input_source_config[0].pga_gain);
    printf (LOG_TAG"amic Ain_gain:[%d dB]\n", _AudioInGainMap(g_audio_in_param_ctrl.input_source_config[0].audio_in_gain));
#endif
#ifdef CONFIG_TYPE_DMIC
    printf (LOG_TAG"dmic          [%d Channel]\n", (CONFIG_TYPE_DMIC_TRACK == 1) ? 1 : 2);
    printf (LOG_TAG"dmic Ain_gain:[%d dB]\n", _AudioInGainMap(g_audio_in_param_ctrl.input_source_config[1].audio_in_gain));
#endif
}

