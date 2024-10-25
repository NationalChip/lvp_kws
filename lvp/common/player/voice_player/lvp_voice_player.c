#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <autoconf.h>
#include <driver/gx_delay.h>
#include <driver/gx_clock.h>
#include <driver/gx_gpio.h>
#include <driver/gx_audio_out.h>
#include <driver/gx_i2s/gx_i2s_v2.h>
#include <driver/gx_audio_out/gx_audio_out_v2.h>
#include <lvp_buffer.h>
#include <lvp_queue.h>
#include <driver/gx_timer.h>
#include "lvp_voice_player.h"
#include "decoder/base_decoder.h"
#include "types.h"
#ifdef USE_OPUS
#include "decoder/opus_decoder.h"
#endif
#ifdef USE_AMR
#include "decoder/amr_decoder.h"
#endif
#ifdef CONFIG_USE_ADPCM
#include "decoder/adpcm_decoder.h"
#endif
// #include <adpcm.h>
#include <board_misc_config.h>
#include <soc_config.h>

#define MAX_DECODE_LEN 100
#define MAX_PCM_LEN PCM_16K_FRAME_LEN

#define AUDIO_OUT_BUFFER_LEN    CONFIG_AUDIO_OUT_BUFFER_SIZE
#define COUNT (AUDIO_OUT_BUFFER_LEN / MAX_PCM_LEN)

#ifdef CONFIG_VOICE_PLAYER_DAC_STEREO
#define DAC_CHANNEL GX_AUDIO_OUT_STEREO_C
#elif defined CONFIG_VOICE_PLAYER_DAC_LEFT
#define DAC_CHANNEL GX_AUDIO_OUT_LEFT_C
#elif defined CONFIG_VOICE_PLAYER_DAC_RIGHT
#define DAC_CHANNEL GX_AUDIO_OUT_RIGHT_C
#elif defined CONFIG_VOICE_PLAYER_DAC_MONO
#define DAC_CHANNEL GX_AUDIO_OUT_MONO_C
#else
#define DAC_CHANNEL GX_AUDIO_OUT_STEREO_C
#endif

static unsigned char * lvp_play_buffer;
static int handle = -1;
static unsigned char *user_pcm_buffer;
static int user_pcm_buffer_len;

typedef struct {
    const unsigned char *resource_position;
    int sample_rate;
    int channels;
    int interlace;
    int len;
} LVP_PLAYER_VOICE;


#define LVP_VOICE_PLAYER_QUEUE_LEN 8
static unsigned char s_voice_player_event_queue_buffer[LVP_VOICE_PLAYER_QUEUE_LEN * sizeof(LVP_PLAYER_VOICE)];
LVP_QUEUE s_voice_player_event_queue;

#define LVP_VOICE_PLAYER_FRAME_COUNT (MAX_PCM_LEN * COUNT / MAX_PCM_LEN)
static unsigned char s_voice_player_frame_queue_buffer[LVP_VOICE_PLAYER_FRAME_COUNT * sizeof(int)];
LVP_QUEUE s_voice_player_frame_queue;

typedef enum {
    TYPE_PCM = 0,
    TYPE_WAV = 1,
    TYPE_ADPCM = 2,
    TYPE_OPUS = 8,
    TYPE_AMR = 9,
    TYPE_FILL = 1000,
} VOICE_TYPE;


typedef struct {
    PLAYER_STATUS status;
    int mute;
    int volume;
} LVP_PLAYER_INFO;

typedef struct {
    int sample_rate;
    int channels;
    int bits;
    int bytes_rate;
    int interlace;
    int endian;
    VOICE_TYPE decoder;
    const unsigned char *resource_position;
    unsigned int enc_frame_size;
    unsigned int dec_frame_size;
    unsigned int play_offset;
    unsigned int play_pos;
    unsigned int voice_size;
} LVP_VOICE_INFO;

LVP_PLAYER_INFO player_info = {
    .status = PLAYER_STATUS_STOP,
    .mute = 0,
    .volume = 30,
};
LVP_VOICE_INFO voice_info;
LvpVoicePlayerEventCallback event_cb;

int LvpVoicePlayerPlay(const unsigned char *resource_position)
{
    LVP_PLAYER_VOICE voice_event;
    if (resource_position == NULL)
        return -1;
    voice_event.resource_position = resource_position;
    voice_event.len = 0;
    voice_event.sample_rate = 0;
    voice_event.channels = 0;
    if (LvpQueuePut(&s_voice_player_event_queue, (const unsigned char *)&voice_event))
        return 0;
    return -1;
}

