/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_LED_H__
#define __GX_LED_H__

/**
 * @brief led开关状态
 */
typedef enum {
	GX_LED_SWITCH_OFF = 0, ///< led关闭
	GX_LED_SWITCH_ON,      ///< led打开
} GX_LED_SWITCH;

/**
 * @brief led亮度等级
 */
typedef enum {
	GX_LED_LIGHT_0 = 0x06, ///< led亮度等级0
	GX_LED_LIGHT_1 = 0x04, ///< led亮度等级1
	GX_LED_LIGHT_2 = 0x02, ///< led亮度等级2
	GX_LED_LIGHT_3 = 0x00, ///< led亮度等级3
} GX_LED_LIGHT;

/**
 * @brief led pwm频率
 */
typedef enum {
	GX_LED_FRE_3K = 0, ///< pwm频率3KHz
	GX_LED_FRE_22K,    ///< pwm频率22KHz
} GX_LED_FRE_SET;

/**
 * @brief led颜色
 */
typedef struct {
	unsigned char r; ///< led绿色分量
	unsigned char b; ///< led蓝色分量
	unsigned char g; ///< led绿色分量
} GX_LED_COLOR;

/**
 * @brief 设置led颜色
 *
 * @param index_v led id
 * @param rbg led颜色分量, 详细说明请参考 gxdocref GX_LED_COLOR
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_led_set_color(int index_v, GX_LED_COLOR rbg);

/**
 * @brief 设置led亮度
 *
 * @param index_v led id
 * @param light led亮度等级, 详细说明请参考 gxdocref GX_LED_LIGHT
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_led_set_light(int index_v, GX_LED_LIGHT light);

/**
 * @brief 设置led开关
 *
 * @param index_v led id
 * @param val led开关状态, 详细说明请参考 gxdocref GX_LED_SWITCH
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_led_switch(int index_v, GX_LED_SWITCH val);

/**
 * @brief 设置led进入shutdown
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_led_enable(void);

/**
 * @brief 设置led退出shutdown
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_led_disable(void);

/**
 * @brief 设置led复位
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_led_reset(void);

/**
 * @brief 设置led pwm频率
 *
 * @param val pwm频率值, 详细说明请参考 gxdocref GX_LED_FRE_SET
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_led_set_frequency(GX_LED_FRE_SET val);

/**
 * @brief 初始化led
 *
 * @param i2c_bus i2c总线id
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_led_init(int i2c_bus);

#endif

/** @}*/
