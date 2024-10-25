/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * uart_message_v2.c: MCU uart communication protocol
 *
 */

#include <stdio.h>
#include <string.h>
#include <common.h>
#include <errno.h>
#include <misc_regs.h>

#include <lvp_pmu.h>

#include <driver/gx_uart.h>
#include <driver/gx_cache.h>
#include <driver/gx_irq.h>

#include "uart_message_v2.h"
#include "lvp_queue.h"

#define LOG_TAG "[MSG2.0]"

#define BSWAP_32(x) \
    (uint32_t)((((uint32_t)(x) & 0xff000000) >> 24) | \
              (((uint32_t)(x) & 0x00ff0000) >> 8) | \
              (((uint32_t)(x) & 0x0000ff00) << 8) | \
              (((uint32_t)(x) & 0x000000ff) << 24) \
             )

#define BSWAP_16(x) \
    ((short)( \
        (((short)(x) & (short)0x00ffU) << 8 ) | \
        (((short)(x) & (short)0xff00U) >> 8 ) ))

#ifdef CONFIG_LVP_UART_MESSAGE_SEND_Q_LEN
#define UART_SEND_QUENE_LEN CONFIG_LVP_UART_MESSAGE_SEND_Q_LEN
#else
#define UART_SEND_QUENE_LEN 8
#endif

#ifdef CONFIG_LVP_UART_MESSAGE_RECV_Q_LEN
#define UART_RECV_QUENE_LEN CONFIG_LVP_UART_MESSAGE_RECV_Q_LEN
#else
#define UART_RECV_QUENE_LEN 8
#endif

#ifdef CONFIG_LVP_UART_MESSAGE_SEND_REGIST_NUM
#define UART_MSG_SEND_CALLBACK_NUM CONFIG_LVP_UART_MESSAGE_SEND_REGIST_NUM
#else
#define UART_MSG_SEND_CALLBACK_NUM 6
#endif

#ifdef CONFIG_LVP_UART_MESSAGE_RECV_REGIST_NUM
#define UART_MSG_REGIST_MAXNUM CONFIG_LVP_UART_MESSAGE_RECV_REGIST_NUM
#else
#define UART_MSG_REGIST_MAXNUM 16
#endif


//=================================================================================================

typedef enum {
    MRS_FIND_MAGIC,
    MRS_CHECK_HEADER,
    MRS_CHECK_BODY,
    MRS_BODY_VEF,
} MESSAGE_REAV_STATE;

typedef enum {
    MSS_SEND_HEADER,
    MSS_SEND_BODY,
    MSS_SEND_BODY_CRC,
    MSS_SEND_DONE,
} MESSAGE_SEND_STATE;


typedef struct {
    unsigned int        magic;
    unsigned int        tmp_vef; // Use for check verification
    unsigned int        port;
    int                 init_flag;
    unsigned int        baudrate;
    unsigned char       send_sequence;
    unsigned char       recv_sequence;
    unsigned int        send_ing;
    MSG_PACK            cur_recv_pack;
    MSG_PACK            cur_send_pack;
    unsigned char       uart_send_pack_queue_buffer[UART_SEND_QUENE_LEN * sizeof(MSG_PACK)];
    LVP_QUEUE           uart_send_pack_queue;
    MESSAGE_REAV_STATE  recv_state;
    MESSAGE_SEND_STATE  send_state;
    int                 pmu_lock;
} MESSAGE_HANDLE;

#ifdef CONFIG_UART_MESSAGE_PORT_BOTH
static MESSAGE_HANDLE s_msg_handle[2];
#else
static MESSAGE_HANDLE s_msg_handle[1];
#endif

static UART_MSG_REGIST s_uart_msg_regist_array[UART_MSG_REGIST_MAXNUM];

static unsigned char s_uart_recv_pack_queue_buffer[UART_RECV_QUENE_LEN * sizeof(MSG_PACK)] = {0};
LVP_QUEUE s_uart_recv_pack_queue;

static MESSAGE_HANDLE *_GetMessageHandle(unsigned char port)
{
#ifdef CONFIG_UART_MESSAGE_PORT_BOTH
    if (port == GX_UART_PORT0)
        return (MESSAGE_HANDLE *)&s_msg_handle[0];
    else if (port == GX_UART_PORT1)
        return (MESSAGE_HANDLE *)&s_msg_handle[1];
    else
        return NULL;
#elif (defined CONFIG_UART_MESSAGE_PORT_0)
    if (port == GX_UART_PORT0)
        return (MESSAGE_HANDLE *)&s_msg_handle[0];
    else
        return NULL;
#elif (defined CONFIG_UART_MESSAGE_PORT_1)
    if (port == GX_UART_PORT1)
        return (MESSAGE_HANDLE *)&s_msg_handle[0];
    else
        return NULL;
#endif
    printf("Haven't enable any port uart message !");
    return NULL;
}

static int _CheckCrc(unsigned char *data, int len, unsigned int crc_stand)
{
    return crc_stand == crc32(0, data, len);
}

