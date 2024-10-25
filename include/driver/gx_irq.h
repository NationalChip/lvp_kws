/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_IRQ_H__
#define __GX_IRQ_H__

#include <stdint.h>
#include <types.h>
#include <soc.h>

typedef int (*irq_handler_t) (int irq, void *pdata);

/**
 * @brief 关闭所有中断
 */
void gx_disable_all_interrupt(void);

/**
 * @brief  中断初始化
 */
void gx_irq_init(void);

/**
 * @brief mask 指定中断
 *
 * @param irq 中断号
 */
void gx_mask_irq(unsigned int irq);

/**
 * @brief unmask 指定中断
 *
 * @param irq 中断号
 */
void gx_unmask_irq(unsigned int irq);

/**
 * @brief 使能中断控制器
 */
void gx_enable_irq(void);

/**
 * @brief 关闭中断控制器
 */
void gx_disable_irq(void);

/**
 * @brief 注册中断服务例程
 *
 * @param irq 中断号
 * @param handler 中断回调函数
 * @param pdata 中断回调参数
 */
void gx_request_irq(int irq, irq_handler_t handler, void *pdata);

/**
 * @brief 释放中断
 *
 * @param irq 中断号
 */
void gx_free_irq(int irq);

/**
 * @brief 关中断并返回中断状态
 *
 * @return uint32_t 中断状态
 */
uint32_t gx_lock_irq_save(void);

/**
 * @brief 开中断，并恢复中断状态
 *
 * @param irq_state 中断状态
 */
void gx_unlock_irq_restore(uint32_t irq_state);


/**
 * @note 快速中断只用于LEO芯片
 */

/**
 * @brief 快速中断初始化
 */
void gx_fiq_init(void);

/**
 * @brief mask 指定的快速中断
 *
 * @param fiq 中断号
 */
void gx_mask_fiq(unsigned int fiq);

/**
 * @brief unmask 指定的快速中断
 *
 * @param fiq 快速中断号
 */
void gx_unmask_fiq(unsigned int fiq);

// Fast Interrupt handler
typedef void (*fiq_handler_t) (int fiq, void *pdata);

/**
 * @brief 使能快速中断控制器
 */
void gx_enable_fiq(void);

/**
 * @brief 注册快速中断服务例程
 *
 * @param fiq 快速中断号
 * @param handler 中断回调函数
 * @param pdata 中断回调参数
 */
void gx_request_fiq(int fiq, fiq_handler_t handler, void *pdata);

/**
 * @brief 释放快速中断
 *
 * @param fiq 快速中断号
 */
void gx_free_fiq(int fiq);

/**
 * @brief 中断唤醒源
 */
typedef enum {
    AUDIO_SOURCE = 0x01, ///< Audio
    RTC_SOURCE   = 0x02, ///< Rtc
    GPIO_SOURCE  = 0x04, ///< Gpio
} WAKE_SOURCE;

typedef int (*wake_handler_t) (int status, void *pdata);

/**
 * @brief 注册中断唤醒回调函数
 *
 * @param wake_source 唤醒源, 详细说明请参考 gxdocref WAKE_SOURCE
 * @param handler 中断回调函数
 * @param pdata 中断回调参数
 */
void gx_request_wake(WAKE_SOURCE wake_source, wake_handler_t handler, void *pdata);

#endif

/** @}*/
