/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_pmu.h:
 *
 */

#ifndef __LVP_PMU_H__
#define __LVP_PMU_H__

typedef int (*LVP_SUSPEND_CALLBACK)(void *priv);
typedef int (*LVP_RESUME_CALLBACK)(void *priv);

typedef struct {
    LVP_SUSPEND_CALLBACK suspend_callback;
    void *priv;
} LVP_SUSPEND_INFO;

typedef struct {
    LVP_RESUME_CALLBACK resume_callback;
    void *priv;
} LVP_RESUME_INFO;

typedef enum {
    LRT_GPIO     =   0x01,
    LRT_RTC      =   0x02,
    LRT_AUDIO_IN =   0x04,
    LRT_I2C      =   0x08,
} LVP_RESUME_TYPE;


int LvpPmuInit(void);

int LvpSuspendInfoRegist(LVP_SUSPEND_INFO *suspend_info);

int LvpResumeInfoRegist(LVP_RESUME_INFO *resume_info);

int LvpPmuSuspendLockCreate(int *lock);

int LvpPmuSuspendLockDestory(int lock);

int LvpPmuSuspendLock(int lock);

int LvpPmuSuspendUnlock(int lock);

int LvpPmuSuspendIsLocked(void);

int LvpPmuSuspend(int type); // LVP_RESUME_TYPE

int LvpPmuSoftOff(void);

int LvpPmuResume(void);


#endif /* __LVP_PMU_H__ */

