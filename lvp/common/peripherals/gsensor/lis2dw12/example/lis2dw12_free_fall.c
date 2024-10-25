/*
 ******************************************************************************
 * @file    free_fall.c
 * @author  Sensors Software Solution Team
 * @brief   This file show the simplest way to detect free fall events
 *          from sensor.
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
#include <string.h>
#include <stdio.h>
#include <common.h>
#include <driver/gx_i2c.h>
#include <driver/gx_delay.h>
#include <driver/gx_gpio.h>
#include <driver/spi.h>
#include <driver/device.h>
#include "../driver/lis2dw12_reg.h"

#define I2CBUS 1
static struct sflash_master g_spi_master={
  .bus_num           =0,
  .spi={
    .master        =NULL,
    .chip_select   =0,
  },
};
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t whoamI, rst;

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

/* Main Example --------------------------------------------------------------*/
void lis2dw12_free_fall(void)
{
  /*
   *  Initialize mems driver interface
   */
  stmdev_ctx_t dev_ctx;
  lis2dw12_reg_t int_route;

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
    while(1)
    {
      /* manage here device not found */
    }

  /*
   * Restore default configuration
   */
  lis2dw12_reset_set(&dev_ctx, PROPERTY_ENABLE);
  do {
    lis2dw12_reset_get(&dev_ctx, &rst);
  } while (rst);

  /*
   * Configure power mode
   */
  lis2dw12_power_mode_set(&dev_ctx, LIS2DW12_HIGH_PERFORMANCE_LOW_NOISE);

  /*
   * Set Output Data Rate
   */
  lis2dw12_data_rate_set(&dev_ctx, LIS2DW12_XL_ODR_200Hz);

  /*
   * Set full scale to 2 g
   */
  lis2dw12_full_scale_set(&dev_ctx, LIS2DW12_2g);

  /*
   * Configure Free Fall duration and samples count
   */
  lis2dw12_ff_dur_set(&dev_ctx, 0x06);
  lis2dw12_ff_threshold_set(&dev_ctx, LIS2DW12_FF_TSH_10LSb_FS2g);

  /*
   * Enable free fall interrupt
   */
  lis2dw12_pin_int1_route_get(&dev_ctx, &int_route.ctrl4_int1_pad_ctrl);
  int_route.ctrl4_int1_pad_ctrl.int1_ff = PROPERTY_ENABLE;
  lis2dw12_pin_int1_route_set(&dev_ctx, &int_route.ctrl4_int1_pad_ctrl);

  /*
   * Set latched interrupt
   */
  lis2dw12_int_notification_set(&dev_ctx, LIS2DW12_INT_LATCHED);

  /*
   * Wait Events
   */
  while(1)
  {
    /*
     * Check Free Fall events
     */
    lis2dw12_all_sources_t src;

    lis2dw12_all_sources_get(&dev_ctx, &src);
    if (src.wake_up_src.ff_ia)
    {
      //sprintf((char*)tx_buffer, "free fall detected\r\n");
      //tx_com(tx_buffer, strlen((char const*)tx_buffer));
    }
  }
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
}