int LvpVoicePlayerSetUserPcmBuffer(unsigned char *resource_position, int len)
{
    if (resource_position == NULL || len <= 0)
        return -1;
    user_pcm_buffer = resource_position;
    user_pcm_buffer_len = len;
    return 0;
}

void LvpVoicePlayerClearUserPcmBuffer(void)
{
    user_pcm_buffer = NULL;
    user_pcm_buffer_len = 0;
}

int LvpVoicePlayerPlayPcm(const unsigned char *resource_position, int len, int sample_rate, int channels, int interlace)
{
    LVP_PLAYER_VOICE voice_event;
    voice_event.resource_position = resource_position;
    voice_event.len = len;
    voice_event.sample_rate = sample_rate;
    voice_event.channels = channels;
    voice_event.interlace = interlace;
    if (resource_position == NULL)
        return -1;
    if (LvpQueuePut(&s_voice_player_event_queue, (const unsigned char *)&voice_event))
        return 0;
    return -1;
}

#ifdef CONFIG_USE_ADPCM
int LvpVoicePlayerPlayAdpcm(const unsigned char *resource_position, int len, int sample_rate)
{
    LVP_PLAYER_VOICE voice_event;
    voice_event.resource_position = resource_position;
    voice_event.len = len;
    voice_event.sample_rate = sample_rate;
    voice_event.channels = 1;
    voice_event.interlace = 1;
    if (resource_position == NULL)
        return -1;
    if (LvpQueuePut(&s_voice_player_event_queue, (const unsigned char *)&voice_event))
        return 0;
    return -1;
}
#endif

static void lvp_player_clear_frame_buffer(void)
{
    int play_saddr;
    int frame_num = LvpQueueGetDataNum(&s_voice_player_frame_queue);
    for (int i = 0; i < frame_num; i++)
        LvpQueueGet(&s_voice_player_frame_queue, (unsigned char *)&play_saddr);
}

static void lvp_audio_out_exit(void)
{
    gx_gpio_set_level(CONFIG_LVP_VOICE_PLAYER_NUTE_PIN, GX_GPIO_LEVEL_HIGH);
    player_info.status = PLAYER_STATUS_STOP;
    lvp_player_clear_frame_buffer();
}


static void lvp_player_callback(unsigned int saddr, unsigned int eaddr)
{
    int play_saddr;
    GX_AUDIO_OUT_FRAME frame;

    if (LvpQueueIsEmpty(&s_voice_player_frame_queue)) {
        return;
    }

    if (player_info.status == PLAYER_STATUS_STOP) {
        lvp_player_clear_frame_buffer();
        return;
    }
    if (LvpQueueGet(&s_voice_player_frame_queue, (unsigned char *)&play_saddr)) {
        frame.saddr = play_saddr;
        frame.eaddr = frame.saddr + (eaddr - saddr);
        // printf("addr :%d %d\n", frame.saddr, frame.eaddr);
        if (LvpQueueGetDataNum(&s_voice_player_frame_queue) == 0 && event_cb) {
            event_cb(EVENT_PLAYER_LAST_FRAME, NULL);

        }
        gx_audio_out_push_frame(handle, &frame);
    }
}

static void lvp_player_finish_callback(void)
{
    printf("play finish\n");
    lvp_audio_out_exit();
    if (event_cb)
        event_cb(EVENT_PLAYER_FINISH, NULL);
    return;
}

static int lvp_vol_to_db(int volume)
{
    const int vol_to_db[] = {-25, -6, 0, 3, 6, 8, 10, 12, 14, 15, 16};
    volume = (volume > 100 ? 100 : volume) < 0 ? 0 : volume;
    volume = (volume + 5) / 10;
    return vol_to_db[volume];
}

