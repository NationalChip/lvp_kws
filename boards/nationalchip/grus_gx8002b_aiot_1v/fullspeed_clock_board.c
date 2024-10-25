#include <autoconf.h>
#include <board_config.h>
#include <driver/gx_clock.h>
#include <driver/gx_pmu_ctrl.h>
#include <driver/gx_flash.h>

#define FULL_SPEED

/******************* CLOCK SOURCE CONFIG *******************/
static GX_CLOCK_TABLE clk_src_xtal_table[] = {
/*       id            source      source0            source1     */
	{CLOCK_32K_PLL     , 0}, /* CLOCK_OSC_32K    | CLOCK_XTAL_32K */
	{CLOCK_1M_INPUT    , 0}, /* CLOCK_PDM_IN     | CLOCK_PWM_IN   */
	{CLOCK_1M          , 0}, /* CLOCK_OSC_1M     | CLOCK_1M_INPUT */
	{CLOCK_OSC_PLL     , 0}, /* CLOCK_32K_PLL    | CLOCK_1M       */

	{CLOCK_PIN_IN      , 1}, /* CLOCK_AUDIO_MCLK | CLOCK_PWM_IN   */
	{CLOCK_24M         , 1}, /* CLOCK_OSC_24M    | CLOCK_PIN_IN   */
	{CLOCK_PLL_DTO     , 0}, /* CLOCK_OSC_PLL    | CLOCK_PIN_IN   */

	{CLOCK_LOW         , 1}, /* CLOCK_OSC_24M    | CLOCK_1M       */
	{CLOCK_HIGH        , 0}, /* CLOCK_24M        | CLOCK_PLL_DTO  */
	{CLOCK_32K         , 0}, /* CLOCK_OSC_32K    | CLOCK_XTAL_32K */
};

/******************* CLOCK SOURCE CONFIG *******************/
static GX_CLOCK_TABLE clk_src_osc_table[] = {
/*       id            source      source0            source1     */
	{CLOCK_32K_PLL     , 0}, /* CLOCK_OSC_32K    | CLOCK_XTAL_32K */
	{CLOCK_1M_INPUT    , 0}, /* CLOCK_PDM_IN     | CLOCK_PWM_IN   */
	{CLOCK_1M          , 0}, /* CLOCK_OSC_1M     | CLOCK_1M_INPUT */
	{CLOCK_OSC_PLL     , 0}, /* CLOCK_32K_PLL    | CLOCK_1M       */

	{CLOCK_PIN_IN      , 0}, /* CLOCK_AUDIO_MCLK | CLOCK_PWM_IN   */
	{CLOCK_24M         , 0}, /* CLOCK_OSC_24M    | CLOCK_PIN_IN   */
	{CLOCK_PLL_DTO     , 0}, /* CLOCK_OSC_PLL    | CLOCK_PIN_IN   */

	{CLOCK_LOW         , 1}, /* CLOCK_OSC_24M/2    | CLOCK_1M       */
	{CLOCK_HIGH        , 1}, /* CLOCK_24M        | CLOCK_PLL_DTO  */
	{CLOCK_32K         , 0}, /* CLOCK_OSC_32K    | CLOCK_XTAL_32K */
};

/*********************** PLL CONFIG ***********************/
static GX_CLOCK_PLL_TABLE pll_table = {
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
	.pll_vco_trim    = 0,
	.pll_2nd3nd_lpf  = 0,
	.pll_lock_tiehi  = 0,
};

/******************* CLOCK MODULE CONFIG *******************/
static GX_CLOCK_TABLE clk_pmu_table[] = {
/*       id            source    source0      source1     source2  */
	{CLOCK_RTC         , 2}, /* CLOCK_LOW | CLOCK_HIGH | CLOCK_32K */
	{CLOCK_PMU         , 0}, /* CLOCK_LOW | CLOCK_HIGH | CLOCK_32K */
	{CLOCK_OSC_REF     , 0}, /* CLOCK_LOW | CLOCK_HIGH | CLOCK_32K */
	{CLOCK_FFT         , 0}, /* CLOCK_LOW | CLOCK_HIGH             */
	{CLOCK_GPIO        , 0}, /* CLOCK_LOW | CLOCK_HIGH             */
	{CLOCK_HW_I2C      , 0}, /* CLOCK_LOW | CLOCK_HIGH             */
	{CLOCK_SRAM        , 1}, /* CLOCK_LOW | CLOCK_HIGH | CLOCK_32K */
	{CLOCK_AUDIO_IN_SYS, 0}, /* CLOCK_LOW | CLOCK_HIGH | CLOCK_32K */

	{CLOCK_AUDIO_IN_ADC, 1}, /* CLOCK_AUDIO_IN_SYS | CLOCK_32K     */
	{CLOCK_AUDIO_IN_PDM, 1}, /* CLOCK_AUDIO_IN_SYS | CLOCK_OSC_1M  */
};

