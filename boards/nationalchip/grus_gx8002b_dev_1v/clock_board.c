/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * clock_board.c
 *
 */

#include <autoconf.h>
#include <board_config.h>
#include <driver/gx_clock.h>
#include <driver/gx_pmu_ctrl.h>
#include <driver/gx_pmu_osc.h>
#include <driver/gx_flash.h>
#include <driver/gx_analog/gx_ldo.h>

/******************* CLOCK SOURCE CONFIG *******************/
static GX_CLOCK_SOURCE_TABLE clk_src_xtal_table[] = {
/*       id            source      source0            source1     */
    {CLOCK_SOURCE_32K_PLL     , 0}, /* CLOCK_SOURCE_OSC_32K    | CLOCK_SOURCE_XTAL_32K */
    {CLOCK_SOURCE_1M_INPUT    , 0}, /* CLOCK_SOURCE_PDM_IN     | CLOCK_SOURCE_PWM_IN   */
    {CLOCK_SOURCE_1M          , 0}, /* CLOCK_SOURCE_OSC_1M     | CLOCK_SOURCE_1M_INPUT */
    {CLOCK_SOURCE_OSC_PLL     , 0}, /* CLOCK_SOURCE_32K_PLL    | CLOCK_SOURCE_1M       */

    {CLOCK_SOURCE_PIN_IN      , 1}, /* CLOCK_SOURCE_AUDIO_MCLK | CLOCK_SOURCE_PWM_IN   */
    {CLOCK_SOURCE_24M         , 1}, /* CLOCK_SOURCE_OSC_24M    | CLOCK_SOURCE_PIN_IN   */
    {CLOCK_SOURCE_PLL_DTO     , 0}, /* CLOCK_SOURCE_OSC_PLL    | CLOCK_SOURCE_PIN_IN   */

    {CLOCK_SOURCE_1M_12M      , 1}, /* CLOCK_SOURCE_OSC_24M    | CLOCK_SOURCE_1M       */
    {CLOCK_SOURCE_24M_PLL     , 0}, /* CLOCK_SOURCE_24M        | CLOCK_SOURCE_PLL_DTO  */
    {CLOCK_SOURCE_32K         , 0}, /* CLOCK_SOURCE_OSC_32K    | CLOCK_SOURCE_XTAL_32K */
};

/******************* CLOCK SOURCE CONFIG *******************/
static GX_CLOCK_SOURCE_TABLE clk_src_osc_table[] = {
/*       id            source      source0            source1     */
    {CLOCK_SOURCE_32K_PLL     , 0}, /* CLOCK_SOURCE_OSC_32K    | CLOCK_SOURCE_XTAL_32K */
    {CLOCK_SOURCE_1M_INPUT    , 0}, /* CLOCK_SOURCE_PDM_IN     | CLOCK_SOURCE_PWM_IN   */
    {CLOCK_SOURCE_1M          , 0}, /* CLOCK_SOURCE_OSC_1M     | CLOCK_SOURCE_1M_INPUT */
    {CLOCK_SOURCE_OSC_PLL     , 0}, /* CLOCK_SOURCE_32K_PLL    | CLOCK_SOURCE_1M       */

    {CLOCK_SOURCE_PIN_IN      , 0}, /* CLOCK_SOURCE_AUDIO_MCLK | CLOCK_SOURCE_PWM_IN   */
    {CLOCK_SOURCE_24M         , 0}, /* CLOCK_SOURCE_OSC_24M    | CLOCK_SOURCE_PIN_IN   */
    {CLOCK_SOURCE_PLL_DTO     , 0}, /* CLOCK_SOURCE_OSC_PLL    | CLOCK_SOURCE_PIN_IN   */

    {CLOCK_SOURCE_1M_12M      , 1}, /* CLOCK_SOURCE_OSC_24M/2  | CLOCK_SOURCE_1M       */
    {CLOCK_SOURCE_24M_PLL     , 1}, /* CLOCK_SOURCE_24M        | CLOCK_SOURCE_PLL_DTO  */
    {CLOCK_SOURCE_32K         , 0}, /* CLOCK_SOURCE_OSC_32K    | CLOCK_SOURCE_XTAL_32K */
};

