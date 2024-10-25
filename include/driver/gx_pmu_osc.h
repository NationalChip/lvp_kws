/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_PMU_OSC_H__
#define __GX_PMU_OSC_H__

#include <driver/gx_irq.h>

#define GX_OSC_32K_VALUE_MAX 0x1FF ///< 32k trim 最大值
#define GX_OSC_1M_VALUE_MAX  0x1FF ///< 1m trim 最大值
#define GX_OSC_24M_VALUE_MAX 0x7F  ///< 24m trim 最大值

#define GX_OSC_CLOCK_32K  32000    ///< 32k 时钟
#define GX_OSC_CLOCK_1M   1024000  ///< 1m 时钟
#define GX_OSC_CLOCK_24M  24576000 ///< 24m 时钟

// trim值下溢测试
// #define GX_OSC_CLOCK_32K  1280000
// #define GX_OSC_CLOCK_1M   50000000
// #define GX_OSC_CLOCK_24M  100000000

// trim值上溢测试
// #define GX_OSC_CLOCK_32K  3200
// #define GX_OSC_CLOCK_1M   100000
// #define GX_OSC_CLOCK_24M  1000000

// 中断MASK
#define GX_OSC_INT_32K_DONE_MASK   0x01 ///< 32k trim 结束中断掩码
#define GX_OSC_INT_1M_DONE_MASK    0x02 ///< 1m trim 结束中断掩码
#define GX_OSC_INT_24M_DONE_MASK   0x04 ///< 24m trim 结束中断掩码
#define GX_OSC_INT_32K_ERROR_MASK  0x08 ///< 32k trim 出错中断掩码
#define GX_OSC_INT_1M_ERROR_MASK   0x10 ///< 1m trim 出错中断掩码
#define GX_OSC_INT_24M_ERROR_MASK  0x20 ///< 24m trim 出错中断掩码

#define GX_OSC_INT_32K_MASK   ((GX_OSC_INT_32K_DONE_MASK) | (GX_OSC_INT_32K_ERROR_MASK))
#define GX_OSC_INT_1M_MASK    ((GX_OSC_INT_1M_DONE_MASK) | (GX_OSC_INT_1M_ERROR_MASK))
#define GX_OSC_INT_24M_MASK   ((GX_OSC_INT_24M_DONE_MASK) | (GX_OSC_INT_24M_ERROR_MASK))

/**< 计算参考时钟cycle数 */
#define GX_OSC_REF_CYCLE(ref_freq, target_freq, target_cycles) \
	(((unsigned long long)(ref_freq) * (target_cycles)) / (target_freq))

/**
 * @brief OSC 配置命令
 */
typedef enum {
	GX_OSC_CMD_TRIM_CONFIG,   ///< OSC TRIM 配置
	GX_OSC_CMD_CURRENT_CYCLE, ///< 实际时钟数获取
	GX_OSC_CMD_TRIM_VALUE,    ///< TRIM值获取
	GX_OSC_CMD_TRIM_DELAY,    ///< TRIM 后等待OSC稳定延时
	GX_OSC_CMD_MAX            ///< OSC 命令最大值
} GX_OSC_CMD;

/**
 * @brief 时钟类型
 */
typedef enum {
	GX_OSC_CLK_TYPE_32K, ///< 32K 目标时钟
	GX_OSC_CLK_TYPE_1M,  ///< 1M 目标时钟
	GX_OSC_CLK_TYPE_24M, ///< 24M 目标时钟
	GX_OSC_CLK_TYPE_MAX  ///< 时钟类型枚举最大值
} GX_OSC_CLK_TYPE;

/**
 * @brief TRIM 模式
 */
typedef enum {
	GX_OSC_TRIM_MODE_AUTO     = 0x01, ///< 手动校正模式
	GX_OSC_TRIM_MODE_MANUAL   = 0x02, ///< 自动校正模式
} GX_OSC_TRIM_MODE;

/**
 * @brief 目标时钟配置
 */
typedef struct osc_target_cycle {
	GX_OSC_CLK_TYPE type;      ///< 时钟类型
	uint16_t target_cycles;    ///< 目标时钟 cycle 数
	uint16_t ref_cycles;       ///< 参考时钟 cycle 数
} GX_OSC_TARGET_CYCLE;

/**
 * @brief 当前参考时钟 cycle
 */