// static int _Sum(unsigned int sum, unsigned char *data, int len)
// {
//     while(len--) {
//         sum += data[len];
//     }
//     return sum;
// }

// static int _CheckSum(unsigned char *data, int len, unsigned int sum_stand)
// {
//     return sum_stand == _Sum(0, data, len);
// }

int UartMessageAsyncRegist(UART_MSG_REGIST *uart_msg_regist)
{
    for (int i = 0; i < sizeof(s_uart_msg_regist_array) / sizeof(UART_MSG_REGIST); i++) {
        if (uart_msg_regist->msg_id == s_uart_msg_regist_array[i].msg_id) {
            s_uart_msg_regist_array[i].msg_id = 0;
            s_uart_msg_regist_array[i].port = -1;
            s_uart_msg_regist_array[i].msg_buffer = NULL;
            s_uart_msg_regist_array[i].msg_buffer_length = 0;
            s_uart_msg_regist_array[i].msg_buffer_offset = 0;
            s_uart_msg_regist_array[i].priv = NULL;
            s_uart_msg_regist_array[i].msg_pack_callback = NULL;
        }
    }

    for (int i = 0; i < sizeof(s_uart_msg_regist_array) / sizeof(UART_MSG_REGIST); i++) {
        if (s_uart_msg_regist_array[i].msg_id == 0) {
            s_uart_msg_regist_array[i].msg_id = uart_msg_regist->msg_id;
            s_uart_msg_regist_array[i].port = uart_msg_regist->port;
            s_uart_msg_regist_array[i].msg_buffer           = uart_msg_regist->msg_buffer;
            s_uart_msg_regist_array[i].msg_buffer_length    = uart_msg_regist->msg_buffer_length;
            s_uart_msg_regist_array[i].msg_buffer_offset    = uart_msg_regist->msg_buffer_offset;
            s_uart_msg_regist_array[i].priv                 = uart_msg_regist->priv;
            s_uart_msg_regist_array[i].msg_pack_callback   = uart_msg_regist->msg_pack_callback;
            return 0;
        }
    }
    return -1;
}

int UartMessageAsyncLogout(UART_MSG_REGIST *uart_msg_regist)
{
    for (int i = 0; i < sizeof(s_uart_msg_regist_array) / sizeof(UART_MSG_REGIST); i++) {
        if (uart_msg_regist->msg_id == s_uart_msg_regist_array[i].msg_id) {
            uart_msg_regist->port         = s_uart_msg_regist_array[i].port;
            uart_msg_regist->msg_buffer         = s_uart_msg_regist_array[i].msg_buffer;
            uart_msg_regist->msg_buffer_length  = s_uart_msg_regist_array[i].msg_buffer_length;
            uart_msg_regist->msg_buffer_offset  = s_uart_msg_regist_array[i].msg_buffer_offset;
            uart_msg_regist->priv               = s_uart_msg_regist_array[i].priv;
            uart_msg_regist->msg_pack_callback = s_uart_msg_regist_array[i].msg_pack_callback;

            s_uart_msg_regist_array[i].msg_id = 0;
            s_uart_msg_regist_array[i].port = -1;
            s_uart_msg_regist_array[i].msg_buffer = NULL;
            s_uart_msg_regist_array[i].msg_buffer_length = 0;
            s_uart_msg_regist_array[i].msg_buffer_offset = 0;
            s_uart_msg_regist_array[i].priv = NULL;
            s_uart_msg_regist_array[i].msg_pack_callback = NULL;
            return 0;
        }
    }

    return -1;
}

int UartMessageAsyncRecvedPop(unsigned int port, unsigned int msg_id, MSG_PACK *uart_pack)
{
    int result = -1;

    if (LvpQueueIsEmpty(&s_uart_recv_pack_queue)) {
        return result;
    }

    MSG_PACK *pack_p = uart_pack;

    if (LvpQueueIsFull(&s_uart_recv_pack_queue)) {
        if (LvpQueueGet(&s_uart_recv_pack_queue, (unsigned char *)pack_p) == 0) return -1; //没有待处理的pack
        unsigned int tmp = pack_p->msg_header.cmd;
        pack_p->msg_header.cmd = 0;
        unsigned int irq_state = gx_lock_irq_save();
        LvpQueuePut(&s_uart_recv_pack_queue, (unsigned char *)pack_p);
        gx_unlock_irq_restore(irq_state);

        pack_p->msg_header.cmd = tmp;
    } else {
        pack_p->msg_header.cmd = 0;
        unsigned int irq_state = gx_lock_irq_save();
        LvpQueuePut(&s_uart_recv_pack_queue, (unsigned char *)pack_p);
        gx_unlock_irq_restore(irq_state);

        if (LvpQueueGet(&s_uart_recv_pack_queue, (unsigned char *)pack_p) == 0) return -1; //没有待处理的pack
    }

    MSG_PACK tmp_pack;

    do {
        if (msg_id == pack_p->msg_header.cmd && port == pack_p->port && pack_p == uart_pack) {
            pack_p = &tmp_pack;
            result = 0;
        } else {
            unsigned int irq_state = gx_lock_irq_save();
            LvpQueuePut(&s_uart_recv_pack_queue, (unsigned char *)pack_p);
            gx_unlock_irq_restore(irq_state);
        }

        if (LvpQueueGet(&s_uart_recv_pack_queue, (unsigned char *)pack_p) == 0) return -1; //没有待处理的pack

    } while (pack_p->msg_header.cmd != 0);

    return result;
}

