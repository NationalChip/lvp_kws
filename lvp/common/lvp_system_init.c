/* LVP
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_system_init.c:
 *
 */
#include <autoconf.h>
#include <stdio.h>
#include <types.h>
#include <string.h>
#include <base_addr.h>
#include <board_config.h>

#include <driver/gx_uart.h>
#include <driver/gx_timer.h>
#include <driver/gx_flash.h>
#include <driver/gx_irq.h>
#include <driver/gx_i2c.h>
#include <driver/irr.h>
#include <driver/gx_gpio.h>
#include <driver/gx_delay.h>
#include <driver/gx_watchdog.h>
#include <driver/gx_rtc.h>
#include <driver/pm.h>
#include <driver/otp.h>
#include <driver/gx_dma_ahb.h>
#include <driver/gx_delay.h>
#include <driver/gx_gpio.h>
#include <driver/gx_audio_in.h>
#include <driver/gx_snpu.h>
#include <driver/gx_flash.h>
#include <driver/gx_timer.h>
#include <driver/gx_pmu_ctrl.h>
#include <driver/misc.h>
#include <driver/spi.h>
#include <driver/device.h>
#include <driver/gx_clock.h>
#include <driver/gx_analog/gx_ldo.h>
#include <driver/gx_dl.h>
#include <driver/gx_rtc.h>
#include <driver/gx_cache.h>

#include <lvp_board.h>
#include <lvp_param.h>
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
#include <ctc_model.h>
#include <decoder.h>
#endif

#include <lvp_pmu.h>
#include <board_misc_config.h>
#include <lvp_system_init.h>
#include <lvp_standby_ratio.h>

#define LOG_TAG "[LVP]"

__attribute__((unused)) static int _GetCpuFreq(void)
{
    return gx_clock_get_module_frequence(CLOCK_MODULE_SCPU);
}

__attribute__((unused)) static int _GetSramFreq(void)
{
    return gx_clock_get_module_frequence(CLOCK_MODULE_SRAM);
}

__attribute__((unused)) static int _GetNpuFreq(void)
{
    return gx_clock_get_module_frequence(CLOCK_MODULE_NPU);
}
__attribute__((unused)) static int _GetFlashFreq(void)
{
    return gx_clock_get_module_frequence(CLOCK_MODULE_FLASH_SPI);
}

__attribute__((unused)) static int _GetLdoTrim(void)
{
    GX_PMU_TRIM_CFG cfg;
    gx_pmu_ctrl_get(GX_PMU_CMD_WORK_LDO_TRIM, &cfg);

    switch(cfg.bits.trim_val) {
        case LDO_DIG_VOLTAGE_0_950V:
            return 950;
        case LDO_DIG_VOLTAGE_0_924V:
            return 924;
        case LDO_DIG_VOLTAGE_0_897V:
            return 897;
        case LDO_DIG_VOLTAGE_0_871V:
            return 871;
        case LDO_DIG_VOLTAGE_0_845V:
            return 845;
        case LDO_DIG_VOLTAGE_0_819V:
            return 819;
        case LDO_DIG_VOLTAGE_0_792V:
            return 792;
        case LDO_DIG_VOLTAGE_0_766V:
            return 766;
        case LDO_DIG_VOLTAGE_0_740V:
            return 740;
        case LDO_DIG_VOLTAGE_0_714V:
            return 714;
        case LDO_DIG_VOLTAGE_0_687V:
            return 687;
        case LDO_DIG_VOLTAGE_0_661V:
            return 661;
    }

    return -1;
}

