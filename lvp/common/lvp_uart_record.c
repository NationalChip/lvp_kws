/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_uart_record.c:
 *
 */
#include <autoconf.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <csi_core.h>

#include <driver/gx_audio_in.h>
#include <driver/gx_pmu_ctrl.h>
#include <driver/gx_watchdog.h>
#include <driver/gx_delay.h>
#include <driver/gx_clock.h>
#include <driver/gx_irq.h>
#include <driver/gx_snpu.h>
#include <driver/gx_timer.h>
#include <driver/gx_gpio.h>
#include <driver/gx_dcache.h>

#include <lvp_context.h>
#include <lvp_buffer.h>
#include <lvp_board.h>

#include "lvp_queue.h"
#include "lvp_audio_in.h"
#include "uart_message_v2.h"
#include "lvp_uart_record.h"
#include "lvp_pmu.h"

#define LOG_TAG "[LVP_RECORD]"

//=================================================================================================

#define UART_RECORD_PORT CONFIG_LVP_UART_RECORD_PORT
#define UART_RECORD_PORT_BAUDRATE CONFIG_LVP_UART_RECORD_PORT_BAUDRATE

typedef struct {
    int                     sample_rate;    // range : 0x01 - 0x07 : 48000 44100 32000 24000 22050 16000 11025 8000
    unsigned int            volume;         // default 0: 0dB, -18 ~ 18 dB
    unsigned int            channel_type;   // 1:STEREO and 0:MONO ,support MONO only for now
    unsigned int            sample_bit;     // support 16:16bit for now
} UART_VOICE_PLAY_CONFIG;

typedef struct {
    unsigned int baudrate;              // 0:default;others:Limited support
    unsigned int channel_num;           // 0or1: one channel;2: tow channel;others: Not support;
    unsigned int channel;
    unsigned int flow_ctrl_enable;
} UART_RECORD_CONFIG;

typedef struct {
    UART_RECORD_CONFIG config;
    unsigned int    flow_ctrl;
    int             record_on;
} UART_RECORD_HANDLE;

static UART_RECORD_HANDLE s_record_handle = {
    .flow_ctrl = 0,
    .record_on = 0,
    .config.channel = 1
};

static int _UartRecordPostInfo(unsigned char *info, unsigned int info_len);

static int _uartRecvCallback(MSG_PACK * pack, void *priv)
{
    if (pack->msg_header.cmd == MSG_NTF_PCM_READY) {
        s_record_handle.flow_ctrl = (s_record_handle.flow_ctrl == 2) ? 2 : 1;
    }

    if (pack->msg_header.cmd == MSG_NTF_START_PCM) {
        if ( pack->len == sizeof(UART_RECORD_CONFIG)) {
            UART_RECORD_CONFIG *config = (UART_RECORD_CONFIG *)pack->body_addr;
            LVP_CONTEXT *context;
            unsigned int ctx_size;
            LvpGetContext(0, &context, &ctx_size);
            if ((context->ctx_header->mic_num == 1) && (s_record_handle.config.channel & 0x2)) {
                config->channel = 0;
                s_record_handle.flow_ctrl = 0;
                return -1;
            }
            if (s_record_handle.config.flow_ctrl_enable == 0) {
                s_record_handle.flow_ctrl = 2;
            } else {
                s_record_handle.flow_ctrl = 1;
            }
            UartRecordInit(UART_RECORD_PORT, config->baudrate);
            _UartRecordPostInfo((unsigned char*)&s_record_handle.config, sizeof(UART_RECORD_CONFIG));
            s_record_handle.record_on = 1;
        } else {
            return -1;
        }
    }

    if (pack->msg_header.cmd == MSG_NTF_STOP_PCM) {
        s_record_handle.flow_ctrl = 0;
        s_record_handle.record_on = 0;
    }
    return 0;
}

static int _UartRecordSend(unsigned char *send_buffer, unsigned int send_len)
{
    MSG_PACK send_pack_data = {
        .port = UART_RECORD_PORT,
        .msg_header.magic = MSG_HOST_MAGIC,
        .msg_header.cmd = MSG_NTF_PCM_DATA,
        .body_addr = (unsigned char*)send_buffer,
        .len = send_len
    };


    UartMessageAsyncSend(&send_pack_data);
    return 0;
}

int UartRecordChannelTask(unsigned char *send_buffer, unsigned int send_len, RECORD_CHANNEL_MASK channel_mask)
{
    if (s_record_handle.config.channel & channel_mask) {
        _UartRecordSend(send_buffer, send_len);
        return 0;
    }
    return -1;
}

