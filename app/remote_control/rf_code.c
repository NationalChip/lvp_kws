#include "rf_code.h"
#include <driver/gx_snpu.h>
#include "lvp_audio_in.h"

unsigned int rf_address = 0x00AEE471;

void getUniqueId(void)
{
    unsigned char buf[16];
    int ret_len;

    gx_spinor_flash_init();
    gx_spinor_flash_getuid(buf, 16 , &ret_len);

    for (int j = 0; j < 16;j++) {
        printf("%02X ", buf[j]);
    }

    printf("\n");

    rf_address = crc32(0, buf, ret_len);

    // return (((temp>>20)^temp)&0x000ffff);
}

// 发码规则 注意：发码的时候，必现关中断模式下，否则时间误差会很大
// 该函数，请根据具体的项目重新实现
int rf_send_code(unsigned int code)
{
    if(code == 0x00) return -1;

    LvpAudioInSuspend();
    while (gx_snpu_get_state() == GX_SNPU_BUSY);
    uint32_t irq_state = gx_lock_irq_save();

    gx_gpio_set_direction(RF_GPIO, GX_GPIO_DIRECTION_OUTPUT);
    gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_LOW);

    unsigned int send_code = code;                              // 需要发送的地址吗以及数据码

#ifdef DEBUG_PRINTF
    printf("rf_address = %#x send_code = %#x\n",rf_address, send_code);
#endif
    // rf_address = 0x00AEE471;
    for(int j = 0;j < SEND_TIMES;j++){

        // 发送引导码
        gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_HIGH);
        gx_udelay(4720);
        gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_LOW);
        gx_udelay(1420);

        // 发送地址码
        for(int i = 31;i >= 0;i--){
            if(rf_address & (1 << i)){
                gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_HIGH);
                gx_udelay(640);
                gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_LOW);
                gx_udelay(280);
            }else{
                 gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_HIGH);
                gx_udelay(280);
                gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_LOW);
                gx_udelay(640);
            }
        }

        for(int i = 0;i < 8;i++){
            if(send_code & (1 << i)){
                gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_HIGH);
                gx_udelay(640);
                gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_LOW);
                gx_udelay(280);
            }else{
                gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_HIGH);
                gx_udelay(280);
                gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_LOW);
                gx_udelay(640);
            }
        }

        // 因为在关中断内，扫描按键的定时器停止，因此，必现根据发码用了多少时间，把扫描按键的动作补回来，否则会在发码的过程按键，导致按键概率丢失
        flex_button_scan();
        flex_button_scan();
        flex_button_scan();


        // if (button_state != 0) // 表示在发码的过程中，又有新的按键触发了，就退出发码，进行下一轮新的发码
        // {
        //     printf("break send code irq %d, %d\n", j, button_state);
        //     return 0;
        // }
    }

    gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_HIGH);
    gx_gpio_set_level(RF_GPIO, GX_GPIO_LEVEL_LOW);

    gx_unlock_irq_restore(irq_state);
    LvpAudioInResume();
    return 0;
}

void rf_send_code_times(unsigned int code, int timers)
{
    do{
        rf_send_code(code);
    }while(timers--);

    return ;
}

