/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_DCACHE_H__
#define __GX_DCACHE_H__

#include <soc.h>
#include <common.h>

/**
 * @brief 获取 dcache 状态
 */
int gx_dcache_get_status(void);

/**
 * @brief 使能 dcache
 */
void gx_dcache_enable(void);

/**
 * @brief 关闭 dcache
 */
void gx_dcache_disable (void);

/**
 * @brief 刷新 cache 中所有的脏数据到内存
 */
void gx_dcache_clean(void);

/**
 * @brief 无效所有 cache 中的数据
 */
void gx_dcache_invalid(void);

/**
 * @brief 刷新 cache 的脏数据到内存, 并无效 cache 中的数据
 */
void gx_dcache_clean_invalid(void);

/**
 * @brief 根据给定的地址和大小，无效 cache 中的数据
 *
 * @param addr 起始地址 需要16字节对齐
 * @param dsize 大小 需要16字节对齐
 */
void gx_dcache_invalid_range (uint32_t *addr, int32_t dsize);

/**
 * @brief 根据给定的地址和大小，刷新 cache 中的数据
 *
 * @param addr 起始地址 需要16字节对齐
 * @param dsize 大小 需要16字节对齐
 */
void gx_dcache_clean_range (uint32_t *addr, int32_t dsize);

/**
 * @brief 根据给定的地址和大小，刷新和无效 cache 中的数据
 *
 * @param addr 起始地址 需要16字节对齐
 * @param dsize 大小 需要16字节对齐
 */
void gx_dcache_clean_invalid_range(uint32_t *addr, int32_t dsize);

/**
 * @brief 使能 dcache 信息统计
 */
void gx_dcache_enable_profile(void);

/**
 * @brief 关闭 dcache 信息统计
 */
void gx_dcache_disable_profile(void);

/**
 * @brief dcache 信息统计数据清零
 */
void gx_dcache_reset_profile(void);

/**
 * @brief 获取 dcache 命中次数
 *
 * @return uint32_t 命中次数
 */
uint32_t gx_dcache_get_access_time(void);

/**
 * @brief 获取 dcache 不命中次数
 *
 * @return uint32_t 不命中次数
 */
uint32_t gx_dcache_get_miss_time(void);

#endif

/** @}*/