/** \addtogroup <scpu>
 *  @{
 */
#ifndef __GX_GPIO_H__
#define __GX_GPIO_H__

/**
 * @brief gpio输入输出方向
 */
typedef enum {
    GX_GPIO_DIRECTION_INPUT = 0, ///< 输入
    GX_GPIO_DIRECTION_OUTPUT,    ///< 输出
    GX_GPIO_DIRECTION_HIZ        ///< 高阻态
} GX_GPIO_DIRECTION;

/**
 * @brief gpio 电平状态
 */
typedef enum {
    GX_GPIO_LEVEL_LOW = 0, ///< 低电平
    GX_GPIO_LEVEL_HIGH     ///< 高电平
} GX_GPIO_LEVEL;

/**
 * @brief gpio中断触发方式
 */
typedef enum {
    GX_GPIO_TRIGGER_EDGE_FALLING   = 0x01, ///< 下降沿触发
    GX_GPIO_TRIGGER_EDGE_RISING    = 0x02, ///< 上升沿触发
    GX_GPIO_TRIGGER_EDGE_BOTH      = 0x03, ///< 降沿上升同时沿触发
    GX_GPIO_TRIGGER_LEVEL_HIGH     = 0x04, ///< 高电平触发
    GX_GPIO_TRIGGER_LEVEL_LOW      = 0x08, ///< 低电平触发
} GX_GPIO_TRIGGER_EDGE;

typedef int (*GPIO_CALLBACK)(int port, void *pdata);

/**
 * @brief 获取GPIO输入输出方向
 *
 * @param port gpio端口
 *
 * @return gpio 输入还是输出
 * @retval GX_GPIO_DIRECTION_INPUT 输入
 * @retval GX_GPIO_DIRECTION_OUTPUT 输出
 * @retval GX_GPIO_DIRECTION_HIZ 高阻态
 */
GX_GPIO_DIRECTION gx_gpio_get_direction(unsigned int port);

/**
 * @brief 设置GPIO输入输出方向
 *
 * @param port gpio端口
 * @param direction gpio输入输出方向, 详细说明请参考 gxdocref GX_GPIO_DIRECTION
 *
 * @return int 设置是否成功
 * @retval 0 设置成功
 * @retval -1 设置失败
 */
int gx_gpio_set_direction(unsigned int port, GX_GPIO_DIRECTION direction);

/**
 * @brief 获取gpio输入电平状态
 *
 * @param port gpio端口
 * @return GX_GPIO_LEVEL gpio输入电平状态
 * @retval GX_GPIO_LEVEL_LOW 低电平
 * @retval GX_GPIO_LEVEL_HIGH 高电平
 */
GX_GPIO_LEVEL gx_gpio_get_level(unsigned int port);

/**
 * @brief 设置gpio输出电平状态
 *
 * @param port gpio端口
 * @param level 输出电平状态, 详细说明请参考 gxdocref GX_GPIO_LEVEL
 *
 * @return int 设置是否成功
 * @retval 0 设置成功
 * @retval -1 设置失败
 */
int gx_gpio_set_level(unsigned int port, GX_GPIO_LEVEL level);

/**
 * @brief gpio 使能中断
 *
 * @param port gpio端口
 * @param edge 触发方式, 详细说明请参考 gxdocref GX_GPIO_TRIGGER_EDGE
 * @param callback 中断回调函数
 * @param pdata 中断回调参数
 *
 * @return int gpio中断使能是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_gpio_enable_trigger(unsigned int port, GX_GPIO_TRIGGER_EDGE edge, GPIO_CALLBACK callback, void *pdata);

/**
 * @brief 关闭gpio中断使能
 *
 * @param port gpio端口
 *
 * @return int 关闭gpio中断使能是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_gpio_disable_trigger(unsigned int port);

/**
 * @brief gpio 输出pwm
 *
 * @param port gpio端口
 * @param period_ns pwm周期(ns)
 * @param duty pwm 高电平时间(ns)
 *
 * @return int pwm使能是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_gpio_enable_pwm(unsigned int port, unsigned long int period_ns, unsigned long int duty);

/**
 * @brief 关闭pwm
 *
 * @param port gpio端口
 * @return int 关闭pwm使能是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_gpio_disable_pwm(unsigned int port);

/**
 * @brief 时钟输出测试
 *
 * @param port 时钟输出的gpio端口
 * @param test_clock 需要测试的时钟
 * @return int 设置是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_gpio_clock_test(unsigned int port, unsigned int test_clock);

/**
 * @brief gpio 初始化
 *
 * @return int 初始化是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_gpio_init(void);

/**
 * @brief gpio 退出
 *
 * @return int 退出是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_gpio_exit(void);

/**
 * @brief gpio 暂停
 *
 * @return int 暂停是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_gpio_suspend(void);

/**
 * @brief gpio 恢复
 *
 * @return int 恢复是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_gpio_resume(void);

#endif /* __GX_GPIO_H__ */

/** @}*/