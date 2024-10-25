/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_pmu.c:
 *
 */

#include <autoconf.h>
#include <stdio.h>
#include <csi_core.h>

#include <driver/gx_pmu_ctrl.h>
#include <driver/gx_audio_in.h>
#include <driver/gx_snpu.h>
#include <driver/gx_irq.h>
#include <driver/gx_clock.h>

#include <lvp_audio_in.h>
#include <lvp_app_core.h>
#include <lvp_pmu.h>
#include <lvp_queue.h>

#define LOG_TAG "[PMU]"

#define SUSPEND_INFO_NUM 8
#define RESUME_INFO_NUM 8

typedef struct {
    unsigned int suspend_lock;
    unsigned int suspend_lock_mask;
    unsigned int s_suspend_info_num;
    unsigned int s_resume_info_num;
    LVP_SUSPEND_INFO s_suspend_info_buffer[SUSPEND_INFO_NUM];
    LVP_RESUME_INFO s_resume_info_buffer[RESUME_INFO_NUM];
} LVP_STANDBY_HANDLE;

static LVP_STANDBY_HANDLE s_handle;

int LvpPmuInit(void)
{
    GX_WAKEUP_SOURCE start_mode = gx_pmu_get_wakeup_source();
    if (start_mode == GX_WAKEUP_SOURCE_COLD || start_mode == GX_WAKEUP_SOURCE_WDT) {
        memset((void *)&s_handle, 0, sizeof(LVP_STANDBY_HANDLE));
        return 1;
    } else {
        for (int i = 0; i < s_handle.s_resume_info_num; i++) {
            s_handle.s_resume_info_buffer[i].resume_callback(s_handle.s_resume_info_buffer[i].priv);
        }
        return 0;
    }

}

int LvpSuspendInfoRegist(LVP_SUSPEND_INFO *suspend_info)
{
    for(int i = 0; i < SUSPEND_INFO_NUM; i++) {
        if(s_handle.s_suspend_info_buffer[i].suspend_callback == suspend_info->suspend_callback) {
            memcpy((void *)&s_handle.s_suspend_info_buffer[i], \
                   (const void *)suspend_info, sizeof(LVP_SUSPEND_INFO));
            return 0;
        }
    }

    if (s_handle.s_suspend_info_num < SUSPEND_INFO_NUM && suspend_info->suspend_callback != NULL) {
        memcpy((void *)&s_handle.s_suspend_info_buffer[s_handle.s_suspend_info_num], \
               (const void *)suspend_info, sizeof(LVP_SUSPEND_INFO));
        s_handle.s_suspend_info_num++;
    } else {
        return -1;
    }

    return 0;
}

int LvpResumeInfoRegist(LVP_RESUME_INFO *resume_info)
{
    for(int i = 0; i < RESUME_INFO_NUM; i++) {
        if(s_handle.s_resume_info_buffer[i].resume_callback == resume_info->resume_callback) {
            memcpy((void *)&s_handle.s_resume_info_buffer[i], \
                   (const void *)resume_info, sizeof(LVP_RESUME_INFO));
            return 0;
        }
    }

    if (s_handle.s_resume_info_num < RESUME_INFO_NUM && resume_info->resume_callback != NULL) {
        memcpy((void *)&s_handle.s_resume_info_buffer[s_handle.s_resume_info_num], \
               (const void *)resume_info, sizeof(LVP_RESUME_INFO));
        s_handle.s_resume_info_num++;
    } else {
        return -1;
    }

    return 0;
}

int LvpPmuSuspendLockCreate(int *lock)
{
    for (int i = 0; i < 32; i++) {
        if (!(s_handle.suspend_lock_mask & (0x1 << i))) {
            *lock = i + 1;
            s_handle.suspend_lock_mask |= (0x1 << i);
            return *lock;
        }

        if (i == 32)
            return -1;
    }

    return -1;
}

int LvpPmuSuspendLockDestory(int lock)
{

    if (lock > 0 && lock <= 32) {
        if (!(s_handle.suspend_lock_mask & (0x1 << lock))) {
            return -1;
        }

        s_handle.suspend_lock_mask &= ~(0x1 << lock);
        return 0;
    } else {
        return -1;
    }
}

int LvpPmuSuspendLock(int lock)
{
    if (!(s_handle.suspend_lock_mask & (0x1 << (lock - 1)))) {
        return -1;
    }

    if (lock <= 32) {
        s_handle.suspend_lock |= (0x1 << (lock - 1));
        return 0;
    } else if (lock != 0) {
        return -1;
    } else {
        s_handle.suspend_lock = 0;
        return 0;
    }

    return -1;
}

int LvpPmuSuspendUnlock(int lock)
{

    if (lock <= 32) {
        s_handle.suspend_lock &= ~(0x1 << (lock - 1));
        return 0;
    } else if (lock != 0) {
        return -1;
    } else {
        s_handle.suspend_lock = 0;
        return 0;
    }

    return -1;
}

int LvpPmuSuspendIsLocked(void)
{
    if (s_handle.suspend_lock)
        return 1;
    else
        return 0;
}


