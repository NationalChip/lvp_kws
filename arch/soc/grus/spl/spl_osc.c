#include <common.h>
#include <base_addr.h>
#include <soc.h>
#include <driver/gx_pmu_osc.h>
#include <driver/gx_clock.h>

// OSC 寄存器
#define OSC_ENABLE          ((GX_REG_BASE_OSC) + 0x00) // OSC 时钟使能
#define OSC_TRIM_CLK_ENABLE ((GX_REG_BASE_OSC) + 0x04) // OSC TRIM 校正时钟开关
#define OSC_TRIM_MODE       ((GX_REG_BASE_OSC) + 0x08) // OSC TRIM 模式
#define TAR_CLOCK32K_NUM    ((GX_REG_BASE_OSC) + 0x0C) // 32k 时钟个数
#define TAR_CLOCK1M_NUM     ((GX_REG_BASE_OSC) + 0x10) // 1M 时钟个数
#define TAR_CLOCK24M_NUM    ((GX_REG_BASE_OSC) + 0x14) // 24M 时钟个数
#define ACTUAL_CLOCK32K_NUM ((GX_REG_BASE_OSC) + 0x18) // 实际采样得到的32K时钟 cycle 数
#define ACTUAL_CLOCK1M_NUM  ((GX_REG_BASE_OSC) + 0x1C) // 实际采样得到的1M时钟 cycle 数
#define ACTUAL_CLOCK24M_NUM ((GX_REG_BASE_OSC) + 0x20) // 实际采样得到的24M时钟 cycle 数
#define OSC_32K_TRIM        ((GX_REG_BASE_OSC) + 0x24) // 32k TRIM 值
#define OSC_1M_TRIM         ((GX_REG_BASE_OSC) + 0x28) // 1M TRIM 值
#define OSC_24M_TRIM        ((GX_REG_BASE_OSC) + 0x2C) // 24M TRIM 值
#define TRIM_INT_EN         ((GX_REG_BASE_OSC) + 0x30) // 中断使能
#define TRIM_INT_STATUS     ((GX_REG_BASE_OSC) + 0x34) // 终端状态
#define OSC_TRIM_DELAY      ((GX_REG_BASE_OSC) + 0x38) // TRIM 后等待OSC稳定延时
#define OSC_TRIM_CLK_32K_EN ((GX_REG_BASE_OSC) + 0x3C) // 32k TRIM 使能
#define OSC_TRIM_CLK_1M_EN  ((GX_REG_BASE_OSC) + 0x40) // 1M TRIM 使能
#define OSC_TRIM_CLK_24M_EN ((GX_REG_BASE_OSC) + 0x44) // 24M TRIM 使能


#define REF_CLK_SHIFT    16
#define TARGET_CLK_MASK  0xFFFF

// TRIM 模式MASK
#define OSC_32K_TRIM_MODE_SHIFT 0
#define OSC_1M_TRIM_MODE_SHIFT  2
#define OSC_24M_TRIM_MODE_SHIFT 4
#define OSC_32K_TRIM_MODE_MASK  (0x3 << (OSC_32K_TRIM_MODE_SHIFT))
#define OSC_1M_TRIM_MODE_MASK   (0x3 << (OSC_1M_TRIM_MODE_SHIFT))
#define OSC_24M_TRIM_MODE_MASK  (0x3 << (OSC_24M_TRIM_MODE_SHIFT))

// TRIM 完成中断回调
struct osc_irq_info {
	irq_handler_t callback;
	void *priv;
};

static struct osc_irq_info osc_irq_info[GX_OSC_CLK_TYPE_MAX];

static int spl_pmu_osc_get_target_cycles(GX_OSC_CLK_TYPE type,
	uint16_t *ref_cycles, uint16_t *target_cycles)
{
	uint32_t tartet_clk;

	switch (type) {
		case GX_OSC_CLK_TYPE_32K:
			tartet_clk = readl(TAR_CLOCK32K_NUM);
			break;
		case GX_OSC_CLK_TYPE_1M:
			tartet_clk = readl(TAR_CLOCK1M_NUM);
			break;
		case GX_OSC_CLK_TYPE_24M:
			tartet_clk = readl(TAR_CLOCK24M_NUM);
			break;
		default:
			return -1;
	}

	*target_cycles = tartet_clk & TARGET_CLK_MASK;
	*ref_cycles = (tartet_clk >> REF_CLK_SHIFT) & TARGET_CLK_MASK;

	return 0;
}

static int spl_pmu_osc_set_target_cycles(GX_OSC_CLK_TYPE type,
	 uint16_t ref_cycles, uint16_t target_cycles)
{
	uint32_t clk_num;
	uint32_t ref_cycles_32 = ref_cycles;

	clk_num = (ref_cycles_32 << REF_CLK_SHIFT) | target_cycles;

	switch (type) {
		case GX_OSC_CLK_TYPE_32K:
			writel(clk_num, TAR_CLOCK32K_NUM);
			break;
		case GX_OSC_CLK_TYPE_1M:
			writel(clk_num, TAR_CLOCK1M_NUM);
			break;
		case GX_OSC_CLK_TYPE_24M:
			writel(clk_num, TAR_CLOCK24M_NUM);
			break;
		default:
			return -1;
			break;
	}

	return 0;
}

