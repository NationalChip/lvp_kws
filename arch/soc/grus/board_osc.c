#include <common.h>
#include <driver/gx_pmu_osc.h>
#include <spl/spl.h>

#if 0
#define osc_printf(...) printf(__VA_ARGS__)
#else
#define osc_printf(...)
#endif

#define REF_CLK_FREQ 24576000

#define TRIM_32K_TARGET_CYCLES 8      // 32k 目标cycles
#define TRIM_1M_TARGET_CYCLES  10     // 1M 目标cycles
#define TRIM_24M_TARGET_CYCLES 10000  // 24M 目标cycles

#define TRIM_32K_TOLERANCE     10    // 32k 目标cycles和参考cycles容差
#define TRIM_1M_TOLERANCE      5     // 1M 目标cycles和参考cycles容差
#define TRIM_24M_TOLERANCE     200   // 2M 目标cycles和参考cycles容差


volatile static int auto_trim_32k_done = -1;
volatile static int auto_trim_1m_done = -1;
volatile static int auto_trim_24m_done = -1;

volatile static int manual_trim_32k_done = -1;
volatile static int manual_trim_1m_done = -1;
volatile static int manual_trim_24m_done = -1;

static uint32_t trim_32k_value = 0;
static uint32_t trim_1m_value = 0;
static uint32_t trim_24m_value = 0;

static int osc_32k_auto_trim_callback(int irq, void *priv);
static int osc_1m_auto_trim_callback(int irq, void *priv);
static int osc_24m_auto_trim_callback(int irq, void *priv);

static GX_OSC_TRIM_CONFIG osc_trim_32k_auto_config= {
	.type = GX_OSC_CLK_TYPE_32K,
	.mode = GX_OSC_TRIM_MODE_AUTO,
	.target_cycles = TRIM_32K_TARGET_CYCLES,
	.ref_cycles = GX_OSC_REF_CYCLE(REF_CLK_FREQ, GX_OSC_CLOCK_32K, TRIM_32K_TARGET_CYCLES),
	.callback = osc_32k_auto_trim_callback,
	.priv = &osc_trim_32k_auto_config,
};

static GX_OSC_TRIM_CONFIG osc_trim_1m_auto_config= {
	.type = GX_OSC_CLK_TYPE_1M,
	.mode = GX_OSC_TRIM_MODE_AUTO,
	.target_cycles = TRIM_1M_TARGET_CYCLES,
	.ref_cycles = GX_OSC_REF_CYCLE(REF_CLK_FREQ, GX_OSC_CLOCK_1M, TRIM_1M_TARGET_CYCLES),
	.callback = osc_1m_auto_trim_callback,
	.priv = &osc_trim_1m_auto_config,
};

static GX_OSC_TRIM_CONFIG osc_trim_24m_auto_config= {
	.type = GX_OSC_CLK_TYPE_24M,
	.mode = GX_OSC_TRIM_MODE_AUTO,
	.target_cycles = TRIM_24M_TARGET_CYCLES,
	.ref_cycles = GX_OSC_REF_CYCLE(REF_CLK_FREQ, GX_OSC_CLOCK_24M, TRIM_24M_TARGET_CYCLES),
	.callback = osc_24m_auto_trim_callback,
	.priv = &osc_trim_24m_auto_config,
};

static GX_OSC_TRIM_CONFIG osc_trim_32k_manual_config= {
	.type = GX_OSC_CLK_TYPE_32K,
	.mode = GX_OSC_TRIM_MODE_MANUAL,
	.target_cycles = TRIM_32K_TARGET_CYCLES,
	.ref_cycles = GX_OSC_REF_CYCLE(REF_CLK_FREQ, GX_OSC_CLOCK_32K, TRIM_32K_TARGET_CYCLES),
	.callback = NULL,
	.priv = &osc_trim_32k_manual_config,
};