int LvpPmuSuspend(int type)
{
    unsigned int irq_state = gx_lock_irq_save();

    if (s_handle.suspend_lock) {
        gx_unlock_irq_restore(irq_state);
        return -1;
    }
    for (int i = 0; i < s_handle.s_suspend_info_num; i++) {
        s_handle.s_suspend_info_buffer[i].suspend_callback(s_handle.s_suspend_info_buffer[i].priv);
    }
    gx_unlock_irq_restore(irq_state);

    LvpAudioInSuspend();
    while (gx_snpu_get_state() == GX_SNPU_BUSY);
    gx_snpu_exit();

    struct gx_pmu_wakeup_addr wakeup_addr = {
        .wakeup_from = GX_PMU_WAKEUP_FROM_SRAM,
        .wakeup_address = CONFIG_STAGE2_IRAM_BASE + 0x100,
    };

    int data = type; //GX_PMU_WAKEUP_AUDIO_IN | GX_PMU_WAKEUP_GPIO;
    gx_pmu_ctrl_set(GX_PMU_CMD_WAKEUP_MODE, &data);
    gx_pmu_ctrl_set(GX_PMU_CMD_WAKEUP_ADDR, &wakeup_addr);

    //        printf("mcu gate          :0xa0300018 = 0x%x\n", readl(0xa0300018));
    //        printf("mcu div           :0xa0300080 = 0x%x, 0xa0300084 = 0x%x\n", readl(0xa0300080), readl(0xa0300084));
    //        printf("mcu source select :0xa0300088 = 0x%x\n", readl(0xa0300088));
    //
    //        printf("pmu gate          :0xa0010018 = 0x%x\n", readl(0xa0010018));
    //        printf("pmu div           :0xa0010080 = 0x%x, 0xa0010084 = 0x%x, 0xa0010088 = 0x%x\n", readl(0xa0010080), readl(0xa0010084), readl(0xa0010088));
    //        printf("pmu source select :0xa0010088 = 0x%x\n", readl(0xa001008C));

    gx_disable_all_interrupt();
    if (data & GX_PMU_WAKEUP_AUDIO_IN)
        gx_audio_in_set_interrupt_enable(AUDIO_IN_IRQ_FFTVAD_START, 1);

    printf("set mcu sleep\n");
    gx_pmu_ctrl_enable();

    return 0;
}

#define PMU_TRIM_CFG_REG        ((GX_REG_BASE_PMU) + 0x38) // 工作时电压trim值
#define PMU_SLEEP_EN            ((GX_REG_BASE_PMU) + 0x00) // 启动　pmu control

static int gx_pmu_ctrl_get_work_ldo_trim(uint32_t *trim_cfg)
{
    *trim_cfg = readl(PMU_TRIM_CFG_REG);

    return 0;
}

static int gx_pmu_ctrl_set_work_ldo_trim(uint32_t *trim_cfg)
{
    writel(*trim_cfg, PMU_TRIM_CFG_REG);

    return 0;
}

int LvpPmuSoftOff(void)
{
    unsigned int irq_state = gx_lock_irq_save();

    if (s_handle.suspend_lock) {
        gx_unlock_irq_restore(irq_state);
        return -1;
    }
    for (int i = 0; i < s_handle.s_suspend_info_num; i++) {
        s_handle.s_suspend_info_buffer[i].suspend_callback(s_handle.s_suspend_info_buffer[i].priv);
    }
    gx_unlock_irq_restore(irq_state);

    LvpAudioInDone();
    gx_snpu_exit();

    printf("set mcu Soft-Off\n");

    GX_PMU_TRIM_CFG trim_cfg;

    //　关中断
    gx_disable_all_interrupt();

    // 休眠参数配置
    GX_PMU_STATE sleep_state;
    gx_pmu_ctrl_get(GX_PMU_CMD_SLEEP_STATE, &sleep_state.value);
    sleep_state.bits.manual_ldo_adjust_en = 1;
    sleep_state.bits.manual_ldo_adjust_val = 8;
    sleep_state.bits.mcu_poweroff = 0;
    sleep_state.bits.osc_1m_en = 0;
    gx_pmu_ctrl_set(GX_PMU_CMD_SLEEP_STATE, &sleep_state.value);

    // 唤醒参数配置
    GX_PMU_STATE work_state;
    gx_pmu_ctrl_get(GX_PMU_CMD_WORK_STATE, &work_state.value);
    work_state.bits.manual_ldo_adjust_en = 1;
    work_state.bits.manual_ldo_adjust_val = 2; // 这个值是 slt 批量测试结果，不宜修改
    work_state.bits.mcu_poweroff = 0;
    gx_pmu_ctrl_set(GX_PMU_CMD_WORK_STATE, &work_state.value);

    // trim_en需要写回０, 否则休眠唤醒trim值不生效
    gx_pmu_ctrl_get_work_ldo_trim((uint32_t *)&trim_cfg);
    trim_cfg.bits.trim_en = 0;
    gx_pmu_ctrl_set_work_ldo_trim((uint32_t *)&trim_cfg);

    extern void clk_switch_soft_off(void);
    clk_switch_soft_off();
    //*(volatile unsigned int *)0xa001008c |= 1<<4;
    gx_clock_set_module_source(CLOCK_MODULE_PMU         , MODULE_SOURCE_32K);
    gx_clock_set_module_source(CLOCK_MODULE_SRAM         , MODULE_SOURCE_32K);
    //*(volatile unsigned int *)0xa000506c &= ~(1<<2);
    //printf("0xa001008c:%x\n", *(volatile unsigned int *)0xa001008c);
    //printf("0xa000506c:%x\n", *(volatile unsigned int *)0xa000506c);
    // mcu休眠使能
    writel(1, PMU_SLEEP_EN);

    // mcu进入休眠
    __DOZE();

    return 0;
}

int LvpPmuResume(void)
{
    return 0;
}



