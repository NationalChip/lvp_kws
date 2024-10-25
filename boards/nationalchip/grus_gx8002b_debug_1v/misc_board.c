/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * misc_board.h
 *
 */

#include <common.h>
#include <lvp_board.h>
#include <driver/gx_padmux.h>
#include <driver/gx_gpio.h>
#include <board_misc_config.h>

static const GX_PIN_CONFIG pin_table[] = {
/*   id  func     func0                 | func1          |  func2              | func3 ... */
#ifdef CONFIG_BOARD_GX8002B_DEBUG_1V0
    { 0,  1},  // 0:JTAG_TMS            | 1:GPIO_00      | 2:I2S_IN_BCLK_OUT_M | 3:I2S_OUT_BCLK_OUT_M
               // 4:SPI_CSn_M           | 5:SPI_CSn_S    | 6:REC_BCLK_OUT      | 7:PDM_DATA
               // 8:I2S_OUT_BCLK_IN_S   | 9:REC_BCLK_IN  | 10:I2S_IN_BCLK_IN_S
    { 1,  1},  // 0:JTAG_CK             | 1:GPIO_01      | 2:I2S_IN_DATA_IN     | 3:I2S_OUT_DATA_OUT
               // 4:SPI_MISO_M          | 5:SPI_MISO_S   | 6:REC_DATA_OUT       | 7:PDM_CLK_IN
               // 8:PWM_CLK_IN          | 9:PDM_CLK_OUT
    { 2,  0},  // 0:GPIO_02             | 1:PWM_CLK_IN    //i2c addr decision: low->0x35, high->0x36

    { 3,  1},  // 0:I2C0_SDA            | 1:GPIO_03      | 2:PDM_CLK_IN         | 3:PDM_CLK_OUT

    { 4,  1},  // 0:I2C0_SCL            | 1:GPIO_04      | 2:PDM_DATA

    { 5,  1},  // 0:UART0_TX            | 1:GPIO_05      | 2:I2C1_SDA           | 3:PDM_CLK_IN
               // 4:DAC_OUT_L           | 5:PDM_CLK_OUT
    { 6,  1},  // 0:UART0_RX            | 1:GPIO_06      | 2:I2C1_SCL           | 3:PDM_DATA
               // 4:DAC_OUT_R           | 5:PWM_CLK_IN
    { 7,  1},  // 0:PDM_CLK_IN          | 1:GPIO_07      | 2:I2S_IN_MCLK_OUT_M  | 3:I2S_OUT_MCLK_OUT_M
               // 4:SPI_SCLK_M          | 5:SPI_SCLK_S   | 6:REC_MCLK_OUT       | 7:DAC_OUT_L
               // 8:I2S_OUT_MCLK_IN_S   | 9:REC_MCLK_IN  | 10:I2S_IN_MCLK_IN_S  | 11:UART0_CTS
               // 12:PDM_CLK_OUT
    { 8,  1},  // 0:PDM_DATA            | 1:GPIO_08      | 2:I2S_IN_LRCLK_OUT_M | 3:I2S_OUT_LRCLK_OUT_M
               // 4:SPI_MOSI_M          | 5:SPI_MOSI_S   | 6:REC_LRCLK_OUT      | 7:DAC_OUT_R
               // 8:I2S_OUT_LRCLK_IN_S  | 9:REC_LRCLK_IN | 10:I2S_IN_LRCLK_IN_S | 11:UART0_RTS
               // 12:PWM_CLK_IN
    { 9,  1},  // 0:I2C1_SDA            | 1:GPIO_09      | 2:I2S_IN_BCLK_OUT_M  | 3:I2S_OUT_BCLK_OUT_M
               // 4:SPI_CSn_M           | 5:SPI_CSn_S    | 6:REC_BCLK_OUT       | 7:UART1_CTS
               // 8:I2S_OUT_BCLK_IN_S   | 9:REC_BCLK_IN  | 10:I2S_IN_BCLK_IN_S
    {10,  1},  // 0:I2C1_SCL            | 1:GPIO_10      | 2:I2S_IN_DATA_IN     | 3:I2S_OUT_DATA_OUT
               // 4:SPI_MISO_M          | 5:SPI_MISO_S   | 6:REC_DATA_OUT       | 7:UART1_RTS
               // 8:PWM_CLK_IN
    {11,  1},  // 0:UART1_TX            | 1:GPIO_11      | 2:I2S_IN_MCLK_OUT_M  | 3:I2S_OUT_MCLK_OUT_M
               // 4:SPI_SCLK_M          | 5:SPI_SCLK_S   | 6:REC_MCLK_OUT       | 7:DAC_OUT_L
               // 8:I2S_OUT_MCLK_IN_S   | 9:REC_MCLK_IN  | 10:I2S_IN_MCLK_IN_S
    {12,  1},  // 0:UART1_RX            | 1:GPIO_12      | 2:I2S_IN_LRCLK_OUT_M | 3:I2S_OUT_LRCLK_OUT_M
               // 4:SPI_MOSI_M          | 5:SPI_MOSI_S   | 6:REC_LRCLK_OUT      | 7:DAC_OUT_R
               // 8:I2S_OUT_LRCLK_IN_S  | 9:REC_LRCLK_IN | 10:I2S_IN_LRCLK_IN_S
# endif
};

