/* Voice Signal Preprocess
* Copyright (C) 2001-2020 NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
* lvp_app_gpio_pwm_demo.c
*
*/

#include <lvp_app.h>
#include <lvp_buffer.h>


#include <lvp_attr.h>
#include <lvp_system_init.h>
#include "lvp_mode.h"
#include "app_core/lvp_app_core.h"
#include <driver/gx_clock.h>
#include <driver/gx_gpio.h>
#include <driver/gx_clock/gx_clock_v2.h>
#include <driver/gx_delay.h>
#include <base_addr.h>
#define LOG_TAG "[GPIO_PWM_APP]"
//=================================================================================================

#define GPIO_DOUT       (( GX_REG_BASE_GPIO0 ) + 0x04) // 输出的数据


float half_cycle_f;
unsigned int gpio_dout_l, gpio_dout_h;
unsigned int gpio_port = 1;


int gpio_level_flip_read(int port)      // 读取gpio的寄存器值
{
    gx_gpio_set_direction(port, GX_GPIO_DIRECTION_OUTPUT);
    gpio_dout_l = readl(GPIO_DOUT) & (~ (1 << (port%32)));
    gpio_dout_h = readl(GPIO_DOUT) | (1 << (port%32));
    return 0;
}
int gpio_level_flip_write(int level)    // 写寄存器翻转gpio电平
{
    writel(level, GPIO_DOUT);
    return 0;
}

int gpio_pwm_init(int opt)              // pwm初始化
{

    half_cycle_f = 1000000.f / CONFIG_LVP_APP_GPIO_PWM_FREQ / 2;    // 计算pwm半个周期的时长us，CONFIG_LVP_APP_GPIO_PWM_FREQ是在编译配置里设置的需要的pwm频率

    int cpu_fre = gx_clock_get_module_frequence(CLOCK_MODULE_SCPU); // 获取cpu的频率
    // 减掉翻转电平所需要的固定延迟
#if 0       // 如果是使用gpio_set_level,就是设为1，使用以下参数
    if (cpu_fre >= 12287990 && cpu_fre <= 12288010)
        half_cycle_f -= 4.2;
    else if (cpu_fre >= 16383990 && cpu_fre <= 16384010)
        half_cycle_f -= 3.2;
    else if (cpu_fre >= 24575990 && cpu_fre <= 24576010)
        half_cycle_f -= 2.3;
    else if (cpu_fre >= 49151990 && cpu_fre <= 49152010)
        half_cycle_f -= 1.3;
#else       // 如果是使用的是gpio_level_flip_write接口，设为0，使用以下参数
    if (cpu_fre >= 12287990 && cpu_fre <= 12288010)
        half_cycle_f -= 1.3;
    else if (cpu_fre >= 16383990 && cpu_fre <= 16384010)
        half_cycle_f -= 1.1;
    else if (cpu_fre >= 24575990 && cpu_fre <= 24576010)
        half_cycle_f -= 0.9;
    else if (cpu_fre >= 49151990 && cpu_fre <= 49152010)
        half_cycle_f -= 0.6;
#endif
    return 0;
}

// 以上接口不需要修改
// =========================================================================================================================
// 以下代码按需求修改

static int GpioPwmAppInit(void)
{
    printf(LOG_TAG" ---- %s ----\n", __func__);

    LvpDynamiciallyAdjustCpuFrequency(CPU_FREQUENCE_24M);       // 将cpu频率调高，如果默认的频率本来就是24M或者大于24M，则不需要调整cpu频率
    LvpDynamiciallyAdjustGpioFrequency(GPIO_FREQUENCE_HIGH_SPEED);  // 将gpio频率调高，这一步必须，否则电平翻的延迟会较大
    int coeff = gx_udelay_fine_config(gx_clock_get_module_frequence(CLOCK_MODULE_SCPU));    // 通过cpu频率获取延时系数
    if (coeff == -1)
    {
        return -1;
    }
    printf("coeff = %d mcu:%d\n", coeff, gx_clock_get_module_frequence(CLOCK_MODULE_SCPU));


    // gx_gpio_set_level 有固定延迟: https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/SDK%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/SDK%E5%BC%80%E5%8F%91_FAQ/GPIO%E6%A8%A1%E6%8B%9F%E7%BA%A2%E5%A4%96%E6%B3%A8%E6%84%8F%E4%BA%8B%E9%A1%B9/#2

    gpio_pwm_init(0);                               // pwm初始化，如果是使用gpio_set_level翻转电平则传非0参数，如果使用的是gpio_level_flip_write接口，传参数0
    int half_cycle = (int)half_cycle_f;
    printf("half_cycle %d\n", half_cycle);

    gpio_level_flip_read(gpio_port);                // 读gpio寄存器，传需要操作的gpio端口

    unsigned int irq = gx_lock_irq_save();          // 保存中断并关闭中断，中断会影响延时
    while(1) {
        gpio_level_flip_write(gpio_dout_l);         // 电平设为低电平
        gx_udelay_fine(half_cycle, coeff);          // 延时，传入延时时间和延时系数
        gpio_level_flip_write(gpio_dout_h);         // 电平设为高电平
        gx_udelay_fine(half_cycle, coeff);          // 延时，传入延时时间和延时系数
    }
    gx_unlock_irq_restore(irq);                     // 操作完成后打开中断
    LvpDynamiciallyAdjustGpioFrequency(GPIO_FREQUENCE_HIGH_SPEED); //操作完成后恢复默认频率
    LvpDynamiciallyAdjustCpuFrequency(CPU_FREQUENCE_DEFAULT);   // 恢复默认cpu频率，和之前的调整cpu频率组合使用

    return 0;
}

// App Event Process
static int GpioPwmAppEventResponse(APP_EVENT *app_event)
{
    return 0;
}

// APP Main Loop
static int GpioPwmAppTaskLoop(void)
{
    return 0;
}


static int GpioPwmAppSuspend(void *priv)
{
    return 0;
}

static int GpioPwmAppResume(void *priv)
{
    return 0;
}



LVP_APP gpio_pwm_app = {
    .app_name = "Gpio Pwm app",
    .AppInit = GpioPwmAppInit,
    .AppEventResponse = GpioPwmAppEventResponse,
    .AppTaskLoop = GpioPwmAppTaskLoop,
    .AppSuspend = GpioPwmAppSuspend,
    .suspend_priv = "GpioPwmAppSuspend",
    .AppResume = GpioPwmAppResume,
    .resume_priv = "GpioPwmAppResume",
};

LVP_REGISTER_APP(gpio_pwm_app);

