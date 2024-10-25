#include <stdio.h>
#include <string.h>
#include <driver/gx_delay.h>
#include <driver/gx_gpio.h>
#include <lvp_queue.h>
#include <driver/gx_timer.h>
#include <board_misc_config.h>
#include "types.h"
#include "lvp_audio_out.h"
#include <driver/gx_dcache.h>
#include <driver/gx_audio_out/gx_audio_out_v2.h>
#include <driver/gx_clock.h>
typedef enum {
    AOUT_STATE_IDLE = 0,
    AOUT_STATE_PREPARE,
    AOUT_STATE_PLAYING,
    AOUT_STATE_NONE,
}LVP_AUDIO_OUT_STATE;

LVP_QUEUE s_aout_frame_queue;
volatile static LVP_AUDIO_OUT_STATE aout_state = AOUT_STATE_NONE;
static GX_AUDIO_OUT_BUFFER aout_buf ;

static int handle = -1;

static void _aoutNewFrameCB(unsigned int saddr, unsigned int eaddr)
{
    GX_AUDIO_OUT_FRAME frame;

    aout_state = AOUT_STATE_PLAYING;

    if (LvpQueueIsEmpty(&s_aout_frame_queue)) {
        aout_state = AOUT_STATE_PREPARE;
        return;
    }

    if (LvpQueueGet(&s_aout_frame_queue, (unsigned char *)&frame)) {
        if(aout_buf.buffer0)
        {
            gx_dcache_clean_range ((unsigned int *)(aout_buf.buffer0 + saddr), eaddr-saddr + 1);
        }
        if(aout_buf.buffer1)
        {
            gx_dcache_clean_range ((unsigned int *)(aout_buf.buffer1 + saddr), eaddr-saddr + 1);
        }
        gx_audio_out_push_frame(handle, &frame);
        return;
    }
}

static void _aoutFinishCB(void)
{
    aout_state = AOUT_STATE_IDLE;
}

int LvpAudioOutInit(GX_AUDIO_OUT_CHANNEL channel_type)
{
    GX_AUDIO_OUT_CALLBACK cb;

    if (handle == -1)
    {
        gx_audio_out_init(0);
        handle = (int)gx_audio_out_alloc_playback(0);
    }

    cb.new_frame_callback = _aoutNewFrameCB;
    cb.frame_over_callback = NULL;
    gx_audio_out_config_cb(handle, &cb);

    gx_audio_out_set_channel(handle, channel_type);
    gx_audio_out_set_db(handle, -12);

    aout_state = AOUT_STATE_IDLE;

    return 0;
}

int LvpAudioOutDeinit(void)
{
    return 0;
}

int LvpAudioOutSetPcmBuffer(unsigned char *pcm0_buffer, unsigned char *pcm1_buffer, unsigned int buffer_size)
{

    aout_buf.buffer0 = pcm0_buffer;
    aout_buf.buffer1 = pcm1_buffer;
    aout_buf.size = buffer_size;
    gx_audio_out_config_buffer(handle, &aout_buf);

    return 0;
}

int LvpAudioOutSetFrameQueue(GX_AUDIO_OUT_FRAME *queue_buffer, unsigned int frame_num)
{
    LvpQueueInit(&s_aout_frame_queue, (unsigned char *)queue_buffer, frame_num * sizeof(GX_AUDIO_OUT_FRAME), sizeof(GX_AUDIO_OUT_FRAME));
    return 0;
}

int LvpAudioOutSetPcmParam(unsigned int sample_rate, unsigned char bits, unsigned char channels)
{
    GX_AUDIO_OUT_PCM pcm;

    pcm.sample_rate = sample_rate;
    pcm.channels    = channels;
    pcm.bits        = bits;
    pcm.interlace   = 0;
    pcm.endian      = 0;
    pcm.module_freq = gx_clock_get_module_frequence(CLOCK_MODULE_AUDIO_PLAY);

    gx_audio_out_config_pcm(handle, &pcm);

    return 0;
}

int LvpAudioOutSetCallback(void)
{
    return 0;
}

static int lvp_vol_to_db(int volume)
{
    const int vol_to_db[] = {-25, -6, 0, 3, 6, 8, 10, 12, 14, 15, 16};
    volume = (volume > 100 ? 100 : volume) < 0 ? 0 : volume;
    volume = (volume + 5) / 10;
    return vol_to_db[volume];
}

int LvpAudioOutSetVolume(unsigned int vol)
{
    return gx_audio_out_set_db(handle, lvp_vol_to_db(vol));
}

static int _aoutStart(GX_AUDIO_OUT_FRAME *frame)
{
    GX_AUDIO_OUT_CALLBACK cb;
    if(aout_buf.buffer0)
    {
        gx_dcache_clean_range ((unsigned int *)(aout_buf.buffer0 + frame->saddr), frame->eaddr-frame->saddr + 1);
    }
    if(aout_buf.buffer1)
    {
        gx_dcache_clean_range ((unsigned int *)(aout_buf.buffer1 + frame->saddr), frame->eaddr-frame->saddr + 1);
    }
    gx_audio_out_push_frame(handle, frame);
    cb.new_frame_callback = NULL;
    cb.frame_over_callback = _aoutFinishCB;
    gx_audio_out_config_cb(handle, &cb);

    return 0;
}

int LvpAudioOutPlay(unsigned int offset, unsigned int length)
{
    GX_AUDIO_OUT_FRAME frame;

    frame.saddr = offset;
    frame.eaddr = frame.saddr + length - 1;

    if(aout_state < AOUT_STATE_PLAYING) {
        _aoutStart(&frame);
    } else if (aout_state == AOUT_STATE_PLAYING){
        LvpQueuePut(&s_aout_frame_queue, (const unsigned char *)&frame);
    }

    return 0;
}