int UartMessageAsyncTick(void)
{
    MSG_PACK uart_pack;
    int ret = 0;
    if (LvpQueueGet(&s_uart_recv_pack_queue, (unsigned char *)&uart_pack) == 0) return -1; //没有待处理的pack
    if (uart_pack.msg_header.flags == 1) {
//        ret = _CheckSum((unsigned char *)uart_pack.body_addr, uart_pack.len, uart_pack.body_vef);
        ret = _CheckCrc((unsigned char *)uart_pack.body_addr, uart_pack.len, uart_pack.body_vef);
        if (ret == 0) {
            printf("Crc Error %d\n", uart_pack.len);
            return -1;
        }
    }

    for (int i = 0; i < sizeof(s_uart_msg_regist_array) / sizeof(UART_MSG_REGIST); i++) {
        if (s_uart_msg_regist_array[i].msg_id == uart_pack.msg_header.cmd && s_uart_msg_regist_array[i].port == uart_pack.port) {
            return s_uart_msg_regist_array[i].msg_pack_callback(&uart_pack, s_uart_msg_regist_array[i].priv);
        }
    }

    return -1; //没有注册回调
}

static int _UartMessagePrepareRecv(MSG_PACK* pack)
{

    for (int i = 0; i < sizeof(s_uart_msg_regist_array) / sizeof(UART_MSG_REGIST); i++) {
        if (s_uart_msg_regist_array[i].msg_id == pack->msg_header.cmd) {
            if ((pack->msg_header.length - (pack->msg_header.flags ? 4 : 0)) > s_uart_msg_regist_array[i].msg_buffer_length) break; // body校验位数据不存到msg_buffer中

            if (s_uart_msg_regist_array[i].msg_buffer_offset + pack->msg_header.length > s_uart_msg_regist_array[i].msg_buffer_length) {
                s_uart_msg_regist_array[i].msg_buffer_offset = 0;
            }

            if (s_uart_msg_regist_array[i].msg_buffer == NULL) break;
            pack->body_addr = s_uart_msg_regist_array[i].msg_buffer + s_uart_msg_regist_array[i].msg_buffer_offset;
            s_uart_msg_regist_array[i].msg_buffer_offset += pack->msg_header.length - (pack->msg_header.flags ? 4 : 0);
            return 0;
        }
    }

    pack->body_addr = NULL;
    pack->len = 0;
    return -1;
}


static int _uartRecvBufferCallback(int port, void *priv);
static int _UartMessageAsyncRecvFindMagic(unsigned int magic, unsigned int *tmp_magic, unsigned char **recv_buffer_p, unsigned int *buffer_len);
static int _UartMessageAsyncRecvCheckHeader(unsigned int port, MESSAGE_HEADER *msg_header, unsigned char **recv_buffer_p, unsigned int *buffer_en);
static int _UartMessageAsyncRecvCheckBody(unsigned int port, MSG_PACK* pack, unsigned char **recv_buffer_p, unsigned int *buffer_len);
static int _UartMessageAsyncRecvBodyVerification(unsigned int port, unsigned int *tmp_vef, MSG_PACK* pack, unsigned char **recv_buffer_p, unsigned int *buffer_len);

static unsigned char s_uart_recv_buffer[64] __attribute__((aligned(16))) = {0};

