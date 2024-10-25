/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_PMU_CTRL_H__
#define __GX_PMU_CTRL_H__

#include <common.h>

/**
 * @brief PMU 中断唤醒控制位
 */
#define GX_PMU_WAKEUP_GPIO     0x01 ///< GPIO 唤醒
#define GX_PMU_WAKEUP_RTC      0x02 ///< RTC 唤醒
#define GX_PMU_WAKEUP_AUDIO_IN 0x04 ///< AUDIO IN 唤醒
#define GX_PMU_WAKEUP_I2C      0x08 ///< I2C时钟唤醒使能

/**
 * @brief PMU WAKEUP后启动位置
 */
#define GX_PMU_WAKEUP_FROM_ROM  0 ///< 从 ROM 启动
#define GX_PMU_WAKEUP_FROM_SRAM 1 ///< 从指定的 SRAM 地址启动

/**
 * @brief SRAM 工作模式
 */
#define GX_SRAM_MODE_NORMAL      0  ///< SRAM 正常工作
#define GX_SRAM_MODE_RETENTION   1  ///< SRAM 数据保持，不能读写
#define GX_SRAM_MODE_SHUTDOWN    2  ///< SRAM 断电

/**
 * @brief LDO 工作电压
 */
#define GX_PMU_LDO_VOLTAGE_0_950V  0
#define GX_PMU_LDO_VOLTAGE_0_924V  1
#define GX_PMU_LDO_VOLTAGE_0_897V  2
#define GX_PMU_LDO_VOLTAGE_0_871V  3
#define GX_PMU_LDO_VOLTAGE_0_845V  4
#define GX_PMU_LDO_VOLTAGE_0_819V  5
#define GX_PMU_LDO_VOLTAGE_0_792V  6
#define GX_PMU_LDO_VOLTAGE_0_766V  7
#define GX_PMU_LDO_VOLTAGE_0_740V  8
#define GX_PMU_LDO_VOLTAGE_0_714V  9
#define GX_PMU_LDO_VOLTAGE_0_687V  10
#define GX_PMU_LDO_VOLTAGE_0_661V  11

/**
 * @brief PMU 外设状态配置位域
 */
typedef union {
	uint32_t value;
	struct {
		unsigned npu_sram_mode:2;           ///< NPU SRAM 工作模式
		unsigned mcu_sram_mode:2;           ///< MCU SRAM 工作模式
		unsigned audio_in_sram_mode:2;      ///< AUDIO_IN SRAM 工作模式
		unsigned isolation_enable:1;        ///< 是否隔离
		unsigned mcu_reset:1;               ///< MCU 是是否复位
		unsigned mcu_poweroff:1;            ///< MCU 是否断电
		unsigned mcu_clk_gate:1;            ///< MCU 时钟是否gate
		unsigned osc_24m_en:1;              ///< OSC 24M时钟是否开启
		unsigned osc_1m_en:1;               ///< OSC 1M时钟是否开启
		unsigned flash_poweroff:1;          ///< FLASH 是否断电
		unsigned manual_ldo_adjust_en:1;    ///< 手动电压调整使能(模式切换时)
		unsigned manual_ldo_adjust_val:4;   ///< 手动电压调整值(模式切换时)
		unsigned resv:4;                    ///< 保留
	} bits;
} GX_PMU_STATE;

/**
 * @brief 工作时电压trim
 */
typedef union {
	uint32_t value;
	struct {
		unsigned trim_val:4; ///< trim 值
		unsigned trim_en:1;  ///< trim值使能
		unsigned resv:27;    ///< 保留
	} bits;
} GX_PMU_TRIM_CFG;

/**
 * @brief PMU　配置命令
 */
typedef enum pmu_cmd {
	GX_PMU_CMD_TIMING,         ///< 配置时序
	GX_PMU_CMD_SLEEP_STATE,    ///< 配置MCU下电外设状态
	GX_PMU_CMD_WORK_STATE,     ///< 配置MCU上电外设状态
	GX_PMU_CMD_WAKEUP_MODE,    ///< 配置唤醒模式
	GX_PMU_CMD_WAKEUP_STATUS,  ///< 唤醒状态
	GX_PMU_CMD_WAKEUP_ADDR,    ///< 唤醒后运行地址
	GX_PMU_CMD_FSM,            ///< 获取状态机状态
	GX_PMU_CMD_SLEEP_AUTO_LDO, ///< 休眠电压自动调节
	GX_PMU_CMD_WORK_LDO_TRIM,  ///< 工作时电压trim配置
	GX_PMU_CMD_FLASH_PD,       ///< flash断电使能
	GX_PMU_CMD_MAX
} GX_PMU_CMD;

/**
 * @brief PMU 状态机
 */
enum {
	GX_PMU_FSM_IDLE,               ///< 空闲
	GX_PMU_FSM_MCU_IDLE,           ///< 等待 MCU 空闲
	GX_PMU_FSM_MCU_DELAY,          ///< 等待
	GX_PMU_FSM_RAM_LOWPOWER_OPEN,  ///< 低功耗控制及等待
	GX_PMU_FSM_ISO_OPEN,           ///< 打开隔离及等待
	GX_PMU_FSM_MCU_RST_LOW,        ///< 复位
	GX_PMU_FSM_OSC_CLOSES,         ///< 关闭时钟及等待
	GX_PMU_FSM_MCU_POWERDOWN,      ///< 关电源
	GX_PMU_FSM_LDO_ADD_POWER,      ///< 调整电压及等待
	GX_PMU_FSM_WAIT_INT,           ///< 等待中断
	GX_PMU_FSM_LDO_SUB_POWER,      ///< 调整电压及等待
	GX_PMU_FSM_MCU_POWERON,        ///< 开电源
	GX_PMU_FSM_OSC_OPEN,           ///< 开时钟
	GX_PMU_FSM_ISO_CLOSE,          ///< 撤销隔离及等待
	GX_PMU_FSM_RAM_LOWPOWER_CLOSE, ///< 退出低功耗
	GX_PMU_FSM_MCU_RST_HIGH,       ///< 撤销复位
	GX_PMU_FSM_MAX
};

