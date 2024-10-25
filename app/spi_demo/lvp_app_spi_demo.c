/* Voice Signal Preprocess
* Copyright (C) 2001-2022 NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
* lvp_app_spi_demo.c
*
*/

#include <lvp_app.h>
#include <lvp_buffer.h>

#include <common.h>
// #include <command.h>
#include <driver/spi.h>
#include <driver/device.h>
#include <driver/gx_spi_slave.h>
#include <driver/gx_delay.h>


#define LOG_TAG "[SPI DEMO]"

//=================================================================================================
#define SPI_TEST_BUF_LEN     1024*64
static uint8_t spi_test_buf[SPI_TEST_BUF_LEN+16] __attribute__((aligned(16))) = {0};

static struct spi_device g_spi_slave_test={
            .master        = NULL,
            .chip_select   = 0,
            .max_speed_hz  = 3*1000*1000,
            .mode          = 0,
            .bits_per_word = 8,
        //    .data_format = 0,
};
static uint8_t g_spi_test_slave_init_flag = 0;
static volatile uint8_t g_spi_test_slave_complete_flag = 0;


static int spi_slave_test_init(struct spi_device *spi_slave)
{
    int iRet = 0;
#ifdef CONFIG_ARCH_AQUILA_AUDIO
    gx_clk_mod_set_gate(GX_CLK_MOD_SPI, 1);
#endif

    if((iRet = spi_slave_init(spi_slave)) != 0)
    {
        printf("spi_flash_register_master error:%d\r\n",iRet);
    }

    return iRet;
}

void spi_slave_test_trans_cb(struct spi_device *spi)
{
    g_spi_test_slave_complete_flag = 1;
}


void print_buffer(int start, char *buffer, int flag, int len, int align)
{
    static unsigned long counts = 0;
    counts++;
    printf("count: %d  xxxxxx\n",counts);
    for (int i = 0; i < len/align; i++)
    {
        for (int j = 0; j < align; j++)
        {
            printf("%2x ",buffer[(i+1)*j]);
        }
        printf("\n");
    }
    
}

static int spi_slave_test_loopback(uint32_t len)
{
    int ret = 0;
    uint64_t count = 0;

    while(1)
    {
        count++;
        printf("test %d times!  ", count);
        memset(spi_test_buf, 0, len);
        g_spi_test_slave_complete_flag = 0;
        if((ret = spi_slave_rx(&g_spi_slave_test, spi_test_buf, len, spi_slave_test_trans_cb)) != 0)
        {
            printf("spi_read faild! %d\r\n", ret);
            return -2;
        }
        while(g_spi_test_slave_complete_flag == 0);
        printf("RECV:\n");
        print_buffer(0,spi_test_buf, 1, len, 16);

        g_spi_test_slave_complete_flag = 0;
        memset(spi_test_buf, 'f', 1024);
        if((ret = spi_slave_tx(&g_spi_slave_test, spi_test_buf, len, spi_slave_test_trans_cb)) != 0)
        {
            printf("spi_write faild! %d\r\n", ret);
            return -1;
        }
        while(g_spi_test_slave_complete_flag == 0);
        printf("SEND:\n");
        print_buffer(0, spi_test_buf, 1, 1024, 16);
    }
    return 0;
}



static int SpiDemoAppSuspend(void *priv){return 0;}

static int SpiDemoAppResume(void *priv){return 0;}

static int SpiDemoAppInit(void)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);
    if (g_spi_test_slave_init_flag == 0) {
        if (spi_slave_test_init(&g_spi_slave_test) == 0) {
            g_spi_test_slave_init_flag = 1;
        }
    }
    spi_slave_test_loopback(1024);
    return 0;
}

// App Event Process
static int SpiDemoAppEventResponse(APP_EVENT *app_event)
{
    LVP_CONTEXT *context;
    unsigned int ctx_size;
    LvpGetContext(app_event->ctx_index, &context, &ctx_size);


    return 0;
}


// APP Main Loop
static int SpiDemoAppTaskLoop(void)
{

    return 0;
}


LVP_APP spi_demo_app = {
    .app_name = "SPI Demo app",
    .AppInit = SpiDemoAppInit,
    .AppEventResponse = SpiDemoAppEventResponse,
    .AppTaskLoop = SpiDemoAppTaskLoop,
    .AppSuspend = SpiDemoAppSuspend,
    .suspend_priv = "SpiDemoAppSuspend",
    .AppResume = SpiDemoAppResume,
    .resume_priv = "SpiDemoAppResume",
};

LVP_REGISTER_APP(spi_demo_app);