static int _UartMessageAsyncRecvCallback(int port, int length, void* priv)
{
    MESSAGE_HANDLE *msg_handle = _GetMessageHandle(port);
    if(msg_handle == NULL) return -1;

    static unsigned char *recv_buffer;
    recv_buffer = s_uart_recv_buffer;

    unsigned int read_len = gx_uart_read(port, recv_buffer, length);

    if  (read_len == 0)
        return 0;

    do {
        switch (msg_handle->recv_state) {
            case MRS_FIND_MAGIC: {
                if (_UartMessageAsyncRecvFindMagic(msg_handle->magic, \
                                                   &msg_handle->cur_recv_pack.msg_header.magic, \
                                                   &recv_buffer, &read_len) == 1) {
                    msg_handle->recv_state = MRS_CHECK_HEADER;
                }
                break;
                                 }
            case MRS_CHECK_HEADER: {
                int tmp = _UartMessageAsyncRecvCheckHeader(port, &msg_handle->cur_recv_pack.msg_header, &recv_buffer, &read_len);
                if (tmp == 1) {
                    if (msg_handle->cur_recv_pack.msg_header.length > 0) {
                        msg_handle->recv_state = MRS_CHECK_BODY;
                    } else {
                        msg_handle->recv_state = MRS_FIND_MAGIC;
                        msg_handle->cur_recv_pack.port = port;
                        msg_handle->cur_recv_pack.len = 0;
                        LvpQueuePut(&s_uart_recv_pack_queue, (unsigned char *)&msg_handle->cur_recv_pack);
                        msg_handle->cur_recv_pack.msg_header.magic = 0;
                        msg_handle->cur_recv_pack.len = 0;
                    }
                } else if (tmp == 0) {
                ;
                } else if ( tmp == -1){
                    msg_handle->recv_state = MRS_FIND_MAGIC;
                }
                break;
                                   }
            case MRS_CHECK_BODY: {
                int tmp = _UartMessageAsyncRecvCheckBody(port, &msg_handle->cur_recv_pack, &recv_buffer, &read_len);
                if (tmp == 1) {
                    msg_handle->recv_state = MRS_FIND_MAGIC;
                    msg_handle->cur_recv_pack.port = port;
                    msg_handle->cur_recv_pack.len = msg_handle->cur_recv_pack.msg_header.length - (msg_handle->cur_recv_pack.msg_header.flags ? 4 : 0);
                    LvpQueuePut(&s_uart_recv_pack_queue, (unsigned char *)&msg_handle->cur_recv_pack);
                    msg_handle->cur_recv_pack.msg_header.magic = 0;
                    msg_handle->cur_recv_pack.len = 0;
                } else if (tmp == 0) {
                ;
                } else if (tmp == -1) {
                    msg_handle->recv_state = MRS_FIND_MAGIC;
                }
                break;
                                 }

            case MRS_BODY_VEF : {
                int tmp = _UartMessageAsyncRecvBodyVerification(port, &msg_handle->tmp_vef, &msg_handle->cur_recv_pack, &recv_buffer, &read_len);
                if (tmp == 1) {
                    msg_handle->recv_state = MRS_FIND_MAGIC;
                    msg_handle->cur_recv_pack.port = port;
                    msg_handle->cur_recv_pack.len = msg_handle->cur_recv_pack.msg_header.length - (msg_handle->cur_recv_pack.msg_header.flags ? 4 : 0);
                    LvpQueuePut(&s_uart_recv_pack_queue, (unsigned char *)&msg_handle->cur_recv_pack);
                    msg_handle->cur_recv_pack.msg_header.magic = 0;
                    msg_handle->cur_recv_pack.len = 0;
                } else if (tmp == 0) {
                ;
                } else if (tmp == -1) {
                    msg_handle->recv_state = MRS_FIND_MAGIC;
                }
                break;
                                }

            default:
                read_len = 0;
                break;
        }
    } while (read_len > 0);

    return 0;
}

static int _UartMessageAsyncRecvFindMagic(unsigned int magic, unsigned int *tmp_magic, unsigned char **recv_buffer_p, unsigned int *buffer_len)
{
    unsigned char *recv_buffer = *recv_buffer_p;
    if (*tmp_magic != magic) {
        int i;
        for (i = 0; i < *buffer_len; i++) {
            *tmp_magic = (*tmp_magic >> 8) | (recv_buffer[i] << 24);
            if (*tmp_magic == magic) {
//                printf("-0x%x\n", *tmp_magic);
//                printf("-\n", *tmp_magic);
                *buffer_len -= (i + 1);
                *recv_buffer_p += (i + 1);
                return 1;
            }
        }
        *buffer_len = 0;
        *recv_buffer_p = s_uart_recv_buffer;

        return 0;
    }
    return 1;
}

static int _UartMessageAsyncRecvCheckHeader(unsigned int port, MESSAGE_HEADER *msg_header, unsigned char **recv_buffer_p, unsigned int *buffer_len)
{
    static unsigned int s_recv_header_data_count[2] = {4, 4};
    unsigned char *recv_buffer = *recv_buffer_p;
    int j = 0; // copy other recved handle data from recv_buffer to cur_send_pack.msg_handler
    for (j = 0; j < *buffer_len && ((j + s_recv_header_data_count[port]) < sizeof(MESSAGE_HEADER)); j++) {
        *(((unsigned char *)msg_header) + s_recv_header_data_count[port] + j) = recv_buffer[j];
    }
    s_recv_header_data_count[port]    += j;
    *buffer_len                 -= j;
    if (*buffer_len != 0)
        *recv_buffer_p           += j;

    if (s_recv_header_data_count[port] == sizeof(MESSAGE_HEADER)) {
        s_recv_header_data_count[port] = 4;
        if (_CheckCrc((unsigned char *)msg_header, 10, msg_header->crc32)) {
            return 1;
        } else {
            msg_header->magic = 0;
            return -1;
        }
    } else {
        return 0;
    }
}

