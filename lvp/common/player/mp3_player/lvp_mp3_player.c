#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <autoconf.h>
#include <driver/gx_delay.h>
#include <driver/gx_gpio.h>
#include <driver/gx_audio_out.h>
#include <driver/gx_audio_out/gx_audio_out_v2.h>
#include <lvp_buffer.h>
#include <lvp_queue.h>
#include <driver/gx_clock.h>
#include <driver/gx_timer.h>
#include <soc_config.h>
#include "lvp_mp3_player.h"
#include "decoder/base_decoder.h"
#include "types.h"
#include <mp3dec.h>
#ifdef USE_MP3
#include "decoder/mp3_decoder.h"
#endif
#include <driver/gx_irq.h>
#include <board_misc_config.h>
#include <autoconf.h>

#define MAX_DECODE_LEN 100
#ifndef USE_MP3
#define MAX_PCM_LEN PCM_16K_FRAME_LEN
#else
#define MAX_PCM_LEN (1152 * 2)
// #define MAX_PCM_LEN (576 * 2)
#endif
//#define COUNT 7

#define AUDIO_OUT_BUFFER_LEN    CONFIG_AUDIO_OUT_BUFFER_SIZE

#define COUNT    (AUDIO_OUT_BUFFER_LEN / MAX_PCM_LEN)

//DRAM0_AUDIO_IN_ATTR
//__attribute__((section(".audio_out"))) static unsigned char lvp_play_buffer[MAX_PCM_LEN * COUNT]; //__attribute__((aligned(16)));
// __attribute__((section(".audio_out,\"aw\",@nobits#"))) static unsigned char lvp_play_buffer[AUDIO_OUT_BUFFER_LEN]__attribute__((aligned(16)));
//static unsigned char lvp_play_buffer_2[MAX_PCM_LEN] __attribute__((aligned(16)));
static int handle = -1;

typedef struct {
    const unsigned char *resource_position;
    int sample_rate;
    int channels;
    int interlace;
    int len;
    int is_repeat;
} LVP_PLAYER_VOICE;


#define LVP_VOICE_PLAYER_QUEUE_LEN 8
static unsigned char s_voice_player_event_queue_buffer[LVP_VOICE_PLAYER_QUEUE_LEN * sizeof(LVP_PLAYER_VOICE)];
LVP_QUEUE s_voice_player_event_queue;

//#define LVP_VOICE_PLAYER_FRAME_COUNT (MAX_PCM_LEN * COUNT / MAX_PCM_LEN)
#define LVP_VOICE_PLAYER_FRAME_COUNT COUNT
static unsigned char s_voice_player_frame_queue_buffer[LVP_VOICE_PLAYER_FRAME_COUNT * sizeof(int)];
LVP_QUEUE s_voice_player_frame_queue;

#ifdef CONFIG_MP3_DAC_STEREO
#define DAC_CHANNEL GX_AUDIO_OUT_STEREO_C
#elif defined CONFIG_MP3_DAC_LEFT
#define DAC_CHANNEL GX_AUDIO_OUT_LEFT_C
#elif defined CONFIG_MP3_DAC_RIGHT
#define DAC_CHANNEL GX_AUDIO_OUT_RIGHT_C
#elif defined CONFIG_MP3_DAC_MONO
#define DAC_CHANNEL GX_AUDIO_OUT_MONO_C
#else
#define DAC_CHANNEL GX_AUDIO_OUT_STEREO_C
#endif

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
    const unsigned char *resource_position;
    unsigned int enc_frame_size;
    unsigned int dec_frame_size;
    unsigned int play_pos;
    unsigned int voice_size;
    unsigned int decoded_length;
    unsigned int is_repeat;
} LVP_VOICE_INFO;

LVP_PLAYER_INFO player_info = {
    .status = PLAYER_STATUS_STOP,
    .mute = 0,
    .volume = 30,
};
LVP_VOICE_INFO voice_info = {0};
LvpVoicePlayerEventCallback event_cb;
static int GetId3v2PackSize(const unsigned char *resource_position);
static int GetPlayerQueueDataNum(void);
static void lvp_player_clear_frame_buffer(void);

#define DecodeSkipFrame 5
#define READBUF_SIZE 20000
unsigned char readBuf[READBUF_SIZE] ={0};
static int bytesLeft;
static unsigned char *decode_ptr;
static int saved_length;