typedef struct osc_current_cycle {
	GX_OSC_CLK_TYPE type;   ///< 时钟类型
	uint32_t cycles;        ///< 参考时钟 cycle 数
} GX_OSC_CURRENT_CYCLE;

/**
 * @brief TRIM 值
 */
typedef struct osc_trim {
	GX_OSC_CLK_TYPE type; ///< 时钟类型
	uint32_t trim_value;  ///< trim 值
} GX_OSC_TRIM;

/**
 * @brief osc 配置结构体
 */
typedef struct osc_trim_config {
	GX_OSC_CLK_TYPE type;   ///< TRIM 时钟类型
	GX_OSC_TRIM_MODE mode;  ///< TRIM 模式
	uint16_t target_cycles; ///< 目标时钟 cycle 数
	uint16_t ref_cycles;    ///< 参考时钟 cycle 数

	irq_handler_t callback; ///< TRIM 中断回调函数
	void *priv;             ///< TRIM 回调参数
	uint32_t int_status;    ///< 中断状态
} GX_OSC_TRIM_CONFIG;

/**
 * @brief 配置 OSC 参数
 *
 * @param cmd 配置命令, 详细说明请参考 gxdocref GX_OSC_CMD
 * @param data 要配置的参数
 *
 * @return int 配置是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_pmu_osc_set(GX_OSC_CMD cmd, void *data);

/**
 * @brief 获取 OSC 参数
 *
 * @param cmd 获取命令, 详细说明请参考 gxdocref GX_OSC_CMD
 * @param data 获取到的参数数据
 *
 * @return int 获取是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_pmu_osc_get(GX_OSC_CMD cmd, void *data);

/**
 * @brief 对比目标 cycles 和当前 cycles 大小
 *
 * @param expected_cycles 目标 cycles
 * @param current_cycles 但其cycles
 * @param tolerance 容差值
 *
 * @return int 对比结果
 * @retval 0 : abs(expected_cycles - current_cycles) <= tolerance
 * @retval 1 : expected_cycles > current_cycles + tolerance
 * @retval -1 : current_cycles > expected_cycles + tolerance
 */
int gx_osc_cycles_compare(uint16_t expected_cycles, uint16_t current_cycles, uint16_t tolerance);

/**
 * @brief 使能 OSC 模块
 *
 * @param type 要使能的时钟类型, 详细说明请参考 gxdocref GX_OSC_CLK_TYPE
 * @return int 使能是否成功
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_pmu_osc_enable(GX_OSC_CLK_TYPE type);

/**
 * @brief 关闭 OSC 模块
 *
 * @param type 要关闭的时钟类型, 详细说明请参考 gxdocref GX_OSC_CLK_TYPE
 * @return int 关闭是否成功
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_pmu_osc_disable(GX_OSC_CLK_TYPE type);

/**
 * @brief OSC 初始化
 *
 * @return int 初始化是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_pmu_osc_init(void);

/**
 * @brief 使能 OSC TRIM 功能
 *
 * @param type 要使能的时钟类型, 详细说明请参考 gxdocref GX_OSC_CLK_TYPE
 *
 * @return int 使能是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_pmu_osc_trim_enable(GX_OSC_CLK_TYPE type);

/**
 * @brief 初始化 OSC TRIM
 *
 * @return int 初始化是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_pmu_osc_trim_init(void);

/**
 * @brief 设置OSC 32k TRIM完成状态
 *
 * @param trim_done, 0:osc32k未完成trim, 1:osc32k已完成trim
 *
 * @return int 设置是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_pmu_osc_set_32k_trim_state(int trim_done);

/**
 * @brief 获取OSC 32k TRIM完成状态
 *
 * @return int trim完成状态
 * @retval 0 未完成trim
 * @retval 1 已完成trim
 */
int gx_pmu_osc_get_32k_trim_state(void);

/**
 * @brief 设置所有OSC TRIM完成状态
 *
 * @param trim_done, 0:未完成trim, 1:已完成trim
 *
 * @return int 设置否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_pmu_osc_get_all_trim_state(void);

/**
 * @brief 获取所有OSC TRIM完成状态
 *
 * @return int trim完成状态
 * @retval 0 未完成trim
 * @retval 1 已完成trim
 */
int gx_pmu_osc_set_all_trim_state(int trim_done);
#endif

/** @}*/
