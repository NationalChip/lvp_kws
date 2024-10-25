/*
 ******************************************************************************
 * @file    read_data_simple.c
 * @author  Sensors Software Solution Team
 * @brief   This file show the simplest way to get data from sensor.
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/*
 * This example was developed using the following STMicroelectronics
 * evaluation boards:
 *
 * - STEVAL_MKI109V3
 * - NUCLEO_F411RE + X_NUCLEO_IKS01A2
 *
 * and STM32CubeMX tool with STM32CubeF4 MCU Package
 *
 * Used interfaces:
 *
 * STEVAL_MKI109V3    - Host side:   USB (Virtual COM)
 *                    - Sensor side: SPI(Default) / I2C(supported)
 *
 * NUCLEO_STM32F411RE + X_NUCLEO_IKS01A2 - Host side: UART(COM) to USB bridge
 *                                       - I2C(Default) / SPI(N/A)
 *
 * If you need to run this example on a different hardware platform a
 * modification of the functions: `platform_write`, `platform_read`,
 * `tx_com` and 'platform_init' is required.
 *
 */

/* STMicroelectronics evaluation boards definition
 *
 * Please uncomment ONLY the evaluation boards in use.
 * If a different hardware is used please comment all
 * following target board and redefine yours.
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <common.h>
#include <driver/gx_i2c.h>
#include <driver/gx_delay.h>
#include <driver/gx_gpio.h>
#include <driver/spi.h>
#include <driver/device.h>
#include "../driver/lis2dw12_reg.h"
#include <lvp_attr.h>
#include <driver/gx_padmux.h>

#define I2CBUS 1
#define FIFI_THRELD 16
static struct sflash_master g_spi_master={
  .bus_num           =0,
  .spi={
    .master        =NULL,
    .chip_select   =0,
  },
};

typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static axis3bit16_t data_raw_acceleration[32] ALIGNED_ATTR(16);
static float acceleration_mg[3];
static uint8_t whoamI, rst;
static stmdev_ctx_t dev_ctx;
static lis2dw12_ctrl4_int1_pad_ctrl_t  ctrl4_int1_pad;
// static lis2dw12_ctrl5_int2_pad_ctrl_t  ctrl5_int2_pad;
/* Extern variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void platform_init(void);

static int _lis2dw12_data_threld_cb(int port, void *pdata)
{
  // lis2dw12_all_sources_t source1;
  lis2dw12_status_t status1;
  // lis2dw12_all_sources_get(&dev_ctx, &source1);
  lis2dw12_status_reg_get(&dev_ctx, &status1);
  unsigned char samples1;
  lis2dw12_fifo_data_level_get(&dev_ctx, &samples1);

  if(samples1 > FIFI_THRELD)
    samples1 = FIFI_THRELD;

  lis2dw12_read_reg(&dev_ctx, LIS2DW12_OUT_X_L, data_raw_acceleration[0].u8bit, samples1*6);
  // printf("samples1 =%d\n",samples1);

  // if(source1.status_dup.ovr)
  //   printf("over int\n");
  // if(status1.fifo_ths)
  //   printf("ths int\n");
  for(int i = 0; i < samples1; i++)
  {
    acceleration_mg[0] = lis2dw12_from_fs2_to_mg(data_raw_acceleration[i].i16bit[0]);
    acceleration_mg[1] = lis2dw12_from_fs2_to_mg(data_raw_acceleration[i].i16bit[1]);
    acceleration_mg[2] = lis2dw12_from_fs2_to_mg(data_raw_acceleration[i].i16bit[2]);

    //printf("x: 0x%04x, y: 0x%04x, z: 0x%04x\n", x_data, y_data, z_data);
    printf("Acceleration [mg]:%4.2f  %4.2f  %4.2f  \n",
              acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
  }
  return 0;
}

void lis2dw12_fifo_test(void)
{
  /*
   *  Initialize mems driver interface
   */
  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.handle = NULL;

  /*
   * Initialize platform specific hardware
   */
  platform_init();

  /*
   *  Check device ID
   */
  lis2dw12_device_id_get(&dev_ctx, &whoamI);
  if (whoamI != LIS2DW12_ID)
  {
    printf("read id error whoamI =0x%x\n",whoamI);
    while(1)
    {
      /* manage here device not found */
    }
  }
  else
  {
    printf("read id success\n");
  }

  /*
   * Restore default configuration
   */
  lis2dw12_reset_set(&dev_ctx, PROPERTY_ENABLE);
  do {
    lis2dw12_reset_get(&dev_ctx, &rst);
  } while (rst);

  /*
   *  Enable Block Data Update
   */
  lis2dw12_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

  /*
   * Set full scale
   */
  //lis2dw12_full_scale_set(&dev_ctx, LIS2DW12_8g);
  lis2dw12_full_scale_set(&dev_ctx, LIS2DW12_2g);

  /*
   * Configure filtering chain
   *
   * Accelerometer - filter path / bandwidth
   */
  lis2dw12_filter_path_set(&dev_ctx, LIS2DW12_LPF_ON_OUT);
  lis2dw12_filter_bandwidth_set(&dev_ctx, LIS2DW12_ODR_DIV_4);

  /*
   * Configure power mode
   */
  lis2dw12_power_mode_set(&dev_ctx, LIS2DW12_HIGH_PERFORMANCE);
  //lis2dw12_power_mode_set(&dev_ctx, LIS2DW12_CONT_LOW_PWR_LOW_NOISE_12bit);

  /*
   * Set Output Data Rate
   */
  lis2dw12_data_rate_set(&dev_ctx, LIS2DW12_XL_ODR_25Hz);
  //lis2dw12_data_rate_set(&dev_ctx, LIS2DW12_XL_ODR_1k6Hz);

  lis2dw12_fmode_t fifo_mode;
  lis2dw12_fifo_mode_get(&dev_ctx, &fifo_mode);

  lis2dw12_fifo_watermark_set(&dev_ctx, FIFI_THRELD);
  lis2dw12_pin_int1_route_get(&dev_ctx, &ctrl4_int1_pad);
  ctrl4_int1_pad.int1_fth = PROPERTY_ENABLE;
  lis2dw12_pin_int1_route_set(&dev_ctx, &ctrl4_int1_pad);

  // lis2dw12_pin_int2_route_get(&dev_ctx,&ctrl5_int2_pad);
  // ctrl5_int2_pad.int2_ovr = PROPERTY_ENABLE;
  // lis2dw12_pin_int2_route_set(&dev_ctx,&ctrl5_int2_pad);


  lis2dw12_fifo_mode_set(&dev_ctx,LIS2DW12_STREAM_MODE);
  lis2dw12_fifo_mode_get(&dev_ctx, &fifo_mode);
  padmux_set(1,1);
  gx_gpio_set_direction(1,GX_GPIO_DIRECTION_INPUT);
  gx_gpio_enable_trigger(1, GX_GPIO_TRIGGER_EDGE_RISING, _lis2dw12_data_threld_cb, NULL);

  // while(1)
  // {
  //   printf("11111\n");
  //   gx_mdelay(500);
  // }
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
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len)
{
  //gx_i2c_tx(I2CBUS, (LIS2DW12_I2C_ADD_H >> 1), reg, bufp, len);
  uint8_t cmd[2];
  cmd[0] = reg;
  cmd[1] = bufp[0];
  return spi_write(&g_spi_master.spi, cmd, 2);
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
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
  //gx_i2c_rx(I2CBUS, (LIS2DW12_I2C_ADD_H >> 1), reg, bufp, len);
  uint8_t cmd;
  cmd = 0x80 | reg;
  return spi_write_then_read(&g_spi_master.spi, &cmd, 1, bufp, len);
}

/*
 * @brief  platform specific initialization (platform dependent)
 */

static void platform_init(void)
{
  g_spi_master.bus_num = 0;
  g_spi_master.spi.chip_select  = 0;
  g_spi_master.spi.max_speed_hz = CONFIG_GENERAL_SPI_CLK_SRC/2;
  g_spi_master.spi.mode         = 0;
  printf("%s %d\n", __func__,__LINE__);

  spi_flash_register_master(&g_spi_master);
  printf("%s %d\n", __func__,__LINE__);

  if(g_spi_master.spi.master == NULL)
  {
    printf("g_spi_master register failed\n");
    return ;
  }
  else
  {
    printf("g_spi_master register success\n");
  }
  printf("%s %d\n", __func__,__LINE__);

  spi_setup(&g_spi_master.spi);
    printf("%s %d\n", __func__,__LINE__);

}