typedef struct {
/*    RIFF Chunk */
    char riff[4];              // "RIFF"
    unsigned int riff_size;    // size - 8
    char riff_format[4];       // "WAVE" "OPUS" "AMR "
/*    FMT Chunk */
    char format[4];            // "fmt "
    unsigned int format_size;  // 0x10 (FMT Chunk size - 8)
/*    1(0x0001)   PCM/非压缩格式      support
 *    2(0x0002)   Microsoft ADPCM
 *    3(0x0003)   IEEE float
 *    6(0x0006)   ITU G.711 a-law
 *    7(0x0007)   ITU G.711 μ-law
 *    49(0x0031)  GSM 6.10
 *    64(0x0040)  ITU G.721 ADPCM
 *    8(0x0008)   opus                support
 *    9(0x0009)   amr                 support
 */
    unsigned short format_tag;
    unsigned short channels;
    unsigned int sample_rate;  // 8k or 16k for opus and amr
    unsigned int bytes_rate;
    unsigned short block_align;
    unsigned short bits;
/*    DATA Chunk*/
    char data_tag[4];
    unsigned int data_size;
} VOICE_HEADER;

static int LvpVoicePlayerCheckInPcmBuffer(const unsigned char *resource_position, int len)
{
    if (user_pcm_buffer == NULL || user_pcm_buffer_len <= 0)
        return 0;
    if ((resource_position < user_pcm_buffer)
            || (resource_position + len > user_pcm_buffer + user_pcm_buffer_len))
        return 0;
    return 1;
}


static void lvp_audio_out_init(void)
{
    GX_AUDIO_OUT_PCM pcm;
    GX_AUDIO_OUT_BUFFER buf;
    GX_AUDIO_OUT_CALLBACK cb;

    pcm.sample_rate = voice_info.sample_rate;
    pcm.channels    = voice_info.channels;
    pcm.bits        = voice_info.bits;
    pcm.interlace   = voice_info.interlace;
    pcm.endian      = voice_info.endian;
#ifdef CONFIG_LVP_HAS_VOICE_PLAYER_SLAVE_MODE
    pcm.module_freq = 12288000;
#else
    pcm.module_freq = gx_clock_get_module_frequence(CLOCK_MODULE_AUDIO_PLAY);
#endif

    lvp_play_buffer = LvpGetAudioOutBuffer();

    if (LvpVoicePlayerCheckInPcmBuffer(voice_info.resource_position, voice_info.voice_size) != 0 && voice_info.decoder == TYPE_PCM) {
        buf.buffer0 = user_pcm_buffer;
        buf.buffer1 = NULL;
        buf.size = user_pcm_buffer_len;
    } else {
        buf.buffer0 = (unsigned char *)lvp_play_buffer;
        buf.buffer1 = NULL;
        buf.size = AUDIO_OUT_BUFFER_LEN;
    }

    lvp_audio_out_exit();
    gx_mdelay(1);

    cb.new_frame_callback = lvp_player_callback;
    cb.frame_over_callback = NULL;

    gx_gpio_set_level(CONFIG_LVP_VOICE_PLAYER_NUTE_PIN, GX_GPIO_LEVEL_HIGH);
    gx_audio_out_config_buffer(handle, &buf);
    gx_audio_out_config_pcm(handle, &pcm);
    gx_audio_out_config_cb(handle, &cb);
    gx_audio_out_set_channel(handle, DAC_CHANNEL);
    gx_audio_out_set_db(handle, lvp_vol_to_db(player_info.volume));
    gx_gpio_set_level(CONFIG_LVP_VOICE_PLAYER_NUTE_PIN, GX_GPIO_LEVEL_LOW);
    // gx_mdelay(200);
}

static int lvp_audio_play(void)
{
    int play_saddr;
    GX_AUDIO_OUT_FRAME frame;
    GX_AUDIO_OUT_CALLBACK cb;
    if (LvpQueueIsEmpty(&s_voice_player_frame_queue))
        return -1;
    if (LvpQueueGet(&s_voice_player_frame_queue, (unsigned char *)&play_saddr)) {
        frame.saddr = play_saddr;
        frame.eaddr = frame.saddr + voice_info.dec_frame_size - 1;

        gx_audio_out_push_frame(handle, &frame);
        cb.new_frame_callback = NULL;
        cb.frame_over_callback = lvp_player_finish_callback;
        gx_audio_out_config_cb(handle, &cb);
        return 0;
    }
    return -1;
}

int LvpVoicePlayerMute(void)
{
    return 0;
}