/**
 * @brief 唤醒源
 */
typedef enum {
	GX_WAKEUP_SOURCE_COLD,     ///< 冷复位
	GX_WAKEUP_SOURCE_WDT,      ///< 看门狗复位
	GX_WAKEUP_SOURCE_GPIO,     ///< GPIO唤醒
	GX_WAKEUP_SOURCE_AUDIO_IN, ///< AUDIO_IN唤醒
	GX_WAKEUP_SOURCE_RTC,      ///< RTC唤醒
	GX_WAKEUP_SOURCE_I2C,      ///< I2C唤醒
} GX_WAKEUP_SOURCE;

typedef enum {
	GX_START_MODE_ROM,     ///< 从ROM启动
	GX_START_MODE_SRAM,    ///< 从SRAM启动
} GX_START_MODE;
/**
 * @brief PMU 时序
 */
typedef struct pmu_timing {
	uint16_t mcu_idle_delay;       ///< delay after mcu idle
	uint16_t ram_retention_delay;  ///< ram retention delay
	uint16_t isolation_high_delay; ///< delay after isolation high
	uint16_t close_osc_delay;      ///< delay after close osc
	uint16_t adjust_ldo_delay0;    ///< delay after adjust ldo
	uint16_t adjust_ldo_delay1;    ///< delay after adjust ldo
	uint16_t power_on_delay;       ///< delay after power on
	uint16_t isolation_low_delay;  ///< delay after isolation low
} GX_PMU_TIMING;

/**
 * @brief PMU 唤醒启动地址配置
 */
typedef struct gx_pmu_wakeup_addr {
	uint8_t wakeup_from;     ///< 唤醒后从哪里启动
	uint32_t wakeup_address; ///< 启动地址
} GX_PMU_WAKEUP_ADDR;

#define GX_PMU_AUTO_LDO_MODE_SINGLE     0  ///< 单次电压自动调节
#define GX_PMU_AUTO_LDO_MODE_CONTINUE   1  ///< 连续电压自动调节

/**
 * @brief 自动电压调节
 */
typedef struct pmu_auto_ldo {
	uint8_t enable;                 ///< 自动调整使能
	uint8_t mode;					///< 自动电压调整模式
	uint8_t low_gate;               ///< 自动调整电压最低门限
	uint8_t high_gate;              ///< 自动调整电压最高门限
	uint16_t adjust_delay;          ///< 两次电压调整时间间隔，时钟数
	uint8_t current_trim_value;     ///< 当前电压调整过的trim值(只读)
	uint8_t current_adjust_gate;    ///< 当前电压值所处档位(只读)
} GX_PMU_AUTO_LDO;

/**
 * @brief pmu 配置
 */
typedef struct pmu_ctrl_config {
	GX_PMU_TIMING timing;            ///< 时序配置
	GX_PMU_STATE sleep_state;        ///< 进入低功耗时外设状态配置
	GX_PMU_STATE work_state;         ///< 退出低功耗时外设状态配置
	uint32_t wakeup_mode;            ///< 唤醒模式配置
	uint32_t fsm;                    ///< pmu 状态
	uint32_t wakeup_status;          ///< 唤醒状态
	GX_PMU_WAKEUP_ADDR wakeup_addr;  ///< 唤醒地址配置
	GX_PMU_AUTO_LDO auto_ldo;        ///< 自动ldo配置
	GX_PMU_TRIM_CFG work_ldo_trim;   ///< 工作时ldo配置
} GX_PMU_CTRL_CONFIG;

/**
 * @brief pmu配置设置
 *
 * @param cmd 设置命令, 详细说明请参考 gxdocref GX_PMU_CMD
 * @param data 设置参数
 *
 * @return int 设置是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_pmu_ctrl_set(int cmd, void *data);

/**
 * @brief pmu配置获取
 *
 * @param cmd 设置命令, 详细说明请参考 gxdocref GX_PMU_CMD
 * @param data 获取到的设置参数
 *
 * @return int 获取设置是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_pmu_ctrl_get(int cmd, void *data);

/**
 * @brief pmu 控制使能
 */
void gx_pmu_ctrl_enable(void);

/**
 * @brief 获取唤醒源
 *
 * @return GX_WAKEUP_SOURCE 唤醒源, 详细说明请参考 gxdocref GX_WAKEUP_SOURCE
 */
GX_WAKEUP_SOURCE gx_pmu_get_wakeup_source(void);

/**
 * @brief 获取启动模式
 *
 * @return GX_START_MODE 启动模式, 详细说明请参考 gxdocref GX_START_MODE
 */
GX_START_MODE gx_pmu_get_start_mode(void);

int gx_pmu_ctrl_get_chipid(uint32_t *chipid);
#endif

/** @}*/