/*********************** PLL CONFIG ***********************/
#if (defined CONFIG_ENABLE_PLL_FREQUENCY_50M)
static GX_CLOCK_PLL pll = {
    .pll_enable      = 1,

    .pll_in          = 32768,
    .pll_fvco        = 98304000,
    .pll_out         = 49152000,

    .pll_div_in      = 0,
    .pll_div_fb      = 3071,
    .pll_icpsel      = 2,
    .pll_bwsel_lpf   = 0,
    .pll_vco_subband = 3,
    .pll_div_out     = 0,

    .pll_itrim       = 0,
    .pll_vco_trim    = 4,
    .pll_2nd3nd_lpf  = 0,
    .pll_lock_tiehi  = 0,
};
#elif (defined CONFIG_ENABLE_PLL_FREQUENCY_48M)
static GX_CLOCK_PLL pll = {
    .pll_enable      = 1,

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
};
#else // ENABLE_PLL_FREQUENCY_24M
static GX_CLOCK_PLL pll = {
    .pll_enable      = 1,

    .pll_in          = 32768,
    .pll_fvco        = 98304000,
    .pll_out         = 24576000,

    .pll_div_in      = 0,
    .pll_div_fb      = 3071,
    .pll_icpsel      = 2,
    .pll_bwsel_lpf   = 0,
    .pll_vco_subband = 3,
    .pll_div_out     = 1,

    .pll_itrim       = 0,
    .pll_vco_trim    = 4,
    .pll_2nd3nd_lpf  = 0,
    .pll_lock_tiehi  = 0,
};
#endif

static int _clk_pll_init(void)
{
	int pll_div_fb = pll.pll_div_fb;
	int offs[] = {0, 10, 15, -10, -15};
    for (int i = 0; i < sizeof(offs)/sizeof(int); i++)
    {
        pll.pll_div_fb = pll_div_fb + offs[i];
        if (gx_clock_set_pll_no_block(&pll, 40) == 0)
            return 0;
    }

    return -1;
}

int stage2_trim_done = -1;
static void _clk_src_init(void)
{
    int i;
    if (gx_pmu_osc_get_all_trim_state() == 1) {
        stage2_trim_done = 1;
        if (_clk_pll_init() == -1)
            while (1);
        for (i = 0; i < sizeof(clk_src_osc_table)/sizeof(GX_CLOCK_SOURCE_TABLE); i++)
            gx_clock_set_source(&clk_src_osc_table[i]);

    } else {
        for (i = 0; i < sizeof(clk_src_xtal_table)/sizeof(GX_CLOCK_SOURCE_TABLE); i++)
            gx_clock_set_source(&clk_src_xtal_table[i]);
    }
}

