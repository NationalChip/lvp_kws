/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * vc_message.c
 *
 */

#include <stdio.h>
#include <driver/gx_gpio.h>
#include <driver/gx_delay.h>
#include <driver/gx_timer.h>
#include <board_misc_config.h>
#include <gpio_func.h>
#include "uart_message_v2.h"
#include "lvp_i2c_msg.h"
#include "vc_message.h"
#include "gsensor.h"
#include "lvp_audio_in.h"

#define LOG_TAG "[VC_MESSAGE]"

#if defined(CONFIG_APP_VC_UART_MESSAGE_QUERY_SUPPORTED) || defined(CONFIG_APP_VC_HW_I2C)
#define MESSAGE_SESSION_ENABLE
#endif

#ifdef MESSAGE_SESSION_ENABLE
#define MSG_SESSION_TIMEOUT_MS    3000

typedef struct {
    char state;
    unsigned short cur_kws;
    unsigned int start_ms;
}MESSAGE_SESSION;

typedef struct
{
    unsigned char BuildVerstion;
    unsigned char CurrectVerstion;
    unsigned char SecondVerstion;
    unsigned char MainVerstion;
}SOFT_VERSTION;

static MESSAGE_SESSION message_session;
static SOFT_VERSTION sv;
#endif

#if defined(CONFIG_APP_VC_UART_PORT)
# define MESSAGE_UART    CONFIG_APP_VC_UART_PORT
#else
# define MESSAGE_UART 0
#endif

#if defined(MESSAGE_UART)
typedef enum {

    UART_MESSAGE_REPLY = 0,
    UART_MESSAGE_NOTIFY,
}UartMessageMode;

static MSG_PACK send_pack_data = {
    .port = MESSAGE_UART,
    .msg_header.magic = MSG_SLAVE_MAGIC,
};

//static int _uartMsgSend(unsigned short event, UartMessageMode mode)
static int _uartMsgSend(UartMessageMode mode, unsigned char cmd, unsigned short event)
{
    static unsigned short send_event = 0;
    send_event = event;

    if(mode == UART_MESSAGE_NOTIFY) {
        //send_pack_data.msg_header.cmd = 0x030C;
        send_pack_data.msg_header.cmd = (0x03 << 8) | cmd ;
    } else {
        //send_pack_data.msg_header.cmd = 0x020C;
        send_pack_data.msg_header.cmd = (0x02 << 8) | cmd;
    }
    send_pack_data.msg_header.flags = 1;
    send_pack_data.body_addr = (unsigned char *)&send_event;
    send_pack_data.len = 2;
    UartMessageAsyncSend(&send_pack_data);

    return 0;
}

#ifdef CONFIG_APP_VC_UART_MESSAGE_QUERY_SUPPORTED
static int MicGetInfo(void)
{
    static int once_vad = 0;
    if(once_vad == 0){
        int vad = LvpAudioInQueryFFTVad(NULL);
        once_vad = vad;
    }
    return once_vad;
}

static unsigned char uart_recv_buffer[32] __attribute__((aligned(16)))= {0};
static int _uartRecvCallback(MSG_PACK * pack, void *priv)
{
    printf("[%s]%d\n", __func__, __LINE__);

    if (pack->msg_header.cmd == 0x010C) {
        printf(">>>>>>> cmd: 0x%x, kws = %d <<<<<<<\n", pack->msg_header.cmd, message_session.cur_kws);

#if 0
        for (int i = 0; i < pack->len; i++) {
            printf("0x%x ",*(pack->body_addr + i));
        }
        printf("\n");
#endif

        _uartMsgSend(UART_MESSAGE_REPLY, 0x0C, message_session.cur_kws);

        message_session.state = 0;
        message_session.cur_kws = 0;
    }

    if(pack->msg_header.cmd == 0x0170)
    {
        printf("receive misc test cmd\n");
        unsigned short send_event = 1;
        if(!MicGetInfo())
        {
            send_event = 0;
        }
        _uartMsgSend(UART_MESSAGE_REPLY, 0x70, send_event);
    }

    if(pack->msg_header.cmd == 0x0171)
    {
        printf("receive gsensor test cmd\n");
        unsigned short send_event = 0;
        if(!GsensorGetInfo())
        {
#if defined (CONFIG_LVP_ENABLE_G_SENSOR_VAD)
            if(LvpGetGvad())
                send_event = 2;//active
            else
                send_event = 1; //idle
#endif
        }
        _uartMsgSend(UART_MESSAGE_REPLY, 0x71, send_event);
    }

    if(pack->msg_header.cmd == 0x0102)
    {
        printf("receive version req\n");

        char str[] = CONFIG_SOFT_VERSTION;
        char *buff = NULL;

        buff = strtok(str,".");
        sv.MainVerstion = ((unsigned char)*buff - '0');

        buff = strtok(NULL,".");
        sv.SecondVerstion = ((unsigned char)*buff - '0');

        buff = strtok(NULL,".");
        sv.CurrectVerstion = ((unsigned char)*buff - '0');

        buff = strtok(NULL,".");
        sv.BuildVerstion = ((unsigned char)*buff - '0');

        static unsigned char send_event[4] = {0x0,0x0,0x0,0x0};
        send_event[0] = sv.MainVerstion;
        send_event[1] = sv.SecondVerstion;
        send_event[2] = sv.CurrectVerstion;
        send_event[3] = sv.BuildVerstion;

        send_pack_data.msg_header.cmd = (0x02 << 8) | 0x02;
        send_pack_data.msg_header.flags = 1;
        send_pack_data.body_addr = (unsigned char *)&send_event;
        send_pack_data.len = 4;
        UartMessageAsyncSend(&send_pack_data);
    }

    return 0;
}

