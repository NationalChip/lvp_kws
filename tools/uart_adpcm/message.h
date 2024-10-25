/* Voice Signal Preprocess
 * Copyright (C) 2001-2018 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * message.h: MCU MESSAGE driver
 *
 */

#ifndef __MESSAGE_H__
#define __MESSAGE_H__


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

/* Get message type */
#define MSG_TYPE(cmd) (((cmd) >> MSG_TYPESHIFT) & MSG_TYPEMASK)

/* Get message nr */
#define MSG_NR(cmd)   (((cmd) >> MSG_NRSHIFT) & MSG_NRMASK)

/*
 * Message flags
 */
#define MSG_FLAG_BCHECK (1 << 0)
#define MSG_FLAG_BCHECK_BITS 1
#define MSG_FLAG_BCHECK_MASK ((1 << MSG_FLAG_BCHECK_BITS)-1)
#define MSG_FLAG_BCHECK_SHIFT 0

/* Check if need message body crc32 */
#define MSG_NEED_BCHECK(flags) (((flags) >> MSG_FLAG_BCHECK_SHIFT) & MSG_FLAG_BCHECK_MASK)

typedef enum {
    MSG_TYPE_REQ = 0x1,
    MSG_TYPE_RSP = 0x2,
    MSG_TYPE_NTF = 0x3,
}MSG_TYPE;

typedef struct {
    unsigned cmd:16;
    unsigned seq:8;
    unsigned flags:8;
    unsigned char *body;
    unsigned bodylen:16;
} __attribute__((packed)) MESSAGE;

/* Stream read callback */
typedef int (*STREAM_READ)(unsigned char *buf, int len);

/* Stream write callback */
typedef int (*STREAM_WRITE)(const unsigned char *buf, int len);

/* Stream is empty callback */
typedef int (*STREAM_EMPTY)(void);

int MessageReceive(MESSAGE *msg);
int MessageSend(MESSAGE *msg);
int MessageInit(STREAM_READ read, STREAM_WRITE write, STREAM_EMPTY is_empty);
void MessageDone(void);

#endif