static GX_OSC_TRIM_CONFIG osc_trim_1m_manual_config= {
	.type = GX_OSC_CLK_TYPE_1M,
	.mode = GX_OSC_TRIM_MODE_MANUAL,
	.target_cycles = TRIM_1M_TARGET_CYCLES,
	.ref_cycles = GX_OSC_REF_CYCLE(REF_CLK_FREQ, GX_OSC_CLOCK_1M, TRIM_1M_TARGET_CYCLES),
	.callback = NULL,
	.priv = &osc_trim_1m_manual_config,
};

static GX_OSC_TRIM_CONFIG osc_trim_24m_manual_config= {
	.type = GX_OSC_CLK_TYPE_24M,
	.mode = GX_OSC_TRIM_MODE_MANUAL,
	.target_cycles = TRIM_24M_TARGET_CYCLES,
	.ref_cycles = GX_OSC_REF_CYCLE(REF_CLK_FREQ, GX_OSC_CLOCK_24M, TRIM_24M_TARGET_CYCLES),
	.callback = NULL,
	.priv = &osc_trim_24m_manual_config,
};

static int osc_32k_auto_trim_callback(int irq, void *priv)
{
	GX_OSC_TRIM trim;
	GX_OSC_CURRENT_CYCLE current_cycle;
	GX_OSC_TRIM_CONFIG *tirm_config = priv;

	trim.type = GX_OSC_CLK_TYPE_32K;
	current_cycle.type = GX_OSC_CLK_TYPE_32K;

	gx_pmu_osc_get(GX_OSC_CMD_TRIM_VALUE, &trim);
	gx_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &current_cycle);

	if (tirm_config->int_status & GX_OSC_INT_32K_DONE_MASK) {
		osc_printf("osc 32k auto trim done! trim value %d, current cycles %d, target_cycles %d\n",
			trim.trim_value, current_cycle.cycles, osc_trim_32k_auto_config.ref_cycles);
		trim_32k_value = trim.trim_value;
		auto_trim_32k_done = 1;
	}

	if (tirm_config->int_status & GX_OSC_INT_32K_ERROR_MASK) {
		osc_printf("osc 32k trim error! trim value %d, current cycles %d, target_cycles %d\n",
			trim.trim_value, current_cycle.cycles, osc_trim_32k_auto_config.ref_cycles);
		auto_trim_32k_done = 2;
	}

	return 0;
}

static int osc_1m_auto_trim_callback(int irq, void *priv)
{
	GX_OSC_TRIM trim;
	GX_OSC_CURRENT_CYCLE current_cycle;
	GX_OSC_TRIM_CONFIG *tirm_config = priv;

	trim.type = GX_OSC_CLK_TYPE_1M;
	current_cycle.type = GX_OSC_CLK_TYPE_1M;

	gx_pmu_osc_get(GX_OSC_CMD_TRIM_VALUE, &trim);
	gx_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &current_cycle);

	if (tirm_config->int_status & GX_OSC_INT_1M_DONE_MASK) {
		osc_printf("osc 1m auto trim done! trim value %d, current cycles %d, target_cycles %d\n",
			trim.trim_value, current_cycle.cycles, osc_trim_1m_auto_config.ref_cycles);
		trim_1m_value = trim.trim_value;
		auto_trim_1m_done = 1;
	}

	if (tirm_config->int_status & GX_OSC_INT_1M_ERROR_MASK) {
		osc_printf("osc 1m auto trim error! trim value %d, current cycles %d, target_cycles %d\n",
			trim.trim_value, current_cycle.cycles, osc_trim_1m_auto_config.ref_cycles);
		auto_trim_1m_done = 2;
	}

	return 0;
}

