/* Voice Signal Preprocess
 * Copyright (C) 2001-2018 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * message.c: MCU MESSAGEA Driver
 *
 */

#include <string.h>
#include <stdlib.h>
#include "message.h"
#include "stdbool.h"
#include "porting/port.h"

#define MSG_HOST 1 /* 1: Host side, 0: Slave side */
#ifdef CHIP_8002
#define MSG_HOST_MAGIC  0x58585542
#define MSG_SLAVE_MAGIC 0x58585542
#else
#define MSG_HOST_MAGIC  0x42555858
#define MSG_SLAVE_MAGIC 0x42555858
#endif
#if MSG_HOST == 1
# define MSG_SND_MAGIC MSG_HOST_MAGIC
# define MSG_RCV_MAGIC MSG_SLAVE_MAGIC
#else
# define MSG_SND_MAGIC MSG_SLAVE_MAGIC
# define MSG_RCV_MAGIC MSG_HOST_MAGIC
#endif

#define MSG_HEAD_LEN       14    /* Message header length */
#define MSG_HEAD_CHECK_LEN 4     /* Message header crc32 length */
#define MSG_MAGIC_LEN      4     /* Message magic length */
#define MSG_BODY_LEN_MAX   65535 /* Message body maximun length */
#define MSG_BODY_CHECK_LEN 4     /* Message body crc32 length */

#define MSG_SEQ_MAX 255  /* Message sequence maximum count */
#define MSG_LEN_MAX (MSG_HEAD_LEN + MSG_BODY_LEN_MAX)   /* Message maximun length */

#define MSG_RCV_MAGIC0 (((MSG_RCV_MAGIC) & 0x000000FF) >> 0 )
#define MSG_RCV_MAGIC1 (((MSG_RCV_MAGIC) & 0x0000FF00) >> 8 )
#define MSG_RCV_MAGIC2 (((MSG_RCV_MAGIC) & 0x00FF0000) >> 16)
#define MSG_RCV_MAGIC3 (((MSG_RCV_MAGIC) & 0xFF000000) >> 24)

static const char msg_rcv_magic[MSG_MAGIC_LEN] = {
    MSG_RCV_MAGIC0, MSG_RCV_MAGIC1,
    MSG_RCV_MAGIC2, MSG_RCV_MAGIC3
};
static unsigned char msg_seq = 0;
static unsigned char initialized = 0;
static STREAM_READ  StreamRead = NULL;
static STREAM_WRITE StreamWrite = NULL;
static STREAM_EMPTY StreamIsEmpty = NULL;

typedef struct {
     unsigned int magic:32;
     unsigned short cmd:16;
     unsigned char seq:8;
     unsigned char flags:8;
     unsigned short length:16;
     unsigned int crc32:32;
} __attribute__((packed)) MESSAGE_HEADER;

static inline int StreamGetc(char *ch)
{
    return StreamRead((unsigned char *)ch, 1);
}

static inline bool MsgByteMatch(char c)
{
    char ch = 0;

    return (StreamGetc(&ch) && (ch == c));
}

static int MsgFindMagic(void)
{
    int i;
    int offset = 0;

    while (offset < MSG_LEN_MAX) {
        for (i = 0; i < MSG_MAGIC_LEN; i++) {
            offset++;
            if (!MsgByteMatch(msg_rcv_magic[i]))
                break;

            if ((i + 1) == MSG_MAGIC_LEN)
                return 0;
        }
    }
    return -1;
}

