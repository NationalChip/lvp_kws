#include <autoconf.h>
#include <common.h>
#include <soc.h>
#include <board_config.h>
#include <spl/spl.h>
#include <clk_priv.h>
#include <base_addr.h>
#include <driver/grus_cfg.h>

static GX_CLOCK_SOURCE_TABLE clock_source_table[] = {
/*      source      sel               < 0 >              < 1 >       */
	{CLOCK_SOURCE_1M_12M  , 0}, /* CLOCK_SOURCE_OSC_24M    | CLOCK_SOURCE_1M       */
	{CLOCK_SOURCE_32K     , 0}, /* CLOCK_SOURCE_OSC_32K    | CLOCK_SOURCE_XTAL_32K */
	{CLOCK_SOURCE_24M     , 0}, /* CLOCK_SOURCE_OSC_24M    | CLOCK_SOURCE_PIN_IN   */
	{CLOCK_SOURCE_24M_PLL , 0}, /* CLOCK_SOURCE_24M        | CLOCK_SOURCE_PLL_DTO  */

	{CLOCK_SOURCE_32K_PLL , 0}, /* CLOCK_SOURCE_OSC_32K    | CLOCK_SOURCE_XTAL_32K */
	{CLOCK_SOURCE_1M_INPUT, 0}, /* CLOCK_SOURCE_PDM_IN     | CLOCK_SOURCE_PWM_IN   */
	{CLOCK_SOURCE_1M      , 0}, /* CLOCK_SOURCE_OSC_1M     | CLOCK_SOURCE_1M_INPUT */
	{CLOCK_SOURCE_OSC_PLL , 0}, /* CLOCK_SOURCE_32K_PLL    | CLOCK_SOURCE_1M       */

	{CLOCK_SOURCE_PIN_IN  , 0}, /* CLOCK_SOURCE_AUDIO_MCLK | CLOCK_SOURCE_PWM_IN   */
	{CLOCK_SOURCE_PLL_DTO , 0}, /* CLOCK_SOURCE_OSC_PLL    | CLOCK_SOURCE_PIN_IN   */
};

void spl_clk_set_source(GX_CLOCK_SOURCE_TABLE *source_table)
{
	_clk_set_source(source_table);
}

int spl_clk_set_module_source(GX_CLOCK_MODULE module, GX_CLOCK_MODULE_SOURCE source)
{
	return _clk_set_module(module, source);
}

void spl_clk_set_div(GX_CLOCK_MODULE module, unsigned int div)
{
	_clk_set_div(module, div);
}

void spl_clk_set_dto(GX_CLOCK_MODULE module, unsigned int dto, int enable)
{
	_clk_set_dto(module, dto, enable);
}

int spl_clk_set_pll_no_block(GX_CLOCK_PLL *pll, unsigned int timeout_ms)
{
	if (pll->pll_enable == 0)
		return 0;

	_clk_set_pll(pll);

	spl_delay_init();
	unsigned int start_time = spl_get_time_ms();
	while((*(volatile unsigned int*)(GX_REG_BASE_HW_I2C+0x98) & (1 << 3)) == 0) {
		if ((timeout_ms >= 0) && ((spl_get_time_ms() - start_time) > timeout_ms))
			return -1;
	}

	return 0;
}

void spl_clk_set_pll(GX_CLOCK_PLL *pll)
{
	if (pll->pll_enable == 0)
		return;

	_clk_set_pll(pll);

	while(!(*(volatile unsigned int*)(GX_REG_BASE_HW_I2C+0x98) & (1 << 3)));
}

void spl_clk_set_gate_enable(GX_CLOCK_MODULE module, unsigned int enable)
{
	_clk_set_high_gate(module, enable);

	_clk_set_all_gate(module, enable);
}

int spl_clk_get_frequence(GX_CLOCK_MODULE module)
{
	return _clk_get_module_frequence(module);
}

static void _clk_init(void)
{
	int i;
	for (i = 0; i < sizeof(clock_source_table)/sizeof(GX_CLOCK_SOURCE_TABLE); i++)
		spl_clk_set_source(&clock_source_table[i]);

	for (i = 0; i < CLOCK_MODULE_MAX; i++)
		spl_clk_set_module_source(i, MODULE_SOURCE_1M_12M);
}

static void _gate_init(void)
{
	*(volatile unsigned int*)PMU_CFG_SRAM_GATE_ADDR = 1; //sram低功耗模式时关闭时钟
	*(volatile unsigned int*)PMU_CFG_CLK_GATE &= ~(1<<0); //pmu cfg时钟低功耗时关闭
	*(volatile unsigned int*)(GX_REG_BASE_HW_I2C+0x84) &= ~(1<<0); //关闭32k xtal
	*(volatile unsigned int*)(GX_REG_BASE_HW_I2C+0xc8) = 0; //test i2c低功耗模式时关闭时钟

	for (int i = CLOCK_MODULE_RTC; i < CLOCK_MODULE_MAX; i++) {
		switch (i) {
		case CLOCK_MODULE_PMU:
		case CLOCK_MODULE_HW_I2C:
		case CLOCK_MODULE_SRAM:
		case CLOCK_MODULE_SCPU:
		case CLOCK_MODULE_UART0:
			spl_clk_set_gate_enable(i, 1);
			break;
		default:
			spl_clk_set_gate_enable(i, 0);
			break;
		}
	}
}

