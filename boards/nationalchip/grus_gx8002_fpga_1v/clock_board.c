/* Voice Signal Preprocess
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
#if defined CONFIG_FULL_SPEED_FREQUENCY
    {CLOCK_SOURCE_24M_PLL     , 1}, /* CLOCK_SOURCE_24M        | CLOCK_SOURCE_PLL_DTO  */
#else
    {CLOCK_SOURCE_24M_PLL     , 0}, /* CLOCK_SOURCE_24M        | CLOCK_SOURCE_PLL_DTO  */
#endif
    {CLOCK_SOURCE_32K         , 0}, /* CLOCK_SOURCE_OSC_32K    | CLOCK_SOURCE_XTAL_32K */
};

/*********************** PLL CONFIG ***********************/
static GX_CLOCK_PLL pll = {
#if defined CONFIG_FULL_SPEED_FREQUENCY
    .pll_enable      = 1,
#else
    .pll_enable      = 0,
#endif

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

static int _clk_pll_init(void)
{
    int pll_div_fb[5] = {3071, 3081, 3086, 3061, 3056}; //0, +10, +15, -10, -15
    for (int i = 0; i < sizeof(pll_div_fb); i++)
    {
        pll.pll_div_fb = pll_div_fb[i];
        if (gx_clock_set_pll_no_block(&pll, 40) == 0)
            return 0;
    }

    return -1;
}

static void _clk_src_init(void)
{
    int i;
    if (gx_pmu_osc_get_all_trim_state() == 1) {
        gx_clock_set_pll(&pll);
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
    gx_clock_set_module_source(CLOCK_MODULE_AUDIO_IN_SYS, MODULE_SOURCE_1M_12M);
    gx_clock_set_module_source(CLOCK_MODULE_AUDIO_IN_ADC, MODULE_SOURCE_ADC_32K);
    gx_clock_set_module_source(CLOCK_MODULE_AUDIO_IN_PDM, MODULE_SOURCE_PDM_OSC_1M);
    gx_clock_set_module_source(CLOCK_MODULE_OSC_REF     , MODULE_SOURCE_1M_12M);

    gx_clock_set_module_source(CLOCK_MODULE_TIMER_WDT   , MODULE_SOURCE_1M_12M);
    gx_clock_set_module_source(CLOCK_MODULE_UART0_UART1 , MODULE_SOURCE_24M_PLL);
    gx_clock_set_module_source(CLOCK_MODULE_AUDIO_PLAY  , MODULE_SOURCE_24M_PLL);
    gx_clock_set_module_source(CLOCK_MODULE_I2C0_I2C1   , MODULE_SOURCE_1M_12M);
    gx_clock_set_module_source(CLOCK_MODULE_NPU         , MODULE_SOURCE_24M_PLL);
    gx_clock_set_module_source(CLOCK_MODULE_FLASH_SPI   , MODULE_SOURCE_24M_PLL);
    gx_clock_set_module_source(CLOCK_MODULE_GENERAL_SPI , MODULE_SOURCE_1M_12M);
    gx_clock_set_module_source(CLOCK_MODULE_AUDIO_LODAC , MODULE_SOURCE_24M_PLL);
}

static void _clk_mod_lowpower_init(void)
{
    for (int module = CLOCK_MODULE_RTC; module < CLOCK_MODULE_SCPU; module++) {
        switch (module) {
        case CLOCK_MODULE_AUDIO_IN_ADC:
            if (gx_clock_get_module_source(module) == MODULE_SOURCE_ADC_SYS)
                gx_clock_set_module_source(module, MODULE_SOURCE_ADC_32K);
            break;
        case CLOCK_MODULE_AUDIO_IN_PDM:
            if (gx_clock_get_module_source(module) == MODULE_SOURCE_PDM_SYS)
                gx_clock_set_module_source(module, MODULE_SOURCE_PDM_OSC_1M);
            break;
        default:
            gx_clock_set_module_source(module, MODULE_SOURCE_1M_12M);
            break;
        }
    }
}

/*********************** DIV CONFIG ***********************/

static void _clk_pmu_normal_div_dto(void)
{
    gx_clock_set_dto(CLOCK_MODULE_AUDIO_IN_SYS, 0x01000000, 0);

#if defined CONFIG_SRAM_FREQUENCY_8M
    gx_clock_set_div(CLOCK_MODULE_SRAM, 3);
#elif defined CONFIG_SRAM_FREQUENCY_12M
    gx_clock_set_div(CLOCK_MODULE_SRAM, 2);
#elif defined CONFIG_SRAM_FREQUENCY_24M
    gx_clock_set_div(CLOCK_MODULE_SRAM, 0);
#elif defined CONFIG_FULL_SPEED_FREQUENCY
    gx_clock_set_div(CLOCK_MODULE_SRAM, 0);
#endif

    gx_clock_set_div(CLOCK_MODULE_RTC         ,  4);
    gx_clock_set_div(CLOCK_MODULE_PMU         ,  3);
    gx_clock_set_div(CLOCK_MODULE_FFT         ,  2);
    gx_clock_set_div(CLOCK_MODULE_GPIO        , 24);
    gx_clock_set_div(CLOCK_MODULE_AUDIO_IN_SYS,  2);
}

static void _clk_pmu_lowpower_div_dto(void)
{
    return;
}

static void _clk_mcu_normal_div_dto(void)
{
    gx_clock_set_dto(CLOCK_MODULE_UART0_UART1, 0x1000000, 0);
    gx_clock_set_dto(CLOCK_MODULE_AUDIO_PLAY , 0x1000000, 0);

#if defined CONFIG_NPU_FREQUENCY_8M
    gx_clock_set_div(CLOCK_MODULE_NPU, 3);
#elif defined CONFIG_NPU_FREQUENCY_12M
    gx_clock_set_div(CLOCK_MODULE_NPU, 2);
#elif defined CONFIG_NPU_FREQUENCY_24M
    gx_clock_set_div(CLOCK_MODULE_NPU, 0);
#elif defined CONFIG_FULL_SPEED_FREQUENCY
    gx_clock_set_div(CLOCK_MODULE_NPU, 0);
#endif


#if defined CONFIG_MCU_FREQUENCY_8M
    gx_clock_set_div(CLOCK_MODULE_SCPU, 3);
#elif defined CONFIG_MCU_FREQUENCY_12M
    gx_clock_set_div(CLOCK_MODULE_SCPU, 2);
#elif defined CONFIG_MCU_FREQUENCY_24M
    gx_clock_set_div(CLOCK_MODULE_SCPU, 0);
#elif defined CONFIG_FULL_SPEED_FREQUENCY
    gx_clock_set_div(CLOCK_MODULE_SCPU, 2);
#endif

}

static void _clk_mcu_lowpower_div_dto(void)
{
    gx_clock_set_div(CLOCK_MODULE_NPU, 0);
    gx_clock_set_div(CLOCK_MODULE_SCPU, 0);
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
    _clk_pmu_lowpower_div_dto();
    _clk_mcu_lowpower_div_dto();
}

void clk_init(void)
{
    GX_START_MODE start_mode = gx_pmu_get_start_mode();
    if (start_mode == GX_START_MODE_ROM) {
        _clk_src_init();

    } else if (start_mode == GX_START_MODE_SRAM) {
        GX_CLOCK_SOURCE_TABLE clk_sram_src_table[] = {
            {CLOCK_SOURCE_24M         , 0}, /* CLOCK_SOURCE_OSC_24M    | CLOCK_SOURCE_PIN_IN   */
            {CLOCK_SOURCE_24M_PLL     , 0}, /* CLOCK_SOURCE_24M        | CLOCK_SOURCE_PLL_DTO  */
        };
        for (int i = 0; i < sizeof(clk_sram_src_table)/sizeof(GX_CLOCK_SOURCE_TABLE); i++)
            gx_clock_set_source(&clk_sram_src_table[i]);
    }

    _clk_mod_normal_init();

    _clk_pmu_normal_div_dto();

    _clk_mcu_normal_div_dto();

    gx_clock_set_module_enable(CLOCK_MODULE_HW_I2C, 1);
    //close 32k xtal
    *(volatile int*)0xa0005084 &= ~(1<<0);

    // flash power down
    //*(volatile int*)0xa000003c = 1;

    writel(0x59, 0x40 + GX_REG_BASE_HW_I2C);
    writel(0x59, 0x44 + GX_REG_BASE_HW_I2C);
    writel(0x59, 0x48 + GX_REG_BASE_HW_I2C);
    writel(0x59, 0x4c + GX_REG_BASE_HW_I2C);

    GX_PMU_TRIM_CFG cfg;
#if ((defined CONFIG_NPU_FREQUENCY_8M) && (defined CONFIG_MCU_FREQUENCY_8M) && !(defined CONFIG_MCU_ENABLE_XIP))
    cfg.bits.trim_val = 0x3;
#elif defined CONFIG_NPU_FREQUENCY_12M
    cfg.bits.trim_val = 0x2;
#elif defined CONFIG_NPU_FREQUENCY_24M
    cfg.bits.trim_val = 0x2;
#elif defined CONFIG_FULL_SPEED_FREQUENCY
    cfg.bits.trim_val = 0;
#endif

    cfg.bits.trim_en  = 1;
    gx_pmu_ctrl_set(GX_PMU_CMD_WORK_LDO_TRIM, &cfg);

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


