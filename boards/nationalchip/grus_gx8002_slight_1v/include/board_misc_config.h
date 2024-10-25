/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * board_misc_config.h
 *
 */

#ifndef __BOARD_MISC_CONFIG_H__
#define __BOARD_MISC_CONFIG_H__

#ifdef CONFIG_BOARD_GX8002_SLIGHT_4KEY_1V0
#define BOARD_HAS_SLIGHT_LED
#if defined(BOARD_HAS_SLIGHT_LED)
# define LED_WARM_PIN 1
# define LED_COLD_PIN 0
# define LED_SWITCH_PIN 7
# define LED_MODE_PIN 9
# define LED_BRIGHTEN_PIN 10
# define LED_DIM_PIN 8
# define LVP_SLIGHT_KEY_PRESSED_VAL 0
#endif
#endif

#ifdef CONFIG_BOARD_GX8002_SLIGHT_6KEY_1V0
#define BOARD_HAS_SLIGHT_LED
#if defined(BOARD_HAS_SLIGHT_LED)
# define LED_WARM_PIN 1
# define LED_COLD_PIN 0
# define LED_SWITCH_PIN 8
# define LED_MODE_PIN 6
# define LED_BRIGHTEN_PIN 10
# define LED_DIM_PIN 9
# define LED_WARM_MODE_PIN 5
# define LED_COLD_MODE_PIN 7
# define LVP_SLIGHT_KEY_PRESSED_VAL 0
#endif
#endif



#endif /* __BOARD_MISC_CONFIG_H__ */