#ifndef CONFIG_USE_ADPCM
static int lvp_update_voice_header_config(LVP_VOICE_INFO *voice_info, const unsigned char *resource_position)
{
    VOICE_HEADER *voice_head = NULL;
    if (voice_info == NULL)
        return -1;
    if (resource_position == NULL)
        return -1;
    voice_head = (VOICE_HEADER *)resource_position;

    if (strncmp(voice_head->riff, "RIFF", 4) != 0 || voice_head->riff_size <= sizeof(VOICE_HEADER)
            || voice_head->format_size != 0x10 || voice_head->data_size <= 0)
    {
        return -1;
    }

    if (strncmp(voice_head->riff_format, "WAVE", 4) == 0 && voice_head->format_tag == TYPE_WAV)
    {
        voice_info->decoder = TYPE_WAV;
        if (voice_head->sample_rate == 8000) {
            voice_info->enc_frame_size = PCM_8K_FRAME_LEN;
            voice_info->dec_frame_size = PCM_8K_FRAME_LEN;
        } else if (voice_head->sample_rate == 16000) {
            voice_info->enc_frame_size = PCM_16K_FRAME_LEN;
            voice_info->dec_frame_size = PCM_16K_FRAME_LEN;
        }
    }
#ifdef USE_OPUS
    else if (strncmp(voice_head->riff_format, "OPUS", 4) == 0 && voice_head->format_tag == TYPE_OPUS)
    {
        voice_info->decoder = TYPE_OPUS;
        voice_info->enc_frame_size = OPUS_FRAME_LEN;
        if (voice_head->sample_rate == 8000) {
            voice_info->dec_frame_size = PCM_8K_FRAME_LEN;
        } else if (voice_head->sample_rate == 16000) {
            voice_info->dec_frame_size = PCM_16K_FRAME_LEN;
        }
    }
#endif
#ifdef USE_AMR
    else if (strncmp(voice_head->riff_format, "AMR ", 4) == 0 && voice_head->format_tag == TYPE_AMR)
    {
        voice_info->decoder = TYPE_AMR;
        if (voice_head->sample_rate == 8000) {
            voice_info->enc_frame_size = AMRNB_FRAME_LEN;
            voice_info->dec_frame_size = PCM_8K_FRAME_LEN;
        } else if (voice_head->sample_rate == 16000) {
            voice_info->enc_frame_size = AMRWB_FRAME_LEN;
            voice_info->dec_frame_size = PCM_16K_FRAME_LEN;
        }
    }
#endif
    else
    {
        printf("unsupport decoder\n");
        return -1;
    }
    voice_info->sample_rate = voice_head->sample_rate;
    voice_info->channels = voice_head->channels;
    voice_info->bits = voice_head->bits;
    voice_info->bytes_rate = voice_head->bytes_rate;
    voice_info->play_offset = sizeof(VOICE_HEADER);
    voice_info->play_pos = voice_info->play_offset;
    voice_info->voice_size = voice_head->data_size;
    voice_info->resource_position = resource_position;
    return 0;
}
#endif

