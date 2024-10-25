/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_audio_board.h:
 *
 */

#ifndef __LVP_PARAM_H__
#define __LVP_PARAM_H__

#include <autoconf.h>
#include <driver/gx_audio_in.h>
#include <driver/gx_padmux.h>

//=================================================================================================
// Audio In Board Ctrl

enum {
    TRACK_MONO,
    TRACK_STEREO,
};

typedef struct {
    unsigned pcm0    :1;
    unsigned pcm1    :1;
    unsigned logfbank:1;
    unsigned i2s     :1;
    unsigned reserve :28;
} LVP_AUDIO_IN_CHANNEL;

typedef struct {
    unsigned pga_gain               :6;
    unsigned audio_in_gain          :4;
    unsigned dc_enable              :1;
    unsigned pcm_output_track       :1;
    unsigned reserve                :20;
    GX_AUDIO_IN_EVAD            evad;
    GX_AUDIO_IN_EVAD_THRESHOLD  evad_threshold;
} LVP_AUDIO_IN_SOURCE_CONFIG;

typedef struct {
    GX_AUDIO_IN_SOURCE          input_source;
    LVP_AUDIO_IN_SOURCE_CONFIG  input_source_config[3];

    LVP_AUDIO_IN_CHANNEL        output_channel;
    GX_AUDIO_IN_INPUT_SADC      sadc;
    GX_AUDIO_IN_INPUT_PDM       pdm;
    GX_AUDIO_IN_INPUT_I2S       i2s_in;

    GX_AUDIO_IN_OUTPUT_PCM      pcm0;
    GX_AUDIO_IN_OUTPUT_PCM      pcm1;
    GX_AUDIO_IN_OUTPUT_LOGFBANK logfbank;
    GX_AUDIO_IN_OUTPUT_I2S      i2s_out; // note: i2s_out dont have tdm mode
    GX_AUDIO_IN_OUTPUT_SPECTRUM spectrum;

} LVP_AUDIO_IN_PARAM_CTRL;

//=================================================================================================
// Hw Pin Config List

typedef struct {
    unsigned int    count;
    GX_PIN_CONFIG  *pin_config_table;
} LVP_PIN_CONFIG_TABLE;

//=================================================================================================

enum {
    KWS_TYPE_MAJOR = 1,             // 1 << 0
    KWS_TYPE_DEALY2DECODE = 0x40,   // 该唤醒词延迟唤醒 1<<6
    KWS_TYPE_DEALY2DECODE_MASK = 0xf<<16,   // 延迟帧数 Mask 1<<6
    KWS_TYPE_REF_KWS = 0x10,        // 该唤醒词只有ref通道去做唤醒 1<<4
    KWS_TYPE_USE_BS  = 1<<7,        // 该唤醒词会经过Beamsearch进行二次确认压串词
};

enum {
    KWS_LEVEL_TOP1 = 1<<8,   // 第一级KWS
    KWS_LEVEL_TOP2 = 2<<8,   // 第一级KWS
    KWS_LEVEL_TOP3 = 3<<8,   // 第一级KWS
    KWS_LEVEL_TOP4 = 4<<8,   // 第一级KWS
    KWS_LEVEL_TOP5 = 5<<8,   // 第一级KWS
    KWS_LEVEL_TOP_MASK = 0xf<<8,   // 第一级KWS Mask
    KWS_LEVEL_MID = 1<<14,   // 第二级KWS
    KWS_LEVEL_END = 1<<15,   // 第三级KWS
    KWS_LEVEL_MASK = 0xff<<8, // KWS Level Mask
};
// Key Words Param List

typedef struct {
    char kws_words[40];          // Key words
    short labels[16];             // The label of key words
    int  label_length;           // The length of key label
    int  threshold;              // The threshold of kws
#ifdef CONFIG_KWS_TYPE_HYBRID
    int  xip_threshold;         // The threshold of xip kws
#endif
#ifdef CONFIG_ENABLE_KWS_LEVEL_DECODER
    int js_threshold;
#endif
    int  kws_value;              // The event id for mcu
    int  major;                  // bit 0 : 1 -->> Major Kws, 0 -->> Shor instruction
                                 // bit 7 -->> beamsearch
                                 // bit 8,9,10,11,12,13,14,15 -->> kws level
                                 // bit 16,17,18,19 delay number
} LVP_KWS_PARAM;

typedef struct {
    unsigned int    count;
    LVP_KWS_PARAM  *kws_param_list;
} LVP_KWS_PARAM_LIST;

typedef struct {
    unsigned char *lm;
    unsigned int   lm_size;
    unsigned char *lst;
    unsigned int   lst_size;
} LANGUAGE_MODEL_PARAM;
//=================================================================================================

enum {
    ENV_LOW_NOISE,
    ENV_MID_NOISE,
    ENV_HIGH_NOISE,
};

//=================================================================================================

#define MAX_STR_STASH 30

typedef struct {
    int  id;
    char kws_words[20];          // Key words
    char labels[14];             // The label of key words
    int  label_length;           // The length of key label
    int  prev;
    int  next;
    int  ctc_enable;            //0: only beamSearch; 1: ctc
    int  threshold;
} LVP_KWS_SEGMENTATION_PARAM;

enum {
    DELAY = -1,
    ROOT = 0,
    LEAF = 0,
    COMMON_MASK_0  =  1 <<  0 ,
    COMMON_MASK_1  =  1 <<  1 ,
    COMMON_MASK_2  =  1 <<  2 ,
    COMMON_MASK_3  =  1 <<  3 ,
    COMMON_MASK_4  =  1 <<  4 ,
    COMMON_MASK_5  =  1 <<  5 ,
    COMMON_MASK_6  =  1 <<  6 ,
    COMMON_MASK_7  =  1 <<  7 ,
    COMMON_MASK_8  =  1 <<  8 ,
    COMMON_MASK_9  =  1 <<  9 ,
    COMMON_MASK_10  =  1 <<  10 ,
    COMMON_MASK_11  =  1 <<  11 ,
    COMMON_MASK_12  =  1 <<  12 ,
    COMMON_MASK_13  =  1 <<  13 ,
    COMMON_MASK_14  =  1 <<  14 ,
    COMMON_MASK_15  =  1 <<  15 ,
    COMMON_MASK_16  =  1 <<  16 ,
    COMMON_MASK_17  =  1 <<  17 ,
    COMMON_MASK_18  =  1 <<  18 ,
    COMMON_MASK_19  =  1 <<  19 ,
    COMMON_MASK_20  =  1 <<  20 ,
    COMMON_MASK_21  =  1 <<  21 ,
    COMMON_MASK_22  =  1 <<  22 ,
    COMMON_MASK_23  =  1 <<  23 ,
    COMMON_MASK_24  =  1 <<  24 ,
    COMMON_MASK_25  =  1 <<  25 ,
    COMMON_MASK_26  =  1 <<  26 ,
    COMMON_MASK_27  =  1 <<  27 ,
    COMMON_MASK_28  =  1 <<  28 ,
    COMMON_MASK_29  =  1 <<  29 ,
    COMMON_MASK_30  =  1 <<  30 ,
    COMMON_MASK_31  =  1 <<  31 ,
};
#endif /* __LVP_PARAM_H__*/