void LvpSystemInit(void)
{
#ifndef CONFIG_MCU_ENABLE_JTAG_DEBUG
    gx_cache_init();
#endif
    gx_dma_init();
#ifdef CONFIG_MCU_ENABLE_PRINTF
# ifdef CONFIG_MCU_PRINT_UART_PORT_0
    gx_console_init(0, CONFIG_SERIAL_BAUD_RATE);
# elif (defined CONFIG_MCU_PRINT_UART_PORT_1)
    gx_console_init(1, CONFIG_SERIAL_BAUD_RATE);
# else
    gx_console_init(1, CONFIG_SERIAL_BAUD_RATE);
# endif
#endif

    GX_WAKEUP_SOURCE start_mode = gx_pmu_get_wakeup_source();
    if (start_mode == GX_WAKEUP_SOURCE_COLD || start_mode == GX_WAKEUP_SOURCE_WDT) {
        printf(LOG_TAG"Low-Power Voice Preprocess\n");
        printf(LOG_TAG"Copyright (C) 2001-2020 NationalChip Co., Ltd\n");
        printf(LOG_TAG"ALL RIGHTS RESERVED!\n");
        printf(LOG_TAG"Board Model: [%s]\n", CONFIG_BOARD_MODEL);
        printf(LOG_TAG"MCU Version: [%s]\n", MCU_VERSION);
        printf(LOG_TAG"Release Ver: [0x%x]\n", RELEASE_VERSION);
        printf(LOG_TAG"Build Date : [%s]\n", BUILD_TIME);

#if defined (CONFIG_BOOT_BY_FLASH)
        // Flash Info
        GX_FLASH_DEV *flash   = gx_spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS, CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
        unsigned int flash_id = gx_spi_flash_getinfo(flash, GX_FLASH_CHIP_ID);
        if (flash_id == 0x00854012 || flash_id == 0x00856013) {
            printf(LOG_TAG"Flash vendor:[%s]\n", "PUYA");
        }
        if (flash_id == 0x001c3812 || flash_id == 0x001c3813) {
            printf(LOG_TAG"Flash vendor:[%s]\n", "ESMT");
        }
        printf(LOG_TAG"Flash type:  [%s]\n",      gx_spi_flash_gettype(flash));
        printf(LOG_TAG"Flash ID:    [%#x]\n",     flash_id);
        printf(LOG_TAG"Flash size:  [%d Byte]\n", gx_spi_flash_getinfo(flash, GX_FLASH_CHIP_SIZE));
#endif
#ifdef CONFIG_MCU_ENABLE_DYNAMICALLY_ADJUSTMENT_CPU_FREQUENCY
        printf(LOG_TAG"CPU   Freq:  [%d Hz][Auto]\n", _GetCpuFreq());
#else
        printf(LOG_TAG"CPU   Freq:  [%d Hz][fix]\n", _GetCpuFreq());
#endif
        printf(LOG_TAG"SRAM  Freq:  [%d Hz]\n",      _GetSramFreq());
        printf(LOG_TAG"NPU   Freq:  [%d Hz]\n",      _GetNpuFreq());
#if defined (CONFIG_BOOT_BY_FLASH)
        printf(LOG_TAG"FLASH Freq:  [%d Hz]\n",      _GetFlashFreq());
#endif
        int result = gx_analog_get_ldo_dig_ctrl();
        if (result == LDO_SW_CTRL_BYPASS) {
            printf(LOG_TAG"enable bypass core Ldo\n");
        }
        else {
            printf(LOG_TAG"Ldo   Trim:  [%d mV]\n",      _GetLdoTrim());
        }

#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
# if (defined CONFIG_LVP_ENABLE_CTC_DECODER) || (defined CONFIG_LVP_ENABLE_CTC_GX_DECODER) || (defined CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER)
        printf(LOG_TAG"Kws Version: [%s]\n", LvpCTCModelGetKwsVersion());
        printf(LOG_TAG"Ctc Version: v0.1.3\n");
        LvpPrintCtcKwsList();
# endif
#ifdef CONFIG_LVP_ENABLE_MAX_DECODER
        LvpPrintMaxKwsList();
#endif
#endif

#ifdef CONFIG_MCU_ENABLE_STACK_MONITORING
        LvpStackInit();
#endif
        gx_gpio_init();
        BoardInit();
#if defined (CONFIG_BOOT_BY_UART)
        gx_uart_init(UART_BOOT_PORT, 1500000);
        gx_uart_send_buffer(UART_BOOT_PORT, (unsigned char*)"8002", 4);
#endif
        gx_rtc_init();
        gx_rtc_set_tick(0);
        gx_rtc_start_tick();
    }

    gx_timer_init();
    gx_irq_init();
    device_list_init();
    spi_master_v3_probe();

#ifdef CONFIG_DL
    int ret = dl_init();
#endif

    LvpPmuInit();

#ifdef CONFIG_ENABLE_CACULATE_STANDBY_RATIO
    LvpStandbyRatioInit(); // Need Called Behind LvpPmuInit
#endif
}

