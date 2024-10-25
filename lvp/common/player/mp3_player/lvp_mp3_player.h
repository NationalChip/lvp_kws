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
/*user interface*/
typedef void (*LvpVoicePlayerEventCallback)(int player_event_id, void *data);

/**
 * @brief 初始化mp3 player
 * @param cb 注册的回调
 * @return int    是否初始化成功
 * @retval 0 成功
 * @retval -1 失败
 */
int LvpMp3PlayerInit(LvpVoicePlayerEventCallback cb);

/**
 * @brief 设置音量(0-100)
 *
 * @param volume  音量(0-100)
 * @return int    是否设置成功
 * @retval 0 成功
 * @retval -1 失败
 */
int LvpMp3PlayerSetVolume(int volume);
/**
 * @brief 获取音量
 * @return int  音量(0-100)
 */
int LvpMp3PlayerGetVolume(void);
/**
 * @brief 暂停播放
 */
void LvpMp3PlayerStop(void);
/**
 * @brief 获取播放状态
 *
 * @return int 播放状态，对应PLAYER_STATUS
 */
int LvpMp3PlayerGetStatus(void);
/**
 * @brief 播放mp3
 * @param resource_position mp3资源指针
 * @param length mp3资源长度
 * @param repeat 是否循环播放
 * @return int    是否初始化成功
 * @retval 0 成功
 * @retval -1 失败
 */
int LvpMp3PlayerPlay(const unsigned char *resource_position, unsigned int length, unsigned int repeat);



int LvpMp3PlayerSuspend(void);
int LvpMp3PlayerResume(void);
int LvpMp3PlayerTask(void *arg);

#endif