static void _clk_mod_normal_init(void)
{
    gx_clock_set_module_source(CLOCK_MODULE_SCPU        , MODULE_SOURCE_24M_PLL);
    gx_clock_set_module_source(CLOCK_MODULE_SRAM        , MODULE_SOURCE_24M_PLL);

    gx_clock_set_module_source(CLOCK_MODULE_RTC         , MODULE_SOURCE_32K);
    gx_clock_set_module_source(CLOCK_MODULE_PMU         , MODULE_SOURCE_1M_12M);
    gx_clock_set_module_source(CLOCK_MODULE_FFT         , MODULE_SOURCE_1M_12M);
    gx_clock_set_module_source(CLOCK_MODULE_GPIO        , MODULE_SOURCE_1M_12M);
    gx_clock_set_module_source(CLOCK_MODULE_HW_I2C      , MODULE_SOURCE_1M_12M);
    gx_clock_set_module_source(CLOCK_MODULE_AUDIO_IN_SYS, MODULE_SOURCE_24M_PLL);
    //gx_clock_set_module_source(CLOCK_MODULE_AUDIO_IN_ADC, MODULE_SOURCE_ADC_32K);
    gx_clock_set_module_source(CLOCK_MODULE_AUDIO_IN_ADC, MODULE_SOURCE_ADC_SYS);
    //gx_clock_set_module_source(CLOCK_MODULE_AUDIO_IN_PDM, MODULE_SOURCE_PDM_OSC_1M);
    gx_clock_set_module_source(CLOCK_MODULE_AUDIO_IN_PDM, MODULE_SOURCE_PDM_SYS);
    gx_clock_set_module_source(CLOCK_MODULE_OSC_REF     , MODULE_SOURCE_1M_12M);

    gx_clock_set_module_source(CLOCK_MODULE_TIMER_WDT   , MODULE_SOURCE_1M_12M);
    gx_clock_set_module_source(CLOCK_MODULE_UART0_UART1 , MODULE_SOURCE_24M_PLL);
    gx_clock_set_module_source(CLOCK_MODULE_AUDIO_PLAY  , MODULE_SOURCE_24M_PLL);
    gx_clock_set_module_source(CLOCK_MODULE_I2C0_I2C1   , MODULE_SOURCE_24M_PLL);
    gx_clock_set_module_source(CLOCK_MODULE_NPU         , MODULE_SOURCE_24M_PLL);
    gx_clock_set_module_source(CLOCK_MODULE_FLASH_SPI   , MODULE_SOURCE_24M_PLL);
    gx_clock_set_module_source(CLOCK_MODULE_GENERAL_SPI , MODULE_SOURCE_24M_PLL);
    gx_clock_set_module_source(CLOCK_MODULE_AUDIO_LODAC , MODULE_SOURCE_24M_PLL);
}

static void _clk_mod_lowpower_init(void)
{
    for (int module = CLOCK_MODULE_RTC; module < CLOCK_MODULE_MAX; module++) {
        switch (module) {
        case CLOCK_MODULE_AUDIO_IN_ADC:
            if (gx_clock_get_module_source(module) == MODULE_SOURCE_ADC_SYS)
                gx_clock_set_module_source(module, MODULE_SOURCE_ADC_32K);
            break;
        case CLOCK_MODULE_AUDIO_IN_PDM:
            if (gx_clock_get_module_source(module) == MODULE_SOURCE_PDM_SYS)
                gx_clock_set_module_source(module, MODULE_SOURCE_PDM_OSC_1M);
            break;
        case CLOCK_MODULE_AUDIO_IN_SYS:
            if ((gx_clock_get_module_source(module) == MODULE_SOURCE_24M_PLL) &&
                    ((((*(volatile unsigned int*)PMU_CFG_SOURCE_SEL0) >> 6) & 0x1) == 1))
                break;
            gx_clock_set_module_source(module, MODULE_SOURCE_1M_12M);
            break;
        default:
            gx_clock_set_module_source(module, MODULE_SOURCE_1M_12M);
            break;
        }
    }
}

static void _clk_pmu_normal_div_dto(void)
{
#if (defined CONFIG_ENABLE_PLL_FREQUENCY_50M)
    gx_clock_set_dto(CLOCK_MODULE_AUDIO_IN_SYS, 0x01000000, 1); //49.152MHz --> 24.576MHz
#elif (defined CONFIG_ENABLE_PLL_FREQUENCY_48M)
    gx_clock_set_dto(CLOCK_MODULE_AUDIO_IN_SYS, 0x0083126F, 1); //48MHz --> 12.288MHz
#else
    gx_clock_set_dto(CLOCK_MODULE_AUDIO_IN_SYS, 0x01000000, 0); //24.576MHz --> 12.288MHz
#endif

    gx_clock_set_div(CLOCK_MODULE_SRAM, SRAM_DIV_PARAM);

    gx_clock_set_div(CLOCK_MODULE_RTC         ,  4);
    gx_clock_set_div(CLOCK_MODULE_PMU         ,  3);
    gx_clock_set_div(CLOCK_MODULE_FFT         ,  2);
#if defined(CONFIG_BOARD_HAS_AIN_I2S_OUT_SLAVE) // IIS 输入时钟为12.288MHz时，需如此配置
    gx_clock_set_div(CLOCK_MODULE_AUDIO_IN_SYS,  0);
#else
#if (defined CONFIG_ENABLE_PLL_FREQUENCY_48M)
    gx_clock_set_div(CLOCK_MODULE_AUDIO_IN_SYS,  0);
#else
    gx_clock_set_div(CLOCK_MODULE_AUDIO_IN_SYS,  2);
#endif
#endif
    gx_clock_set_div(CLOCK_MODULE_AUDIO_IN_ADC, 384);
    gx_clock_set_div(CLOCK_MODULE_AUDIO_IN_PDM, 12);
    gx_clock_set_div(CLOCK_MODULE_I2C0_I2C1,     2);

}

