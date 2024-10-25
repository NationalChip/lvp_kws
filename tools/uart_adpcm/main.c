#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "porting/port.h"
#include "message.h"
#include "adpcm/adpcm.h"

#define BUFF_SIZE 16000
static unsigned char recv_buf[BUFF_SIZE] = {0};
static unsigned char pcm_buf[BUFF_SIZE*4] = {0};

static int de_predict = 0;
static int de_predict_idx = 0;


FILE *fp;
int _UartGetPacket(void)
{
    MESSAGE request;

    request.body = (unsigned char *)recv_buf;

    if (MessageReceive(&request) < 0)
        return 0;
    if(request.cmd != 0xff) {
        MESSAGE send_msg;
        printf("send 0x0102\n");
        memset(&send_msg, 0, sizeof(MESSAGE));
        send_msg.cmd = 0x0102;
        send_msg.flags = 0;
        send_msg.body = NULL;
        send_msg.bodylen = 0;
        MessageSend(&send_msg);   // 发送一个接收完成的消息包
    }

    printf("request.cmd is %#x\n",request.cmd);
    if(request.cmd > 0x03A0)
    {
        printf("offline cmd %04x\n", request.cmd);
    }

    switch(request.cmd) 
    {
        case 0x030C:    // adpcm编码的起始点，数据包，两个int型数据
            printf("adpcm predict : len:%d \n",request.bodylen);
            fp =  fopen("./test.pcm","w");
            de_predict = *((int *)recv_buf);
            de_predict_idx = *((int *)(recv_buf + 4));
            AdpcmSetDePredict(de_predict);
            AdpcmSetDePredictIdx(de_predict_idx);
            // printf("de_predict:%d de_predict_idx:%d\n",de_predict, de_predict_idx);
            break;
        case 0x030D:    // adpcm buffer1
            printf("adpcm pack1 : len:%d \n",request.bodylen);
            Adpcm2Pcm((signed short*)recv_buf, (signed short*)pcm_buf, request.bodylen*2);

            break;
        case 0x030E:    // adpcm buffer2
            printf("adpcm pack2 : len:%d \n",request.bodylen);
            Adpcm2Pcm((signed short*)recv_buf, (signed short*)(pcm_buf + (sizeof(pcm_buf) - request.bodylen*4)),request.bodylen*2);

            break;

        case 0x00FF:    // adpcm end
            printf("adpcm send finish! \n");
            fwrite(pcm_buf, 1, BUFF_SIZE*4, fp);
            fflush(fp);
            fclose(fp);
            break;

        default:
            break;
    }

    return 0;
}

void *thread_function(void *arg)
{
    pthread_detach(pthread_self());

    while(1)
    {
    	_UartGetPacket();
        fflush(stdout);
    }
}

// sample.exe COM5
// sudo ./sample.elf /dev/ttyUSB0
int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("argc error!\n");
        return -1;
    }
    //INIT
    if(uart_config(argv[1],1000000, 8, 1, 'N') <= 0) {
        printf("Uart config error! \n");
        return -1;
    }
    sys_init(); 

  
    printf("8002 UART!\n");
    pthread_t a_thread;
    pthread_create(&a_thread,NULL,thread_function,NULL);
    printf("please input ""2"": Get 2S ADPCM.\n");

    while(1)
    {
        char input = 0;
START:
        input = getchar();
        if(input == '\n')
            goto START;

        switch(input)
        {
            case '2':
            {
                // sleep(2);
                printf("send 0x170\n");
                MESSAGE send_msg;
                memset(&send_msg, 0, sizeof(MESSAGE));
                send_msg.cmd = 0x0170;
                send_msg.flags = 0;
                send_msg.body = NULL;
                send_msg.bodylen = 0;
                MessageSend(&send_msg);

                break;
            }
            default:
                break;
        }
    }
    return 0;
}
