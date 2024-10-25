/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_CLOCK_V2_H__
#define __GX_CLOCK_V2_H__

#include <stdint.h>

#define GX_CLOCK_PMU_INHIBIT_NORM     (0x18)
#define GX_CLOCK_PMU_INHIBIT_SET      (0x1c)
#define GX_CLOCK_PMU_INHIBIT_CLR      (0x20)
#define GX_CLOCK_PMU_DTO_CONFIG0      (0x24)
#define GX_CLOCK_PMU_DIV_CONFIG0      (0x80)
#define GX_CLOCK_PMU_DIV_CONFIG1      (0x84)
#define GX_CLOCK_PMU_DIV_CONFIG2      (0x88)
#define GX_CLOCK_PMU_SOURCE_SEL       (0x8c)
#define GX_CLOCK_PMU_TCI_ONFIG0       (0x104)
#define GX_CLOCK_PMU_SOURCE_SEL1      (0x108)

#define GX_CLOCK_MCU_INHIBIT_NORM     (0x18)
#define GX_CLOCK_MCU_INHIBIT_SET      (0x1c)
#define GX_CLOCK_MCU_INHIBIT_CLR      (0x20)
#define GX_CLOCK_MCU_DTO_CONFIG0      (0x24)
#define GX_CLOCK_MCU_DTO_CONFIG1      (0x28)
#define GX_CLOCK_MCU_DIV_CONFIG0      (0x80)
#define GX_CLOCK_MCU_DIV_CONFIG1      (0x84)
#define GX_CLOCK_MCU_SOURCE_SEL       (0x88)
#define GX_CLOCK_DTO_BASE             (0x24)

/**
 * @brief 各个时钟模块
 */
typedef enum  {
	CLOCK_MODULE_RTC          =  0,
	CLOCK_MODULE_PMU          =  1,
	CLOCK_MODULE_AUDIO_IN_SYS =  2,
	CLOCK_MODULE_FFT          =  3,
	CLOCK_MODULE_GPIO         =  4,
	CLOCK_MODULE_HW_I2C       =  5,
	CLOCK_MODULE_SRAM         =  6,
	CLOCK_MODULE_AUDIO_IN_ADC =  7,
	CLOCK_MODULE_AUDIO_IN_PDM =  8,
	CLOCK_MODULE_OSC_REF      =  9,

	CLOCK_MODULE_SCPU         = 10,
	CLOCK_MODULE_AUDIO_PLAY   = 11,
	CLOCK_MODULE_NPU          = 12,
	CLOCK_MODULE_FLASH_SPI    = 13,
	CLOCK_MODULE_GENERAL_SPI  = 14,
	CLOCK_MODULE_AUDIO_LODAC  = 15,
	CLOCK_MODULE_UART0_UART1  = 16,
	CLOCK_MODULE_UART0        = 17, ///< 仅用于时钟gate配置
	CLOCK_MODULE_UART1        = 18, ///< 仅用于时钟gate配置
	CLOCK_MODULE_I2C0_I2C1    = 19,
	CLOCK_MODULE_I2C0         = 20, ///< 仅用于时钟gate配置
	CLOCK_MODULE_I2C1         = 21, ///< 仅用于时钟gate配置
	CLOCK_MODULE_TIMER_WDT    = 22,
	CLOCK_MODULE_TIMER        = 23, ///< 仅用于时钟gate配置
	CLOCK_MODULE_WDT          = 24, ///< 仅用于时钟gate配置
	CLOCK_MODULE_DMA          = 25, ///< 仅用于时钟gate配置

	CLOCK_MODULE_MAX   = 26,
} GX_CLOCK_MODULE;

/**
 * @brief 时钟源来源的类型
 */
typedef enum {
	CLOCK_SOURCE_24M_PLL =  0,
	CLOCK_SOURCE_32K     = 14,
	CLOCK_SOURCE_32K_PLL = 15,
	CLOCK_SOURCE_24M     = 16,
	CLOCK_SOURCE_PLL_DTO = 17,
	CLOCK_SOURCE_1M_12M  = 18,
	CLOCK_SOURCE_PIN_IN  = 26,
	CLOCK_SOURCE_1M_INPUT= 27,
	CLOCK_SOURCE_1M      = 28,
	CLOCK_SOURCE_OSC_PLL = 29,
} GX_CLOCK_SOURCE;

/**
 * @brief 时钟配置表
 */
typedef struct {
	GX_CLOCK_SOURCE source;  ///< 时钟源类型
	uint8_t clk;  ///< 目标时钟
} GX_CLOCK_SOURCE_TABLE;

/**
 * @brief 时钟模块的时钟来源
 */
typedef enum {
	MODULE_SOURCE_1M_12M     = 0, ///< 模块时钟来自1M或12M选项
	MODULE_SOURCE_24M_PLL    = 1, ///< 模块时钟来自24M或PLL选项
	MODULE_SOURCE_32K        = 2, ///< 模块时钟来自32K选项

	MODULE_SOURCE_ADC_SYS    = 3, ///< ADC 独享配置, ADC时钟来自Audio In系统
	MODULE_SOURCE_ADC_32K    = 4, ///< ADC 独享配置, ADC时钟来自OSC_32K

	MODULE_SOURCE_PDM_SYS    = 5, ///< PDM 独享配置, PDM时钟来自Audio In系统
	MODULE_SOURCE_PDM_OSC_1M = 6, ///< PDM 独享配置, PDM时钟来自OSC_1M
	MODULE_SOURCE_PDM_IN     = 7, ///< PDM 独享配置, PDM时钟来自外部时钟，只支持1.024MHz和2.048MHz

	MODULE_SOURCE_ERROR      = -1,
} GX_CLOCK_MODULE_SOURCE;

/**
 * @brief PLL配置表
 */
