/* LVP
 * Copyright (C) 1991-2017 Nationalchip Co., Ltd
 *
 * lvp_standby_ratio.h:
 *
 */

#ifndef __LVP_STANDBY_RATIO_H__
#define __LVP_STANDBY_RATIO_H__

void LvpStandbyRatioInit(void);
int LvpCountRealTimeStandbyRatio(void);
int LvpCountAverageStandbyRatio(void);

#endif /* __LVP_STANDBY_RATIO_H__ */
