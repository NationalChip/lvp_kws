#ifndef __LVP_VOICE_PLAYER__
#define __LVP_VOICE_PLAYER__

typedef enum {
    PLAYER_STATUS_PREPARE = 0,
    PLAYER_STATUS_PLAY,
    PLAYER_STATUS_PAUSE,
    PLAYER_STATUS_STOP,
} PLAYER_STATUS;

typedef enum {
    EVENT_PLAYER_FINISH,      //播放结束
    EVENT_PLAYER_LAST_FRAME,  //播放至最后一帧
    EVENT_PLAYER_COUNT
} PLAYER_EVENT;

typedef void (*LvpVoicePlayerEventCallback)(int player_event_id, void *data);

//int LvpVoicePlayerMute(void);
int LvpVoicePlayerSetVolume(int volume);
int LvpVoicePlayerGetVolume(void);
//int LvpvoicePlayerPause(void);
//int LvpVoicePlayerResume(void);
int LvpVoicePlayerStop(void);
int LvpVoicePlayerInit(LvpVoicePlayerEventCallback cb);
int LvpVoicePlayerGetStatus(void);
int LvpVoicePlayerPlay(const unsigned char *resource_position);
int LvpVoicePlayerPlayPcm(const unsigned char *resource_position, int len, int sample_rate, int channels, int interlace);
int LvpVoicePlayerPlayAdpcm(const unsigned char *resource_position, int len, int sample_rate);
int LvpVoicePlayerSetUserPcmBuffer(unsigned char *resource_position, int len);
void LvpVoicePlayerClearUserPcmBuffer(void);

int LvpVoicePlayerTask(void *arg);

int LvpVoicePlayerSuspend(void);
int LvpVoicePlayerResume(void);

#endif
