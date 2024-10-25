#include <autoconf.h>
#include <driver/misc.h>
#include <driver/gx_clock.h>
#include <driver/gx_pmu_ctrl.h>

#include "soc.h"
#include "csi_core.h"

/*
 * 内存权限配置
 *  0-1G 可执行, 可读写, 非安全区域
 *  2-4G 不可执行, 可读写, 非安全区域
 */
static inline void system_mpu_init(void)
{
    mpu_region_attr_t attr;

    attr.nx = 0;
    attr.ap = AP_BOTH_RW;
    attr.s = 0;
    csi_mpu_config_region(0, 0x0, REGION_SIZE_4GB, attr, 1);

    csi_mpu_enable();
}

static inline void system_vic_init(void)
{
    int i;

    __set_VBR((uint32_t) & (__Vectors));    /* Set VBR */

    VIC->TSPR = 0xFF;

    /* Clear active and pending IRQ */
    for (i = 0; i < 4; i++) {
        VIC->IABR[i] = 0x0;
        VIC->ICPR[i] = 0xFFFFFFFF;
    }

    /* Enable interrupts and exceptions */
    __enable_excp_irq();
}

extern void clear_bss(void);
static inline void system_bss_clear(void)
{
    /* 只有从ROM启动时才清bss段 */
    if (gx_pmu_get_start_mode() == GX_START_MODE_ROM)
        clear_bss();
}

extern void clk_init(void);
static inline void system_clk_init(void)
{
    clk_init();
}

static inline void system_cache_init(void)
{
#ifndef CONFIG_GDB_DEBUG
    gx_cache_init();
#endif
}

extern void board_init(void);
void system_init(void)
{
    system_mpu_init();
    system_clk_init();
    system_bss_clear();
    board_init();
    system_vic_init();

}
