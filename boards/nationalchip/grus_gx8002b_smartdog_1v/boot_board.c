/* LVP
 * Copyright (C) 2001-2021 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * boot_board.h
 *
 */

#include <autoconf.h>
#include <common.h>
#include <spl/spl.h>

static int spl_padmux_set(int pad_id, int function)
{
    int base, offset, func;
    unsigned long pinconfig;

    base = pad_id / 8;
    offset = pad_id % 8;

    pinconfig = readl(0xa0010090 + base * 4);
    pinconfig &= ~((0xF) << (offset * 4));
    func = (function & 0xF) << (offset * 4);
    pinconfig |= func;
    writel(pinconfig, 0xa0010090 + base * 4);

    return 0;
}

void spl_board_init(void)
{
    if (spl_osc_get_all_trim_state() == 0)
        spl_padmux_set(8, 12);
}

void spl_board_clk_init(void)
{
    if (spl_clk_trim() == 0)
        spl_osc_set_all_trim_state(1);
}