static int LvpVoicePlayerUpdate(LVP_PLAYER_VOICE *voice_event)
{
    int ret = -1;
    if (voice_event->resource_position == NULL)
        return ret;

#ifdef CONFIG_USE_ADPCM
    {
        voice_info.sample_rate = voice_event->sample_rate;
        voice_info.channels = voice_event->channels;
        voice_info.resource_position = voice_event->resource_position;
        voice_info.bits = 16;
        voice_info.bytes_rate = voice_event->sample_rate * 16 / 8;
        voice_info.voice_size = voice_event->len;
        voice_info.decoder = TYPE_ADPCM;
        voice_info.play_offset = 0;
        voice_info.play_pos = voice_info.play_offset;
        voice_info.interlace = voice_event->interlace;
         if (voice_info.sample_rate == 16000) {
            voice_info.enc_frame_size = 80;
            voice_info.dec_frame_size = 320;
        } else if (voice_info.sample_rate == 8000) {
            voice_info.enc_frame_size = 40;
            voice_info.dec_frame_size = 160;
        } else {
            return ret;
        }
    }
#else
    if (voice_event->sample_rate != 0 && voice_event->channels > 0 && voice_event->len > 0)
    {
        voice_info.sample_rate = voice_event->sample_rate;
        voice_info.channels = voice_event->channels;
        voice_info.resource_position = voice_event->resource_position;
        voice_info.bits = 16;
        voice_info.bytes_rate = voice_event->sample_rate * 16 / 8;
        voice_info.voice_size = voice_event->len;
        voice_info.decoder = TYPE_PCM;
        voice_info.interlace = voice_event->interlace;
        if (LvpVoicePlayerCheckInPcmBuffer(voice_event->resource_position, voice_event->len)) {
            voice_info.play_offset = voice_event->resource_position - user_pcm_buffer;
        } else {
            voice_info.play_offset = 0;
        }
        voice_info.play_pos = voice_info.play_offset;

        if (voice_info.sample_rate == 16000) {
            voice_info.enc_frame_size = PCM_16K_FRAME_LEN;
            voice_info.dec_frame_size = PCM_16K_FRAME_LEN;
        } else if (voice_info.sample_rate == 8000) {
            voice_info.enc_frame_size = PCM_8K_FRAME_LEN;
            voice_info.dec_frame_size = PCM_8K_FRAME_LEN;
        } else {
            return ret;
        }
    }
    else
    {
        if (lvp_update_voice_header_config(&voice_info, voice_event->resource_position))
            return ret;
    }
#endif
    switch (voice_info.decoder)
    {
        case TYPE_PCM:
        case TYPE_WAV:
            ret = 0;
            break;
#ifdef USE_OPUS
        case TYPE_OPUS:
            ret = lvp_opus_decoder_init(voice_info.sample_rate, voice_info.channels);
            break;
#endif
#ifdef USE_AMR
        case TYPE_AMR:
            ret = lvp_amr_decoder_init(voice_info.sample_rate, voice_info.channels, voice_info.bytes_rate);
            break;
#endif
#ifdef CONFIG_USE_ADPCM
        case TYPE_ADPCM:
            ret = lvp_adpcm_decoder_init(voice_info.voice_size);
            break;
#endif
        default:
            printf("no this type voice resource\n");
            break;
    }
    if (ret != 0) {
        printf("decoder init fail\n");
        memset((void *)&voice_info, 0, sizeof(voice_info));
        player_info.status = PLAYER_STATUS_STOP;
        return ret;
    }

    if (player_info.status != PLAYER_STATUS_PLAY && player_info.status != PLAYER_STATUS_PREPARE) {
        lvp_audio_out_init();
    }
    player_info.status         = PLAYER_STATUS_PREPARE;
    return ret;
}

int LvpVoicePlayerSetVolume(int volume)
{
    if (handle != -1) {
        if (volume > 100)
            volume = 100;
        else if (volume < 0)
            volume = 0;
        player_info.volume = volume;
        return 0;
    }
    return -1;
}

int LvpVoicePlayerGetVolume(void)
{
    if (handle != -1) {
        return player_info.volume;
    }
    return -1;
}

int LvpVoicePlayerStop(void)
{
    player_info.status = PLAYER_STATUS_STOP;
    return 0;
}

//int LvpVoicePlayerPause(void)
//{
//    player_info.status = PLAYER_STATUS_PAUSE;
//    return 0;
//}

//int LvpVoicePlayerResume(void)
//{
//    player_info.status = PLAYER_STATUS_PLAY;
//    return 0;
//}

static void LvpVoicePlayerPutFrame(const int base_offset, const int decode_offset_count, const int dec_len)
{
    int play_saddr = base_offset + decode_offset_count * dec_len;
    LvpQueuePut(&s_voice_player_frame_queue, (unsigned char *)&play_saddr);
}