static GX_CLOCK_TABLE clk_mcu_table[] = {
/*       id            source    source0      source1  */
	{CLOCK_SCPU        , 1}, /* CLOCK_LOW | CLOCK_HIGH */
	{CLOCK_TIMER_WDT   , 0}, /* CLOCK_LOW | CLOCK_HIGH */
	{CLOCK_UART0_UART1 , 1}, /* CLOCK_LOW | CLOCK_HIGH */
	{CLOCK_AUDIO_PLAY  , 1}, /* CLOCK_LOW | CLOCK_HIGH */
	{CLOCK_I2C0_I2C1   , 0}, /* CLOCK_LOW | CLOCK_HIGH */
	{CLOCK_NPU         , 1}, /* CLOCK_LOW | CLOCK_HIGH */
	{CLOCK_FLASH_SPI   , 1}, /* CLOCK_LOW | CLOCK_HIGH */
	{CLOCK_GENERAL_SPI , 0}, /* CLOCK_LOW | CLOCK_HIGH */
	{CLOCK_AUDIO_LODAC , 1}, /* CLOCK_LOW | CLOCK_HIGH */
};

/*********************** DTO CONFIG ***********************/
static GX_CLOCK_DTO_TABLE dto_pmu_table[] = {
	{1, 4, 0, 0x00800000},  // Audio In     24.576MHz->12.288MHz
};

static GX_CLOCK_DTO_TABLE dto_mcu_table[] = {
	{1, 5, 0, 0x01000000},  // Uart0 Uart1  24.576MHz->12.288MHz
	{2, 6, 0, 0x00800000},  // Audio Play   24.576MHz->12.288MHz
};

/*********************** DIV CONFIG ***********************/
static GX_CLOCK_DIV_TABLE div_pmu_table[] = {
/*            addr           offs   gate  mask   div              */
	{GX_CLOCK_PMU_DIV_CONFIG0,  0,   13,  0x3f,   2},  // SRAM
	{GX_CLOCK_PMU_DIV_CONFIG0,  8,    0,  0x3f,   4},  // RTC
	{GX_CLOCK_PMU_DIV_CONFIG0, 16,    2,  0x3f,   3},  // PMU
	{GX_CLOCK_PMU_DIV_CONFIG0, 24,    4,  0x3f,   2},  // AUDIO IN

	{GX_CLOCK_PMU_DIV_CONFIG1,  0,   11,  0x3f,   0},  // TEST I2C
	{GX_CLOCK_PMU_DIV_CONFIG1,  8,    4,  0x3f,   0},  // AUDIO PDM
	{GX_CLOCK_PMU_DIV_CONFIG1, 16,    7,  0x3f,   2},  // FFT
	{GX_CLOCK_PMU_DIV_CONFIG1, 24,    9,  0x3f,   0},  // GPIO

	{GX_CLOCK_PMU_DIV_CONFIG2,  0,    4, 0x3ff,   0},  // SAR ADC
	{GX_CLOCK_PMU_DIV_CONFIG2, 12,   20,  0x3f,   0},  // OSC REF
	{GX_CLOCK_PMU_DIV_CONFIG2, 20, 0xff, 0xfff,   0},  // PLL
};

static GX_CLOCK_DIV_TABLE div_mcu_table[] = {
/*            addr           offs   gate  mask   div              */
	{GX_CLOCK_MCU_DIV_CONFIG0,  0,   27,  0x3f,   0},  // TIMER & WDT
	{GX_CLOCK_MCU_DIV_CONFIG0,  8,    7,  0x3f,   0},  // I2C0 & I2C1
	{GX_CLOCK_MCU_DIV_CONFIG0, 16,    9,  0x3f,   0},  // FLASH SPI
	{GX_CLOCK_MCU_DIV_CONFIG0, 24,   10,  0x3f,   0},  // GENERAL SPI

	{GX_CLOCK_MCU_DIV_CONFIG1,  0,   11,  0x3f,   0},  // LODAC
	{GX_CLOCK_MCU_DIV_CONFIG1,  8,    8,  0x3f,   2},  // NPU
	{GX_CLOCK_MCU_DIV_CONFIG1, 16,    4,  0x3f,   0},  // SCPU
};