static inline void _osc_set_32k_trim_value(unsigned int trim_value)
{
	writel(trim_value, GX_REG_BASE_OSC + 0x24);
}

static inline void _osc_set_1m_trim_value(unsigned int trim_value)
{
	writel(trim_value, GX_REG_BASE_OSC + 0x28);
}

static inline void _osc_set_24m_trim_value(unsigned int trim_value)
{
	writel(trim_value, GX_REG_BASE_OSC + 0x2C);
}

extern int sflash_readdata(unsigned int offset, void *to, unsigned int len);
static int _spl_clk_get_trim(uint32_t *trim_32k_value,
		uint32_t *trim_1m_value, uint32_t *trim_24m_value)
{
	struct grus_cfg cfg __attribute__ ((aligned(DCACHE_LINE_SIZE)));
	unsigned char xor_result;
	unsigned char *pcfg = (unsigned char *)&cfg;
	int i = 0;

	sflash_readdata(GRUS_CFG_ADDR, (void *)&cfg, sizeof(struct grus_cfg));

	if (cfg.magic != GRUS_CFG_MAGIC)
		return -1;

	xor_result = pcfg[0];
	for (i = 1; i < sizeof(struct grus_cfg)-GRUS_CFG_XOR_LEN; i++)
		xor_result ^= pcfg[i];

	if (xor_result != cfg.xor_result) {
		return -1;
	}

	*trim_32k_value = cfg.trim_32k_value;
	*trim_1m_value = cfg.trim_1m_value;
	*trim_24m_value = cfg.trim_24m_value;

	return 0;
}

static int _spl_clk_switch_low_clk(void)
{
	int i = 0;
	static GX_CLOCK_SOURCE_TABLE clk_low_table[] = {
	/*       id            source      source0            source1     */
		{CLOCK_SOURCE_1M          , 0}, /* CLOCK_OSC_1M     | CLOCK_1M_INPUT */
		{CLOCK_SOURCE_1M_12M      , 1}, /* CLOCK_OSC_24M    | CLOCK_1M       */
	};

	for (i = 0; i < ARRAY_SIZE(clk_low_table); i++)
		spl_clk_set_source(&clk_low_table[i]);

	return 0;
}

static int _spl_clk_switch_high_clk(void)
{
	int i = 0;
	static GX_CLOCK_SOURCE_TABLE clk_src_high_table[] = {
	/*       id            source      source0            source1     */
		{CLOCK_SOURCE_24M         , 0}, /* CLOCK_OSC_24M    | CLOCK_PIN_IN   */
		{CLOCK_SOURCE_24M_PLL     , 0}, /* CLOCK_24M        | CLOCK_PLL_DTO  */
	};

	for (i = 0; i < ARRAY_SIZE(clk_src_high_table); i++)
		spl_clk_set_source(&clk_src_high_table[i]);

	spl_clk_set_module_source(CLOCK_MODULE_SCPU, MODULE_SOURCE_24M_PLL);
	spl_clk_set_module_source(CLOCK_MODULE_SRAM, MODULE_SOURCE_24M_PLL);
	spl_clk_set_module_source(CLOCK_MODULE_FLASH_SPI, MODULE_SOURCE_24M_PLL);

	return 0;
}

/**********************************************************/
int spl_clk_trim(void)
{
	int ret;
	unsigned int trim_32k_value;
	unsigned int trim_1m_value;
	unsigned int trim_24m_value;

	spl_clk_set_gate_enable(CLOCK_MODULE_OSC_REF, 1);
	ret = _spl_clk_get_trim(&trim_32k_value, &trim_1m_value, &trim_24m_value);
	if (ret != 0) {
		return -1;
	}

	_osc_set_32k_trim_value(trim_32k_value);
	_osc_set_1m_trim_value(trim_1m_value);
	_spl_clk_switch_low_clk();

	_osc_set_24m_trim_value(trim_24m_value);
	_spl_clk_switch_high_clk();
	spl_clk_set_gate_enable(CLOCK_MODULE_OSC_REF, 0);

	return 0;
}

void spl_clk_init(void)
{
	spl_osc_set_32k_trim_state(0);
	spl_osc_set_all_trim_state(0);

	int i = 0;
	for (i = 0; i < CLOCK_MODULE_MAX; i++) {
		spl_clk_set_div(i, 0);
		spl_clk_set_dto(i, 0, 0);
	}

	_clk_init();

	spl_board_clk_init();

	_gate_init();
}
