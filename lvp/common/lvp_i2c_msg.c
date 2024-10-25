/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_i2c_msg.c:
 *
 */

#include <stdio.h>
#include <driver/gx_hw_i2c.h>
#include <lvp_pmu.h>
#include "lvp_i2c_msg.h"
#include <common.h>
#include <string.h>
#include <stdlib.h>

#define VOICE_EVEVNT_REG    0xa0
#define ISR_CONFIRM_EVENT    0x10

#define I2C_LINK_REG    0xac
#define ISR_TEST_LINK    0x80
#define ISR_CONFIRM_LINK    0x11
#define ISR_GET_SOFT_VERSTION 0x68       // 获取Soft Verstion

static char i2c_msg_initialized = 0;
static I2C_EVENT_CONFIRM_CB event_confirm_cb;
static int pmu_lock = 0;

typedef struct
{
    unsigned char BuildVerstion;
    unsigned char CurrectVerstion;
    unsigned char SecondVerstion;
    unsigned char MainVerstion;
}SOFT_VERSTION;

static SOFT_VERSTION sv;

static void LvpSoftVerstion(void)
{
    char str[] = CONFIG_SOFT_VERSTION;
    char *buff = NULL;

    buff = strtok(str,".");
    sv.MainVerstion = ((unsigned char)*buff - '0');

    buff = strtok(NULL,".");
    sv.SecondVerstion = ((unsigned char)*buff - '0');

    buff = strtok(NULL,".");
    sv.CurrectVerstion = ((unsigned char)*buff - '0');

    buff = strtok(NULL,".");
    sv.BuildVerstion = ((unsigned char)*buff - '0');

    gx_hw_i2c_write_reg(0xa0, sv.MainVerstion);
    gx_hw_i2c_write_reg(0xa4, sv.SecondVerstion);
    gx_hw_i2c_write_reg(0xa8, sv.CurrectVerstion);
    gx_hw_i2c_write_reg(0xac, sv.BuildVerstion);
}

static int _lvpI2CMsgISR(void *private, unsigned char status)
{
    if(!i2c_msg_initialized)
        return -1;

    // printf("======[%s]%d i2c irq: 0x%02x ======\n", __func__, __LINE__, status);

    switch(status)
    {
        case ISR_CONFIRM_EVENT:
            // clear event
            gx_hw_i2c_write_reg(VOICE_EVEVNT_REG, 0);
            if(event_confirm_cb)
            {
                event_confirm_cb();
                event_confirm_cb = NULL;
            }
            break;

        case ISR_TEST_LINK:
            LvpPmuSuspendLock(pmu_lock);
            gx_hw_i2c_write_reg(I2C_LINK_REG, 1);
            break;

        case ISR_CONFIRM_LINK:
            gx_hw_i2c_write_reg(I2C_LINK_REG, 0);
            LvpPmuSuspendUnlock(pmu_lock);
            break;
        case ISR_GET_SOFT_VERSTION:
            LvpSoftVerstion();
            break;
        default:
            break;

    }

    return 0;
}

int LvpI2CMsgInit(void)
{
    if(i2c_msg_initialized == 1)
        return 0;

    gx_hw_i2c_enter_config_mode();

    gx_hw_i2c_write_reg(0xa0, 0);
    gx_hw_i2c_write_reg(0xa4, 0);
    gx_hw_i2c_write_reg(0xa8, 0);
    gx_hw_i2c_write_reg(0xac, 0);

    gx_hw_i2c_request_irq(_lvpI2CMsgISR, NULL);
    gx_hw_i2c_set_irq_enable(0xff);

    LvpPmuSuspendLockCreate(&pmu_lock);

    i2c_msg_initialized = 1;

    return 0;
}

int LvpI2CMsgWriteVoiceEvent(unsigned char event, I2C_EVENT_CONFIRM_CB confirm_cb)
{
    event_confirm_cb = confirm_cb;
    gx_hw_i2c_write_reg(VOICE_EVEVNT_REG, event);
    return 0;
}