static void _clk_pmu_lowpower_div_dto(void)
{
    gx_clock_set_div(CLOCK_MODULE_AUDIO_IN_ADC, 0);
    gx_clock_set_div(CLOCK_MODULE_AUDIO_IN_PDM, 0);
    return;
}

static void _clk_mcu_normal_div_dto(void)
{
#if (defined CONFIG_ENABLE_PLL_FREQUENCY_50M)
    gx_clock_set_dto(CLOCK_MODULE_UART0_UART1, 0x01000000, 1); //49.152MHz --> 24.576MHz
    gx_clock_set_dto(CLOCK_MODULE_AUDIO_PLAY , 0x00800000, 1); //49.152MHz --> 12.288MHz
#elif (defined CONFIG_ENABLE_PLL_FREQUENCY_48M)
    gx_clock_set_dto(CLOCK_MODULE_AUDIO_PLAY , 0x0083126F, 1); //48MHz --> 12.288MHz
#else
    gx_clock_set_dto(CLOCK_MODULE_UART0_UART1, 0x01000000, 0); //24.576MHz --> 12.288MHz
    gx_clock_set_dto(CLOCK_MODULE_AUDIO_PLAY , 0x01000000, 1); //24.576MHz --> 12.288MHz
#endif

#ifdef CONFIG_MCU_ENABLE_DYNAMICALLY_ADJUSTMENT_CPU_FREQUENCY
    gx_clock_set_div(CLOCK_MODULE_SCPU, 0);
#else
    gx_clock_set_div(CLOCK_MODULE_SCPU, MCU_DIV_PARAM);
#endif

    gx_clock_set_div(CLOCK_MODULE_NPU, NPU_DIV_PARAM);
    gx_clock_set_div(CLOCK_MODULE_FLASH_SPI, FLASH_DIV_PARAM);

#if (defined CONFIG_ENABLE_PLL_FREQUENCY_50M || defined CONFIG_ENABLE_PLL_FREQUENCY_48M)
    gx_clock_set_div(CLOCK_MODULE_AUDIO_LODAC, 4);
    gx_clock_set_div(CLOCK_MODULE_GENERAL_SPI, 4);
#else
    gx_clock_set_div(CLOCK_MODULE_AUDIO_LODAC, 2);
    gx_clock_set_div(CLOCK_MODULE_GENERAL_SPI, 2);
#endif
}

static void _clk_mcu_lowpower_div_dto(void)
{
    gx_clock_set_div(CLOCK_MODULE_SCPU, 0);

    return;
}

static unsigned int s_clk_mod_gate = 0;
static void _clk_mod_lowpower_gate(void)
{
    for (int module = CLOCK_MODULE_AUDIO_PLAY; module < CLOCK_MODULE_MAX; module++) {
        int enable = gx_clock_get_module_enable(module);
        if (enable != -1)
            s_clk_mod_gate |= (enable & 0x1) << module;
    }
    return;
}

