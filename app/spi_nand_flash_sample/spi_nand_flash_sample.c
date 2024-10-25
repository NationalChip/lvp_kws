/* Voice Signal Preprocess
* Copyright (C) 2001-2024 NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
* spi_nand_flash_SpiNandFlash.c
*
*/

#include <stdio.h>
#include <string.h>
#include <types.h>
#include <board_config.h>

#include <lvp_app.h>
#include <lvp_buffer.h>

#include <driver/gx_flash.h>

#define LOG_TAG "[SPI_NAND_FLASH_APP]"


GX_FLASH_DEV *flash = NULL;
uint8_t test_data[4096] = {0};

//=================================================================================================

static int SpiNandFlashAppSuspend(void *priv)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);
    printf("%s\n", (unsigned char *)priv);

    return 0;
}

static int SpiNandFlashAppResume(void *priv)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);
    printf("%s\n", (unsigned char *)priv);

    return 0;
}

static int SpiNandFlashAppInit(void)
{
    extern GX_FLASH_DEV spi_nand_flash_dev;
    flash = spi_nand_flash_dev.init(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS, CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);

    if (flash == NULL) {
        printf("spi_nand_flash_dev init fail !!!\n", __LINE__);

        return -1;
    }

    printf("GX_FLASH_CHIP_TYPE:     %#x\n", gx_spi_flash_getinfo(flash, GX_FLASH_CHIP_TYPE));
    printf("GX_FLASH_CHIP_SIZE:     %#x\n", gx_spi_flash_getinfo(flash, GX_FLASH_CHIP_SIZE));
    printf("GX_FLASH_BLOCK_SIZE:    %#x\n", gx_spi_flash_getinfo(flash, GX_FLASH_BLOCK_SIZE));
    printf("GX_FLASH_BLOCK_NUM:     %#x\n", gx_spi_flash_getinfo(flash, GX_FLASH_BLOCK_NUM));
    printf("GX_FLASH_ERASE_SIZE:    %#x\n", gx_spi_flash_getinfo(flash, GX_FLASH_ERASE_SIZE));
    printf("GX_FLASH_ERASE_NUM:     %#x\n", gx_spi_flash_getinfo(flash, GX_FLASH_ERASE_NUM));
    printf("GX_FLASH_PAGE_SIZE:     %#x\n", gx_spi_flash_getinfo(flash, GX_FLASH_PAGE_SIZE));
    printf("GX_FLASH_PAGE_NUM:      %#x\n", gx_spi_flash_getinfo(flash, GX_FLASH_PAGE_NUM));

    printf(LOG_TAG" ---- %s OK ----\n", __func__);

    return 0;
}

// App Event Process
static int SpiNandFlashAppEventResponse(APP_EVENT *app_event)
{
    if (app_event->event_id < 100)
        return 0;

    return 0;
}

// APP Main Loop
static int SpiNandFlashAppTaskLoop(void)
{
    static int test_flag = 0;
    int ret = 0;

    if (test_flag == 0) {
        test_flag = 1;

        printf(LOG_TAG"Test start\n");

        // 准备测试数据
        for (int i = 0; i < sizeof(test_data); i++) {
            test_data[i] = i % 256;
        }

        // 注： flash 操作地址从 0 开始， 应不超过最大 flash 大小。
        //      写、读：操作地址 应该按照 page(2K) 大小对齐， 长度也应该为 page 的整数倍
        //      擦：操作地址 应该按照 block(128K) 大小对齐， 擦除大小也应该为 block 的整数倍
        //      对于坏块，在调用 读，写，擦 等接口时是不需要关心的，驱动内部会进行相应处理。

        // 读、写的操作地址应该确保 page(2K) 对齐
        unsigned int test_flash_addr = 0x400000 - 0x1000;

        // 1. 打印当前 flash 的坏块信息，若有坏块信息，会打印出来
        gx_spi_flash_badinfo(flash);

        // 2. 判断某地址处是否是坏块， 返回 1: 是坏块，0: 不是坏块
        ret = gx_spi_flash_block_isbad(flash, test_flash_addr);
        printf(LOG_TAG"addr: %#x isbad? %s\n", test_flash_addr, ret == 1 ? "yes" : "no");

        // 3. 擦除, 地址必须 block(128K) 对齐，擦除大小应为 block(128K) 的倍数，遇到坏块会跳过当前坏块，继续擦除下一块，直到满足擦除长度
        // [注意：擦除长度不满足block倍数大小并不会报错，其会按照block倍数向上取整大小擦除]
        int block_size = gx_spi_flash_getinfo(flash, GX_FLASH_BLOCK_SIZE);
        int block_num = gx_spi_flash_getinfo(flash, GX_FLASH_BLOCK_NUM);
        ret = gx_spi_flash_erasedata(flash, 0x0, block_size * block_num);   // 全部擦除
        if (ret < 0)
            printf(LOG_TAG"erasedata error %d\n",  ret);
        else
            printf(LOG_TAG"erasedata success\n");

        // 4. 写入, 地址必须 page(2K) 对齐，长度为 page(2K) 倍数，遇到坏块会跳过当前坏块，向下一块写数据，直到满足写入长度
        // [注意：写入长度不满足 page 倍数大小并不会报错，写入仅写入指定长度大小， 但这个 page 后面未写入的位置将无法写入了，只能按 block 擦除后再进行写入]
        ret = gx_spi_flash_pageprogram(flash, test_flash_addr, test_data, sizeof(test_data));
        if (ret < 0)
            printf(LOG_TAG"write data error %d\n",  ret);
        else if (ret >= sizeof(test_data))
            printf(LOG_TAG"write data success, len: %d\n",  ret);

        // 将读取要写入的位置清空
        memset(test_data, 0, sizeof(test_data));
        for (int i = 0; i < sizeof(test_data); i++) {
            if (test_data[i] != 0)
                printf(LOG_TAG"error index [%d] is not 0: %d", i, test_data[i]);
        }

        // 5. 读取, 地址必须 page(2K) 对齐, 长度 为 page(2K) 倍数，遇到坏块会跳过当前坏块，向下一块读数据，直到满足读取长度
        // [注意：读取长度不满足 page 倍数大小并不会报错，读取仅读取指定长度大小]
        ret = gx_spi_flash_readdata(flash, test_flash_addr, test_data, sizeof(test_data));
        if (ret < 0)
            printf(LOG_TAG"readdata error %d\n",  ret);
        else if (ret >= sizeof(test_data))
            printf(LOG_TAG"readdata success, len: %d\n",  ret);

        // 对比读取的数据与写入的数据是否一致
        for (int i = 0; i < sizeof(test_data); i++) {
            if (test_data[i] != (i % 256))
                printf(LOG_TAG"error index[%d] read(%d) != write(%d)\n", i, test_data[i], i % 256);
        }
        printf(LOG_TAG"Test Successful\n");
    }

    return 0;
}


LVP_APP spi_nand_flash_app = {
    .app_name = "Spi Nand Flash app",
    .AppInit = SpiNandFlashAppInit,
    .AppEventResponse = SpiNandFlashAppEventResponse,
    .AppTaskLoop = SpiNandFlashAppTaskLoop,
    .AppSuspend = SpiNandFlashAppSuspend,
    .suspend_priv = "SpiNandFlashAppSuspend",
    .AppResume = SpiNandFlashAppResume,
    .resume_priv = "SpiNandFlashAppResume",
};

LVP_REGISTER_APP(spi_nand_flash_app);

