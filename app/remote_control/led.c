#include "led.h"


void ledOn(void)
{
    gx_gpio_set_direction(LED_GPIO, GX_GPIO_DIRECTION_OUTPUT);
    gx_gpio_set_level(LED_GPIO, GX_GPIO_LEVEL_HIGH);

}

void ledOff(void)
{
    gx_gpio_set_direction(LED_GPIO, GX_GPIO_DIRECTION_OUTPUT);
    gx_gpio_set_level(LED_GPIO, GX_GPIO_LEVEL_LOW);
}

void ledBright(const int delay_ms)
{
    gx_gpio_set_direction(LED_GPIO, GX_GPIO_DIRECTION_OUTPUT);

    gx_gpio_set_level(LED_GPIO, GX_GPIO_LEVEL_HIGH);
    gx_mdelay(delay_ms);
    gx_gpio_set_level(LED_GPIO, GX_GPIO_LEVEL_LOW);
}