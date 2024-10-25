/** \addtogroup <scpu>
 *  @{
 */
#ifndef __GX_DELAY_H__
#define __GX_DELAY_H__

/**
 * @brief 延时初始化
 */
void gx_delay_init(void);

/**
 * @brief us 级延时
 *
 * @param us 延时时长(us)
 */
void gx_udelay(unsigned int us);

/**
 * @brief ms 级延时
 *
 * @param ms 延时时长(ms)
 */
void gx_mdelay(unsigned int ms);

/**
 * @brief 定时器初始化
 */
void gx_timer_init(void);

#ifdef CONFIG_ARCH_GRUS
#include <csi_core.h>

/**
 * @brief 精准us delay配置
 *
 * @param cpu_fre CPU频率(Hz)
 *
 * @return 配置情况
 * - -1        配置失败
 * - Others    拿到精准delay系数
 */
static inline unsigned int gx_udelay_fine_config(unsigned int cpu_fre)
{
	csi_vic_disable_irq(0);
	csi_coret_config(0xffffff, 0);

	if (cpu_fre >= 12287990 && cpu_fre <= 12288010)
		return 8138;
	else if (cpu_fre >= 16383990 && cpu_fre <= 16384010)
		return 6050;
	else if (cpu_fre >= 24575990 && cpu_fre <= 24576010)
		return 4069;
	else if (cpu_fre >= 49151990 && cpu_fre <= 49152010)
		return 2034;
	else
		return -1;
}

/**
 * @brief 精准us delay
 *
 * @param us    延时时长
 *        coeff gx_udelay_fine_config()返回的系数
 */
static inline void gx_udelay_fine(unsigned int us, unsigned int coeff)
{
	int tick = (us * 100000) / coeff - (15 + ((us%5 + 1) * 3 / 2));
	tick = (tick <= 0 ) ? 1 : tick;
	unsigned int start = *(volatile unsigned int*)0xe000e018;
	while (1) {
		unsigned int cur = *(volatile unsigned int*)0xe000e018;
		if (cur > start) {
			if (cur + (0xffffff - start) > (unsigned int)tick)
				break;
		} else if ((start - cur) >= (unsigned int)tick)
			break;
	}
}

/**
 * @brief 初始化us级定时器
 *
 * @param timeout_us 定时上报时间(必须为62.5us的整数倍)
 * @param func 定时上报回调函数
 *
 * @return 定时器初始化是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_timer_us_init(int timeout_us, int (*func)(void*));
#endif

#endif

/** @}*/