static int spl_pmu_osc_get_trim_mode(GX_OSC_CLK_TYPE type, GX_OSC_TRIM_MODE *mode)
{
	uint32_t trim_mode = readl(GX_OSC_TRIM_MODE_AUTO);

	switch (type) {
		case GX_OSC_CLK_TYPE_32K:
			trim_mode &= ~(OSC_32K_TRIM_MODE_MASK);
			*mode = trim_mode >> OSC_32K_TRIM_MODE_SHIFT;
			break;
		case GX_OSC_CLK_TYPE_1M:
			trim_mode &= ~(OSC_1M_TRIM_MODE_MASK);
			*mode = trim_mode >> OSC_1M_TRIM_MODE_SHIFT;
			break;
		case GX_OSC_CLK_TYPE_24M:
			trim_mode &= ~(OSC_24M_TRIM_MODE_MASK);
			*mode = trim_mode >> OSC_24M_TRIM_MODE_SHIFT;
			break;
		default:
			return -1;
			break;
	}

	return 0;
}

static int spl_pmu_osc_set_trim_mode(GX_OSC_CLK_TYPE type, GX_OSC_TRIM_MODE mode)
{
	uint32_t trim_mode = readl(OSC_TRIM_MODE);

	switch (type) {
		case GX_OSC_CLK_TYPE_32K:
			trim_mode &= ~(OSC_32K_TRIM_MODE_MASK);
			trim_mode |= mode << OSC_32K_TRIM_MODE_SHIFT;
			break;
		case GX_OSC_CLK_TYPE_1M:
			trim_mode &= ~(OSC_1M_TRIM_MODE_MASK);
			trim_mode |= mode << OSC_1M_TRIM_MODE_SHIFT;
			break;
		case GX_OSC_CLK_TYPE_24M:
			trim_mode &= ~(OSC_24M_TRIM_MODE_MASK);
			trim_mode |= mode << OSC_24M_TRIM_MODE_SHIFT;
			break;
		default:
			return -1;
			break;
	}
	writel(trim_mode, OSC_TRIM_MODE);

	return 0;
}

static int spl_pmu_osc_set_trim_callback(GX_OSC_CLK_TYPE type, irq_handler_t callback, void *priv)
{
	osc_irq_info[type].callback = callback;
	osc_irq_info[type].priv = priv;

	return 0;
}

static int spl_pmu_osc_get_trim_config(GX_OSC_TRIM_CONFIG *trim_config)
{
	spl_pmu_osc_get_trim_mode(trim_config->type, &trim_config->mode);
	spl_pmu_osc_get_target_cycles(trim_config->type, &trim_config->ref_cycles,
		 &trim_config->target_cycles);

	return 0;
}

static int spl_pmu_osc_set_trim_config(GX_OSC_TRIM_CONFIG *trim_config)
{
	spl_pmu_osc_set_trim_mode(trim_config->type, trim_config->mode);
	spl_pmu_osc_set_target_cycles(trim_config->type, trim_config->ref_cycles,
		trim_config->target_cycles);
	spl_pmu_osc_set_trim_callback(trim_config->type, trim_config->callback, trim_config->priv);

	return 0;
}

static int spl_pmu_osc_get_current_cycle(GX_OSC_CURRENT_CYCLE *clk)
{
	switch (clk->type) {
		case GX_OSC_CLK_TYPE_32K:
			clk->cycles = readl(ACTUAL_CLOCK32K_NUM);
			break;
		case GX_OSC_CLK_TYPE_1M:
			clk->cycles = readl(ACTUAL_CLOCK1M_NUM);
			break;
		case GX_OSC_CLK_TYPE_24M:
			clk->cycles = readl(ACTUAL_CLOCK24M_NUM);
			break;
		default:
			return -1;
	}

	return 0;
}

static int spl_pmu_osc_get_trim_value(GX_OSC_TRIM *trim)
{
	switch (trim->type) {
		case GX_OSC_CLK_TYPE_32K:
			trim->trim_value = readl(OSC_32K_TRIM);
			break;
		case GX_OSC_CLK_TYPE_1M:
			trim->trim_value = readl(OSC_1M_TRIM);
			break;
		case GX_OSC_CLK_TYPE_24M:
			trim->trim_value = readl(OSC_24M_TRIM);
			break;
		default:
			return -1;
	}
	return 0;
}

static int spl_pmu_osc_set_trim_value(GX_OSC_TRIM *trim)
{
	switch (trim->type) {
		case GX_OSC_CLK_TYPE_32K:
			writel(trim->trim_value, OSC_32K_TRIM);
			break;
		case GX_OSC_CLK_TYPE_1M:
			writel(trim->trim_value, OSC_1M_TRIM);
			break;
		case GX_OSC_CLK_TYPE_24M:
			writel(trim->trim_value, OSC_24M_TRIM);
			break;
		default:
			return -1;
	}
	return 0;
}