static int s_board_init_flag = 0; // 1: finish init

static int _BoardPadmuxSet(int pin_id, int function)
{
    if (s_board_init_flag == 0) {
        int ret = padmux_check(pin_id, pin_id == 2 ? 0 :1);
        if (0 != ret) {
            printf("pin %d function conflict !\n", pin_id);
            return -1;
        }
    }

    padmux_set(pin_id, function);
    return 0;
}

void BoardSetUserPinMux(void)
{
    int ret = 0;
#if !defined(CONFIG_BOARD_NO_DMIC)
    ret += _BoardPadmuxSet(0, 7);
    ret += _BoardPadmuxSet(1, 9);
#endif

#if defined(CONFIG_BOARD_HAS_JTAG)
    ret += _BoardPadmuxSet(0, 0);
    ret += _BoardPadmuxSet(1, 0);
#endif

#if (defined(CONFIG_BOARD_HAS_I2C0) || defined(CONFIG_BOARD_HAS_HW_I2C))
    ret += _BoardPadmuxSet(3, 0);
    ret += _BoardPadmuxSet(4, 0);
#endif

#if defined(CONFIG_BOARD_HAS_I2C1)
    ret += _BoardPadmuxSet(5, 2);
    ret += _BoardPadmuxSet(6, 2);
#endif
#if defined(CONFIG_BOARD_HAS_UART0)
    ret += _BoardPadmuxSet(5, 0);
    ret += _BoardPadmuxSet(6, 0);
#endif

#if defined(CONFIG_BOARD_HAS_UART1)
    ret += _BoardPadmuxSet(11, 0);
# ifndef CONFIG_BOARD_HAS_UART1_JUST_SUPPORT_PRINTF_LOG
    ret += _BoardPadmuxSet(12, 0);
# endif
#endif

#if defined(CONFIG_BOARD_HAS_SPI)
    ret += _BoardPadmuxSet(7, 4);
    ret += _BoardPadmuxSet(8, 4);
    ret += _BoardPadmuxSet(9, 4);
    ret += _BoardPadmuxSet(10, 4);
#endif

#if defined(CONFIG_BOARD_HAS_AIN_I2S_MASTER)
    ret += _BoardPadmuxSet(0, 2);
    ret += _BoardPadmuxSet(1, 2);
    ret += _BoardPadmuxSet(11, 2);
    ret += _BoardPadmuxSet(12, 2);
#endif

#if defined(CONFIG_BOARD_HAS_AIN_I2S_SLAVE)
    ret += _BoardPadmuxSet(0, 10);
    ret += _BoardPadmuxSet(1, 2);
    ret += _BoardPadmuxSet(11, 10);
    ret += _BoardPadmuxSet(12, 10);
#endif

#if defined(CONFIG_BOARD_HAS_I2S_FIVE_WIRE_MODE)
    ret += _BoardPadmuxSet(0, 2);
    ret += _BoardPadmuxSet(1, 2);
    ret += _BoardPadmuxSet(10, 3);
    ret += _BoardPadmuxSet(11, 2);
    ret += _BoardPadmuxSet(12, 2);
#endif

#if defined(CONFIG_BOARD_AOUT_DAC_OUT)
#endif

    if (0 != ret) {
        padmux_set(5, 0);
        padmux_set(6, 0);
        padmux_set(11, 0);
        padmux_set(12, 0);
        printf("!!!pin set error!\n Please check Board Options!!!");
        while(1);
    }
}

void BoardSetPowerSavePinMux(void)
{
    for (int pin_id = 0; pin_id < 13; pin_id++) {
        int ret = padmux_check(pin_id, pin_id == 2 ? 0 :1);      // 排查被复用为gpio之外功能的pin
        if (0 != ret) {
            padmux_set(pin_id, pin_id == 2 ? 0 :1);              // 排查出的pin设为gpio高电平
            gx_gpio_set_direction(pin_id, GX_GPIO_DIRECTION_INPUT);
            gx_gpio_set_level(pin_id, GX_GPIO_LEVEL_HIGH);
        }
    }
}

void BoardInit(void) {
    int table_size = ARRAY_SIZE(pin_table);

    padmux_init(pin_table, table_size);

    for (int i = 0; i < table_size; i++) {
        int ret = padmux_check(pin_table[i].pin_id, pin_table[i].function);
        if (0 != ret) {
            printf("pin %d set error!\n", pin_table[i].pin_id);
        }

        // set gpio status
        if(((pin_table[i].pin_id == 2) && (pin_table[i].function == 0))
                || ((pin_table[i].pin_id != 2) && (pin_table[i].function == 1))) {
            gx_gpio_set_direction(pin_table[i].pin_id, GX_GPIO_DIRECTION_INPUT);
            gx_gpio_set_level(pin_table[i].pin_id, GX_GPIO_LEVEL_HIGH);
        }
    }

    BoardSetUserPinMux();
    s_board_init_flag = 1;
}

