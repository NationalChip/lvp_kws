/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_PADMUX_H__
#define __GX_PADMUX_H__

/**
 * @brief 管脚复用配置
 */
typedef struct pin_config {
	unsigned char pin_id;   ///< pin 脚号
	unsigned char function; ///< 复用功能
} GX_PIN_CONFIG;

/**
 * @brief 配置管脚复用功能
 *
 * @param pad_id pin 脚号
 * @param function 复用功能
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int padmux_set(int pad_id, int function);

/**
 * @brief 获取管脚复用功能
 *
 * @param pad_id pin 脚号
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int padmux_get(int pad_id);

/**
 * @brief 管脚复用功能检查
 *
 * @param pad_id pin 脚号
 * @param function 复用功能
 * @return int 检测是否正常
 * @retval 0 正常
 * @retval -1 异常
 */
int padmux_check(int pad_id, int function);

/**
 * @brief 管脚复用初始化
 *
 * @param pin_table 管脚复用表, 详细说明请参考 gxdocref GX_PIN_CONFIG
 * @param size 管脚复用表大小
 * @return int 是否成功
 * @retval 0 正常
 * @retval -1 异常
 */
int padmux_init(const GX_PIN_CONFIG *pin_table, int size);

#endif

/** @}*/