static int osc_24m_auto_trim_callback(int irq, void *priv)
{
	GX_OSC_TRIM trim;
	GX_OSC_CURRENT_CYCLE current_cycle;
	GX_OSC_TRIM_CONFIG *tirm_config = priv;

	trim.type = GX_OSC_CLK_TYPE_24M;
	current_cycle.type = GX_OSC_CLK_TYPE_24M;

	gx_pmu_osc_get(GX_OSC_CMD_TRIM_VALUE, &trim);
	gx_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &current_cycle);

	if (tirm_config->int_status & GX_OSC_INT_24M_DONE_MASK) {
		osc_printf("osc 24m auto trim done! trim value %d, current cycles %d, target_cycles %d\n",
			trim.trim_value, current_cycle.cycles, osc_trim_24m_auto_config.target_cycles);
		trim_24m_value = trim.trim_value;
		auto_trim_24m_done = 1;
	}

	if (tirm_config->int_status & GX_OSC_INT_24M_ERROR_MASK) {
		osc_printf("osc 24m auto trim error! trim value %d, current cycles %d, target_cycles %d\n",
			trim.trim_value, current_cycle.cycles, osc_trim_24m_auto_config.target_cycles);
		auto_trim_24m_done = 2;
	}

	return 0;
}

static int osc_32k_fine_trim_callback(int irq, void *priv)
{
	osc_printf("32k fine trim done!\n");
	manual_trim_32k_done = 1;
	return 0;
}

static int osc_1m_fine_trim_callback(int irq, void *priv)
{
	osc_printf("1m fine trim done!\n");
	manual_trim_1m_done = 1;
	return 0;
}

static int osc_24m_fine_trim_callback(int irq, void *priv)
{
	osc_printf("24m fine trim done!\n");
	manual_trim_24m_done = 1;
	return 0;
}

static int pmu_osc_32k_auto_trim(void)
{
	gx_pmu_osc_set(GX_OSC_CMD_TRIM_CONFIG, &osc_trim_32k_auto_config);
	gx_pmu_osc_enable(GX_OSC_CLK_TYPE_32K);
	gx_pmu_osc_trim_enable(GX_OSC_CLK_TYPE_32K);

	return 0;
}

static int pmu_osc_1m_auto_trim(void)
{
	gx_pmu_osc_set(GX_OSC_CMD_TRIM_CONFIG, &osc_trim_1m_auto_config);
	gx_pmu_osc_enable(GX_OSC_CLK_TYPE_1M);
	gx_pmu_osc_trim_enable(GX_OSC_CLK_TYPE_1M);

	return 0;
}

static int pmu_osc_24m_auto_trim(void)
{
	gx_pmu_osc_set(GX_OSC_CMD_TRIM_CONFIG, &osc_trim_24m_auto_config);
	gx_pmu_osc_enable(GX_OSC_CLK_TYPE_24M);
	gx_pmu_osc_trim_enable(GX_OSC_CLK_TYPE_24M);

	return 0;
}

static int pmu_osc_32k_manual_trim(void)
{
	gx_pmu_osc_set(GX_OSC_CMD_TRIM_CONFIG, &osc_trim_32k_manual_config);
	gx_pmu_osc_enable(GX_OSC_CLK_TYPE_32K);
	gx_pmu_osc_trim_enable(GX_OSC_CLK_TYPE_32K);

	return 0;
}

static int pmu_osc_1m_manual_trim(void)
{
	gx_pmu_osc_set(GX_OSC_CMD_TRIM_CONFIG, &osc_trim_1m_manual_config);
	gx_pmu_osc_enable(GX_OSC_CLK_TYPE_1M);
	gx_pmu_osc_trim_enable(GX_OSC_CLK_TYPE_1M);

	return 0;
}

static int pmu_osc_24m_manual_trim(void)
{
	gx_pmu_osc_set(GX_OSC_CMD_TRIM_CONFIG, &osc_trim_24m_manual_config);
	gx_pmu_osc_enable(GX_OSC_CLK_TYPE_24M);
	gx_pmu_osc_trim_enable(GX_OSC_CLK_TYPE_24M);

	return 0;
}

static uint32_t pmu_osc_get_closest_cycle(uint32_t target, uint32_t current, uint32_t up, uint32_t down)
{
	uint32_t tc = max(target, current) - min(target, current);
	uint32_t tu = max(target, up) - min(target, up);
	uint32_t td = max(target, down) - min(target, down);
	uint32_t min_val = min(min(tc, td), tu);

	osc_printf("target %d, current %d, up %d, down %d\n",
		target, current, up, down);
	osc_printf("tc %d, tu %d, td %d, min_val %d\n", tc, tu, td, min_val);
	if (min_val == tc)
		return current;
	else if (min_val == tu)
		return up;
	else if (min_val == td)
		return down;

	return current;
}

