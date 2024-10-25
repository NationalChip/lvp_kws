/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * uart_message_v2.h: MCU uart communication protocol driver
 *
 */

#ifndef __MCU_UART_MESSAGE_V2__
#define __MCU_UART_MESSAGE_V2__

#define MSG_HOST_MAGIC  0x42555858 // BUXX
#define MSG_SLAVE_MAGIC 0x58585542

/*
 * Message command
 */
#define MSG_TYPEBITS 8
#define MSG_NRBITS   8

#define MSG_NRMASK   ((1 << MSG_NRBITS)-1)
#define MSG_TYPEMASK ((1 << MSG_TYPEBITS)-1)

#define MSG_NRSHIFT  0
#define MSG_TYPESHIFT (MSG_NRSHIFT +  MSG_NRBITS)


/* Create new message */
#define NEW_MSG(type, nr) \
    (((type) << MSG_TYPESHIFT) | \
     ((nr)   << MSG_NRSHIFT))
//=================================================================================================
typedef enum {
    MSG_TYPE_REQ = 0x1,
    MSG_TYPE_RSP = 0x2,
    MSG_TYPE_NTF = 0x3,
}MSG_TYPE;

typedef enum {
    MSG_REQ_REBOOT                  = NEW_MSG(MSG_TYPE_REQ, 0x01),    // This used for OTA.
    MSG_REQ_FW_VERSION              = NEW_MSG(MSG_TYPE_REQ, 0x02),
    MSG_REQ_FW_CRC32_RESULT         = NEW_MSG(MSG_TYPE_REQ, 0x03),
    MSG_REQ_UPGRADEABLE             = NEW_MSG(MSG_TYPE_REQ, 0x04),
    MSG_REQ_UPGRADE_MODE            = NEW_MSG(MSG_TYPE_REQ, 0x05),
    MSG_SET_UPGRADE_INFO            = NEW_MSG(MSG_TYPE_REQ, 0x06),
    MSG_SET_UPGRADE_DATA            = NEW_MSG(MSG_TYPE_REQ, 0x07),
    MSG_REQ_UPGRADE_STATUS          = NEW_MSG(MSG_TYPE_REQ, 0X08),
    MSG_REQ_FW_BACKUP               = NEW_MSG(MSG_TYPE_REQ, 0x09),
    MSG_REQ_FW_ROLLING              = NEW_MSG(MSG_TYPE_REQ, 0x0A),

    MSG_NTF_BT_STATUS               = NEW_MSG(MSG_TYPE_NTF, 0x01),
    MSG_NTF_AUDIO_OUT_FORMAT        = NEW_MSG(MSG_TYPE_NTF, 0x02),
    MSG_NTF_PHONE_STATUS            = NEW_MSG(MSG_TYPE_NTF, 0x03),
    MSG_NTF_MUSIC_STATUS            = NEW_MSG(MSG_TYPE_NTF, 0x04),
    MSG_NTF_MIC_ON                  = NEW_MSG(MSG_TYPE_NTF, 0x05),
    MSG_NTF_AUDIO_CODE_FORMAT       = NEW_MSG(MSG_TYPE_NTF, 0x06),

/* PCM data */
    MSG_NTF_START_PCM               = NEW_MSG(MSG_TYPE_NTF, 0x07),    // This used for post PCM to others.
    MSG_NTF_STOP_PCM                = NEW_MSG(MSG_TYPE_NTF, 0x08),
/* -------- */

    MSG_NTF_SET_BAUDRATE            = NEW_MSG(MSG_TYPE_NTF, 0x09),

    MSG_NTF_UART_PROXY_START        = NEW_MSG(MSG_TYPE_NTF, 0x0A),
    MSG_NTF_UART_PROXY_DATA         = NEW_MSG(MSG_TYPE_NTF, 0x0B),
    MSG_NTF_UART_PROXY_READY        = NEW_MSG(MSG_TYPE_NTF, 0x0C),
    MSG_NTF_UART_PROXY_OPERATION    = NEW_MSG(MSG_TYPE_NTF, 0x0D),

    MSG_NTF_UART_GET_HOST_CONDITION = NEW_MSG(MSG_TYPE_NTF, 0x0E),
    MSG_NTF_UART_HOST_CONDITION     = NEW_MSG(MSG_TYPE_NTF, 0x0F),

/* SLEEP */
    MSG_NTF_SET_SLEEP               = NEW_MSG(MSG_TYPE_NTF, 0x10),
    MSG_NTF_SET_WAKEUP              = NEW_MSG(MSG_TYPE_NTF, 0x11),
    MSG_NTF_SLEEP                   = NEW_MSG(MSG_TYPE_NTF, 0x12),
    MSG_NTF_WAKEUP                  = NEW_MSG(MSG_TYPE_NTF, 0x13),
/* ----- */

/* FEATURE data */
    MSG_NTF_FEATURE_START           = NEW_MSG(MSG_TYPE_NTF, 0x14),    //Used for uart get feature data.
    MSG_NTF_FEATURE_DATA            = NEW_MSG(MSG_TYPE_NTF, 0x15),
    MSG_NTF_FEATURE_DONE            = NEW_MSG(MSG_TYPE_NTF, 0x16),
    MSG_NTF_FEATURE_READY           = NEW_MSG(MSG_TYPE_NTF, 0x17),
/* -------- */

    MSG_NTF_HEARTBEATS              = NEW_MSG(MSG_TYPE_NTF, 0x80),

/* PCM data */
    MSG_NTF_PCM_START               = NEW_MSG(MSG_TYPE_NTF, 0x81),    // This used for uart player by PCM.
    MSG_NTF_PCM_DATA                = NEW_MSG(MSG_TYPE_NTF, 0x82),
    MSG_NTF_PCM_DONE                = NEW_MSG(MSG_TYPE_NTF, 0x83),
    MSG_NTF_PCM_READY               = NEW_MSG(MSG_TYPE_NTF, 0x84),
/* -------- */

} UART_MSG_ID;