int LvpMp3PlayerPlay(const unsigned char *resource_position, unsigned int length, unsigned int repeat)
{
    int id3size, offset = 0;
    LVP_PLAYER_VOICE voice_event;
    if (resource_position == NULL)
        return -1;
    id3size = GetId3v2PackSize(resource_position);
    if(id3size)
    {
        offset = id3size;
    }
    voice_event.resource_position = resource_position + offset;
    voice_event.len = length - offset;
    voice_event.sample_rate = 0;
    voice_event.channels = 0;
    voice_event.is_repeat = repeat;

    bytesLeft = 0;
    decode_ptr = readBuf;
    saved_length = 0;
    lvp_player_clear_frame_buffer();
    if (LvpQueuePut(&s_voice_player_event_queue, (const unsigned char *)&voice_event))
        return 0;
    return -1;
}

static int GetId3v2PackSize(const unsigned char *resource_position)
{
    int res = 0;
    if(!resource_position)
        return res;
    // printf("resource_position[0] =0x%x resource_position[1] =0x%x resource_position[2] =0x%x\n",
    //  resource_position[0],resource_position[1], resource_position[2]);
    if(resource_position[0] == 0x49 && resource_position[1] == 0x44 && resource_position[2] == 0x33)
    //"I","D", "3"
    {
        return ((resource_position[6]&0X7f) << 21)|((resource_position[7]&0X7f) << 14)
        |((resource_position[8]&0X7f) << 7)|(resource_position[9]&0X7f);
    }
    return res;
}

static void lvp_player_clear_frame_buffer(void)
{
    int play_saddr;
    int frame_num = GetPlayerQueueDataNum();
    for (int i = 0; i < frame_num; i++)
        LvpQueueGet(&s_voice_player_frame_queue, (unsigned char *)&play_saddr);
}

static void lvp_audio_out_exit(void)
{
    gx_gpio_set_level(CONFIG_LVP_MP3_PLAYER_NUTE_PIN, GX_GPIO_LEVEL_HIGH);
    // player_info.status = PLAYER_STATUS_STOP;
    lvp_player_clear_frame_buffer();
}


static void lvp_player_callback(unsigned int saddr, unsigned int eaddr)
{
    int play_saddr;
    GX_AUDIO_OUT_FRAME frame;

    if (LvpQueueIsEmpty(&s_voice_player_frame_queue)) {
        player_info.status = PLAYER_STATUS_PREPARE;
        return;
    }

    if (player_info.status == PLAYER_STATUS_STOP) {
        lvp_player_clear_frame_buffer();
        return;
    }
    if (LvpQueueGet(&s_voice_player_frame_queue, (unsigned char *)&play_saddr)) {
        frame.saddr = play_saddr;
        frame.eaddr = frame.saddr + (eaddr - saddr);
        // printf("frame.saddr =%d frame.eaddr =%d\n", frame.saddr, frame.eaddr);
        // printf("saddr =%d eaddr=%d\n", saddr, eaddr);
        if (GetPlayerQueueDataNum() == 0 && event_cb) {
            // bytesLeft = 0;
            // decode_ptr = readBuf;
            event_cb(EVENT_PLAYER_LAST_FRAME, NULL);
        }
        gx_audio_out_push_frame(handle, &frame);
    }
}

static void lvp_player_finish_callback(void)
{
    if(!voice_info.is_repeat)
    {
        player_info.status = PLAYER_STATUS_PREPARE;
    }
    if(voice_info.play_pos == voice_info.voice_size)
    {
        if(!voice_info.is_repeat)
        {
            player_info.status = PLAYER_STATUS_STOP;
        }
        lvp_audio_out_exit();
        if (event_cb)
            event_cb(EVENT_PLAYER_FINISH, NULL);
    }
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
#ifdef CONFIG_LVP_HAS_MP3_PLAYER_SLAVE_MODE
    pcm.module_freq = 12288000;
#else
    pcm.module_freq = gx_clock_get_module_frequence(CLOCK_MODULE_AUDIO_PLAY);
#endif

    buf.buffer0 = (unsigned char *)LvpGetAudioOutBuffer();
    buf.buffer1 = NULL;
    buf.size = AUDIO_OUT_BUFFER_LEN;


    lvp_audio_out_exit();
    gx_mdelay(1);

    cb.new_frame_callback = lvp_player_callback;
    cb.frame_over_callback = NULL;

    gx_gpio_set_direction(CONFIG_LVP_MP3_PLAYER_NUTE_PIN, GX_GPIO_DIRECTION_OUTPUT);
    gx_gpio_set_level(CONFIG_LVP_MP3_PLAYER_NUTE_PIN, GX_GPIO_LEVEL_HIGH);
    gx_audio_out_config_buffer(handle, &buf);
    gx_audio_out_config_pcm(handle, &pcm);
    gx_audio_out_config_cb(handle, &cb);
    gx_audio_out_set_channel(handle, DAC_CHANNEL);
    gx_audio_out_set_db(handle, lvp_vol_to_db(player_info.volume));
    gx_gpio_set_level(CONFIG_LVP_MP3_PLAYER_NUTE_PIN, GX_GPIO_LEVEL_LOW);
}