void LvpSystemDone(void)
{

}

int LvpDynamiciallyAdjustGpioFrequency(GPIO_FREQUENCE_LEVEL gpio_frequence_level)
{
    unsigned int state = gx_lock_irq_save();
#if (defined CONFIG_ENABLE_PLL_FREQUENCY_50M || defined CONFIG_ENABLE_PLL_FREQUENCY_48M)
    int pll_multiple = 2;
#else
    int pll_multiple = 1;
#endif
    if (gpio_frequence_level == GPIO_FREQUENCE_HIGH_SPEED) {
        gx_clock_set_module_source(CLOCK_MODULE_PMU         , MODULE_SOURCE_24M_PLL);
        gx_clock_set_module_source(CLOCK_MODULE_GPIO        , MODULE_SOURCE_24M_PLL);
        gx_clock_set_div(CLOCK_MODULE_PMU         ,  1 * pll_multiple);
    } else if (gpio_frequence_level == GPIO_FREQUENCE_STANDARD_SPEED) {
        gx_clock_set_module_source(CLOCK_MODULE_PMU         , MODULE_SOURCE_1M_12M);
        gx_clock_set_module_source(CLOCK_MODULE_GPIO        , MODULE_SOURCE_1M_12M);
        gx_clock_set_div(CLOCK_MODULE_PMU         ,  3);
    }

    gx_unlock_irq_restore(state);
    return 0;
}

int LvpDynamiciallyAdjustCpuFrequency(CPU_FREQUENCE_LEVEL cpu_frequence_level)
{
    unsigned int state = gx_lock_irq_save();
#if (defined CONFIG_ENABLE_PLL_FREQUENCY_50M || defined CONFIG_ENABLE_PLL_FREQUENCY_48M)
    int pll_multiple = 2;
#else
    int pll_multiple = 1;
#endif
    if (cpu_frequence_level == CPU_FREQUENCE_4M) {
        gx_clock_set_div(CLOCK_MODULE_SCPU, 6 * pll_multiple);
    } else if (cpu_frequence_level == CPU_FREQUENCE_6M) {
        gx_clock_set_div(CLOCK_MODULE_SCPU, 4 * pll_multiple);
    } else if (cpu_frequence_level == CPU_FREQUENCE_8M) {
        gx_clock_set_div(CLOCK_MODULE_SCPU, 3 * pll_multiple);
    } else if (cpu_frequence_level == CPU_FREQUENCE_12M) {
        gx_clock_set_div(CLOCK_MODULE_SCPU, 2 * pll_multiple);
    } else if (cpu_frequence_level == CPU_FREQUENCE_24M) {
        gx_clock_set_div(CLOCK_MODULE_SCPU, 1 * pll_multiple);
    } else if (cpu_frequence_level == CPU_FREQUENCE_50M) {
        if (pll_multiple == 2) {
            gx_clock_set_div(CLOCK_MODULE_SCPU, 0);
        } else {
            gx_unlock_irq_restore(state);
            return -1;
        }
    } else if (cpu_frequence_level == CPU_FREQUENCE_DEFAULT) {
        gx_clock_set_div(CLOCK_MODULE_SCPU, CUSTOMIZE_FREQUENCY_MCU_DIV_PARAM);
    } else {
        gx_unlock_irq_restore(state);
        return -1;
    }
# ifdef CONFIG_SYSTICK_COUNTER
    gx_timer_init();
# endif
    gx_unlock_irq_restore(state);
    return 0;
}

