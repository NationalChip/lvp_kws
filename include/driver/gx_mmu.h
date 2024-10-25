/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_MMU_H__
#define __GX_MMU_H__

#include <stdint.h>

/**
 * @brief mmu 初始化
 *
 * @return int 初始化是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_mmu_init(void);

/**
 * @brief mmu 地址映射
 *
 * @param virt_addr 虚拟地址
 * @param phy_addr 物理地址
 * @param size 映射大小
 *
 * @return int 映射是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_mmu_map(uint32_t virt_addr, uint32_t phy_addr, size_t size);

/**
 * @brief mmu 地址解除映射
 *
 * @param virt_addr 要解除映射的虚拟首地址
 *
 * @return int 解除映射是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_mmu_unmap(uint32_t virt_addr);

#endif

/** @}*/