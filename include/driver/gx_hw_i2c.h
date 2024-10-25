/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_HW_I2C_H__
#define __GX_HW_I2C_H__

#define HW_I2C_COMMON_REG0  (0xa0)
#define HW_I2C_COMMON_REG1  (0xa4)
#define HW_I2C_COMMON_REG2  (0xa8)
#define HW_I2C_COMMON_REG3  (0xac)

/**
 * @brief hw i2c复位目标
 */
typedef enum {
	HW_I2C_RESET_GLOBAL = 0, ///< 全局复位
	HW_I2C_RESET_MCU    = 1, ///< mcu复位
	HW_I2C_RESET_PMU    = 2, ///< pmu复位
} GX_HW_I2C_RESET_TARGET;

/**
 * @brief hw i2c复位模式
 */
typedef enum {
	HW_I2C_AUTO_RESET_MODE   = 0, ///< 自动复位
	HW_I2C_MANUAL_RESET_MODE = 1, ///< 手动复位
} GX_HW_I2C_RESET_MODE;

/**
 * @brief hw i2c中断回调函数
 *
 * @param private 私有参数
 * @param status hw i2c中断状态
 *
 * @retval 0 成功
 * @retval -1 失败
 */
typedef int (*gx_hw_i2c_cb)(void *private, unsigned char status);

/**
 * @brief 注册hw i2c中断
 *
 * @param callback hw i2c irq回调函数, 详细说明请参考 gxdocref gx_hw_i2c_cb
 * @param private 回调函数私有参数
 */
void gx_hw_i2c_request_irq(gx_hw_i2c_cb callback, void *private);

/**
 * @brief 注销hw i2c中断
 *
 */
void gx_hw_i2c_free_irq(void);

/**
 * @brief 注销hw i2c中断
 *
 * @param mask 中断使能掩码
 */
void gx_hw_i2c_set_irq_enable(unsigned char mask);

/**
 * @brief 使用hw i2c自动复位相应模块
 *
 * @param target 复位目标, 详细说明请参考 gxdocref GX_HW_I2C_RESET_TARGET
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_hw_i2c_auto_reset(GX_HW_I2C_RESET_TARGET target);

/**
 * @brief 使用hw i2c复位模式
 *
 * @param target 复位目标, 详细说明请参考 gxdocref GX_HW_I2C_RESET_TARGET
 * @param mode 复位模式, 详细说明请参考 gxdocref GX_HW_I2C_RESET_MODE
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_hw_i2c_set_reset_mode(GX_HW_I2C_RESET_TARGET target, GX_HW_I2C_RESET_MODE mode);

/**
 * @brief hw i2c进入功能模式
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_hw_i2c_enter_config_mode(void);

/**
 * @brief 写hw i2c所有寄存器
 *
 * @param val 写入的值
 */
void gx_hw_i2c_write_all(unsigned char val);

/**
 * @brief 读hw i2c所有寄存器
 */
void gx_hw_i2c_read_all(void);

/**
 * @brief 读hw i2c寄存器
 *
 * @param reg 需要读取的寄存器
 */
unsigned char gx_hw_i2c_read_reg(unsigned int reg);

/**
 * @brief 写hw i2c寄存器
 *
 * @param reg hw i2c寄存器
 * @param val 写入的值
 */
void gx_hw_i2c_write_reg(unsigned int reg, unsigned char val);

#endif

/** @}*/