static int _UartMessageAsyncRecvCheckBody(unsigned int port, MSG_PACK* pack, unsigned char **recv_buffer_p, unsigned int *buffer_len)
{
    MESSAGE_HANDLE *msg_handle = _GetMessageHandle(port);
    static unsigned int body_recv_count[2] = {0};
    unsigned char *recv_buffer = *recv_buffer_p;
    unsigned int body_size = 0;
    body_size = pack->msg_header.length - (pack->msg_header.flags ? 4 : 0);

    if (body_recv_count[port] == body_size) { // 结束
        body_recv_count[port] = 0;

        gx_uart_start_async_recv(msg_handle->port, _UartMessageAsyncRecvCallback, NULL);

        if (pack->msg_header.flags == 0) {
            msg_handle->recv_state = MRS_FIND_MAGIC;
            msg_handle->cur_recv_pack.port = port;
            msg_handle->cur_recv_pack.len = msg_handle->cur_recv_pack.msg_header.length - (msg_handle->cur_recv_pack.msg_header.flags ? 4 : 0);
            LvpQueuePut(&s_uart_recv_pack_queue, (unsigned char *)&msg_handle->cur_recv_pack);
            msg_handle->cur_recv_pack.msg_header.magic = 0;
            msg_handle->cur_recv_pack.len = 0;
        } else {
            msg_handle->recv_state = MRS_BODY_VEF;
        }
        return 0;
    }

    if (body_recv_count[port] == 0) { // 开始接收
        if (_UartMessagePrepareRecv(pack)) {
            return -1;
        }
    }

    if (recv_buffer_p == NULL || *recv_buffer_p == NULL || buffer_len == NULL || *buffer_len == 0) {
        gx_uart_start_async_recv(msg_handle->port, _UartMessageAsyncRecvCallback, NULL);
        return -1;
    }

    unsigned int need_copy_num = (body_size - body_recv_count[port]) > *buffer_len ? *buffer_len : (body_size - body_recv_count[port]);
    for (int i = 0; i < need_copy_num; i++) { // 拷贝s_uart_recv_buffer内的数据到pack->body_addr
        *(pack->body_addr + body_recv_count[port] + i) = *(recv_buffer + i);
    }
    body_recv_count[port] += need_copy_num;
    *buffer_len -= need_copy_num;

    if (body_recv_count[port] == body_size) { // body接收完成
        *recv_buffer_p += need_copy_num;
        body_recv_count[port] = 0;
        if (pack->msg_header.flags == 0)
            return 1;

        if (pack->msg_header.flags) {
            msg_handle->recv_state = MRS_BODY_VEF;
            return 0;
        }

        return 0;
    } else {
        unsigned char *addr = pack->body_addr + body_recv_count[port];
        unsigned int len = body_size - body_recv_count[port];

        if (len > 32 && ((unsigned int)addr) > (((unsigned int)pack->body_addr) - ((unsigned int)pack->body_addr) % 16 + 16)) {
            len -= (len % 16);

            gx_uart_stop_async_recv(port);
            gx_uart_async_recv_buffer(port, addr, len, _uartRecvBufferCallback, NULL);

            *buffer_len = 0;
            body_recv_count[port] += len;
            *recv_buffer_p = s_uart_recv_buffer;
        }
        return 0;
    }

    return -1;
}


static int _UartMessageAsyncRecvBodyVerification(unsigned int port, unsigned int *tmp_vef, MSG_PACK* pack, unsigned char **recv_buffer_p, unsigned int *buffer_len)
{
    static unsigned int vef_recv_count[2] = {0};
    unsigned char *recv_buffer = *recv_buffer_p;

    unsigned int need_copy_num = (4 - vef_recv_count[port]) > *buffer_len ? *buffer_len : (4 - vef_recv_count[port]);
    for (int i = 0; i < need_copy_num; i++) { // 拷贝s_uart_recv_buffer内的数据到tmp_vef
        *tmp_vef = (*tmp_vef >> 8) | (recv_buffer[i] << 24);
    }

    *buffer_len -= need_copy_num;
    *recv_buffer_p += need_copy_num;

    if (vef_recv_count[port] + need_copy_num == 4) {
        pack->body_vef = *tmp_vef;
        *tmp_vef = 0;
        vef_recv_count[port] = 0;
        return 1;
    } else {
        vef_recv_count[port] += need_copy_num;
        return 0;
    }
}

static int _uartRecvBufferCallback(int port, void *priv)
{
    MESSAGE_HANDLE *msg_handle = _GetMessageHandle(port);
    if(msg_handle == NULL) return -1;

    _UartMessageAsyncRecvCheckBody(port, &msg_handle->cur_recv_pack, NULL, NULL);
    return 0;
}


typedef struct {
    unsigned int port;
    UART_MSG_ID msg_id;
    MSG_PACK_CALLBACK msg_pack_callback;
    void* priv;
} UART_MSG_SEND_CALLBACK;

static UART_MSG_SEND_CALLBACK s_send_callback[UART_MSG_SEND_CALLBACK_NUM];

static int _UartMessageAsyncSendCheckCallback(MSG_PACK *pack, void *priv)
{
    MESSAGE_HANDLE *msg_handle = _GetMessageHandle(pack->port);
    if(msg_handle == NULL) return -1;

    LvpPmuSuspendUnlock(msg_handle->pmu_lock);

    for (int i = 0; i < sizeof(s_send_callback) / sizeof(UART_MSG_SEND_CALLBACK); i++) {
        if (s_send_callback[i].msg_id == pack->msg_header.cmd && s_send_callback[i].port == pack->port) {
//            printf("%x,%x\n", s_send_callback[i].msg_id, pack->msg_header.cmd);
            s_send_callback[i].msg_pack_callback(pack, s_send_callback[i].priv);
            return 0;
        }
    }

    return -1;
}

