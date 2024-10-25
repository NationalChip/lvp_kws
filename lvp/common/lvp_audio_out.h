#ifndef __LVP_AUDIO_OUT_H__
#define __LVP_AUDIO_OUT_H__

#include <driver/gx_audio_out.h>

int LvpAudioOutInit(GX_AUDIO_OUT_CHANNEL channel_type);
int LvpAudioOutDeinit(void);
int LvpAudioOutSetPcmBuffer(unsigned char *pcm0_buffer, unsigned char *pcm1_buffer, unsigned int buffer_size);
int LvpAudioOutSetFrameQueue(GX_AUDIO_OUT_FRAME *queue_buffer, unsigned int frame_num);
int LvpAudioOutSetPcmParam(unsigned int sample_rate, unsigned char bits, unsigned char channels);
int LvpAudioOutSetCallback(void);
int LvpAudioOutSetVolume(unsigned int vol);
int LvpAudioOutPlay(unsigned int offset, unsigned int length);

#endif