static void _clk_pmu_init(void)
{
	extern int trim_done;
	int i;
	if (trim_done == 1) {
        gx_clock_pll_config(&pll_table);
		for (i = 0; i < sizeof(clk_src_osc_table)/sizeof(GX_CLOCK_TABLE); i++)
			gx_clock_set_source(&clk_src_osc_table[i]);

	} else {
		for (i = 0; i < sizeof(clk_src_xtal_table)/sizeof(GX_CLOCK_TABLE); i++)
			gx_clock_set_source(&clk_src_xtal_table[i]);
	}

	for (i = 0; i < sizeof(clk_pmu_table)/sizeof(GX_CLOCK_TABLE); i++)
		gx_clock_set_module(&clk_pmu_table[i]);

	for (i = 0; i < sizeof(dto_pmu_table)/sizeof(GX_CLOCK_DTO_TABLE); i++)
		gx_clock_dto_config(GX_REG_BASE_PMU_CONFIG, &dto_pmu_table[i]);

	for (i = 0; i < sizeof(div_pmu_table)/sizeof(GX_CLOCK_DIV_TABLE); i++)
		gx_clock_div_config(GX_REG_BASE_PMU_CONFIG, &div_pmu_table[i]);

}

static void _clk_mcu_init(void)
{
	int i;
	for (i = 0; i < sizeof(clk_mcu_table)/sizeof(GX_CLOCK_TABLE); i++)
		gx_clock_set_module(&clk_mcu_table[i]);

	for (i = 0; i < sizeof(dto_mcu_table)/sizeof(GX_CLOCK_DTO_TABLE); i++)
		gx_clock_dto_config(GX_REG_BASE_MCU_CONFIG, &dto_mcu_table[i]);

	for (i = 0; i < sizeof(div_mcu_table)/sizeof(GX_CLOCK_DIV_TABLE); i++)
		gx_clock_div_config(GX_REG_BASE_MCU_CONFIG, &div_mcu_table[i]);
}