static int _UartMessageAsyncSendBody(int port, MSG_PACK* pack, unsigned int *send_len);
static int _UartMessageAsyncSendCrC(int port, MSG_PACK* pack, unsigned int *send_len);
static int _UartMessageAsyncSendHeader(int port, MESSAGE_HEADER *msg_header, unsigned int *send_len);
static int _UartMessageAsyncSendCallback(int port, int length, void *priv);
static int _UartMessageSendCheck(unsigned char port)
{
    MESSAGE_HANDLE *msg_handle = _GetMessageHandle(port);
    if(msg_handle == NULL) return -1;

    if (LvpQueueIsEmpty(&msg_handle->uart_send_pack_queue)) {
        msg_handle->send_ing = 0;
        gx_uart_stop_async_send(port);
        return -1;
    }
    msg_handle->send_ing = 1;

    LvpQueueGet(&msg_handle->uart_send_pack_queue, (unsigned char *)&msg_handle->cur_send_pack);
    msg_handle->send_state = MSS_SEND_HEADER;

    LvpPmuSuspendLock(msg_handle->pmu_lock);

    gx_uart_start_async_send(msg_handle->port, _UartMessageAsyncSendCallback, NULL);

    return 0;
}


static int _UartMessageAsyncSendCallback(int port, int length, void *priv)
{
    MESSAGE_HANDLE *msg_handle = _GetMessageHandle(port);
    if(msg_handle == NULL) return -1;

    unsigned int send_len = length;

    do {
        switch (msg_handle->send_state) {
        case MSS_SEND_HEADER: {
                                  int ret = _UartMessageAsyncSendHeader(port, &msg_handle->cur_send_pack.msg_header, &send_len);
                                  if (ret == 1){
                                      if (msg_handle->cur_send_pack.msg_header.length > 0) {
                                          msg_handle->send_state = MSS_SEND_BODY;
                                      } else {
                                          msg_handle->send_state = MSS_SEND_DONE;
                                      }
                                  }
                                  break;
                              }
        case MSS_SEND_BODY: {
                                  int ret = _UartMessageAsyncSendBody(port, &msg_handle->cur_send_pack, &send_len);
                                  if (ret == 1) {
                                      if (msg_handle->cur_send_pack.msg_header.flags) {
                                          msg_handle->send_state = MSS_SEND_BODY_CRC;
                                      } else {
                                          msg_handle->send_state = MSS_SEND_DONE;
                                      }
                                  }

                                  break;
                            }
        case MSS_SEND_BODY_CRC: {
                                    int ret = _UartMessageAsyncSendCrC(port, &msg_handle->cur_send_pack, &send_len);
                                    if (ret == 1)
                                        msg_handle->send_state = MSS_SEND_DONE;

                                    break;
                                }
        case MSS_SEND_DONE: {
                                _UartMessageAsyncSendCheckCallback(&msg_handle->cur_send_pack, NULL);
                                int ret = _UartMessageSendCheck(port);
                                if (ret == -1)
                                    return 0;

                                break;
                            }
        default:
                            break;
        }
    } while (send_len > 0);

    return 0;
}

static int _UartMessageAsyncSendHeader(int port, MESSAGE_HEADER *msg_header, unsigned int *send_len)
{
    static unsigned int send_count[2] = {0, 0};
    unsigned int s_len = *send_len < (sizeof(MESSAGE_HEADER) - send_count[port]) ? *send_len : (sizeof(MESSAGE_HEADER) - send_count[port]);

    gx_uart_write(port, ((unsigned char*)msg_header + send_count[port]), s_len);

    send_count[port] += s_len;
    send_count[port] = send_count[port] == sizeof(MESSAGE_HEADER) ? 0 : send_count[port];
    *send_len -= s_len;

    return send_count[port] == 0;
}

static unsigned int _UartAsyncSendCallback(int port, void *priv)
{
    MESSAGE_HANDLE *msg_handle = _GetMessageHandle(port);
    if(msg_handle == NULL) return -1;

    _UartMessageAsyncSendBody(port, &msg_handle->cur_send_pack, NULL);
    return 0;
}

