/* Voice Signal Preprocess
* Copyright (C) 2001-2022 NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
*
*/

#include <lvp_app.h>
#include <lvp_buffer.h>
#include <adpcm.h>
#include <driver/gx_dcache.h>
#include <driver/gx_gpio.h>
#include "uart_message_v2.h"
#include <driver/gx_delay.h>




#define LOG_TAG "[BYTEDANCE_ROBIN_APP]"

# define MESSAGE_UART 0
#define SIMPLE_NUM 55   // 40ms * 50 = 2s   5个缓冲context
unsigned char out_buff[SIMPLE_NUM * 320] = {0};
int predict_buffer[2 * SIMPLE_NUM]={0};
static char outbuff_offset = 0;

static unsigned char send_flag = 0;
static unsigned char send_done_flag = 1;


typedef struct encode_start_parameter
{
    int de_predict;
    int de_predict_idx;
}EnCodeSP;


//=================================================================================================

static int BytedanceRobinAppSuspend(void *priv)
{
    AdpcmClearDecode();
    return 0;
}

static int BytedanceRobinAppResume(void *priv)
{
    AdpcmClearEncode();
    return 0;
}

int SendDoneCallback(MSG_PACK * pack, void *priv)
{
    int cmd = (int)priv;
    printf("[%s]:cmd = %d\n",__func__, cmd);
    if(cmd == 0x030E)
        send_done_flag = 1;

    return 0;
}


static int _UartAdpcmSend(unsigned char *send_buffer, unsigned int send_len, unsigned int cmd)
{
    MSG_PACK send_pack_data = {
        .port = 0,
        .msg_header.magic = MSG_SLAVE_MAGIC,
        .msg_header.cmd = cmd,
        .msg_header.flags = 1,
        .body_addr = (unsigned char*)send_buffer,
        .len = send_len
    };
    // for (int i = 0; i < send_pack_data.len; i ++)
    //     printf("0x%02x \n", send_pack_data.body_addr[i]);

    UartMessageAsyncSendWithCallback(&send_pack_data, SendDoneCallback, (void *)cmd);
    return 0;
}
static int write_offset = 0;
static int send_offset = 0;
static int _uartRecvCallback(MSG_PACK * pack, void *priv)
{
    printf("%s\n", priv);

    if(pack->msg_header.cmd == 0x0170) {
        write_offset = outbuff_offset;
        send_offset = (write_offset + 5) % 55;
        send_flag = 1;
        send_done_flag = 0;
        _UartAdpcmSend((unsigned char *)(predict_buffer + send_offset * 2), 8, 0x030C);
        // printf("send predict:%d %d\n", *(predict_buffer + buffer_offset * 2), *(predict_buffer + buffer_offset * 2 + 1));

    }

    if(pack->msg_header.cmd == 0x0102) {
        if(send_flag == 1){
            send_flag = 2;
            if(write_offset >= 50){
                _UartAdpcmSend(out_buff + send_offset * 320, 16000, 0x030D);
            }
            else{
                _UartAdpcmSend(out_buff + send_offset * 320, sizeof(out_buff) - send_offset * 320, 0x030D);
            }
        }
        else if(send_flag == 2) {
            send_flag = 3;
            if(write_offset >= 50){
                _UartAdpcmSend(out_buff, 0, 0x030E);
            }
            else{
                _UartAdpcmSend(out_buff, write_offset * 320, 0x030E);
            }
        }
        else if (send_flag == 3)
        {
            send_flag = 0;
            _UartAdpcmSend(NULL, 0, 0x00FF);
        }
    }
    return 0;
}

static void _uartRecvInit(void)
{
    UART_MSG_REGIST regist_info_1 = {
        .msg_id = 0x0102,
        .port = MESSAGE_UART,
        .msg_buffer = NULL,
        .msg_buffer_length = 0,
        .msg_pack_callback = _uartRecvCallback,
        .priv = "recv done"
    };

    UART_MSG_REGIST regist_info_2 = {
        .msg_id = 0x0170,
        .port = MESSAGE_UART,
        .msg_buffer = NULL,
        .msg_buffer_length = 0,
        .msg_pack_callback = _uartRecvCallback,
        .priv = "get adpcm"

    };

    UartMessageAsyncRegist(&regist_info_1);
    UartMessageAsyncRegist(&regist_info_2);

}



static void _uartMsgInit(void)
{
    printf("[%s]%d\n", __func__, __LINE__);

    UartMessageAsyncDone();

    UART_MSG_INIT_CONFIG init_config = {
        .magic = MSG_SLAVE_MAGIC,
        .port = MESSAGE_UART,
        .baudrate = 1000000,
        .reinit_flag = 1
    };
    UartMessageAsyncInit(&init_config);
    _uartRecvInit();

}

static int BytedanceRobinAppInit(void)
{
    printf(LOG_TAG"func:%s   line:%d\n", __func__, __LINE__);
    AdpcmClearEncode();
    AdpcmClearDecode();
    _uartMsgInit();

    return 0;
}

// App Event Process
static int BytedanceRobinAppEventResponse(APP_EVENT *app_event)
{

    if (!send_done_flag)
        return 0;
    LVP_CONTEXT *context;
    unsigned int ctx_size;

    LvpGetContext(app_event->ctx_index, &context, &ctx_size);
    LVP_CONTEXT_HEADER *ctx_header = context->ctx_header;
    unsigned int  mic_buffer_len_per_context = ctx_header->pcm_frame_num_per_context * \
                                               ctx_header->frame_length * \
                                               ctx_header->sample_rate * 2 / 1000;
     unsigned char *cur_mic_buffer = (unsigned char *)ctx_header->mic_buffer + \
                                    mic_buffer_len_per_context * \
                                    (context->ctx_index % \
                                    (ctx_header->mic_buffer_size / ctx_header->mic_num / mic_buffer_len_per_context));

    gx_dcache_invalid_range((unsigned int*)cur_mic_buffer, mic_buffer_len_per_context);
    signed short *in = (signed short *)cur_mic_buffer;
    int len = mic_buffer_len_per_context / sizeof(short); //采样点数
    // printf(LOG_TAG"len: %d\n",len/2);
    outbuff_offset = (app_event->ctx_index % 55);

    EnCodeSP *curr_sp = (EnCodeSP*)(predict_buffer + outbuff_offset * 2);
    curr_sp->de_predict = AdpcmGetEnPredict();
    curr_sp->de_predict_idx = AdpcmGetEnpredictIdx();
    // printf("ctx: %d en_predict: %d, en_predict_idx: %d \n",app_event->ctx_index,curr_sp->de_predict, curr_sp->de_predict_idx);


    Pcm2Adpcm(in, (signed short *)(out_buff + outbuff_offset * 320), len);
    gx_dcache_clean_range((unsigned int*)(out_buff + outbuff_offset * 320), len / 2);



    return 0;
}

// APP Main Loop
static int BytedanceRobinAppTaskLoop(void)
{
    return 0;
}


LVP_APP bytedance_robin_app = {
    .app_name = "bytedance robin app",
    .AppInit = BytedanceRobinAppInit,
    .AppEventResponse = BytedanceRobinAppEventResponse,
    .AppTaskLoop = BytedanceRobinAppTaskLoop,
    .AppSuspend = BytedanceRobinAppSuspend,
    .suspend_priv = "BytedanceRobinAppSuspend",
    .AppResume = BytedanceRobinAppResume,
    .resume_priv = "BytedanceRobinAppResume",
};

LVP_REGISTER_APP(bytedance_robin_app);