static int spl_pmu_osc_get_trim_delay(uint32_t *trim_delay)
{
	*trim_delay = readl(OSC_TRIM_DELAY);

	return 0;
}

static int spl_pmu_osc_set_trim_delay(uint32_t *trim_delay)
{
	writel(*trim_delay, OSC_TRIM_DELAY);

	return 0;
}

int spl_pmu_osc_get(GX_OSC_CMD cmd, void *data)
{
	int ret;

	switch (cmd) {
		case GX_OSC_CMD_TRIM_CONFIG:
			ret = spl_pmu_osc_get_trim_config(data);
			break;
		case GX_OSC_CMD_CURRENT_CYCLE:
			ret = spl_pmu_osc_get_current_cycle(data);
			break;
		case GX_OSC_CMD_TRIM_VALUE:
			ret = spl_pmu_osc_get_trim_value(data);
			break;
		case GX_OSC_CMD_TRIM_DELAY:
			ret = spl_pmu_osc_get_trim_delay(data);
		default:
			return -1;
			break;
	}

	return ret;
}

int spl_pmu_osc_set(GX_OSC_CMD cmd, void *data)
{
	int ret;

	switch (cmd) {
		case GX_OSC_CMD_TRIM_CONFIG:
			ret = spl_pmu_osc_set_trim_config(data);
			break;
		case GX_OSC_CMD_TRIM_VALUE:
			ret = spl_pmu_osc_set_trim_value(data);
			break;
		case GX_OSC_CMD_TRIM_DELAY:
			ret = spl_pmu_osc_set_trim_delay(data);
			break;
		case GX_OSC_CMD_CURRENT_CYCLE:
		default:
			return -1;
			break;
	}

	return ret;
}

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
int spl_osc_cycles_compare(uint16_t expected_cycles, uint16_t current_cycles, uint16_t tolerance)
{
	if (expected_cycles == current_cycles ) {
		return 0;
	} else if (expected_cycles > current_cycles) {
		if (expected_cycles - current_cycles <= tolerance) {
			return 0;
		} else {
			return 1;
		}
	} else if (expected_cycles < current_cycles) {
		if (current_cycles - expected_cycles <= tolerance) {
			return 0;
		} else {
			return -1;
		}
	}

	return 0;
}

int spl_pmu_osc_enable(GX_OSC_CLK_TYPE type)
{
	uint32_t osc_en = readl(OSC_ENABLE);

	if (GX_OSC_CLK_TYPE_1M == type) {
		osc_en |= 0x1;
	} else if (GX_OSC_CLK_TYPE_24M == type) {
		osc_en |= 0x2;
	} else {
		return -1;
	}
	writel(osc_en, OSC_ENABLE);

	return 0;
}

int spl_pmu_osc_disable(GX_OSC_CLK_TYPE type)
{
	uint32_t osc_en;
	osc_en = readl(OSC_ENABLE);

	if (GX_OSC_CLK_TYPE_1M == type) {
		osc_en &= ~0x1;
	} else if (GX_OSC_CLK_TYPE_24M == type) {
		osc_en &= ~0x2;
	} else {
		return -1;
	}
	writel(osc_en, OSC_ENABLE);

	return 0;
}

int spl_pmu_osc_trim_enable(GX_OSC_CLK_TYPE type)
{
	if (GX_OSC_CLK_TYPE_32K == type) {
		writel(1, OSC_TRIM_CLK_32K_EN);
	} else if (GX_OSC_CLK_TYPE_1M == type) {
		writel(1, OSC_TRIM_CLK_1M_EN);
	} else if (GX_OSC_CLK_TYPE_24M == type) {
		writel(1, OSC_TRIM_CLK_24M_EN);
	} else {
		return -1;
	}

	return 0;
}

extern void spl_clk_set_gate_enable(GX_CLOCK_MODULE module, unsigned int enable);
int spl_pmu_osc_trim_init(void)
{
	spl_clk_set_gate_enable(CLOCK_MODULE_OSC_REF, 1);

	// 开启trim时钟
	writel(0x7, OSC_TRIM_CLK_ENABLE);

	writel(0x3F, TRIM_INT_EN);

	return 0;
}

int spl_pmu_osc_trim_done(GX_OSC_CLK_TYPE type)
{
	uint32_t int_mask;
	uint32_t int_status;

	switch (type) {
		case GX_OSC_CLK_TYPE_32K:
			int_mask = GX_OSC_INT_32K_DONE_MASK;
			break;
		case GX_OSC_CLK_TYPE_1M:
			int_mask = GX_OSC_INT_1M_DONE_MASK;
			break;
		case GX_OSC_CLK_TYPE_24M:
			int_mask = GX_OSC_INT_24M_DONE_MASK;
			break;
		default:
			return 0;
	}

	while (1) {
		int_status = readl(TRIM_INT_STATUS);
		if (int_status & int_mask) {
			writel(int_status & int_mask, TRIM_INT_STATUS); // 清中断状态
			return 1;
		}
	}

	return 0;
}