static int pmu_osc_32k_fine_triming(void)
{
	uint32_t current_32k_trim_value = trim_32k_value;
	uint32_t target_cycle = osc_trim_32k_auto_config.ref_cycles; // 32k 对比的是参考时钟
	GX_OSC_TRIM trim;
	GX_OSC_CURRENT_CYCLE current_cycle, round_up_cycle, round_down_cycle;
	uint32_t closest_cycle;

	trim.type = GX_OSC_CLK_TYPE_32K;
	current_cycle.type = GX_OSC_CLK_TYPE_32K;
	round_up_cycle.type = GX_OSC_CLK_TYPE_32K;
	round_down_cycle.type = GX_OSC_CLK_TYPE_32K;

	osc_trim_32k_manual_config.callback = osc_32k_fine_trim_callback;

	// 获取自动trim后的cycle
	gx_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &current_cycle);

	// trim 值增加， 手动 trim 一次
	manual_trim_32k_done = 0;
	trim.trim_value = current_32k_trim_value + 1;
	gx_pmu_osc_set(GX_OSC_CMD_TRIM_VALUE, &trim);
	pmu_osc_32k_manual_trim();
	while (manual_trim_32k_done == 0)
		continue;
	gx_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &round_up_cycle);

	// trim 值减小， 手动 trim 一次
	manual_trim_32k_done = 0;
	trim.trim_value = current_32k_trim_value - 1;
	gx_pmu_osc_set(GX_OSC_CMD_TRIM_VALUE, &trim);
	pmu_osc_32k_manual_trim();
	while (manual_trim_32k_done == 0)
		continue;
	gx_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &round_down_cycle);

	closest_cycle = pmu_osc_get_closest_cycle(target_cycle, current_cycle.cycles,
		round_up_cycle.cycles, round_down_cycle.cycles);

	osc_printf("32k target cycle %d, current cycle %d, up cycle %d, down cycle %d, closest cycle %d\n",
		target_cycle, current_cycle.cycles, round_up_cycle.cycles,
		round_down_cycle.cycles, closest_cycle);

	if (closest_cycle == round_down_cycle.cycles) {
		osc_printf("trim 32k value - 1\n");
		trim_32k_value -= 1;
	} else if (closest_cycle == round_up_cycle.cycles) {
		osc_printf("trim 32 value + 1\n");
		trim_32k_value += 1;
	}

	return 0;
}

static int pmu_osc_1m_fine_triming(void)
{
	uint32_t current_1m_trim_value = trim_1m_value;
	uint32_t target_cycle = osc_trim_1m_auto_config.ref_cycles;  // 1m 对比的是参考时钟
	GX_OSC_TRIM trim;
	GX_OSC_CURRENT_CYCLE current_cycle, round_up_cycle, round_down_cycle;
	uint32_t closest_cycle;

	trim.type = GX_OSC_CLK_TYPE_1M;
	current_cycle.type = GX_OSC_CLK_TYPE_1M;
	round_up_cycle.type = GX_OSC_CLK_TYPE_1M;
	round_down_cycle.type = GX_OSC_CLK_TYPE_1M;

	osc_trim_1m_manual_config.callback = osc_1m_fine_trim_callback;

	// 获取自动trim后的cycle
	gx_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &current_cycle);

	// trim 值增加， 手动 trim 一次
	manual_trim_1m_done = 0;
	trim.trim_value = current_1m_trim_value + 1;
	gx_pmu_osc_set(GX_OSC_CMD_TRIM_VALUE, &trim);
	pmu_osc_1m_manual_trim();
	while (manual_trim_1m_done == 0)
		continue;
	gx_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &round_up_cycle);

	// trim 值减小， 手动 trim 一次
	manual_trim_1m_done = 0;
	trim.trim_value = current_1m_trim_value - 1;
	gx_pmu_osc_set(GX_OSC_CMD_TRIM_VALUE, &trim);
	pmu_osc_1m_manual_trim();
	while (manual_trim_1m_done == 0)
		continue;
	gx_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &round_down_cycle);

	closest_cycle = pmu_osc_get_closest_cycle(target_cycle, current_cycle.cycles,
		round_up_cycle.cycles, round_down_cycle.cycles);

	osc_printf("1m target cycle %d, current cycle %d, up cycle %d, down cycle %d, closest cycle %d\n",
		target_cycle, current_cycle.cycles, round_up_cycle.cycles,
		round_down_cycle.cycles, closest_cycle);

	if (closest_cycle == round_down_cycle.cycles) {
		osc_printf("trim 1m value - 1\n");
		trim_1m_value -= 1;
	} else if (closest_cycle == round_up_cycle.cycles) {
		osc_printf("trim 1m value + 1\n");
		trim_1m_value += 1;
	}

	return 0;
}

