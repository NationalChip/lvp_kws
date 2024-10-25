/* Grus
 * Copyright (C) 1991-2017 Nationalchip Co., Ltd
 *
 * lvp_auidio.h: a Circular Queue using array
 *
 */

#ifndef __LVP_AUDIO_IN_H__
#define __LVP_AUDIO_IN_H__

#include <lvp_context.h>

typedef int (*AUDIO_IN_RECORD_CALLBACK)(int ctx_index, void *priv);

int LvpAudioInUpdateReadIndex(int offset);
int LvpAudioInGetDelayedFFTVad(void);
int LvpGetAudioInCtrlVad(void);
int LvpSetAudioInCtrlVad(int vad);
int LvpGetAudioInCtrlStartCtxIndex(void);
int LvpAudioInInit(AUDIO_IN_RECORD_CALLBACK callback);
int LvpAudioInQueryFFTVad(unsigned int *sdc_addr);
int LvpAuidoInQueryEnvNoise(LVP_CONTEXT *context);
void LvpAudioInSuspend(void);
void LvpAudioInResume(void);
int LvpAudioInDone(void);
void LvpAudioInStandbyToStartup(void);

#endif /* __LVP_AUDIO_IN_H__ */