int MessageReceive(MESSAGE *msg)
{
    MESSAGE_HEADER msg_header;
    unsigned int newcrc32 = 0;
    unsigned int *oldcrc32;
    unsigned short bodylen;
    unsigned char *pheader = (unsigned char *)&msg_header;

    if (!msg || !initialized)
        return -1;

    if (StreamIsEmpty())
        return -1;

    /* Read stream until find mseeage magic */
    if (MsgFindMagic() != 0)
    {
        printf("***MsgFindMagic error\n");
        return -1;
    }

    memset(&msg_header, 0, sizeof(MESSAGE_HEADER));
    msg_header.magic = MSG_RCV_MAGIC;

    /* Read the rest of message header */
    //printf("sizeof(MESSAGE_HEADER) = %d\n", sizeof(MESSAGE_HEADER));
    StreamRead(pheader + MSG_MAGIC_LEN,
            sizeof(MESSAGE_HEADER) - MSG_MAGIC_LEN);

    // printf("msg_header.cmd   = %X\n", msg_header.cmd);
    // printf("msg_header.seq   = %X\n", msg_header.seq);
    // printf("msg_header.flags = %X\n", msg_header.flags);

    /* Check message header integrity */
    newcrc32 = crc32(0, (unsigned char *)&msg_header,
             sizeof(MESSAGE_HEADER) - MSG_HEAD_CHECK_LEN);
    if (newcrc32 != msg_header.crc32)
         return -1;

    /* Read message body */
    if (msg_header.length > 0) {
        StreamRead(msg->body, msg_header.length);
        if (MSG_NEED_BCHECK(msg_header.flags)) {
            bodylen = msg_header.length - MSG_BODY_CHECK_LEN;
            newcrc32 = crc32(0, msg->body, bodylen);
            oldcrc32 = (unsigned int *)(msg->body + bodylen);
            if (*oldcrc32 != newcrc32){
                printf("crc校验出错!calculated crc:%d  received crc:%d\n", newcrc32, *(oldcrc32));
                exit(EXIT_SUCCESS);
                return -1;
            }
            printf("crc校验正确!calculated crc:%d  received crc:%d\n", newcrc32, *(oldcrc32));
        } else {
            bodylen = msg_header.length;
        }
    } else {
        bodylen = 0;
    }

    msg->cmd = msg_header.cmd;
    msg->seq = msg_header.seq;
    msg->flags = msg_header.flags;
    msg->bodylen = bodylen;

    return bodylen;
}

int MessageSend(MESSAGE *msg)
{
    MESSAGE_HEADER msg_header;
    unsigned int body_crc32 = 0;

    if (!msg || !initialized)
        return -1;

    memset(&msg_header, 0, sizeof(MESSAGE_HEADER));

    msg_header.magic = MSG_SND_MAGIC;
    msg_header.cmd = msg->cmd;
    msg_header.flags = msg->flags;

    if (MSG_TYPE(msg->cmd) == MSG_TYPE_RSP) {
        msg_header.seq = msg->seq;
    } else {
        msg_seq = (msg_seq + 1) % MSG_SEQ_MAX;
        msg_header.seq = msg_seq;
    }

    if (!msg->body) {
        msg_header.length = 0;
    } else {
        if (MSG_NEED_BCHECK(msg->flags)) {
            body_crc32 = crc32(0, (unsigned char *)msg->body, msg->bodylen);
            msg_header.length = msg->bodylen + MSG_BODY_CHECK_LEN;
        } else {
            msg_header.length = msg->bodylen;
        }
    }

    msg_header.crc32 = crc32(0, (unsigned char *)&msg_header,
            sizeof(MESSAGE_HEADER) - MSG_BODY_CHECK_LEN);
    // printf("\n");
    // unsigned char *data = (unsigned char *)&msg_header;

    // for (int i=0; i<sizeof(MESSAGE_HEADER) - MSG_BODY_CHECK_LEN; i++)
	// 		printf("%02x,", data[i]);
    // printf("\n");
    // printf("crc = 0x%x\n", msg_header.crc32);

    StreamWrite((unsigned char *)&msg_header, sizeof(MESSAGE_HEADER));

    if (msg->body) {
        StreamWrite(msg->body, msg->bodylen);
        //printf("send1\n");
        if (MSG_NEED_BCHECK(msg->flags))
            StreamWrite((unsigned char *)&body_crc32, MSG_BODY_CHECK_LEN);
    }

    return msg->bodylen;
}

int MessageInit(STREAM_READ read, STREAM_WRITE write, STREAM_EMPTY is_empty)
{
    if (!read || !write || !is_empty) {
        return -1;
    }

    StreamRead = read;
    StreamWrite = write;
    StreamIsEmpty = is_empty;
    initialized = 1;

    return 0;
}

void MessageDone(void)
{
    return;
}
