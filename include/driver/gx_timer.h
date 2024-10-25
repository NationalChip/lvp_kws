/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_TIMER_H__
#define __GX_TIMER_H__

/**
 * @brief 定时器模式
 */
typedef enum {
    GX_TIMER_MODE_SINGLE,    ///< 单次模式
    GX_TIMER_MODE_CONTINUE,  ///< 连续模式
} GX_TIMER_MODE;

/**
 * @brief 获取当前时间(us)
 *
 * @return unsigned long long 当前时间
 */
unsigned long long gx_get_time_us(void);

/**
 * @brief 获取 cpu 运行的 cycle 数
 *
 * @return unsigned long long 当前 cpu 运行 cycle 数
 */
unsigned long long gx_get_cpu_cycles(void);

/**
 * @brief 获取当前时间(ms)
 *
 * @return unsigned int 当前时间
 */
unsigned int gx_get_time_ms(void);

/**
 * @brief 基于某一时间，获取相对时间
 *
 * @param base 时间基值
 * @return unsigned int 相对时间
 */
unsigned int gx_get_timer(unsigned int base);

/**
 * @brief 注册一个软定时器
 *
 * @param fun 定时回调
 * @param timeout_ms 定时时间，计数模式该参数无效，默认0xFFFFFFFF
 * @param priv 回调参数
 * @param mode 定时器模式, 详细说明请参考 gxdocref GX_TIMER_MODE
 * @retval -1 失败
 * @retval 软定时器id
 */
int gx_timer_register(int (*fun)(void*), int timeout_ms, void *priv, GX_TIMER_MODE mode);

/**
 * @brief 注销软定时器
 *
 * @param id 软定时器id
 * @retval 注销是否成功
 */
int gx_timer_unregister(int id);

#endif

/** @}*/