/**********************************************************/
void clk_init(void)
{
	int wakeup_from;

	switch (gx_pmu_get_wakeup_source()) {
	case GX_WAKEUP_SOURCE_COLD:
	case GX_WAKEUP_SOURCE_WDT:
		{
			wakeup_from = GX_PMU_WAKEUP_FROM_ROM;
			break;
		}
	case GX_WAKEUP_SOURCE_GPIO:
	case GX_WAKEUP_SOURCE_AUDIO_IN:
	case GX_WAKEUP_SOURCE_RTC:
	case GX_WAKEUP_SOURCE_I2C:
		{
			GX_PMU_WAKEUP_ADDR wakeup_addr;
			gx_pmu_ctrl_get(GX_PMU_CMD_WAKEUP_ADDR, &wakeup_addr);
			wakeup_from = wakeup_addr.wakeup_from;
			break;
		}
	default:
		wakeup_from = GX_PMU_WAKEUP_FROM_ROM;
		break;
	}

	if (wakeup_from == GX_PMU_WAKEUP_FROM_ROM) {
		_clk_pmu_init();
	}
	else if ((wakeup_from == GX_PMU_WAKEUP_FROM_SRAM)) {
		
		int i;
		GX_CLOCK_TABLE gx_osc_table[] = {
		/*       id            source      source0            source1     */
            {CLOCK_OSC_PLL     , 0}, /* CLOCK_32K_PLL    | CLOCK_1M       */
            {CLOCK_PLL_DTO     , 0}, /* CLOCK_OSC_PLL    | CLOCK_PIN_IN   */
            {CLOCK_HIGH        , 1}, /* CLOCK_24M        | CLOCK_PLL_DTO  */

		};
		for (i = 0; i < sizeof(gx_osc_table)/sizeof(GX_CLOCK_TABLE); i++) {
			gx_clock_set_source(&gx_osc_table[i]);
		}
		static GX_CLOCK_TABLE clk_table[] = {
		/*       id            source    source0      source1     source2  */
			{CLOCK_RTC         , 2}, /* CLOCK_LOW | CLOCK_HIGH | CLOCK_32K */
			{CLOCK_PMU         , 0}, /* CLOCK_LOW | CLOCK_HIGH | CLOCK_32K */
			{CLOCK_OSC_REF     , 1}, /* CLOCK_LOW | CLOCK_HIGH | CLOCK_32K */
			{CLOCK_FFT         , 0}, /* CLOCK_LOW | CLOCK_HIGH             */
			{CLOCK_GPIO        , 1}, /* CLOCK_LOW | CLOCK_HIGH             */
			{CLOCK_HW_I2C      , 0}, /* CLOCK_LOW | CLOCK_HIGH             */
			{CLOCK_SRAM        , 1}, /* CLOCK_LOW | CLOCK_HIGH | CLOCK_32K */
			{CLOCK_AUDIO_IN_SYS, 1}, /* CLOCK_LOW | CLOCK_HIGH | CLOCK_32K */

			{CLOCK_AUDIO_IN_ADC, 1}, /* CLOCK_AUDIO_IN_SYS | CLOCK_32K     */
			{CLOCK_AUDIO_IN_PDM, 1}, /* CLOCK_AUDIO_IN_SYS | CLOCK_OSC_1M  */
		};

		for (i = 0; i < ARRAY_SIZE(clk_table); i++) {
			gx_clock_set_module(&clk_table[i]);
		}

		GX_CLOCK_DIV_TABLE gx_div_pmu_table[] = {
		/*            addr           offs mask   div              */
            {GX_CLOCK_PMU_DIV_CONFIG0,  8,    0,  0x3f,   4},  // RTC
            {GX_CLOCK_PMU_DIV_CONFIG1, 24,    9,  0x3f,   24},  // GPIO
		};

		for (i = 0; i < sizeof(gx_div_pmu_table)/sizeof(GX_CLOCK_DIV_TABLE); i++) {
			gx_clock_div_config(GX_REG_BASE_PMU_CONFIG, &gx_div_pmu_table[i]);
		}

		gx_clock_module_enable(CLOCK_AUDIO_PLAY,   0);
		gx_clock_module_enable(CLOCK_I2C0_I2C1,    0);
		gx_clock_module_enable(CLOCK_I2C0,         0);
		gx_clock_module_enable(CLOCK_I2C1,         0);
		gx_clock_module_enable(CLOCK_NPU,          0);
		gx_clock_module_enable(CLOCK_TIMER_WDT,    0);
		gx_clock_module_enable(CLOCK_TIMER,        0);
		gx_clock_module_enable(CLOCK_WDT,          0);
		gx_clock_module_enable(CLOCK_DMA,          0);
		gx_clock_module_enable(CLOCK_FLASH_SPI,    0);
		gx_clock_module_enable(CLOCK_GENERAL_SPI,  0);
		gx_clock_module_enable(CLOCK_AUDIO_LODAC,  0);
		gx_clock_module_enable(CLOCK_UART1,        0);
	}
	_clk_mcu_init();

	gx_clock_module_enable(CLOCK_HW_I2C, 1);
	//close 32k xtal
	*(volatile int*)0xa0005084 &= ~(1<<0);

	// flash power down
	//*(volatile int*)0xa000003c = 1;

	writel(0x59, 0x40 + GX_REG_BASE_HW_I2C);
	writel(0x59, 0x44 + GX_REG_BASE_HW_I2C);
	writel(0x59, 0x48 + GX_REG_BASE_HW_I2C);
	writel(0x59, 0x4c + GX_REG_BASE_HW_I2C);

	GX_PMU_TRIM_CFG cfg;
	cfg.bits.trim_val = 0;
	cfg.bits.trim_en  = 1;
	gx_pmu_ctrl_set(GX_PMU_CMD_WORK_LDO_TRIM, &cfg);

}

void board_init(void)
{
    GX_WAKEUP_SOURCE start_mode = gx_pmu_get_wakeup_source();
    if (start_mode == GX_WAKEUP_SOURCE_COLD || start_mode == GX_WAKEUP_SOURCE_WDT) {
        extern void clear_bss(void);
        clear_bss();
    }
    else {
#ifdef CONFIG_MCU_ENABLE_XIP
        extern GX_FLASH_DEV * xip_sflash_init(void);
        xip_sflash_init();
#endif
    }
    extern void gx_analog_config_update_enable(void);
    gx_analog_config_update_enable();
}

