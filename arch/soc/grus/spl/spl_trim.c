#include <common.h>
#include <base_addr.h>
#include <soc.h>

#include "spl_osc.h"

#define REF_CLK_FREQ 24576000

#define TRIM_32K_TARGET_CYCLES 8      // 32k 目标cycles
#define TRIM_32K_TOLERANCE     10    // 32k 目标cycles和参考cycles容差

volatile static int spl_manual_trim_32k_done = -1;

static GX_OSC_TRIM_CONFIG osc_trim_32k_manual_config= {
	.type = GX_OSC_CLK_TYPE_32K,
	.mode = GX_OSC_TRIM_MODE_MANUAL,
	.target_cycles = TRIM_32K_TARGET_CYCLES,
	.ref_cycles = GX_OSC_REF_CYCLE(REF_CLK_FREQ, GX_OSC_CLOCK_32K, TRIM_32K_TARGET_CYCLES),
};

static int osc_32k_manual_retrim(void *priv)
{
	int result;
	GX_OSC_TRIM_CONFIG *trim_config = priv;
	GX_OSC_TRIM trim;
	GX_OSC_CURRENT_CYCLE current_cycle;

	trim.type = GX_OSC_CLK_TYPE_32K;
	current_cycle.type = GX_OSC_CLK_TYPE_32K;

	spl_pmu_osc_get(GX_OSC_CMD_TRIM_VALUE, &trim);
	spl_pmu_osc_get(GX_OSC_CMD_CURRENT_CYCLE, &current_cycle);
	result = spl_osc_cycles_compare(trim_config->ref_cycles, current_cycle.cycles, TRIM_32K_TOLERANCE);

	if (result == 0) {
		spl_manual_trim_32k_done = 1;
	} else if (result > 0) {
		if (trim.trim_value < GX_OSC_32K_VALUE_MAX) {
			trim.trim_value++;
		} else {
			spl_manual_trim_32k_done = 2;
			return 0;
		}

		spl_pmu_osc_set(GX_OSC_CMD_TRIM_VALUE, &trim);
		spl_pmu_osc_trim_enable(trim.type);

	} else if (result < 0) {
		if (trim.trim_value != 0) {
			trim.trim_value--;
		} else {
			spl_manual_trim_32k_done = 3;
			return 0;
		}

		spl_pmu_osc_set(GX_OSC_CMD_TRIM_VALUE, &trim);
		spl_pmu_osc_trim_enable(trim.type);
	}

	return 0;
}

int spl_osc_32k_manual_trim(void)
{
	spl_pmu_osc_trim_init();

	spl_pmu_osc_set(GX_OSC_CMD_TRIM_CONFIG, &osc_trim_32k_manual_config);
	spl_pmu_osc_enable(GX_OSC_CLK_TYPE_32K);
	spl_pmu_osc_trim_enable(GX_OSC_CLK_TYPE_32K);

	while (spl_pmu_osc_trim_done(GX_OSC_CLK_TYPE_32K)) {
		osc_32k_manual_retrim(&osc_trim_32k_manual_config);
		if (spl_manual_trim_32k_done == 1)
			return 0;
		else if (spl_manual_trim_32k_done > 1)
			return -1;
	}

	return 0;
}