typedef struct {
	unsigned int pll_enable;
	unsigned int pll_in;
	unsigned int pll_fvco;
	unsigned int pll_out;
	unsigned int pll_div_in;
	unsigned int pll_div_fb;
	unsigned int pll_icpsel;
	unsigned int pll_bwsel_lpf;
	unsigned int pll_vco_subband;
	unsigned int pll_div_out;

	unsigned int pll_itrim;
	unsigned int pll_vco_trim;
	unsigned int pll_2nd3nd_lpf;
	unsigned int pll_lock_tiehi;
} GX_CLOCK_PLL;

/**
 * @brief DTO配置表
 */
typedef struct {
	uint8_t addr;      ///< dto 地址
} GX_CLOCK_DTO;

/**
 * @brief DIV配置表
 */
typedef struct {
	uint8_t  addr;      ///< div 地址
	uint8_t  offs;      ///< 偏移
	uint16_t mask;      ///< 掩码
} GX_CLOCK_DIV;

/**
 * @brief pmu模块时钟gate开关
 *
 * @param module 时钟模块名，详细说明请参考 gxdocref GX_CLOCK_MODULE
 * @param enable 0: 关 1:开
 *
 * @return 无返回值
 */
void gx_clock_set_module_enable(GX_CLOCK_MODULE module, unsigned int enable);

/*****************************************************/
/**
 * @brief 设置模块时钟来源
 *
 * @param module 时钟模块名，详细说明请参考 gxdocref GX_CLOCK_MODULE
 * @param source 时钟源，详细说明请参考 gxdocref GX_CLOCK_MODULE_SOURCE
 *
 * @return 无返回值
 */
int gx_clock_set_module_source(GX_CLOCK_MODULE module, GX_CLOCK_MODULE_SOURCE source);

/**
 * @brief 设置时钟源
 *
 * @param clock_table 时钟配置表格，详细说明请参考 gxdocref GX_CLOCK_SOURCE_TABLE
 *
 * @return 0, -1
 */
void gx_clock_set_source(GX_CLOCK_SOURCE_TABLE *source_table);

/**
 * @brief 获取模块时钟来源
 *
 * @param module 时钟模块名，详细说明请参考 gxdocref GX_CLOCK_MODULE
 *
 * @return 无返回值
 */
GX_CLOCK_MODULE_SOURCE gx_clock_get_module_source(GX_CLOCK_MODULE module);

/**
 * @brief 获取时钟源
 *
 * @param source 想要获取的时钟源，详细说明请参考 gxdocref GX_CLOCK_SOURCE
 *
 * @return 无返回值
 */
unsigned int gx_clock_get_source(GX_CLOCK_SOURCE source);

/**
 * @brief DIV分配配置
 *
 * @param module 时钟模块名，详细说明请参考 gxdocref GX_CLOCK_MODULE
 * @param div    目标div值
 *
 * @return 无返回值
 */
void gx_clock_set_div(GX_CLOCK_MODULE module, int div);

/**
 * @brief 获取div值
 *
 * @param module 时钟模块名，详细说明请参考 gxdocref GX_CLOCK_MODULE
 *
 * @return div值
 */
int gx_clock_get_div(GX_CLOCK_MODULE module);

/**
 * @brief DTO分配配置
 *
 * @param module 时钟模块名，详细说明请参考 gxdocref GX_CLOCK_MODULE
 * @param div    目标dto值
 * @param enable 使能开关
 *
 * @return 无返回值
 */
void gx_clock_set_dto(GX_CLOCK_MODULE module, unsigned int dto, int enable);

/**
 * @brief DTO分频值更新
 * 		只适用于快速动态调频场景，正常场景请使用 gx_clock_set_dto
 *
 * @param module 时钟模块名，详细说明请参考 gxdocref GX_CLOCK_MODULE
 * @param dto    目标dto值
 *
 * @return 无返回值
 */
void gx_clock_adjust_dto(GX_CLOCK_MODULE module, unsigned int dto);

/**
 * @brief 获取dto值
 *
 * @param module 时钟模块名，详细说明请参考 gxdocref GX_CLOCK_MODULE
 *
 * @return dto值
 */
unsigned int gx_clock_get_dto(GX_CLOCK_MODULE module);

/**
 * @brief PLL配置非阻塞接口
 *
 * @param pll pll配置表，详细说明请参考 gxdocref GX_CLOCK_PLL
 * @param timeout_ms 等待pll锁定超时时间
 *
 * @return int
 * @retval  0 pll配置成功
 * @retval -1 pll未锁定，配置失败
 */
int gx_clock_set_pll_no_block(GX_CLOCK_PLL *pll, unsigned int timeout_ms);

/**
 * @brief PLL配置阻塞接口
 *
 * @param pll pll配置表，详细说明请参考 gxdocref GX_CLOCK_PLL
 *
 * @return 无返回值
 */
void gx_clock_set_pll(GX_CLOCK_PLL *pll);

/**
 * @brief 设置SNPU模块时钟gate开关
 *
 * @param enable 0: 关 1:开
 *
 * @return 无返回值
 */
void gx_clock_set_module_snpu_enable(int enable);

/**
 * @brief 获取模块时钟gate状态
 *
 * @param module 时钟模块名，详细说明请参考 gxdocref GX_CLOCK_MODULE
 *
 * @return enable 0:关 1:开 -1:获取失败
 */
int gx_clock_get_module_enable(GX_CLOCK_MODULE module);

/**
 * @brief 获取模块时钟频率
 *
 * @param module 时钟模块名，详细说明请参考 gxdocref GX_CLOCK_MODULE
 *
 * @return 时钟频率
 */
unsigned int gx_clock_get_module_frequence(GX_CLOCK_MODULE module);
#endif

/** @}*/