static int lvp_audio_play(void)
{
    int play_saddr;
    GX_AUDIO_OUT_FRAME frame;
    GX_AUDIO_OUT_CALLBACK cb;
    if (LvpQueueIsEmpty(&s_voice_player_frame_queue))
    {
        return -1;
    }
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

static int LvpMp3PlayerUpdate(LVP_PLAYER_VOICE *voice_event)
{
    int ret = -1;
    if (voice_event->resource_position == NULL)
        return ret;
    voice_info.sample_rate = 0;
    voice_info.channels = 0;
    voice_info.bits = 0;
    voice_info.play_pos = 0;
    voice_info.voice_size = voice_event->len;
    voice_info.resource_position = voice_event->resource_position;
    voice_info.decoded_length = 0;
    voice_info.is_repeat = voice_event->is_repeat;
    player_info.status         = PLAYER_STATUS_PREPARE;
    return ret;
}

int LvpMp3PlayerSetVolume(int volume)
{
    if (handle != -1) {
        if (volume > 100)
            volume = 100;
        else if (volume < 0)
            volume = 0;
        player_info.volume = volume;
        gx_audio_out_set_db(handle, lvp_vol_to_db(player_info.volume));
        return 0;
    }
    return -1;
}

int LvpMp3PlayerGetVolume(void)
{
    if (handle != -1) {
        return player_info.volume;
    }
    return -1;
}

void LvpMp3PlayerStop(void)
{
    player_info.status = PLAYER_STATUS_STOP;
    return 0;
}

static void LvpMp3PlayerPutFrame(const int base_offset, const int decode_offset_count, const int dec_len)
{
    int play_saddr = base_offset + decode_offset_count * dec_len;
    LvpQueuePut(&s_voice_player_frame_queue, (unsigned char *)&play_saddr);
}

static int GetPlayerQueueDataNum()
{
    int state = gx_lock_irq_save();
    int num = LvpQueueGetDataNum(&s_voice_player_frame_queue);
    gx_unlock_irq_restore(state);
    return num;
}

int LvpMp3PlayerTask(void *arg)
{
    LVP_PLAYER_VOICE voice_event;
    int out_len = 0;
    int decode_offset_count;
    int first_offset = 1;
    if (player_info.status == PLAYER_STATUS_STOP)
    {
        memset(&voice_info, 0, sizeof(voice_info));
        voice_info.resource_position = NULL;
    }
    else if (player_info.status == PLAYER_STATUS_PAUSE)
    {
    }
    else if (player_info.status <= PLAYER_STATUS_PLAY && (voice_info.play_pos < voice_info.voice_size) \
            && GetPlayerQueueDataNum() < (LVP_VOICE_PLAYER_FRAME_COUNT - 3)
            )
    {
        out_len = voice_info.dec_frame_size;
        if(out_len)
            decode_offset_count = (voice_info.decoded_length/out_len)%(AUDIO_OUT_BUFFER_LEN/out_len);
        else
            decode_offset_count = 0;

        if(bytesLeft < 2*1940)
        {
            int left = voice_info.voice_size - saved_length;
            memmove(readBuf, decode_ptr, bytesLeft);
            // printf("memmove from %d move length %d\n",decode_ptr - readBuf, bytesLeft);
            int read_size;
            if(left >= READBUF_SIZE - bytesLeft)
            {
                read_size = READBUF_SIZE - bytesLeft;
                memcpy(readBuf + bytesLeft, (unsigned char *)voice_info.resource_position + saved_length, READBUF_SIZE - bytesLeft);
            }
            else
            {
                read_size =  left;
                memcpy(readBuf + bytesLeft, (unsigned char *)voice_info.resource_position + saved_length, left);
                memset(readBuf + bytesLeft + left, 0, READBUF_SIZE - bytesLeft-left);
            }
            // printf("memcpy from %d copy length:%d\n", bytesLeft, read_size);
            decode_ptr = readBuf;
            bytesLeft += read_size;
            saved_length += read_size;
        }

        MP3FrameInfo info;
        lvp_mp3_getinfo(decode_ptr, bytesLeft, &first_offset, &info);
        if(voice_info.sample_rate == 0)
        {
            voice_info.sample_rate = info.samprate;
            voice_info.channels =info.nChans;
            voice_info.bits = info.bitsPerSample;
            voice_info.interlace = 0;
            voice_info.endian = 0;
            // printf("voice_info.sample_rate =%d voice_info.channels =%d\n", voice_info.sample_rate, voice_info.channels);
            lvp_audio_out_init();
        }
        if(first_offset < 0)
        {
            // printf("decode over!!! voice_info.play_pos =%d voice_info.voice_size=%d\n",
                // voice_info.play_pos, voice_info.voice_size);
            if(voice_info.is_repeat)
            {
                bytesLeft = 0;
                decode_ptr = readBuf;
                saved_length = 0;
            }
            return 0;
        }
        else
        {
            decode_ptr += first_offset;
            bytesLeft -= first_offset;
            voice_info.play_pos += first_offset;
        }
        unsigned char * addr = decode_ptr;
        lvp_mp3_decode(&decode_ptr, &bytesLeft, (unsigned char *)LvpGetAudioOutBuffer() + decode_offset_count * out_len, &out_len);
        voice_info.play_pos += (decode_ptr - addr);
        voice_info.decoded_length += out_len;
        voice_info.dec_frame_size = out_len;

        if (voice_info.is_repeat)
        {
            static int i = 0;
            if(i >= DecodeSkipFrame)
                LvpMp3PlayerPutFrame(0, decode_offset_count, out_len);
            i++;
            if(voice_info.play_pos >= voice_info.voice_size)
            {
                voice_info.play_pos = 0;
                i = 0;
            }
        }
        else
        {
            LvpMp3PlayerPutFrame(0, decode_offset_count, out_len);
        }

        if (player_info.status == PLAYER_STATUS_PREPARE
                && GetPlayerQueueDataNum() >= 2) {
            player_info.status = PLAYER_STATUS_PLAY;
            lvp_audio_play();
        }
    }

    if (LvpQueueGet(&s_voice_player_event_queue, (unsigned char *)&voice_event))
    {
        // printf("+++[%s]%d+++\n", __func__, __LINE__);
        LvpMp3PlayerUpdate(&voice_event);
    }
    return 0;
}

int LvpMp3PlayerGetStatus(void)
{
    return player_info.status;
}

int LvpMp3PlayerInit(LvpVoicePlayerEventCallback cb)
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
        int ret = lvp_mp3_decoder_init();

        // ret = lvp_mp3_getinfo(voice_info.resource_position, voice_info.voice_size, NULL, NULL);

        if (ret != 0) {
            printf("decoder init fail\n");
            memset((void *)&voice_info, 0, sizeof(voice_info));
            player_info.status = PLAYER_STATUS_STOP;
            return ret;
        }
    }
    if (cb != NULL) {
        event_cb = cb;
    }
    LvpMp3PlayerSetVolume(player_info.volume);
    LvpQueueInit(&s_voice_player_event_queue, s_voice_player_event_queue_buffer, LVP_VOICE_PLAYER_QUEUE_LEN * sizeof(LVP_PLAYER_VOICE), sizeof(LVP_PLAYER_VOICE));
    LvpQueueInit(&s_voice_player_frame_queue, s_voice_player_frame_queue_buffer, LVP_VOICE_PLAYER_FRAME_COUNT * sizeof(int), sizeof(int));
    return 0;
}
int LvpMp3PlayerSuspend()
{
    if (handle != -1) {
        gx_audio_out_free(handle);
        gx_audio_out_suspend(handle);
        handle = -1;
        //gx_audio_out_exit();
    }
    return 0;
}

int LvpMp3PlayerResume()
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