int LvpVoicePlayerTask(void *arg)
{
    LVP_PLAYER_VOICE voice_event;
    const unsigned char *resource_position;
    int enc_len = 0;
    int dec_len = 0;
    int decode_offset_count;
    if (player_info.status == PLAYER_STATUS_STOP)
    {
        memset(&voice_info, 0, sizeof(voice_info));
        voice_info.resource_position = NULL;
    }
    else if (player_info.status == PLAYER_STATUS_PAUSE)
    {
    }
    else if (player_info.status <= PLAYER_STATUS_PLAY && ((voice_info.play_pos - voice_info.play_offset) < voice_info.voice_size)
            && LvpQueueGetDataNum(&s_voice_player_frame_queue) < (LVP_VOICE_PLAYER_FRAME_COUNT - 2))
    {
        resource_position = voice_info.resource_position;
        enc_len = voice_info.enc_frame_size;
        dec_len = voice_info.dec_frame_size;
        decode_offset_count = ((voice_info.play_pos - voice_info.play_offset) / enc_len ) % (MAX_PCM_LEN * COUNT / dec_len);

        switch (voice_info.decoder)
        {
            case TYPE_PCM:
                if (!LvpVoicePlayerCheckInPcmBuffer(resource_position, voice_info.voice_size)) {
                    memcpy(lvp_play_buffer + decode_offset_count * dec_len,
                            resource_position + voice_info.play_pos, dec_len);
                }
                LvpVoicePlayerPutFrame(voice_info.play_offset, decode_offset_count, dec_len);
                voice_info.play_pos += enc_len;
                break;
            case TYPE_WAV:
                memcpy(lvp_play_buffer + decode_offset_count * dec_len,
                        resource_position + voice_info.play_pos, dec_len);
                LvpVoicePlayerPutFrame(0, decode_offset_count, dec_len);
                voice_info.play_pos += enc_len;
                break;
#ifdef USE_OPUS
            case TYPE_OPUS:
                    // printf("----%d %d\n",voice_info.play_pos,voice_info.voice_size);

                lvp_opus_decode(resource_position + voice_info.play_pos, enc_len,
                        (char *)(lvp_play_buffer + decode_offset_count * dec_len), dec_len);
                LvpVoicePlayerPutFrame(0, decode_offset_count, dec_len);
                voice_info.play_pos += enc_len;
                break;
#endif
#ifdef USE_AMR
            case TYPE_AMR:
                lvp_amr_decode(resource_position + voice_info.play_pos,
                        enc_len, lvp_play_buffer + decode_offset_count * dec_len, dec_len);
                LvpVoicePlayerPutFrame(0, decode_offset_count, dec_len);
                voice_info.play_pos += enc_len;
                break;
#endif
#ifdef CONFIG_USE_ADPCM
            case TYPE_ADPCM:
                lvp_adpcm_decode(resource_position + voice_info.play_pos,
                        lvp_play_buffer + decode_offset_count * dec_len, enc_len);
                LvpVoicePlayerPutFrame(0, decode_offset_count, dec_len);
                voice_info.play_pos += enc_len;
                break;
#endif
            case TYPE_FILL:
                break;
            default:
                player_info.status = PLAYER_STATUS_STOP;
                break;
        }
        if (player_info.status == PLAYER_STATUS_PREPARE
                && LvpQueueGetDataNum(&s_voice_player_frame_queue) >= 3) {
            player_info.status = PLAYER_STATUS_PLAY;
            lvp_audio_play();
        }
    }

    if (LvpQueueGet(&s_voice_player_event_queue, (unsigned char *)&voice_event))
    {
        LvpVoicePlayerUpdate(&voice_event);
    }
    return 0;
}

int LvpVoicePlayerGetStatus(void)
{
    return player_info.status;
}

int LvpVoicePlayerInit(LvpVoicePlayerEventCallback cb)
{
    if (handle == -1)
    {
#ifdef CONFIG_LVP_HAS_VOICE_PLAYER_SLAVE_MODE
        gx_audio_out_init(1);
        gx_i2s_set_single_mode(MODULE_I2S_OUT ,I2S_MODE_SLAVE_COMPLETE);
# ifdef _FIVE_WIRE
        gx_i2s_set_five_wire_mode(I2S_MODE_I2S_IN_M_I2S_OUT_S);
# endif

#else
        gx_audio_out_init(0);
#endif
        handle = (int)gx_audio_out_alloc_playback(0);
    }
    if (cb != NULL) {
        event_cb = cb;
    }
    LvpVoicePlayerSetVolume(player_info.volume);
    LvpQueueInit(&s_voice_player_event_queue, s_voice_player_event_queue_buffer, LVP_VOICE_PLAYER_QUEUE_LEN * sizeof(LVP_PLAYER_VOICE), sizeof(LVP_PLAYER_VOICE));
    LvpQueueInit(&s_voice_player_frame_queue, s_voice_player_frame_queue_buffer, LVP_VOICE_PLAYER_FRAME_COUNT * sizeof(int), sizeof(int));
    return 0;
}
int LvpVoicePlayerSuspend(void)
{
    if (handle != -1) {
        gx_audio_out_free(handle);
        gx_audio_out_suspend(handle);
        handle = -1;
        //gx_audio_out_exit();
    }
    return 0;
}

int LvpVoicePlayerResume(void)
{
    if (handle == -1) {
#ifdef CONFIG_LVP_HAS_VOICE_PLAYER_SLAVE_MODE
        gx_audio_out_init(1);
#else
        gx_audio_out_init(0);
#endif
        handle = (int)gx_audio_out_alloc_playback(0);
    }
    return 0;
}

