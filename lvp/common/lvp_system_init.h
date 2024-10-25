/* LVP
 * Copyright (C) 1991-2021 Nationalchip Co., Ltd
 *
 * lvp_system_init.h:
 *
 */

#ifndef __LVP_SYSTEM_INIT_H__
#define __LVP_SYSTEM_INIT_H__
#include <autoconf.h>
typedef enum
{
    CPU_FREQUENCE_4M  = 4,
    CPU_FREQUENCE_6M  = 6,
    CPU_FREQUENCE_8M  = 8,
    CPU_FREQUENCE_12M = 12,
    CPU_FREQUENCE_24M = 24,
    CPU_FREQUENCE_50M = 50,
    CPU_FREQUENCE_DEFAULT = 0,
} CPU_FREQUENCE_LEVEL;

typedef enum
{
    SYSTEM_FREQUENCE_STANDARD_SPEED,
    SYSTEM_FREQUENCE_HIGH_SPEED,
    SYSTEM_FREQUENCE_CUSTOMIZE_SPEED
} SYSTEM_FREQUENCE_LEVEL;

typedef enum
{
    GPIO_FREQUENCE_STANDARD_SPEED,
    GPIO_FREQUENCE_HIGH_SPEED,
} GPIO_FREQUENCE_LEVEL;

void LvpSystemInit(void);
void LvpSystemDone(void);

int LvpDynamiciallyAdjustCpuFrequency(CPU_FREQUENCE_LEVEL cpu_frequence_level);
int LvpDynamiciallyAdjustSystemFrequency(SYSTEM_FREQUENCE_LEVEL system_frequence_level);
int LvpDynamiciallyAdjustGpioFrequency(GPIO_FREQUENCE_LEVEL gpio_frequence_level);

int LvpXipSuspend(void);
int LvpXipResume(void);

int LvpStackInit(void);
int LvpStackSecurityMonitoring(void);

#ifdef CONFIG_BOARD_SUPPORT_MULTIBOOT
void SwitchAnotherFirmeware(void);
#endif
#endif /* __LVP_SYSTEM_INIT_H__ */

