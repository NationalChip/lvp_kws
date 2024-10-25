#ifndef __SPL_OSC_H__
#define __SPL_OSC_H__
#include <driver/gx_pmu_osc.h>

/*
 * 判断设置参考时钟和实际参考时钟cycle是否在容差范围内
 * input:
 *    expected_cycles - 预期时钟cycles
 *    current_cycles - 当前时钟cycles
 *    tolerance       - 容差值
 * output:
 *    0 : abs(expected_cycles - current_cycles) <= tolerance
 *    1 : expected_cycles > current_cycles + tolerance
 *   -1 : current_cycles > expected_cycles + tolerance
 */
int spl_osc_cycles_compare(uint16_t expected_cycles, uint16_t current_cycles, uint16_t tolerance);

int spl_pmu_osc_get(GX_OSC_CMD cmd, void *data);
int spl_pmu_osc_set(GX_OSC_CMD cmd, void *data);
int spl_pmu_osc_enable(GX_OSC_CLK_TYPE type);
int spl_pmu_osc_disable(GX_OSC_CLK_TYPE type);
int spl_pmu_osc_trim_enable(GX_OSC_CLK_TYPE type);
int spl_pmu_osc_trim_init(void);
int spl_pmu_osc_trim_done(GX_OSC_CLK_TYPE type);

#endif