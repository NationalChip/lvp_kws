/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * board_misc_config.h
 *
 */

#ifndef __BOARD_MISC_CONFIG_H__
#define __BOARD_MISC_CONFIG_H__

#define BOARD_HAS_NECK_FAN
#if defined(BOARD_HAS_NECK_FAN)
# define NECK_FAN_LED_PIN 9
# define NECK_FAN_SWITCH_PIN 2
# define NECK_FAN_SPEED_PIN 3
# define NECK_FAN_BUTTON_PIN 4
# define LVP_FAN_KEY_PRESSED_VAL 0
# endif

#endif /* __BOARD_MISC_CONFIG_H__ */