DRAM0_STAGE2_SRAM_ATTR int LvpDynamiciallyAdjustSystemFrequency(SYSTEM_FREQUENCE_LEVEL system_frequence_level)
{
    if (system_frequence_level == SYSTEM_FREQUENCE_STANDARD_SPEED) {
        gx_clock_set_div(CLOCK_MODULE_SCPU,         STANDARD_FREQUENCY_MCU_DIV_PARAM);  // 8M
        gx_clock_set_div(CLOCK_MODULE_NPU,          STANDARD_FREQUENCY_NPU_DIV_PARAM);  // 8M
        gx_clock_set_div(CLOCK_MODULE_FLASH_SPI,    STANDARD_FREQUENCY_FLASH_DIV_PARAM);// 24
    } else if (system_frequence_level == SYSTEM_FREQUENCE_HIGH_SPEED) {
        gx_clock_set_div(CLOCK_MODULE_SCPU,         HIGH_FREQUENCY_MCU_DIV_PARAM);      // 24M
        gx_clock_set_div(CLOCK_MODULE_NPU,          HIGH_FREQUENCY_NPU_DIV_PARAM);      // 24M
        gx_clock_set_div(CLOCK_MODULE_FLASH_SPI,    HIGH_FREQUENCY_FLASH_DIV_PARAM);    // 50M
    } else if (system_frequence_level == SYSTEM_FREQUENCE_CUSTOMIZE_SPEED) {
        gx_clock_set_div(CLOCK_MODULE_SCPU,         CUSTOMIZE_FREQUENCY_MCU_DIV_PARAM);
        gx_clock_set_div(CLOCK_MODULE_NPU,          CUSTOMIZE_FREQUENCY_NPU_DIV_PARAM);
        gx_clock_set_div(CLOCK_MODULE_FLASH_SPI,    CUSTOMIZE_FREQUENCY_FLASH_DIV_PARAM);
    } else {
        gx_clock_set_div(CLOCK_MODULE_SCPU,         MCU_DIV_PARAM);
        gx_clock_set_div(CLOCK_MODULE_NPU,          NPU_DIV_PARAM);
        gx_clock_set_div(CLOCK_MODULE_FLASH_SPI,    FLASH_DIV_PARAM);
        return -1;
    }

    return 0;
}

#if defined (CONFIG_BOOT_BY_FLASH)
DRAM0_STAGE2_SRAM_ATTR int LvpXipSuspend(void)
{
#ifdef CONFIG_KWS_TYPE_HYBRID
    return 0;
#endif

#ifndef CONFIG_NPU_RUN_IN_FLASH
    gx_disable_irq();
    gx_spinor_flash_suspend();
#endif
    return 0;
}

DRAM0_STAGE2_SRAM_ATTR int LvpXipResume(void)
{
#ifdef CONFIG_KWS_TYPE_HYBRID
    return 0;
#endif

#ifndef CONFIG_NPU_RUN_IN_FLASH
    gx_spinor_flash_resume();
    extern GX_FLASH_DEV * xip_sflash_init(void);
    xip_sflash_init();
    gx_enable_irq();
#endif

    return 0;
}
#endif

DRAM0_STAGE2_SRAM_ATTR int LvpStackInit(void)
{
    extern int _start_stack_section_ , _end_stack_section_;
    memset(&_start_stack_section_, 0, (unsigned int)&_end_stack_section_ - (unsigned int)&_start_stack_section_);
    printf (LOG_TAG" The Stack Scope:[%#x - %#x], Stack Size:%x\n", CONFIG_STAGE2_STACK, &_start_stack_section_, CONFIG_STAGE2_STACK - (int)&_start_stack_section_);

    return 0;
}

DRAM0_STAGE2_SRAM_ATTR int LvpStackSecurityMonitoring(void)
{
    extern int _start_stack_section_;
    unsigned int *start_stack = (unsigned int*)&_start_stack_section_;
    for (int i = 0; i < 50; i++) {
        if (start_stack[i] != 0) {
            printf (LOG_TAG"Error, The Stack May Used Up, Top[%#x],[Addr:%#x, Value:%#x]\n", &_start_stack_section_, &start_stack[i], start_stack[i]);
        }
    }

    return 0;
}

#ifdef CONFIG_BOARD_SUPPORT_MULTIBOOT
void SwitchAnotherFirmeware(void)
{
    extern unsigned int _multboot_info_addr_;
    volatile unsigned int *data = (volatile unsigned int*)(u32)&_multboot_info_addr_;
    if(*data == 0xaabbccdd) {
        printf("boot from first firmware\n");
        *data = 0;
    } else {
        printf("boot from second firmware\n");
        *data = 0xaabbccdd;
    }
    gx_dcache_clean_range((unsigned int *)((u32)&_multboot_info_addr_), 16);
    gx_reboot();
}
#endif
