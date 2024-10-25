/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_i2c_proxy.c:
 *
 */

#include <stdio.h>
#include <driver/gx_hw_i2c.h>
#include "lvp_i2c_proxy.h"

#define VOICE_EVEVNT_REG    0xA0


#define ISR_EVENT_CONFIRM    0x10

static EVENT_CONFIRM_CB event_confirm_cb;

static int lvp_i2c_proxy_isr(void *private, unsigned char status)
{
    printf("======[%s]%d status = 0x%02x ======\n", __func__, __LINE__, status);

    switch(status)
    {
        case ISR_EVENT_CONFIRM:
            // clear event
            gx_hw_i2c_write_reg(VOICE_EVEVNT_REG, 0);
            if(event_confirm_cb)
            {
                event_confirm_cb();
                event_confirm_cb = NULL;
            }
            break;

        default:
            break;

    }

    return 0;
}

int lvp_i2c_proxy_init(void)
{
    gx_hw_i2c_enter_config_mode();

    gx_hw_i2c_write_reg(VOICE_EVEVNT_REG, 0);
    gx_hw_i2c_write_reg(0xa4, 0);
    gx_hw_i2c_write_reg(0xa8, 0);
    gx_hw_i2c_write_reg(0xac, 0);

    gx_hw_i2c_request_irq(lvp_i2c_proxy_isr, NULL);
    gx_hw_i2c_set_irq_enable(0xff);

    return 0;
}

int lvp_i2c_proxy_write_voice_event(unsigned char event, EVENT_CONFIRM_CB confirm_cb)
{
    event_confirm_cb = confirm_cb;
    gx_hw_i2c_write_reg(VOICE_EVEVNT_REG, event);
    return 0;
}

