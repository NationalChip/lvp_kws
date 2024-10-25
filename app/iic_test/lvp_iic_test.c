/* Voice Signal Preprocess
* Copyright (C) 2001-2020 NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
* lvp_app_sample.c
*
*/

#include <lvp_app.h>
#include <lvp_buffer.h>
#include <driver/gx_i2c.h>
#include <driver/gx_delay.h>
#define LOG_TAG "[IIC_TEST_APP]"

//=================================================================================================

#ifdef CONFIG_LVP_APP_IIC_TEST_MASTER
int ret = 0;
unsigned char bus = 0, slv_addr = 0x33;
unsigned char buf = 0;
unsigned short len = sizeof(buf);
unsigned char data_slave = 0;
int iicmaster_test(void)
{

    buf++;
    /* 使用i2c0向设备地址为 */
    ret = gx_i2c_write(bus, slv_addr, &buf, len);
    if (ret != 0)
    {
        printf("error\n");
        return -1;
    }
    /* 使用i2c0读取设备地址为0xxx的slave */
    ret = gx_i2c_read(bus, slv_addr, &data_slave, len);
    if (ret != 0)
    {
        printf("error\n");
        return -1;
    }

    printf("data_slave=%d  ",data_slave);

    printf("\n");
    return 0;
}
#else

int data_slave = 100;
int data_master = 0;
static int i2c_slave_cb(GX_I2C_SLAVE_EVENT event, char *val, void *private)
{


    switch (event) {
        /* 检测到stop信号 */
        case I2C_SLAVE_STOP:
            // do_someting();
            break;
        /* 检测到读请求 */
        case I2C_SLAVE_REQUESTED_DATA:
            /* 返回给master 0x33 */
            *val = data_slave;
            break;
        /* 检测到写请求 */
        case I2C_SLAVE_RECEIVE_DATA:
            /* 获取master发送的数据 */
            data_master = *val;
            break;
        default:
            break;
    }
    return 0;
}

int i2c_slave_example(void)
{
    unsigned char bus = 0, slv_addr = 0x33;

    /* 初始化i2c0为设备地址0x33的slave */
    gx_i2c_slave_open(bus, slv_addr, i2c_slave_cb, NULL);
    return 0;
}
#endif

static int SampleAppSuspend(void *priv){return 0;}

static int SampleAppResume(void *priv){return 0;}

static int SampleAppInit(void)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);
#ifndef CONFIG_LVP_APP_IIC_TEST_MASTER
    i2c_slave_example();
#endif
    return 0;
}


// App Event Process
static int SampleAppEventResponse(APP_EVENT *app_event)
{

    if (app_event->event_id < 100)
        return 0;
}

// APP Main Loop
static int SampleAppTaskLoop(void)
{
#ifdef CONFIG_LVP_APP_IIC_TEST_MASTER
    iicmaster_test();
    // gx_mdelay(2000);
#else
    data_slave++;
    printf("data_master = %d\n",data_master);
#endif
gx_mdelay(2000);
    return 0;
}


LVP_APP sample_app = {
    .app_name = "sample app",
    .AppInit = SampleAppInit,
    .AppEventResponse = SampleAppEventResponse,
    .AppTaskLoop = SampleAppTaskLoop,
    .AppSuspend = SampleAppSuspend,
    .suspend_priv = "SampleAppSuspend",
    .AppResume = SampleAppResume,
    .resume_priv = "SampleAppResume",
};

LVP_REGISTER_APP(sample_app);