/**********************************************************/
void clk_switch_1m(void)
{
    GX_CLOCK_SOURCE_TABLE clk_src_1m[] = {
        {CLOCK_SOURCE_1M          , 0}, /* CLOCK_SOURCE_OSC_1M     | CLOCK_SOURCE_1M_INPUT */
        {CLOCK_SOURCE_1M_12M      , 1}, /* CLOCK_SOURCE_OSC_24M    | CLOCK_SOURCE_1M       */
    };
    for (int i = 0; i < ARRAY_SIZE(clk_src_1m); i++)
        gx_clock_set_source(&clk_src_1m[i]);

    _clk_mod_lowpower_init();
    _clk_mod_lowpower_gate();
    _clk_pmu_lowpower_div_dto();
    _clk_mcu_lowpower_div_dto();

#ifdef CONFIG_ENABLE_BYPASS_CORE_LDO
    gx_analog_set_ldo_dig_ctrl(LDO_SW_CTRL_ENABLE);
#endif

    if (pll.pll_enable == 1) {
        pll.pll_enable = 0;
        gx_clock_set_pll(&pll);
        pll.pll_enable = 1;
    }
}

void clk_switch_soft_off(void)
{
    GX_CLOCK_SOURCE_TABLE clk_src_1m[] = {
        {CLOCK_SOURCE_1M          , 0}, /* CLOCK_SOURCE_OSC_1M     | CLOCK_SOURCE_1M_INPUT */
        {CLOCK_SOURCE_1M_12M      , 1}, /* CLOCK_SOURCE_OSC_24M    | CLOCK_SOURCE_1M       */
    };
    for (int i = 0; i < ARRAY_SIZE(clk_src_1m); i++)
        gx_clock_set_source(&clk_src_1m[i]);

    _clk_mod_lowpower_init();

    for (int i = CLOCK_MODULE_RTC; i < CLOCK_MODULE_SCPU; i++) {
        switch (i) {
        case CLOCK_MODULE_PMU:
        case CLOCK_MODULE_SRAM:
        case CLOCK_MODULE_SCPU:
            gx_clock_set_module_enable(i, 1);
            break;
        default:
            gx_clock_set_module_enable(i, 0);
            break;
        }
    }

    _clk_pmu_lowpower_div_dto();
    _clk_mcu_lowpower_div_dto();

#ifdef CONFIG_ENABLE_BYPASS_CORE_LDO
    gx_analog_set_ldo_dig_ctrl(LDO_SW_CTRL_ENABLE);
#endif

    if (pll.pll_enable == 1) {
        pll.pll_enable = 0;
        gx_clock_set_pll(&pll);
        pll.pll_enable = 1;
    }

    *((unsigned int*)0xa0005094) = *((unsigned int*)0xa0005094) & 0xfffffff7;
    *((unsigned int*)0xa0005090) = *((unsigned int*)0xa0005090) & 0xffffffbf;

    *((unsigned int*)0xa0010018) = *((unsigned int*)0xa0010018) | 0x160;
}

int _dig_ldo_voltage(void)
{
#ifdef CONFIG_CORE_LDO_VOLTAGE_950_MV
    return LDO_DIG_VOLTAGE_0_950V;
#endif

#ifdef CONFIG_CORE_LDO_VOLTAGE_924_MV
    return LDO_DIG_VOLTAGE_0_924V;
#endif

#ifdef CONFIG_CORE_LDO_VOLTAGE_897_MV
    return LDO_DIG_VOLTAGE_0_897V;
#endif

#ifdef CONFIG_CORE_LDO_VOLTAGE_871_MV
    return LDO_DIG_VOLTAGE_0_871V;
#endif

#ifdef CONFIG_CORE_LDO_VOLTAGE_845_MV
    return LDO_DIG_VOLTAGE_0_845V;
#endif

#ifdef CONFIG_CORE_LDO_VOLTAGE_819_MV
    return LDO_DIG_VOLTAGE_0_819V;
#endif

#ifdef CONFIG_CORE_LDO_VOLTAGE_792_MV
    return LDO_DIG_VOLTAGE_0_792V;
#endif

#ifdef CONFIG_CORE_LDO_VOLTAGE_766_MV
    return LDO_DIG_VOLTAGE_0_766V;
#endif

#ifdef CONFIG_CORE_LDO_VOLTAGE_740_MV
    return LDO_DIG_VOLTAGE_0_740V;
#endif

#ifdef CONFIG_CORE_LDO_VOLTAGE_714_MV
    return LDO_DIG_VOLTAGE_0_714V;
#endif

#ifdef CONFIG_CORE_LDO_VOLTAGE_687_MV
    return LDO_DIG_VOLTAGE_0_687V;
#endif

#ifdef CONFIG_CORE_LDO_VOLTAGE_661_MV
    return LDO_DIG_VOLTAGE_0_661V;
#endif

    return 0;
}

