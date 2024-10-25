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
#include <board_config.h>
#include  <clk_priv.h>

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

__attribute__ ((unused)) static int _boot_clk_init(void)
{
    int i = 0;
    static GX_CLOCK_PLL pll = {
        .pll_enable      = 1,

#if 0
        .pll_in          = 32768,
        .pll_fvco        = 98304000,
        .pll_out         = 48000000,

        .pll_div_in      = 0,
        .pll_div_fb      = 2999,
        .pll_icpsel      = 2,
        .pll_bwsel_lpf   = 0,
        .pll_vco_subband = 3,
        .pll_div_out     = 0,

        .pll_itrim       = 0,
        .pll_vco_trim    = 4,
        .pll_2nd3nd_lpf  = 0,
        .pll_lock_tiehi  = 0,
#else
    .pll_in          = 32000,
    .pll_fvco        = 73728000,
    .pll_out         = 36864000,

    .pll_div_in      = 0,
    .pll_div_fb      = 2303,
    .pll_icpsel      = 2,
    .pll_bwsel_lpf   = 0,
    .pll_vco_subband = 1,
    .pll_div_out     = 0,

    .pll_itrim       = 0,
    .pll_vco_trim    = 4,
    .pll_2nd3nd_lpf  = 0,
    .pll_lock_tiehi  = 0,

#endif
    };

    static GX_CLOCK_SOURCE_TABLE clk_high_table[] = {
        {CLOCK_SOURCE_PLL_DTO     , 0}, /* CLOCK_OSC_PLL  | CLOCK_PIN_IN   */
        {CLOCK_SOURCE_24M_PLL     , 1}, /* CLOCK_24M      | CLOCK_PLL_DTO  */
    };

    extern int spl_clk_set_pll_no_block(GX_CLOCK_PLL *pll, unsigned int timeout_ms);
    extern void spl_clk_set_div(GX_CLOCK_MODULE module, unsigned int div);

    while (1) {
#ifdef CONFIG_ENABLE_PLL_FREQUENCY
        if (pll.pll_div_fb > 3200)
            continue;
#else
        if (pll.pll_div_fb > 2400)
            continue;
#endif

        if (spl_clk_set_pll_no_block(&pll, 40) == 0)
            break;
        pll.pll_div_fb += 20;
    }

    spl_clk_set_div(CLOCK_MODULE_SRAM, 2);

    extern void spl_clk_set_source(GX_CLOCK_SOURCE_TABLE *clock_table);
    extern int spl_clk_set_module_soruce(GX_CLOCK_MODULE module, GX_CLOCK_MODULE_SOURCE source);

    for (i = 0; i < ARRAY_SIZE(clk_high_table); i++)
        spl_clk_set_source(&clk_high_table[i]);

    extern void spl_clk_set_dto(GX_CLOCK_MODULE module, unsigned int dto, int enable);
#ifdef CONFIG_ENABLE_PLL_FREQUENCY
    spl_clk_set_dto(CLOCK_MODULE_UART0_UART1, 0x1000000, 1);
#else
    spl_clk_set_dto(CLOCK_MODULE_UART0_UART1, 0x1000000, 0);
#endif

    extern int spl_clk_set_module_source(GX_CLOCK_MODULE module, GX_CLOCK_MODULE_SOURCE source);
    spl_clk_set_module_source(CLOCK_MODULE_UART0_UART1, MODULE_SOURCE_24M_PLL);
    spl_clk_set_module_source(CLOCK_MODULE_I2C0_I2C1, MODULE_SOURCE_24M_PLL);

    spl_clk_set_module_source(CLOCK_MODULE_GENERAL_SPI, MODULE_SOURCE_24M_PLL);
    spl_clk_set_module_source(CLOCK_MODULE_SCPU, MODULE_SOURCE_24M_PLL);
    spl_clk_set_module_source(CLOCK_MODULE_SRAM, MODULE_SOURCE_24M_PLL);

    return 0;
}

void spl_board_init(void)
{
    spl_padmux_set(1, 8);
}

void spl_board_clk_init(void)
{
    static GX_CLOCK_SOURCE_TABLE clk_src_table[] = {
        /*       id            source      source0          source1     */
        {CLOCK_SOURCE_PIN_IN      , 1}, /* CLOCK_AUDIO_MCLK | CLOCK_PWM_IN   */
        {CLOCK_SOURCE_24M         , 1}, /* CLOCK_OSC_24M  | CLOCK_PIN_IN   */
        {CLOCK_SOURCE_24M_PLL     , 0}, /* CLOCK_24M      | CLOCK_PLL_DTO  */
    };

    extern void spl_clk_set_source(GX_CLOCK_SOURCE_TABLE *source_table);
    extern int spl_clk_set_module_source(GX_CLOCK_MODULE module, GX_CLOCK_MODULE_SOURCE source);
    extern int spl_osc_32k_manual_trim(void);

    for (int i = 0; i < ARRAY_SIZE(clk_src_table); i++)
        spl_clk_set_source(&clk_src_table[i]);

    spl_clk_set_module_source(CLOCK_MODULE_OSC_REF, MODULE_SOURCE_24M_PLL);

    int ret = 0;
    ret |= spl_osc_32k_manual_trim();
    ret |= _boot_clk_init();

    if (ret == 0)
        spl_osc_set_32k_trim_state(1);

}