static int pmu_osc_24m_fine_triming(void)
{
	uint32_t current_24m_trim_value = trim_24m_value;
	uint32_t target_cycle = osc_trim_24m_auto_config.target_cycles; // 24m 对比的是目标时钟
	GX_OSC_TRIM trim;
	GX_OSC_CURRENT_CYCLE current_cycle, round_up_cycle, round_down_cycle;
	uint32_t closest_cycle;

	trim.type = GX_OSC_CLK_TYPE_24M;
	current_cycle.type = GX_OSC_CLK_TYPE_24M;
	round_up_cycle.type = GX_OSC_CLK_TYPE_24M;
	round_down_cycle.type = GX_OSC_CLK_TYPE_24M;

	osc_trim_24m_manual_config.callback = osc_24m_fine_trim_callback;

	// 获取自动trim后的cycle
	gx_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &current_cycle);

	// trim 值增加， 手动 trim 一次
	manual_trim_24m_done = 0;
	trim.trim_value = current_24m_trim_value + 1;
	gx_pmu_osc_set(GX_OSC_CMD_TRIM_VALUE, &trim);
	pmu_osc_24m_manual_trim();
	while (manual_trim_24m_done == 0)
		continue;
	gx_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &round_up_cycle);

	// trim 值减小， 手动 trim 一次
	manual_trim_24m_done = 0;
	trim.trim_value = current_24m_trim_value - 1;
	gx_pmu_osc_set(GX_OSC_CMD_TRIM_VALUE, &trim);
	pmu_osc_24m_manual_trim();
	while (manual_trim_24m_done == 0)
		continue;
	gx_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &round_down_cycle);

	closest_cycle = pmu_osc_get_closest_cycle(target_cycle, current_cycle.cycles,
		round_up_cycle.cycles, round_down_cycle.cycles);

	osc_printf("24m target cycle %d, current cycle %d, up cycle %d, down cycle %d, closest cycle %d\n",
		target_cycle, current_cycle.cycles, round_up_cycle.cycles,
		round_down_cycle.cycles, closest_cycle);

	if (closest_cycle == round_down_cycle.cycles) {
		osc_printf("trim 24m value - 1\n");
		trim_24m_value -= 1;
	} else if (closest_cycle == round_up_cycle.cycles) {
		osc_printf("trim 24m value + 1\n");
		trim_24m_value += 1;
	}


	return 0;
}

static int pmu_osc_fine_triming(void)
{
	pmu_osc_32k_fine_triming();
	pmu_osc_1m_fine_triming();
	pmu_osc_24m_fine_triming();

	return 0;
}

static int pmu_osc_auto_trim(void)
{
	pmu_osc_32k_auto_trim();
	pmu_osc_1m_auto_trim();
	pmu_osc_24m_auto_trim();

	return 0;
}

int gx_pmu_do_osc_trim(void)
{
	gx_pmu_osc_trim_init();
	pmu_osc_auto_trim();

	while (1) {
		if (auto_trim_32k_done == 1 && auto_trim_1m_done == 1 && auto_trim_24m_done == 1) {
			pmu_osc_fine_triming();
			break;
		}
	}
	spl_osc_set_all_trim_state(1);
    return 0;
}