void clk_init(void)
{
    GX_START_MODE start_mode = gx_pmu_get_start_mode();
    if (start_mode == GX_START_MODE_ROM) {
        _clk_src_init();
    } else if (start_mode == GX_START_MODE_SRAM) {
        GX_CLOCK_SOURCE_TABLE clk_sram_src_table[] = {
            {CLOCK_SOURCE_24M         , 0}, /* CLOCK_SOURCE_OSC_24M    | CLOCK_SOURCE_PIN_IN   */
            {CLOCK_SOURCE_24M_PLL     , 1}, /* CLOCK_SOURCE_24M        | CLOCK_SOURCE_PLL_DTO  */
        };
        if (stage2_trim_done == 1)
            gx_clock_set_pll(&pll);

        for (int i = 0; i < sizeof(clk_sram_src_table)/sizeof(GX_CLOCK_SOURCE_TABLE); i++)
            gx_clock_set_source(&clk_sram_src_table[i]);
    }

    _clk_mod_normal_init();

    _clk_pmu_normal_div_dto();

    _clk_mcu_normal_div_dto();

    if (start_mode == GX_START_MODE_SRAM) {
        for (int module = CLOCK_MODULE_AUDIO_PLAY; module < CLOCK_MODULE_MAX; module++)
            gx_clock_set_module_enable(module, (s_clk_mod_gate >> module) & 0x1);
    }

    gx_clock_set_module_enable(CLOCK_MODULE_HW_I2C, 1);
    //close 32k xtal
	*(volatile int*)0xa0005084 &= ~(1<<0);

	//close 24M osc
	*(volatile int*)0xa0005060 &= ~(1<<1);

    // flash power down
    //*(volatile int*)0xa000003c = 1;

#ifdef CONFIG_ENABLE_BYPASS_CORE_LDO
    gx_analog_set_ldo_dig_ctrl(LDO_SW_CTRL_BYPASS);

#else

    // for set ldo voltage
    writel(0x59, 0x40 + GX_REG_BASE_HW_I2C);
    writel(0x59, 0x44 + GX_REG_BASE_HW_I2C);
    writel(0x59, 0x48 + GX_REG_BASE_HW_I2C);
    writel(0x59, 0x4c + GX_REG_BASE_HW_I2C);


    gx_analog_set_ldo_ana_voltage(LDO_ANA_VOLTAGE_0_9V);
# ifndef CONFIG_CUSTOMIZE_CORE_LDO_VOLTAGE
#if (defined CONFIG_ENABLE_PLL_FREQUENCY_50M || defined CONFIG_ENABLE_PLL_FREQUENCY_48M)
    gx_analog_set_ldo_dig_voltage(LDO_DIG_VOLTAGE_0_950V);
#  else
    gx_analog_set_ldo_dig_voltage(LDO_DIG_VOLTAGE_0_924V);
#  endif
# else
    gx_analog_set_ldo_dig_voltage(_dig_ldo_voltage());
# endif
#endif
}

void board_init(void)
{
    GX_WAKEUP_SOURCE start_mode = gx_pmu_get_wakeup_source();
    if (!(start_mode == GX_WAKEUP_SOURCE_COLD || start_mode == GX_WAKEUP_SOURCE_WDT)) {
#ifdef CONFIG_MCU_ENABLE_XIP
        extern GX_FLASH_DEV * xip_sflash_init(void);
        xip_sflash_init();
#endif
    }
    extern void gx_analog_config_update_enable(void);
    gx_analog_config_update_enable();
}
