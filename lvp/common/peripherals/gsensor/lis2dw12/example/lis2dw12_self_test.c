/*
 ******************************************************************************
 * @file    test_self_test.c
 * @author  Sensors Software Solution Team
 * @brief   This file run selt test procedure
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

#define I2CBUS 1
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
/* Self-test recommended samples */
#define SELF_TEST_SAMPLES	5

/* Self-test positive difference */
#define ST_MIN_POS			70.0f
#define ST_MAX_POS			1500.0f

/* Private variables ---------------------------------------------------------*/
static axis3bit16_t data_raw_acceleration[SELF_TEST_SAMPLES];
static float acceleration_mg[SELF_TEST_SAMPLES][3];
static uint8_t whoamI, rst;
static uint8_t tx_buffer[1000];

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
static void tx_com( uint8_t *tx_buffer, uint16_t len );
static void platform_init(void);

/* Utility functions ---------------------------------------------------------*/
static inline float ABSF(float _x)
{
	return (_x < 0.0f) ? -(_x) : _x;
}

static int flush_samples(stmdev_ctx_t *dev_ctx)
{
  lis2dw12_reg_t reg;
  axis3bit16_t dummy;
  int samples = 0;

  /*
   * Discard old samples
   */
  lis2dw12_status_reg_get(dev_ctx, &reg.status);
  if (reg.status.drdy)
  {
    lis2dw12_acceleration_raw_get(dev_ctx, dummy.u8bit);
    samples++;
  }

  return samples;
}

static void test_self_test_lis2dw12(stmdev_ctx_t *dev_ctx)
{
  lis2dw12_reg_t reg;
  float media[3] = { 0.0f, 0.0f, 0.0f };
  float mediast[3] = { 0.0f, 0.0f, 0.0f };
  uint8_t match[3] = { 0, 0, 0 };
  uint8_t j = 0;
  uint16_t i = 0;
  uint8_t k = 0;
  uint8_t axis;

  /*
   * Restore default configuration
   */
  lis2dw12_reset_set(dev_ctx, PROPERTY_ENABLE);
  do
  {
    lis2dw12_reset_get(dev_ctx, &rst);
  } while (rst);

  lis2dw12_block_data_update_set(dev_ctx, PROPERTY_ENABLE);
  lis2dw12_full_scale_set(dev_ctx, LIS2DW12_4g);
  lis2dw12_power_mode_set(dev_ctx, LIS2DW12_HIGH_PERFORMANCE);
  lis2dw12_data_rate_set(dev_ctx, LIS2DW12_XL_ODR_50Hz);
  gx_mdelay(100);

  /*
   * Flush old samples
   */
  flush_samples(dev_ctx);

  do
  {
    lis2dw12_status_reg_get(dev_ctx, &reg.status);
    if (reg.status.drdy)
    {
      /*
       * Read accelerometer data
       */
      memset(data_raw_acceleration[i].u8bit, 0x00, 3 * sizeof(int16_t));
      lis2dw12_acceleration_raw_get(dev_ctx, data_raw_acceleration[i].u8bit);
      for (axis = 0; axis < 3; axis++) {
        acceleration_mg[i][axis] =
          lis2dw12_from_fs4_to_mg(data_raw_acceleration[i].i16bit[axis]);
      }
      i++;
      }
  } while (i < SELF_TEST_SAMPLES);

  for (k = 0; k < 3; k++)
  {
    for (j = 0; j < SELF_TEST_SAMPLES; j++)
    {
      media[k] += acceleration_mg[j][k];
    }

    media[k] = (media[k] / j);
  }

  /*
   * Enable self test mode
   */
  lis2dw12_self_test_set(dev_ctx, LIS2DW12_XL_ST_POSITIVE);
  gx_mdelay(100);
  i = 0;

  /*
   * Flush old samples
   */
  flush_samples(dev_ctx);

  do
  {
    lis2dw12_status_reg_get(dev_ctx, &reg.status);
    if (reg.status.drdy)
    {
      /*
       * Read accelerometer data
       */
      memset(data_raw_acceleration[i].u8bit, 0x00, 3 * sizeof(int16_t));
      lis2dw12_acceleration_raw_get(dev_ctx, data_raw_acceleration[i].u8bit);
      for (axis = 0; axis < 3; axis++)
        acceleration_mg[i][axis] =
          lis2dw12_from_fs4_to_mg(data_raw_acceleration[i].i16bit[axis]);

      i++;
    }
  } while (i < SELF_TEST_SAMPLES);

  for (k = 0; k < 3; k++)
  {
      for (j = 0; j < SELF_TEST_SAMPLES; j++)
      {
        mediast[k] += acceleration_mg[j][k];
      }

    mediast[k] = (mediast[k] / j);
  }

  /*
   * Check for all axis self test value range
   */
  for (k = 0; k < 3; k++)
  {
    if ((ABSF(mediast[k] - media[k]) >= ST_MIN_POS) &&
        (ABSF(mediast[k] - media[k]) <= ST_MAX_POS))
    {
      match[k] = 1;
    }

    sprintf((char*)tx_buffer, "%d: |%f| <= |%f| <= |%f| %s\r\n", k,
            ST_MIN_POS, ABSF(mediast[k] - media[k]), ST_MAX_POS,
            match[k] == 1 ? "PASSED" : "FAILED");
    tx_com(tx_buffer, strlen((char const*)tx_buffer));
  }

  /*
   * Disable self test mode
   */
  lis2dw12_data_rate_set(dev_ctx, LIS2DW12_XL_ODR_OFF);
  lis2dw12_self_test_set(dev_ctx, LIS2DW12_XL_ST_DISABLE);
}

/* Main Example --------------------------------------------------------------*/
void lis2dw12_self_test(void)
{
  /*
   * Initialize mems driver interface
  */
  stmdev_ctx_t dev_ctx;

  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.handle = NULL;

  /*
   * Initialize platform specific hardware
   */
  platform_init();

  /*
   * Check device ID
   */
  lis2dw12_device_id_get(&dev_ctx, &whoamI);
  if (whoamI != LIS2DW12_ID)
  while(1)
  {
    /* manage here device not found */
  }

  /*
   * Start self test
   */
  while(1)
  {
    test_self_test_lis2dw12(&dev_ctx);
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
 * @brief  Write generic device register (platform dependent)
 *
 * @param  tx_buffer     buffer to trasmit
 * @param  len           number of byte to send
 *
 */
static void tx_com(uint8_t *tx_buffer, uint16_t len)
{
  printf("%s\n", tx_buffer);
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

