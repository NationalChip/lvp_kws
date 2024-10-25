#include <stdio.h>
#include <common.h>
#include <driver/gx_i2c.h>
#include <driver/gx_delay.h>
#include <driver/gx_gpio.h>
#include <driver/spi.h>
#include <driver/device.h>
#include <board_misc_config.h>
#include <lvp_attr.h>
#include <driver/gx_padmux.h>
#include <string.h>
#include <gsensor.h>
#include "lvp_queue.h"


#define I2CBUS 1
#define FIFO_THRELD 16
#define FIFO_MAX 32

#ifndef GSENSOR_INTERRUPT_PIN_ID
#define GSENSOR_INTERRUPT_PIN_ID 1
#endif

#ifdef CONFIG_LVP_GSENSOR_USE_I2C
static void *i2c_dev;
#endif

#ifdef CONFIG_LVP_GSENSOR_USE_SPI
static struct sflash_master g_spi_master = {
  .bus_num = 0,
  .spi = {
    .master        = NULL,
    .chip_select   = 0,
  },
};
#endif

/* Private functions ---------------------------------------------------------*/
/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */
static int32_t _PlatformWrite(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t _PlatformRead(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static void _PlatformInit(void);

/* Private variables ---------------------------------------------------------*/
static sample_t data_raw_acceleration[FIFO_MAX] ALIGNED_ATTR(16);

DRAM0_STAGE2_SRAM_ATTR static int _Ds_da662_data_threshold_cb(int port, void *pdata)
{
    uint8_t reg_data = 0;
    // fifo status
    _PlatformRead(NULL, 0x08, &reg_data, 1);
    if(reg_data & 0x80)
    {
        // watermark int
        uint8_t samples = reg_data & 0x3f;
        //printf("0x08:%d\n", samples);

        // 0x02 ~ 0x07 MSB: 0x07 first, decrements
        _PlatformRead(NULL, 0x07, data_raw_acceleration[0].u8bit, samples * 6);
        for(int i = 0; i < samples; i++)
        {
            uint8_t temp;
            for(int j = 0; j < 3; j++)
            {
                temp = data_raw_acceleration[i].u8bit[j];
                data_raw_acceleration[i].u8bit[j] = data_raw_acceleration[i].u8bit[5 - j];
                data_raw_acceleration[i].u8bit[5 - j] = temp;
            }
        }
        GsensorDataFill((unsigned char*)data_raw_acceleration[0].u8bit, samples);
        //_PlatformRead(NULL, 0x08, &reg_data, 1);
        //printf("0x08:%d\n", reg_data & 0x3f);
    }

#if 0
    if(reg_data & 0x40) {
        printf("gsensor fifo full!!!\n");
    }
#endif

    return 0;
}

int Ds_da662Init(unsigned char *buffer, int size)
{
    int ret;
    uint8_t reg_data = 0;

    _PlatformInit();

    // read chip id
    ret = _PlatformRead(NULL, 0x01, &reg_data, 1);
    //printf("ret = %d\n", ret);
    if (reg_data != 0x13)
    {
        printf("read DA662 id error: 0x%x\n",reg_data);
        return -1;
    }
    else
    {
        printf("read id success\n");
    }
    gx_gpio_set_direction(GSENSOR_INTERRUPT_PIN_ID, GX_GPIO_DIRECTION_INPUT);
    gx_gpio_enable_trigger(GSENSOR_INTERRUPT_PIN_ID, GX_GPIO_TRIGGER_EDGE_RISING, _Ds_da662_data_threshold_cb, NULL);

    // resolution
    //ret = _PlatformRead(NULL, 0x0f, &reg_data, 1);
    //printf("0x0f: 0x%02x\n", reg_data);

    // mode odr
//    reg_data = 0x80 | (0x02 << 4) | 0x0b;
    reg_data = 0x80 | (0x0 << 4) | 0x0b;
    ret = _PlatformWrite(NULL, 0x10, &reg_data, 1);
    //ret = _PlatformRead(NULL, 0x10, &reg_data, 1);
    //printf("0x10: 0x%02x\n", reg_data);

    // mode axis
    reg_data = 0x00;
    ret = _PlatformWrite(NULL, 0x11, &reg_data, 1);

    // fifo ctrl
    reg_data = (0x02 << 6) | (0x01 << 5) | FIFO_THRELD;
    ret = _PlatformWrite(NULL, 0x14, &reg_data, 1);

    // int set0
    reg_data = 0x08;
    ret = _PlatformWrite(NULL, 0x15, &reg_data, 1);

    // int map2
    reg_data = 0x02;
    ret = _PlatformWrite(NULL, 0x1a, &reg_data, 1);

    // int config
    reg_data = 0x80;
    ret = _PlatformWrite(NULL, 0x20, &reg_data, 1);

    return ret;
}


void Ds_da662RegisterCb(gsensor_event_cb_t cb, void *arg)
{

}

void Ds_da662EventProc(void)
{

}

int Ds_da662Suspend(void* prv)
{
  uint8_t reg_data;
  reg_data = 0x80;
  _PlatformWrite(NULL, 0x11, &reg_data, 1);
  return 0;
}

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t _PlatformWrite(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len)
{
#ifdef CONFIG_LVP_GSENSOR_USE_SPI
  uint8_t cmd[3];
  cmd[0] = reg >> 8;
  cmd[1] = reg & 0xff;
  cmd[2] = bufp[0];
  return spi_write(&g_spi_master.spi, cmd, 3);
#endif
  return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t _PlatformRead(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
#ifdef CONFIG_LVP_GSENSOR_USE_SPI
   uint8_t cmd[2];
   uint8_t nbytes = 3;

#if 0
   if(len >= 4)
       nbytes = 3;
   else
       nbytes = len - 1;
#endif

   cmd[0] = 0x80 | ((nbytes) << 5) | (reg >> 8);
   cmd[1] = reg & 0xff;
   return spi_write_then_read(&g_spi_master.spi, cmd, 2, bufp, len);
#endif
   return 0;
}

/*
 * @brief  platform specific initialization (platform dependent)
 */

static void _PlatformInit(void)
{
#ifdef CONFIG_LVP_GSENSOR_USE_SPI
  g_spi_master.bus_num = CONFIG_GENERAL_SPI_BUS_SN;
  g_spi_master.spi.chip_select  = 0;
  g_spi_master.spi.max_speed_hz = CONFIG_GENERAL_SPI_CLK_SRC / 2;
  g_spi_master.spi.mode         = 0;

  spi_flash_register_master(&g_spi_master);

  if(g_spi_master.spi.master == NULL)
  {
    printf("g_spi_master register failed\n");
    return ;
  }
  else
  {
    printf("g_spi_master register success\n");
  }

  spi_setup(&g_spi_master.spi);
#endif
}

#ifdef GSENSOR_USE_DS_DA662
gsensor_t gsensor_ins = {
  .name = "ds_da662",
  .GsensorInit = Ds_da662Init,
  .GsensorRegisterCb = Ds_da662RegisterCb,
  .GsensorEventProc = Ds_da662EventProc,
  .GsensorSuspend = Ds_da662Suspend,
};

REGISTER_GSENSOR(gsensor_ins);
#endif
