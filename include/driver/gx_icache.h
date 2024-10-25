/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_ICACHE_H__
#define __GX_ICACHE_H__

#include <common.h>
#include <soc.h>

/**
 * @brief 获取 icache 状态
 */
int gx_icache_get_status(void);

/**
 * @brief 使能 icache
 */
void gx_icache_enable(void);

/**
 * @brief 关闭 icache
 */
void gx_icache_disable(void);

/**
 * @brief 无效所有 icache 中的数据
 */
void gx_icache_invalid(void);

/**
 * @brief 使能 icache 信息统计
 */
void gx_icache_enable_profile(void);

/**
 * @brief 关闭 icache 信息统计
 */
void gx_icache_disable_profile(void);

/**
 * @brief icache 信息统计数据清零
 */
void gx_icache_reset_profile(void);

/**
 * @brief 获取 icache 命中次数
 *
 * @return uint32_t icache 命中次数
 */
uint32_t gx_icache_get_access_time(void);

/**
 * @brief 获取 icache 不命中次数
 *
 * @return uint32_t icache 不命中次数
 */
uint32_t gx_icache_get_miss_time(void);

/**
 * @brief 初始化 icache
 */
void gx_icache_init(void);

#endif

/** @}*/