typedef struct {
     unsigned int magic;                        // When you send a pack, just need to prepare this-
     unsigned cmd:16;                           // -this-
     unsigned seq:8;
     unsigned flags:8;                          // -this-
     unsigned length:16;
     unsigned crc32:32;
}  __attribute__((packed)) MESSAGE_HEADER;
//}  __attribute__((aligned(16))) MESSAGE_HEADER;

typedef struct {
    MESSAGE_HEADER msg_header; // Its default magic is that you set in init, set magic value is 0 to use it.
    unsigned char *body_addr;                   // -this-
    unsigned char port;                         // -this-
    unsigned int len;                           // -and this.
    unsigned int body_vef;
} MSG_PACK;

typedef int (*MSG_PACK_CALLBACK)(MSG_PACK * pack, void *priv);

typedef struct {
    unsigned char port;
    UART_MSG_ID msg_id;
    unsigned char *msg_buffer;
    unsigned int msg_buffer_length;
    unsigned int msg_buffer_offset;
    MSG_PACK_CALLBACK msg_pack_callback;    // It will be called when find one recved pack which you registed to care.
    void* priv;
} UART_MSG_REGIST;

typedef struct {
    int             port;
    unsigned int    baudrate;
    unsigned int    magic;
    unsigned int    reinit_flag; // If you want init it anyway, set 1.
} UART_MSG_INIT_CONFIG;
//=================================================================================================

int UartMessageAsyncInit(UART_MSG_INIT_CONFIG *config);
int UartMessageAsyncDone(void);

int UartMessageAsyncRegist(UART_MSG_REGIST *uart_msg_regist); // Regist recv infomation for which uart cmd you care for.
int UartMessageAsyncLogout(UART_MSG_REGIST *uart_msg_regist); // Logout it.
int UartMessageAsyncRecvedPop(unsigned int port, unsigned int msg_id, MSG_PACK *uart_pack);

int UartMessageAsyncSend(MSG_PACK *pack); // when body data is less than 32Byte and send_common buffer isnot full, it will be hold by this inside.
int UartMessageAsyncSendWithCallback(MSG_PACK *pack, MSG_PACK_CALLBACK send_callback, void *priv);

int UartMessageAsyncTick(void); // This used for call recv callback, need be called in tick_loop.
#endif
