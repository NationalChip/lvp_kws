/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * board_misc_config.h
 *
 */

#ifndef __BOARD_MISC_CONFIG_H__
#define __BOARD_MISC_CONFIG_H__

#define BOARD_HAS_HW_I2C    0

#define BOARD_HAS_LED_1    1
#if (BOARD_HAS_LED_1 == 1)

#if (BOARD_HAS_HW_I2C == 1)
#define LED_1_PIN_ID    10
#else
#define LED_1_PIN_ID    3
#endif /* BOARD_HAS_HW_I2C */

#define LED_1_PIN_FUNC_GPIO    1
#define LED_1_PIN_FUNC_DEFAULT    0

#endif /* BOARD_HAS_GPIO_LED_1 */


#define BOARD_HAS_LED_2    1
#if (BOARD_HAS_LED_2 == 1)

#if (BOARD_HAS_HW_I2C == 1)
#define LED_2_PIN_ID    9
#else
#define LED_2_PIN_ID    4
#endif

#define LED_2_PIN_FUNC_GPIO    1
#define LED_2_PIN_FUNC_DEFAULT    0

#endif /* BOARD_HAS_GPIO_LED_2 */

#define BOARD_HAS_NOTIFY_PIN    1
#if (BOARD_HAS_NOTIFY_PIN == 1)

#define NOTIFY_PIN_ID    0
#define NOTIFY_PIN_FUNC_GPIO    1
#define NOTIFY_PIN_FUNC_DEFAULT    0

#endif /* BOARD_HAS_NOTIFY_PIN */


#endif /* __BOARD_MISC_CONFIG_H__ */


