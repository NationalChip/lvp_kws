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
#include "../include/lis2dw12_config.h"
#include "../driver/lis2dw12_reg.h"
#include <gsensor.h>

#define I2CBUS 1
#define FIFI_THRELD 28

#ifdef CONFIG_LVP_GSENSOR_USE_I2C
static void *i2c_dev;
#endif

#ifdef CONFIG_LVP_GSENSOR_USE_SPI
static struct sflash_master g_spi_master={
  .bus_num =0,
  .spi = {
    .master        =NULL,
    .chip_select   =0,
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
static int32_t _PlatformWrite(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
static int32_t _PlatformRead(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static int32_t _PlatformReadAsync(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len, void (*func)(void *), void* arg);
static int32_t _PlatformWriteAsync(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len, void (*func)(void *), void* arg);
static void _PlatformInit(void);

/* Private variables ---------------------------------------------------------*/
static sample_t data_raw_acceleration[FIFI_THRELD] ALIGNED_ATTR(16);

//static float acceleration_mg[3];
static uint8_t whoamI, rst;
static stmdev_ctx_t dev_ctx;
static lis2dw12_ctrl4_int1_pad_ctrl_t  ctrl4_int1_pad;
static volatile uint8_t is_single_taped;
static gsensor_event_cb_t _callback;
static void * private_arg;

void Lis2dw12FIFOReadOverCb(void *private)
{
  unsigned char read_samples = *((unsigned char *)private);
  GsensorDataFill((unsigned char*)data_raw_acceleration[0].u8bit, read_samples* sizeof(sample_t));
   // end_ms = gx_get_time_us();
   // printf("%d\n",end_ms - start_ms);
  // printf("q\n");
}

int Lis2dw12Suspend(void *priv)
{
  lis2dw12_mode_t val;
  lis2dw12_fifo_mode_set(&dev_ctx, LIS2DW12_BYPASS_MODE);
  lis2dw12_power_mode_set(&dev_ctx, LIS2DW12_CONT_LOW_PWR_12bit);
  do
  {
    lis2dw12_power_mode_get(&dev_ctx, &val);
  }while(val != LIS2DW12_CONT_LOW_PWR_12bit);

  return 0;
}

#ifdef BOARD_HAS_GSNEOR
DRAM0_STAGE2_SRAM_ATTR static int _lis2dw12_data_threshold_cb(int port, void *pdata)
{
  lis2dw12_all_sources_t source;
  uint8_t wtm_flag;
  unsigned char samples1;

  lis2dw12_all_sources_get(&dev_ctx, &source);

#if GSENSOR_SUPPORT_SINGLE_TAP
    if (source.tap_src.single_tap)
    {
      if(source.tap_src.z_tap)
      {
        is_single_taped = 1;
        printf("z_tap\n");
      }
      if(source.tap_src.y_tap)
      {
        is_single_taped = 1;
        printf("y_tap\n");
      }
      if(source.tap_src.x_tap)
      {
        is_single_taped = 1;
        printf("x_tap\n");
      }
    }
#endif
  lis2dw12_fifo_wtm_flag_get(&dev_ctx, &wtm_flag);
  if(wtm_flag)
  {
    lis2dw12_fifo_data_level_get(&dev_ctx, &samples1);
#ifdef CONFIG_LVP_GSENSOR_USE_SPI
    lis2dw12_read_reg(&dev_ctx, LIS2DW12_OUT_X_L, data_raw_acceleration[0].u8bit, samples1*6);
#endif

#ifdef CONFIG_LVP_GSENSOR_USE_I2C
    #if 0
    lis2dw12_read_reg_async(&dev_ctx, LIS2DW12_OUT_X_L, data_raw_acceleration[0].u8bit, samples1*6, Lis2dw12FIFOReadOverCb, &samples1);
    #else
    lis2dw12_read_reg(&dev_ctx, LIS2DW12_OUT_X_L, data_raw_acceleration[0].u8bit, samples1*6);
    #endif
#endif
    GsensorDataFill((unsigned char*)data_raw_acceleration[0].u8bit, samples1);
  }
  return 0;
}
#endif

#if 0
DRAM0_STAGE2_SRAM_ATTR static int _lis2dw12_data_timer_cb(int port, void *pdata)
{
  lis2dw12_all_sources_t source;
  unsigned char samples1;
  lis2dw12_all_sources_get(&dev_ctx, &source);

  lis2dw12_fifo_data_level_get(&dev_ctx, &samples1);

  lis2dw12_read_reg(&dev_ctx, LIS2DW12_OUT_X_L, data_raw_acceleration[0].u8bit, samples1*6);

  GsensorDataFill((unsigned char*)data_raw_acceleration[0].u8bit, samples1);

  return 0;
}
#endif

int Lis2dw12Init(unsigned char *buffer, int size)
{
  /*
   *  Initialize mems driver interface
   */
  dev_ctx.write_reg = _PlatformWrite;
  dev_ctx.read_reg = _PlatformRead;
  dev_ctx.write_reg_async = _PlatformReadAsync;
  dev_ctx.read_reg_async = _PlatformWriteAsync;
  dev_ctx.handle = NULL;

  /*
   * Initialize platform specific hardware
   */
  _PlatformInit();

  /*
   *  Check device ID
   */

  lis2dw12_device_id_get(&dev_ctx, &whoamI);

  if (whoamI != LIS2DW12_ID)
  {
    printf("read id error whoamI =0x%x\n",whoamI);
    return -1;
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
  // lis2dw12_data_rate_set(&dev_ctx, LIS2DW12_XL_ODR_25Hz);
  lis2dw12_data_rate_set(&dev_ctx, LIS2DW12_XL_ODR_1k6Hz);

#if GSENSOR_SUPPORT_SINGLE_TAP
   /*
   * Enable Tap detection on X, Y, Z
   */
  lis2dw12_tap_detection_on_z_set(&dev_ctx, PROPERTY_ENABLE);
  lis2dw12_tap_detection_on_y_set(&dev_ctx, PROPERTY_ENABLE);
  lis2dw12_tap_detection_on_x_set(&dev_ctx, PROPERTY_ENABLE);

  /*
   * Set Tap threshold on all axis
   */
  lis2dw12_tap_threshold_x_set(&dev_ctx, 9);
  lis2dw12_tap_threshold_y_set(&dev_ctx, 9);
  lis2dw12_tap_threshold_z_set(&dev_ctx, 9);

  /*
   * Configure Single Tap parameter
   */
  lis2dw12_tap_quiet_set(&dev_ctx, 1);
  lis2dw12_tap_shock_set(&dev_ctx, 2);

  /*
   * Enable Single Tap detection only
   */
  lis2dw12_tap_mode_set(&dev_ctx, LIS2DW12_ONLY_SINGLE);
#endif
#if 1
  lis2dw12_fifo_watermark_set(&dev_ctx, FIFI_THRELD);

  lis2dw12_int_notification_set(&dev_ctx,PROPERTY_ENABLE);
  lis2dw12_pin_int1_route_get(&dev_ctx, &ctrl4_int1_pad);
  ctrl4_int1_pad.int1_fth = PROPERTY_ENABLE;
#if GSENSOR_SUPPORT_SINGLE_TAP
  ctrl4_int1_pad.int1_single_tap = PROPERTY_ENABLE;
#endif
  lis2dw12_pin_int1_route_set(&dev_ctx, &ctrl4_int1_pad);

  lis2dw12_fifo_mode_set(&dev_ctx,LIS2DW12_STREAM_MODE);

#ifdef BOARD_HAS_GSNEOR
  gx_gpio_set_direction(GSENSOR_INTERRUPT_PIN_ID,GX_GPIO_DIRECTION_INPUT);
  gx_gpio_enable_trigger(GSENSOR_INTERRUPT_PIN_ID, GX_GPIO_TRIGGER_EDGE_RISING, _lis2dw12_data_threshold_cb, NULL);
#else
  return -1;
#endif

#else
  lis2dw12_fifo_mode_set(&dev_ctx,LIS2DW12_STREAM_MODE);
  if(gx_timer_register((int(*)(void*))_lis2dw12_data_timer_cb, 5, NULL, GX_TIMER_MODE_CONTINUE))
  {
      return -1;
  }
#endif
  return 0;
}

void Lis2dw12RegisterCb(gsensor_event_cb_t cb, void *arg)
{
  if(cb)
    _callback = cb;
  if(arg)
    private_arg = arg;
}

void Lis2dw12EventProc(void)
{
  if(is_single_taped)
  {
    _callback(SINGLE_TAP_EVENT,private_arg);
    is_single_taped = 0;
  }
}

static int32_t _PlatformReadAsync(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len, void (*func)(void *), void* arg)
{
  #ifdef CONFIG_LVP_GSENSOR_USE_I2C
  gx_i2c_async_rx(I2CBUS, (LIS2DW12_I2C_ADD_H >> 1), reg, bufp, len, (gx_i2c_async_done_cb_t)func, arg);
  #endif
  return 0;
}

static int32_t _PlatformWriteAsync(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len, void (*func)(void *), void* arg)
{
  #ifdef CONFIG_LVP_GSENSOR_USE_I2C
  gx_i2c_async_tx(I2CBUS, (LIS2DW12_I2C_ADD_H >> 1), reg, bufp, len, (gx_i2c_async_done_cb_t)func, arg);
  #endif
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
  // gx_i2c_tx(I2CBUS, (LIS2DW12_I2C_ADD_H >> 1), reg, bufp, len);
#ifdef CONFIG_LVP_GSENSOR_USE_I2C
  // uint8_t cmd[2];
  // GX_I2C_MSG msgs;
  // cmd[0] = reg;
  // cmd[1] = bufp[0];
  // msgs.addr = (LIS2DW12_I2C_ADD_H >> 1);
  // msgs.flags = 0;
  // msgs.len = len + 1;
  // msgs.buf = cmd;
  // uint8_t ret = gx_i2c_transfer(i2c_dev, &msgs, 1);
  gx_i2c_tx(I2CBUS, (LIS2DW12_I2C_ADD_H >> 1), reg, bufp, len);
#endif

#ifdef CONFIG_LVP_GSENSOR_USE_SPI
  uint8_t cmd[2];
  cmd[0] = reg;
  cmd[1] = bufp[0];
  return spi_write(&g_spi_master.spi, cmd, 2);
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
#ifdef CONFIG_LVP_GSENSOR_USE_I2C
  // GX_I2C_MSG msgs[2];
  // msgs[0].addr = (LIS2DW12_I2C_ADD_H >> 1);
  // msgs[0].flags = 0;
  // msgs[0].len = sizeof(uint8_t);
  // msgs[0].buf = &reg;

  // msgs[1].addr = (LIS2DW12_I2C_ADD_H >> 1);
  // msgs[1].flags = I2C_M_RD;
  // msgs[1].len = len;
  // msgs[1].buf = bufp;
  // return gx_i2c_transfer(i2c_dev, msgs, 2);
  gx_i2c_rx(I2CBUS, (LIS2DW12_I2C_ADD_H >> 1), reg, bufp, len);
#endif

#ifdef CONFIG_LVP_GSENSOR_USE_SPI
   uint8_t cmd;
   cmd = 0x80 | reg;
   return spi_write_then_read(&g_spi_master.spi, &cmd, 1, bufp, len);
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
#endif

#ifdef CONFIG_LVP_GSENSOR_USE_I2C
  gx_i2c_set_speed(I2CBUS, 1000);
  i2c_dev = gx_i2c_open(I2CBUS);
#endif
}

#ifdef GSENSOR_USE_LIS2DW12
gsensor_t gsensor_ins = {
  .name = "lis2dw12",
  .GsensorInit = Lis2dw12Init,
  .GsensorRegisterCb = Lis2dw12RegisterCb,
  .GsensorEventProc = Lis2dw12EventProc,
  .GsensorSuspend = Lis2dw12Suspend,
};

REGISTER_GSENSOR(gsensor_ins);
#endif
