/* Grus
 * Copyright (C) 1991-2020 Nationalchip Co., Ltd
 *
 * lvp_queue.c: a Circular Queue using array
 *
 */

#include <stdio.h>
#include <lvp_attr.h>

#include "lvp_queue.h"

void LvpQueueInit(LVP_QUEUE *queue, unsigned char *buffer, int size, int member_size)
{
    queue->buffer = buffer;
    queue->size = size - size % member_size;
    queue->member_size = member_size;
    queue->tail = 0;
    queue->head = 0;
}

DRAM0_STAGE2_SRAM_ATTR int LvpQueuePut(LVP_QUEUE *queue, const unsigned char *value)
{
    if (((queue->tail + queue->member_size) % queue->size) == queue->head)
        return 0;

    for (int i = 0; i < queue->member_size; i++) {
        queue->buffer[(queue->tail + i) % queue->size] = value[i];
    }
    queue->tail = (queue->tail + queue->member_size) % queue->size;
    return 1;
}

int LvpQueueGet(LVP_QUEUE *queue, unsigned char *value)
{
    if (queue->head == queue->tail)
        return 0;

    for (int i = 0; i < queue->member_size; i++) {
        value[i] = queue->buffer[(queue->head + i) % queue->size];
    }
    queue->head = (queue->head + queue->member_size) % queue->size;
    return 1;
}

int LvpQueueGetCapacity(LVP_QUEUE *queue)
{
    return queue->size / queue->member_size;
}

int LvpQueueGetDataNum(LVP_QUEUE *queue)
{
    return ((queue->tail >= queue->head) ? (queue->tail - queue->head)
        : (queue->size - queue->head + queue->tail)) / queue->member_size;
}

int LvpQueueIsEmpty(LVP_QUEUE *queue)
{
    return (queue->head == queue->tail);
}

int LvpQueueIsFull(LVP_QUEUE *queue)
{
    return (((queue->tail + queue->member_size) % queue->size) == queue->head);
}

#ifdef __LVP_QUEUE_SUPPORT_BUFFER__
int LvpQueuePutBuffer(LVP_QUEUE *queue, const unsigned int *buffer, int count)
{
    if (LvpQueueGetFreeSize(queue) < count * queue->member_size)
        return 0;

    int frontFreeSize = queue->size - queue->tail;
    if (frontFreeSize > count * queue->member_size) {
        for (int i = 0; i < count * queue->member_size; i++)
            queue->buffer[queue->tail + i] = buffer[i];
        queue->tail += count * queue->member_size;
    } else {
        for (int i = 0; i < frontFreeSize; i++)
            queue->buffer[queue->tail + i] = buffer[i];
        for (int i = 0; i < count * queue->member_size - frontFreeSize; i++)
            queue->buffer[i] = buffer[frontFreeSize + i];
        queue->tail = count * queue->member_size - frontFreeSize;
    }
    return count * queue->member_size;
}

int LvpQueueGetBuffer(LVP_QUEUE *queue, unsigned int *buffer, int count)
{
    if (LvpQueueGetDataSize(queue) < count * queue->member_size)
        return 0;

    int frontDataSize = queue->size - queue->head;
    if (frontDataSize > count * queue->member_size) {
        for (int i = 0; i < count * queue->member_size; i++)
            buffer[i] = queue->buffer[queue->head + i];
        queue->head += count * queue->member_size;
    } else {
        for (int i = 0; i < frontDataSize; i++)
            buffer[i] = queue->buffer[queue->head + i];
        for (int i = 0; i < count * queue->member_size - frontDataSize; i++)
            buffer[frontDataSize + i] = queue->buffer[i];
        queue->head = count * queue->member_size - frontDataSize;
    }
    return count * queue->member_size;
}

int LvpQueuePeekBuffer(LVP_QUEUE *queue, unsigned int *buffer, int size)
{
    if (LvpQueueGetDataSize(queue) < size)
        return 0;

    int frontDataSize = queue->size - queue->head;
    if (frontDataSize > size) {
        for (int i = 0; i < size; i++)
            buffer[i] = queue->buffer[queue->head + i];
    } else {
        for (int i = 0; i < frontDataSize; i++)
            buffer[i] = queue->buffer[queue->head + i];
        for (int i = 0; i < size - frontDataSize; i++)
            buffer[frontDataSize + i] = queue->buffer[i];
    }
    return size;
}

int LvpQueueGetFreeSize(LVP_QUEUE *queue)
{
    return (queue->head > queue->tail) ? (queue->head - queue->tail - 1)
    : (queue->size - queue->tail + queue->head - 1);
}

int LvpQueueGetDataSize(LVP_QUEUE *queue)
{
    return (queue->tail >= queue->head) ? (queue->tail - queue->head)
    : (queue->size - queue->head + queue->tail);
}

#endif
