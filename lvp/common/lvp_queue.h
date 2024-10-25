/* Grus
 * Copyright (C) 1991-2020 Nationalchip Co., Ltd
 *
 * lvp_queue.h: a Circular Queue using array
 *
 */

#ifndef __LVP_QUEUE_H__
#define __LVP_QUEUE_H__

#define __LVP_QUEUE_SUPPORT_BUFFER__

typedef struct {
    int tail;      // next write index
    int head;      // next read index, if back == front, the queue is empty
    unsigned char *buffer;
    int size;
    int member_size;
} LVP_QUEUE;

void LvpQueueInit(LVP_QUEUE *queue, unsigned char *buffer, int size, int member_size);
int LvpQueuePut(LVP_QUEUE *queue, const unsigned char *value);
int LvpQueueGet(LVP_QUEUE *queue, unsigned char *value);
int LvpQueueGetCapacity(LVP_QUEUE *queue);
int LvpQueueGetDataNum(LVP_QUEUE *queue);
int LvpQueueIsEmpty(LVP_QUEUE *queue);
int LvpQueueIsFull(LVP_QUEUE *queue);

#ifdef __LVP_QUEUE_SUPPORT_BUFFER__

int LvpQueuePutBuffer(LVP_QUEUE *queue, const unsigned int *buffer, int size);
int LvpQueueGetBuffer(LVP_QUEUE *queue, unsigned int *buffer, int size);
int LvpQueuePeekBuffer(LVP_QUEUE *queue, unsigned int *buffer, int size);

int LvpQueueGetFreeSize(LVP_QUEUE *queue);
int LvpQueueGetDataSize(LVP_QUEUE *queue);

#endif

#endif /* __LVP_QUEUE_H__ */