static int _UartMessageAsyncSendBody(int port, MSG_PACK* pack, unsigned int *send_len)
{
    static unsigned int send_count[2] = {0, 0};
    static unsigned int need_send_num[2] = {0};
    if (send_count[port] == 0)
        need_send_num[port] = pack->len;

    if (send_count[port] == need_send_num[port]) {
        send_count[port] = 0;
        MESSAGE_HANDLE *msg_handle = _GetMessageHandle(port);
        if(msg_handle == NULL) return -1;

        if (msg_handle->cur_send_pack.msg_header.flags) {
            msg_handle->send_state = MSS_SEND_BODY_CRC;
        } else {
            msg_handle->send_state = MSS_SEND_DONE;
        }

        gx_uart_start_async_send(port, _UartMessageAsyncSendCallback, NULL);

        return 1;
    }

    if (pack == NULL || send_len == NULL || *send_len == 0) {
        gx_uart_start_async_send(port, _UartMessageAsyncSendCallback, NULL);
        return -1;
    }

    unsigned int s_len = *send_len < (need_send_num[port] - send_count[port]) ? *send_len : (need_send_num[port] - send_count[port]);
    gx_uart_write(port, ((unsigned char*)pack->body_addr + send_count[port]), s_len);

    send_count[port] += s_len;
    send_count[port] = send_count[port] == need_send_num[port] ? 0 : send_count[port];
    *send_len -= s_len;
    if (send_count[port] == 0)
        return 1;
    if (((unsigned int)pack->body_addr + send_count[port]) % 16 == 0 || (send_count[port] >= 16 && need_send_num[port] - send_count[port]  > 32)) {
        s_len = need_send_num[port] - send_count[port];
        s_len = s_len - (s_len % 16);

        gx_uart_stop_async_send(port);
        gx_uart_async_send_buffer(port, (unsigned char *)pack->body_addr + send_count[port], s_len,
                                  (UART_SEND_DONE_CALLBACK) _UartAsyncSendCallback, NULL);
        send_count[port] += s_len;

        *send_len = 0; // 置0，不影响串口fifo，返回后会直接退出回调
        return 0;
    } //////////// len 16 字节对齐
    return send_count[port] == 0;
}

#define MESSAGE_BODY_VEF_LEN 4
static int _UartMessageAsyncSendCrC(int port, MSG_PACK* pack, unsigned int *send_len)
{
    static unsigned int send_count[2] = {0};
    unsigned int need_send_num = MESSAGE_BODY_VEF_LEN;

    static unsigned int crc[2] = {0};

    if (send_count[port] == 0) {
        crc[port] = crc32(0, pack->body_addr, pack->len);
//        crc[port] = _Sum(0, pack->body_addr, pack->len);
    }

    unsigned int s_len = *send_len < (need_send_num - send_count[port]) ? *send_len : (need_send_num - send_count[port]);
    gx_uart_write(port, ((unsigned char*)&crc[port] + send_count[port]), s_len);

    send_count[port] += s_len;
    send_count[port] = send_count[port] == need_send_num ? 0 : send_count[port];
    *send_len -= s_len;

    return send_count[port] == 0;
}

int UartMessageAsyncSend(MSG_PACK *pack)
{
    MESSAGE_HANDLE *msg_handle = _GetMessageHandle(pack->port);
    if(msg_handle == NULL) return -1;

    if (msg_handle->init_flag == 0) return -1;

    if (pack->len && pack->body_addr == NULL) return -1;

    if (LvpQueueIsFull(&msg_handle->uart_send_pack_queue)) {
        return -1;
    }

    pack->msg_header.length = pack->len + (pack->msg_header.flags ? 4 : 0);

    if (pack->msg_header.magic == 0) pack->msg_header.magic = msg_handle->magic;

    if (!((pack->msg_header.cmd & 0xff00) == 0x200)) pack->msg_header.seq = msg_handle->send_sequence++ % 256;

    pack->msg_header.crc32 = crc32(0, (unsigned char *)(&(pack->msg_header)), 10);
    gx_dcache_clean_range((unsigned int *)&(pack->msg_header), 16);

    int ret = LvpQueuePut(&msg_handle->uart_send_pack_queue, (unsigned char *)pack);
    if (ret == 0)
        return -1;

    if (!msg_handle->send_ing) _UartMessageSendCheck(msg_handle->port);

    return 0;
}

int UartMessageAsyncSendWithCallback(MSG_PACK *pack, MSG_PACK_CALLBACK send_callback, void *priv)
{
    MESSAGE_HANDLE *msg_handle = _GetMessageHandle(pack->port);
    if(msg_handle == NULL) return -1;

    if (msg_handle->init_flag == 0) return -1;

    if (pack->len && pack->body_addr == NULL) return -1;

    if (LvpQueueIsFull(&msg_handle->uart_send_pack_queue)) return -1;

    pack->msg_header.length = pack->len + (pack->msg_header.flags ? 4 : 0);

    if (pack->msg_header.magic == 0) pack->msg_header.magic = msg_handle->magic;

    if (!((pack->msg_header.cmd & 0xff00) == 0x200)) pack->msg_header.seq = msg_handle->send_sequence++ % 256;

    pack->msg_header.crc32 = crc32(0, (unsigned char *)(&(pack->msg_header)), 10);
    gx_dcache_clean_range((unsigned int *)&(pack->msg_header), 16);

    int ret = LvpQueuePut(&msg_handle->uart_send_pack_queue, (unsigned char *)pack);
    if (ret == 0)
        return -1;

    if (!msg_handle->send_ing) _UartMessageSendCheck(msg_handle->port);

    for (int i = 0; i < sizeof(s_send_callback) / sizeof(UART_MSG_SEND_CALLBACK); i++) {
        if (s_send_callback[i].msg_id == pack->msg_header.cmd && s_send_callback[i].port == pack->port) {
            s_send_callback[i].msg_pack_callback = send_callback;
            s_send_callback[i].priv = priv;
            return 0;
        }
    }

    for (int i = 0; i < sizeof(s_send_callback) / sizeof(UART_MSG_SEND_CALLBACK); i++) {
        if (s_send_callback[i].msg_id == 0) {
            s_send_callback[i].msg_id = pack->msg_header.cmd;
            s_send_callback[i].port = pack->port;
            s_send_callback[i].msg_pack_callback = send_callback;
            s_send_callback[i].priv = priv;
            return 0;
        }
    }

    return -1;
}

