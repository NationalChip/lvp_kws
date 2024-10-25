/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_app_sample.c
 *
 */
#include <stdio.h>
#include <lvp_app.h>
#include <lvp_buffer.h>
#include "types.h"
#define LOG_TAG "[FLASH_DEMO]"

#ifdef CONFIG_LVP_APP_FLASH_TEST_DETAIL_LOG
#define DETAIL_LOG(...)                printf( __VA_ARGS__ )
#else
#define DETAIL_LOG(...)
#endif

#ifdef CONFIG_LVP_APP_FLASH_TEST_LOOP
#define _LOOP (1)
#else
#define _LOOP (0)
#endif

static int app_init = 0;

static unsigned char s_test_buf[CONFIG_LVP_APP_FLASH_TEST_SIZE] = {0};
//=================================================================================================

#include <driver/gx_flash_common.h>
#include <driver/gx_flash.h>
#include <driver/gx_snpu.h>

#define TEST_LEN 2048

#ifdef CONFIG_MCU_DEFAULT_TEXT_IN_FLASH
DRAM0_STAGE2_SRAM_ATTR int flash_example(void)
#else
int flash_example(void)
#endif
{
#if 1
    // Flash 功能初始化 该接口只需要调用一次
    gx_spinor_flash_init();

    printf(LOG_TAG"Init\n");
    unsigned int flash_id = gx_spinor_flash_getinfo(GX_FLASH_CHIP_ID);
    if (flash_id == 0x00854012 || flash_id == 0x00856013) {
        printf(LOG_TAG"Vendor:  [%s]\n", "PUYA");
    }
    if (flash_id == 0x001c3812 || flash_id == 0x001c3813) {
        printf(LOG_TAG"Vendor:  [%s]\n", "ESMT");
    }
    printf(LOG_TAG"Type:        [%s]\n",      gx_spinor_flash_gettype());
    printf(LOG_TAG"ID:          [%#x]\n",     flash_id);
    int flash_size = gx_spinor_flash_getinfo(GX_FLASH_CHIP_SIZE);
    printf(LOG_TAG"Size:        [%d Bytes]\n", flash_size);

    int erase_block_size = gx_spinor_flash_getinfo(GX_FLASH_ERASE_SIZE);
    printf(LOG_TAG"Erase block: [%d Bytes]\n", erase_block_size);

    // 获取 Flash 擦除块大小,擦除的地址必须是擦除块对齐的，擦除大小必须是擦除块的整数倍
    int test_size = CONFIG_LVP_APP_FLASH_TEST_SIZE;
    int flash_addr = (CONFIG_LVP_APP_FLASH_TEST_OFFSET + flash_size) % flash_size;
    int erase_addr = flash_addr - (flash_addr % erase_block_size);
    int erase_end  = ((flash_addr + test_size - 1) / erase_block_size + 1) * erase_block_size;
    int erase_size = erase_end - erase_addr;

    printf(LOG_TAG"Test addr:   [%d]\n", flash_addr);
    printf(LOG_TAG"Erase size:  [%d]\n", erase_size);

    printf(LOG_TAG"Prepare Test Data ...\n");
    for (int i = 0; i < sizeof(s_test_buf); ++i) {
        s_test_buf[i] = i;
    }

    for (int i = 0; i < sizeof(s_test_buf); ++i) {
        if (i % 16 == 0 && i != 0)
            DETAIL_LOG("\n");
        DETAIL_LOG("%d ", s_test_buf[i]);
    }
    DETAIL_LOG("\n");
    DETAIL_LOG("\n");
    printf(LOG_TAG"Prepare Test Data Done!\n");

    int ret = 0;

    static unsigned int test_count = 0;
    while (_LOOP) {
        printf("\n"LOG_TAG"Test count: %d\n", ++test_count);
        printf("\n"LOG_TAG"Test size : %d\n", test_size);

#ifdef CONFIG_LVP_APP_FLASH_TEST_LOCK_IRQ
        // 关闭中断
        unsigned int irq_state = gx_lock_irq_save();
#endif

#ifdef CONFIG_NPU_RUN_IN_FLASH
        printf(LOG_TAG"Wait Npu Idle..\n");
        while (gx_snpu_get_state() == GX_SNPU_BUSY);
        printf(LOG_TAG"Npu Idle\n");
#endif
        // 指定地址和长度读取 Flash
        printf(LOG_TAG"Read 1 Start ...\n");
        ret = gx_spinor_flash_readdata(flash_addr, s_test_buf, test_size);

        for (int i = 0; i < sizeof(s_test_buf); ++i) {
            if (i % 16 == 0 && i != 0)
                DETAIL_LOG("\n");
            DETAIL_LOG("%d ", s_test_buf[i]);
        }
        DETAIL_LOG("\n");
        DETAIL_LOG("\n");
        printf(LOG_TAG"Read 1 Done <%d>\n", ret);

#ifdef CONFIG_LVP_APP_FLASH_TEST_ERASE
        // 指定地址和长度擦除 Flash
        printf(LOG_TAG"Erase Start ...\n");
        ret = gx_spinor_flash_erasedata(flash_addr, erase_size);
        printf(LOG_TAG"Erase Done! <%d>\n", ret);
#endif

#ifdef CONFIG_LVP_APP_FLASH_TEST_WRITE
        // 指定地址和长度写 Flash
        printf(LOG_TAG"Write Start ...\n");
        ret = gx_spinor_flash_pageprogram(flash_addr, s_test_buf, test_size);
        printf(LOG_TAG"Write Done! <%d>\n", ret);
#endif

#ifdef CONFIG_LVP_APP_FLASH_TEST_READ
        // 指定地址和长度读取 Flash
        printf(LOG_TAG"Read 2 Start ...\n");
        ret = gx_spinor_flash_readdata(flash_addr, s_test_buf, test_size);

        for (int i = 0; i < sizeof(s_test_buf); ++i) {
            if (i % 16 == 0 && i != 0)
                DETAIL_LOG("\n");
            DETAIL_LOG("%d ", s_test_buf[i]);
        }
        DETAIL_LOG("\n");
        DETAIL_LOG("\n");
        printf(LOG_TAG"Read 2 Done <%d>\n", ret);
#endif

#ifdef CONFIG_LVP_APP_FLASH_TEST_LOCK_IRQ
        // 恢复中断
        gx_unlock_irq_restore(irq_state);
#endif
    }
    return 0;

#else

    GX_FLASH_DEV *dev;
    int flash_size;
    int erase_size;
    unsigned char s_test_buf[TEST_LEN];

    int zzz = 20;

    for (int i = 0; i < sizeof(s_test_buf); ++i) {
        s_test_buf[i] = zzz + i;
    }

    // Flash 功能初始化
    // 该接口只需要调用一次
    dev = gx_spi_flash_probe(0,0,0,0);

    // 获取 Flash 容量大小
    flash_size = gx_spi_flash_getinfo(dev, GX_FLASH_CHIP_SIZE);
    int flash_addr = flash_size - 49152;

    printf("LINE:%d\n", __LINE__);
    for (int i = 0; i < sizeof(s_test_buf); ++i) {
        if (i % 16 == 0 && i != 0)
            printf("\n");
        printf("%d ", s_test_buf[i]);
    }
    printf("\n");
    printf("\n");

    // 获取 Flash 擦除块大小,擦除的地址必须是擦除块对齐的，擦除大小必须是擦除块的整数倍
    erase_size = gx_spi_flash_getinfo(dev, GX_FLASH_ERASE_SIZE);

    printf("LINE:%d\n", __LINE__);
    for (int i = 0; i < sizeof(s_test_buf); ++i) {
        if (i % 16 == 0 && i != 0)
            printf("\n");
        printf("%d ", s_test_buf[i]);
    }
    printf("\n");
    printf("\n");

    // 指定地址和长度擦除 Flash
    gx_spi_flash_erasedata(dev, flash_addr, erase_size);

    printf("LINE:%d\n", __LINE__);
    for (int i = 0; i < sizeof(s_test_buf); ++i) {
        if (i % 16 == 0 && i != 0)
            printf("\n");
        printf("%d ", s_test_buf[i]);
    }
    printf("\n");
    printf("\n");

    // 指定地址和长度写 Flash
    gx_spi_flash_pageprogram(dev, flash_addr, s_test_buf, TEST_LEN);

    // 指定地址和长度读取 Flash
    gx_spi_flash_readdata(dev, flash_addr, s_test_buf, TEST_LEN);

    printf("LINE:%d\n", __LINE__);
    for (int i = 0; i < sizeof(s_test_buf); ++i) {
        if (i % 16 == 0 && i != 0)
            printf("\n");
        printf("%d ", s_test_buf[i]);
    }
    printf("\n");
    printf("\n");

    return 0;
#endif
}

static int SampleAppInit(void)
{
    if (!app_init) {
        app_init = 1;
        printf(LOG_TAG" ---- %s ----\n", __func__);

        //        while(1);


        flash_example();
    }
    return 0;
}

// App Event Process
static int SampleAppEventResponse(APP_EVENT *app_event)
{
    return 0;
}

// APP Main Loop
static int SampleAppTaskLoop(void)
{
    return 0;
}


static int SampleSuspend(void *priv)
{
    return 0;
}

static int SampleResume(void *priv)
{
    return 0;
}
LVP_APP sample_app = {
    .app_name = "FlashDemo",
    .AppInit = SampleAppInit,
    .AppEventResponse = SampleAppEventResponse,
    .AppTaskLoop = SampleAppTaskLoop,
    .AppSuspend = SampleSuspend,
    .suspend_priv = "FlashDemo",
    .AppResume = SampleResume,
    .resume_priv = "FlashDemo",
};

LVP_REGISTER_APP(sample_app);
