/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_WATCHDOG_H__
#define __GX_WATCHDOG_H__

#include <common.h>
#include <driver/gx_irq.h>

/**
 * @brief 看门狗初始化
 *
 * @param reset_timeout_ms 看门狗复位芯片超时时间
 * @param level_timeout_ms 看门狗产生超时时间
 * @param irq_handler 中断服务回调函数
 * @param pdata 中断服务回调函数私有参数
 */
void gx_watchdog_init(uint16_t reset_timeout_ms,
		uint16_t level_timeout_ms, irq_handler_t irq_handler, void *pdata);

/**
 * @brief 看门狗喂狗
 */
void gx_watchdog_ping(void);

/**
 * @brief a7 看门狗初始化，仅仅用于leo芯片
 */
void gx_a7_wdt_init(void);

/**
 * @brief 重启
 *
 * @return int 复位函数，不会返回
 */
int gx_reboot(void);

/**
 * @brief 看门狗关闭
 *
 * @return 无
 */
void gx_watchdog_stop(void);

#endif

/** @}*/