static void _uartRecvInit(void)
{
    UART_MSG_REGIST regist_info_1 = {
        .msg_id = 0x010C,
        .port = MESSAGE_UART,
        .msg_buffer = uart_recv_buffer,
        .msg_buffer_length = sizeof(uart_recv_buffer),
        .msg_pack_callback = _uartRecvCallback,
        .priv = "voice event"

    };

    UART_MSG_REGIST regist_info_2 = {
        .msg_id = 0x0170,
        .port = MESSAGE_UART,
        .msg_buffer = NULL,
        .msg_buffer_length = 0,
        .msg_pack_callback = _uartRecvCallback,
        .priv = "mic status"

    };

    UART_MSG_REGIST regist_info_3 = {
        .msg_id = 0x0171,
        .port = MESSAGE_UART,
        .msg_buffer = NULL,
        .msg_buffer_length = 0,
        .msg_pack_callback = _uartRecvCallback,
        .priv = "gsensor status"
    };

    UART_MSG_REGIST regist_info_4 = {
        .msg_id = 0x0102,
        .port = MESSAGE_UART,
        .msg_buffer = NULL,
        .msg_buffer_length = 0,
        .msg_pack_callback = _uartRecvCallback,
        .priv = "version"
    };

    UartMessageAsyncRegist(&regist_info_1);
    UartMessageAsyncRegist(&regist_info_2);
    UartMessageAsyncRegist(&regist_info_3);
    UartMessageAsyncRegist(&regist_info_4);
}
#endif

static void _uartMsgInit(void)
{
    printf("[%s]%d\n", __func__, __LINE__);

    UartMessageAsyncDone();

    UART_MSG_INIT_CONFIG init_config = {
        .magic = MSG_SLAVE_MAGIC,
        .port = MESSAGE_UART,
        .baudrate = 115200,
        .reinit_flag = 1
    };
    UartMessageAsyncInit(&init_config);

#ifdef CONFIG_APP_VC_UART_MESSAGE_QUERY_SUPPORTED
    _uartRecvInit();
#endif
}
#endif

#ifdef CONFIG_APP_VC_HAS_MESSAGE_NOTIFY_PIN
static void _gpioMessageInit(void)
{
    gx_gpio_set_level(CONFIG_APP_VC_MESSAGE_NOTIFY_PIN_ID, GX_GPIO_LEVEL_HIGH);
    gx_gpio_set_direction(CONFIG_APP_VC_MESSAGE_NOTIFY_PIN_ID, GX_GPIO_DIRECTION_OUTPUT);
}

static void _gpioMessageNotify(void)
{
    gx_gpio_set_level(CONFIG_APP_VC_MESSAGE_NOTIFY_PIN_ID, GX_GPIO_LEVEL_LOW);
    gx_mdelay(60);
    gx_gpio_set_level(CONFIG_APP_VC_MESSAGE_NOTIFY_PIN_ID, GX_GPIO_LEVEL_HIGH);
}
#endif

#ifdef CONFIG_APP_VC_HW_I2C
static void _i2cEventConfirm(void)
{
    printf("---------- [%s]%d message confirm ----------\n", __func__, __LINE__);
    message_session.state = 0;
}
#endif

int VCMessageInit(void)
{
#ifdef CONFIG_APP_VC_HAS_MESSAGE_NOTIFY_PIN
    printf (LOG_TAG"NOTIFY PIN %d\n", CONFIG_APP_VC_MESSAGE_NOTIFY_PIN_ID);
    _gpioMessageInit();
#endif

#ifdef CONFIG_APP_VC_HW_I2C
    printf (LOG_TAG"HW I2C\n");
    LvpI2CMsgInit();
#endif

#if defined(MESSAGE_UART)
    printf (LOG_TAG"MASSAGE UART %d\n", MESSAGE_UART);
    _uartMsgInit();
#endif

    return 0;
}

void VCNewMessageNotify(unsigned short kws)
{
#ifdef MESSAGE_SESSION_ENABLE
    message_session.state = 1;
    message_session.start_ms = gx_get_time_ms();
    message_session.cur_kws = kws;
    printf("--- voice_message_notify, start_ms = %u ---\n", message_session.start_ms);
#endif

#ifdef CONFIG_APP_VC_UART_MESSAGE_NOTIFY_SUPPORTED
    _uartMsgSend(UART_MESSAGE_NOTIFY, 0x0C, kws);
#endif

#ifdef CONFIG_APP_VC_HW_I2C
    LvpI2CMsgWriteVoiceEvent(kws, _i2cEventConfirm);
#endif

#ifdef CONFIG_APP_VC_HAS_MESSAGE_NOTIFY_PIN
    _gpioMessageNotify();
#endif
}

int VCMessageSessionPoll(void)
{
#ifdef MESSAGE_SESSION_ENABLE
    if(message_session.state) {
        unsigned int cur_ms = gx_get_time_ms();

        if(cur_ms - message_session.start_ms >= MSG_SESSION_TIMEOUT_MS) {
            printf("--- poll session timeout, cur_ms = %u ---\n", cur_ms);
            message_session.state = 0;
        }
    }

    return message_session.state;
#else
    return 0;
#endif
}

