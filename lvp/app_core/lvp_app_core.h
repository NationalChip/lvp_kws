/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_app_event.h:
 */
#ifndef __LVP_APP_EVENT_H__
#define __LVP_APP_EVENT_H__

typedef struct {
    unsigned int event_id;
    unsigned int ctx_index;
} APP_EVENT;

// Private Event ID [1~99] for APP
#define LVP_WAKE_UP_EVENT_ID          (90)
#define AUDIO_IN_RECORD_DONE_EVENT_ID (91)
#define LVP_DENOISE_DONE_ID           (92)
#define LVP_SED_DONE_ID               (92)

//=============================================================================//
typedef enum {
    PBT_UNDEFINE        = 0x0,
    PBT_OTHER           = 0x1,
    PBT_VOICE_PLAY      = 0x2,
    PBT_UART_ASYNC_SEND = 0x4,
    PBT_LED_CONTRL      = 0x8,
    PBT_ENABLE_UART_SEND_WAV = 0xa,
} APP_BEHAVIOR_TYPE;

//=============================================================================//
typedef enum {
    PST_SWITCH_VPA_MODE = 1,
    PST_NEW_VOICE_PLAY_TASK = 2,
} APP_STATUS_TYPE;

typedef struct {
    enum {
        APP_STATUS_VPA_ACTIVE,
        APP_STATUS_VPA_TALKING,
        APP_STATUS_VPA_BYPASS,
        APP_STATUS_VPA_IDLE
    } vpa_mode;
    unsigned char new_voice_play_task;
} APP_STATUS;


int LvpInitializeAppEvent(void);
int LvpTriggerAppEvent(APP_EVENT *app_event);
int LvpAppEventTick(void);

#endif /* __LVP_APP_EVENT_H__ */