int UartMessageAsyncSuspend(void *priv)
{
//    printf("UartMessageAsyncSuspend\n");

    int port_index = 0;
    while (port_index < 2) {
        MESSAGE_HANDLE *msg_handle = _GetMessageHandle(port_index);
        port_index ++;
        if(msg_handle == NULL) continue;
        gx_uart_async_send_buffer_stop(msg_handle->port);
        gx_uart_async_recv_buffer_stop(msg_handle->port);

    }
    return 0;
}

int UartMessageAsyncResume(void *priv)
{
//    printf("UartMessageAsyncResume\n");

    int port_index = 0;
    while (port_index < 2) {
        MESSAGE_HANDLE *msg_handle = _GetMessageHandle(port_index);
        port_index ++;
        if(msg_handle == NULL) continue;
        if (msg_handle->init_flag == 1) {
            gx_uart_async_send_buffer_stop(msg_handle->port);
            gx_uart_async_recv_buffer_stop(msg_handle->port);

            if (gx_uart_init(msg_handle->port, msg_handle->baudrate)) return -1;
            gx_uart_start_async_recv(msg_handle->port, _UartMessageAsyncRecvCallback, NULL);
        }
    }
    return 0;
}

int UartMessageAsyncInit(UART_MSG_INIT_CONFIG *config)
{
    if (config == NULL) return -1;

    MESSAGE_HANDLE *msg_handle = _GetMessageHandle(config->port);
    if(msg_handle == NULL) return -1;

    msg_handle->port = config->port;

    if (msg_handle->init_flag && (config->reinit_flag == 0)) return -1; // 重新初始化需要特殊标注

    if (config->magic != 0) {
        msg_handle->magic = config->magic;
    } else {
        msg_handle->magic = MSG_HOST_MAGIC;
    }

    msg_handle->baudrate = config->baudrate;

    gx_uart_async_send_buffer_stop(msg_handle->port);
    gx_uart_async_recv_buffer_stop(msg_handle->port);

    if (gx_uart_init(msg_handle->port, msg_handle->baudrate)) return -1;

    // LvpQueueInit can not call again.
    int port_index = 0;
    while (port_index < 2) {
        MESSAGE_HANDLE *msg_handle = _GetMessageHandle(port_index);
        port_index ++;
        if(msg_handle == NULL) continue;
        if (msg_handle->init_flag == 0) {
            LvpQueueInit(&s_uart_recv_pack_queue, s_uart_recv_pack_queue_buffer, UART_RECV_QUENE_LEN * sizeof(MSG_PACK), sizeof(MSG_PACK));
            break;
        }
    }

    if (msg_handle->init_flag == 0) {
        LvpQueueInit(&msg_handle->uart_send_pack_queue, msg_handle->uart_send_pack_queue_buffer, UART_SEND_QUENE_LEN * sizeof(MSG_PACK), sizeof(MSG_PACK));
    }

    // 8008`s gx_uart_async_recv_buffer need to be call again when reinit, but 8008c`s need not and can not call again.
#ifdef CONFIG_GX8008B
    if (msg_handle->init_flag == 1) return 0;
#endif
    gx_uart_start_async_recv(msg_handle->port, _UartMessageAsyncRecvCallback, NULL);

    LVP_SUSPEND_INFO suspend_info = {
        .suspend_callback = UartMessageAsyncSuspend,
        .priv = "UartMessageAsyncSuspend"
    };

    LVP_RESUME_INFO resume_info = {
        .resume_callback = UartMessageAsyncResume,
        .priv = "UartMessageAsyncResume"
    };

    LvpSuspendInfoRegist(&suspend_info);
    LvpResumeInfoRegist(&resume_info);

    LvpPmuSuspendLockCreate(&msg_handle->pmu_lock);

    msg_handle->init_flag = 1;
    return 0;
}

int UartMessageAsyncDone(void)
{
    int port_index = 0;

    while (port_index < 2) {
        MESSAGE_HANDLE *msg_handle = _GetMessageHandle(port_index);
        port_index ++;
        if(msg_handle == NULL) continue;
        gx_uart_async_send_buffer_stop(msg_handle->port);
        gx_uart_async_recv_buffer_stop(msg_handle->port);

        msg_handle->init_flag = 0;
    }

    memset(&s_uart_msg_regist_array, 0, sizeof(s_uart_msg_regist_array));
    memset(&s_uart_recv_pack_queue, 0, sizeof(s_uart_recv_pack_queue));
    memset(&s_msg_handle, 0, sizeof(s_msg_handle));
    return 0;
}


