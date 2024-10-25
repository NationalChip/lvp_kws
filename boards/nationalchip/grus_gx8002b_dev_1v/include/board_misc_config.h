/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * board_misc_config.h
 *
 */

#ifndef __BOARD_MISC_CONFIG_H__
#define __BOARD_MISC_CONFIG_H__

/// G_sensor
#ifdef CONFIG_LVP_HAS_G_SENSOR
# define BOARD_HAS_GSNEOR
//# define GSENSOR_USE_LIS2DW12
# define GSENSOR_USE_DS_DA662

#if defined(CONFIG_LVP_GSENSOR_USE_I2C)
# define GSENSOR_INTERRUPT_PIN_ID 1
#else
# define GSENSOR_INTERRUPT_PIN_ID 12
#endif
#endif

#endif /* __BOARD_MISC_CONFIG_H__ */