int UartRecordTask(LVP_CONTEXT *context)
{
    if (s_record_handle.flow_ctrl == 0)
        return -1;
    s_record_handle.flow_ctrl = (s_record_handle.flow_ctrl == 2) ? 2 : 0;

    LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;

    if (s_record_handle.config.channel & RECORD_CHANNEL_MIC0) {
        unsigned int  mic_buffer_len_per_context = ctx_header->pcm_frame_num_per_context * \
                                                   ctx_header->frame_length * \
                                                   ctx_header->sample_rate * 2 / 1000;
        unsigned char *cur_mic_buffer = (unsigned char *)ctx_header->mic_buffer + \
                                        mic_buffer_len_per_context * \
                                        (context->ctx_index % \
                                         (ctx_header->mic_buffer_size / ctx_header->mic_num / mic_buffer_len_per_context));
        gx_dcache_invalid_range((unsigned int*)cur_mic_buffer, mic_buffer_len_per_context);
        _UartRecordSend(cur_mic_buffer, mic_buffer_len_per_context);
    }

    if ((s_record_handle.config.channel & RECORD_CHANNEL_MIC1) && ctx_header->mic_num > 1) {
        unsigned int  mic_buffer_len_per_context = ctx_header->pcm_frame_num_per_context * \
                                                   ctx_header->frame_length * \
                                                   ctx_header->sample_rate * 2 / 1000;
        unsigned char *cur_mic_buffer = (unsigned char *)ctx_header->mic_buffer + ctx_header->mic_buffer_size / ctx_header->mic_num + \
                                        mic_buffer_len_per_context * \
                                        (context->ctx_index % \
                                         (ctx_header->mic_buffer_size / ctx_header->mic_num / mic_buffer_len_per_context));
        gx_dcache_invalid_range((unsigned int*)cur_mic_buffer, mic_buffer_len_per_context);
        _UartRecordSend(cur_mic_buffer, mic_buffer_len_per_context);
    }

    if (s_record_handle.config.channel & RECORD_CHANNEL_FFT) {
        unsigned int send_len = ctx_header->fft_buffer_size / ctx_header->fft_frame_num_per_channel * ctx_header->pcm_frame_num_per_context;

        unsigned char *send_buffer = (unsigned char *)ctx_header->fft_buffer + \
                                     send_len * (context->ctx_index % (ctx_header->fft_frame_num_per_channel / ctx_header->pcm_frame_num_per_context));
        gx_dcache_invalid_range((unsigned int*)send_buffer, send_len);
        _UartRecordSend(send_buffer, send_len);
    }

    if (s_record_handle.config.channel & RECORD_CHANNEL_LOGFBANK) {
        unsigned int send_len = ctx_header->logfbank_buffer_size / ctx_header->logfbank_frame_num_per_channel * ctx_header->pcm_frame_num_per_context;

        unsigned char *send_buffer = (unsigned char *)LvpGetLogfbankBuffer(context, context->ctx_index);

        gx_dcache_invalid_range((unsigned int*)send_buffer, send_len);
        _UartRecordSend(send_buffer, send_len);
    }

    if (s_record_handle.config.channel & RECORD_CHANNEL_G_SENSOR) {

    }

    return 0;
}

static int _UartRecordPostInfo(unsigned char *info, unsigned int info_len)
{
    UART_VOICE_PLAY_CONFIG play_config = {
        .sample_rate = 5,    // range : 0x01 - 0x07 : 48000 44100 32000 24000 22050 16000 11025 8000
        .volume = 0,         // default 0: 0dB, -18 ~ 18 dB
        .channel_type = 0,   // 1:STEREO and 0:MONO ,support MONO only for now
        .sample_bit = 16
    };


    MSG_PACK send_pack_start = {
        .port = UART_RECORD_PORT,
        .msg_header.magic = MSG_HOST_MAGIC,
        .msg_header.cmd = MSG_NTF_PCM_START,
        .body_addr = info != NULL ? info : (unsigned char*)&play_config,
        .len = info != NULL ? info_len : sizeof(play_config)
    };
    UartMessageAsyncSend(&send_pack_start);
    return 0;
}

int UartRecordSuspend(void *priv)
{
    UartMessageAsyncDone();
    return 0;
}

int UartRecordResume(void *priv)
{
    if (s_record_handle.record_on) {
        s_record_handle.flow_ctrl = (s_record_handle.flow_ctrl == 2) ? 2 : 1;
    }
    return 0;
}

int UartRecordInit(int port, unsigned int baudrate){

    UART_MSG_INIT_CONFIG init_config = {
        .port = (port == 0 || port == 1) ? port : UART_RECORD_PORT,
        .baudrate = baudrate == 0 ? UART_RECORD_PORT_BAUDRATE : baudrate,
        .reinit_flag = 1
    };

    UartMessageAsyncDone();

    if (UartMessageAsyncInit(&init_config))
        return -1;

    UART_MSG_REGIST regist_info_1 = {
        .msg_id = MSG_NTF_PCM_READY,
        .port = init_config.port,
        .msg_pack_callback = _uartRecvCallback,

    };
    UartMessageAsyncRegist(&regist_info_1);

    UART_MSG_REGIST regist_info_2 = {
        .msg_id = MSG_NTF_START_PCM,
        .port = init_config.port,
        .msg_buffer = (unsigned char*)&s_record_handle.config,
        .msg_buffer_length = sizeof(UART_RECORD_CONFIG),
        .msg_pack_callback = _uartRecvCallback,
    };
    UartMessageAsyncRegist(&regist_info_2);

    UART_MSG_REGIST regist_info_3 = {
        .msg_id = MSG_NTF_STOP_PCM,
        .port = init_config.port,
        .msg_pack_callback = _uartRecvCallback,
    };
    UartMessageAsyncRegist(&regist_info_3);

    static int once = 1;
    if (once) {
        _UartRecordPostInfo(NULL, 0);
        once = 0;
    }

    LVP_SUSPEND_INFO suspend_info = {
        .suspend_callback = UartRecordSuspend,
        .priv = "UartRecordSuspend"
    };

    LVP_RESUME_INFO resume_info = {
        .resume_callback = UartRecordResume,
        .priv = "UartRecordResume"
    };

    LvpSuspendInfoRegist(&suspend_info);
    LvpResumeInfoRegist(&resume_info);

    return 0;
}

int UartRecordTick(void)
{
    UartMessageAsyncTick();
    return 0;
}

void _UartRecordDone(void)
{
    UartMessageAsyncDone();
}

//-------------------------------------------------------------------------------------------------
