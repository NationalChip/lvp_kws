/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * grus_loop.c:
 *
 */
#include <autoconf.h>
#include <types.h>
#include <stdio.h>
#include <string.h>
#include <board_config.h>

#include <driver/gx_audio_in.h>
#include <driver/gx_snpu.h>
#include <driver/gx_flash.h>
#include <driver/gx_timer.h>
#include <driver/gx_cache.h>
#include <driver/gx_pmu_ctrl.h>
#include <driver/gx_uart.h>
#include <driver/gx_irq.h>

#include <lvp_system_init.h>
#include <lvp_app.h>
#include <lvp_pmu.h>
#include <lvp_context.h>
#include <lvp_buffer.h>
#include <lvp_attr.h>
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
#include <ctc_model.h>
#include <decoder.h>
#endif

#include "common/lvp_audio_in.h"
#include "common/lvp_standby_ratio.h"
#include <board_misc_config.h>
#include "lvp_mode_tws.h"

#define LOG_TAG "[LVP_KWS ]"

static GX_SNPU_CALLBACK s_snpu_callback = NULL;

#ifdef CONFIG_ENABLE_NPU_CYCLE_STATISTIC
static unsigned int s_start_ms;
static unsigned int s_end_ms;
#endif



//=================================================================================================
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
DRAM0_STAGE2_SRAM_ATTR static int _SnpuCallback(int module_id, GX_SNPU_STATE state, void *priv)
{
    if (s_snpu_callback && priv) {
        s_snpu_callback(module_id, state, priv);
    }
# ifdef CONFIG_ENABLE_NPU_CYCLE_STATISTIC
    s_end_ms = gx_get_time_ms();
    printf ("npu:%d ms\n", s_end_ms - s_start_ms);
# endif
    return 0;
}
#endif

//-------------------------------------------------------------------------------------------------
DRAM0_STAGE2_SRAM_ATTR int LvpKwsRun(LVP_CONTEXT *context)
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    if (NULL == context) return -1;

# ifdef CONFIG_ENABLE_DATA_PREPARATION_CYCLE_STATISTIC
    unsigned int start_ms = gx_get_time_ms();
# endif

    short *snpu_input = LvpGetFeatsBuffer();
    short *cur_feats  = LvpGetLogfbankBuffer(context, context->ctx_index);
    unsigned int feat_dim_per_frame = CONFIG_KWS_MODEL_FEATURES_DIM_PER_FRAME;
    unsigned int ipnut_stride       = LvpGetPcmFrameNumPerContext();
    unsigned int input_win_length   = CONFIG_KWS_MODEL_INPUT_WIN_LENGTH;
    gx_dcache_invalid_range((uint32_t *)cur_feats, feat_dim_per_frame * ipnut_stride * sizeof(short));
    memmove(snpu_input, &snpu_input[ipnut_stride * feat_dim_per_frame], (input_win_length - ipnut_stride) * feat_dim_per_frame * sizeof(short));
    memcpy(&snpu_input[(input_win_length - ipnut_stride) * feat_dim_per_frame], cur_feats, ipnut_stride * feat_dim_per_frame * sizeof(short));
    memcpy(context->snpu_buffer, snpu_input, input_win_length * feat_dim_per_frame * sizeof(short));
    gx_dcache_clean_range((uint32_t *)context->snpu_buffer, feat_dim_per_frame * input_win_length * sizeof(short));

    if (1) {
        GX_SNPU_TASK snpu_task;
        LVP_CONTEXT *pre_context;
        LVP_CONTEXT *cur_context;
        unsigned int size;
        LvpGetContext(context->ctx_index, &pre_context, &size);
        LvpGetContext(context->ctx_index + 1, &cur_context, &size);
        LvpCTCModelInitSnpuTask(&snpu_task);

        short *input  = LvpCTCModelGetSnpuFeatsBuffer(pre_context->snpu_buffer);
# ifdef  CONFIG_LVP_MODEL_USE_RNN
        short *output = LvpCTCModelGetSnpuStateBuffer(cur_context->snpu_buffer);
# else
        short *output = LvpCTCModelGetSnpuOutBuffer(cur_context->snpu_buffer);
# endif
        gx_dcache_clean_range((uint32_t *)input, LvpCTCModelGetSnpuFeatsDim() * sizeof(short));
        snpu_task.input  = (void *)MCU_TO_DEV(input);
        snpu_task.output = (void *)MCU_TO_DEV(output);
# ifdef CONFIG_ENABLE_DATA_PREPARATION_CYCLE_STATISTIC
        unsigned int end_ms = gx_get_time_ms();
        printf ("data:%d ms\n", end_ms - start_ms);
# endif
# ifdef CONFIG_ENABLE_NPU_CYCLE_STATISTIC
        s_start_ms = gx_get_time_ms();
# endif
        gx_snpu_run_task(&snpu_task, _SnpuCallback, cur_context);
    } else {
        context->vad = 0;
        LvpAudioInUpdateReadIndex(1);
# if (defined CONFIG_LVP_ENABLE_CTC_DECODER) || (defined CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER)
        void ResetCtcWinodw(void);
        ResetCtcWinodw();
# endif
    }
#else
    LvpAudioInUpdateReadIndex(1);
#endif
    return 0;
}

//-------------------------------------------------------------------------------------------------
__attribute__ ((unused)) static int _LvpKwsLoadSnpuFromFlash(const GX_SNPU_TASK *snpu_task, unsigned int cmd_size, unsigned int weight_size)
{

    unsigned int start_ms = gx_get_time_ms();
    unsigned int bus   = CONFIG_SF_DEFAULT_BUS;
    unsigned int cs    = CONFIG_SF_DEFAULT_CS;
    unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
    unsigned int mode  = CONFIG_SF_DEFAULT_MODE;
    int ret = 0;

    GX_FLASH_DEV *dev = gx_spi_flash_probe(bus, cs, speed, mode);
    if (!dev) {
        printf(LOG_TAG"Init Flash Failed\n");
        ret = -1;
        goto _return;
    }
    extern int LD_NPU_IMAGE_OFFSET;

#ifdef CONFIG_MULTIBOOT_SECOND_BIN
    unsigned int load_addr = (unsigned int)&LD_NPU_IMAGE_OFFSET + CONFIG_SECOND_BIN_OFFSET;
#else
    unsigned int load_addr = (unsigned int)&LD_NPU_IMAGE_OFFSET;
#endif
    ret = gx_spi_flash_readdata(dev, load_addr, snpu_task->cmd, cmd_size);
    ret |= gx_spi_flash_readdata(dev, load_addr + (cmd_size+3)/4*4, snpu_task->weight, weight_size);

# if 0
      unsigned char *cmd = snpu_task->cmd;
      unsigned int cmd_total = 0;
      for (int i = 0; i < cmd_size; i++) {
         cmd_total += cmd[i];
      }
      printf("cmd :%x, cmd_total:%x\n", cmd, cmd_total);
      printf("cmd head:\n");
      for (int i = 0; i < 32; i++) {
            printf("%x ", cmd[i]);
      }
      printf("\n");
      printf("cmd tail:\n");
      for (int i = 0; i < 32; i++) {
            printf("%x ", cmd[cmd_size - 1 - i]);
      }
      printf("\n");

      unsigned char *weight = snpu_task->weight;
      unsigned int weight_total = 0;
      for (int i = 0; i < weight_size; i++) {
         weight_total += weight[i];
      }
      printf("\nweight :%x, weight_total:%x\n", weight, weight_total);
      printf("weight head:\n");
      for (int i = 0; i < 70; i++) {
            printf("%x ", weight[i]);
            if ((i+1)%32 == 0) printf("\n");
      }
      printf("\n");
      printf("weight tail:\n");
      for (int i = 0; i < 70; i++) {
            printf("%x ", weight[weight_size - 1 - i]);
            if ((i+1)%32 == 0) printf("\n");
      }
      printf("\n");

      unsigned char *data = snpu_task->data;
      printf("data :%x\n", data);

# endif
    unsigned int end_ms = gx_get_time_ms();
    printf (LOG_TAG"Kws Use:%d ms\n", end_ms - start_ms);

    if (0 != ret) {
        printf(LOG_TAG"Read Flash Failed\n");
        ret = -1;
        goto _return;
    }

_return:

    return ret;
}

#ifndef UART_BOOT_PORT
# define UART_BOOT_PORT 0
#endif

__attribute__ ((unused)) static int _LvpKwsLoadSnpuFromUart(const GX_SNPU_TASK *snpu_task, unsigned int cmd_size, unsigned int weight_size)
{
    int ret = 0;
    // printf("ops size:%x data size :%x tmp_mem size :%x cmd size: %x, weight size: %x\n",
    //                                                 LvpModelGetOpsSize(),
    //                                                 LvpModelGetDataSize(),
    //                                                 LvpModelGetTmpSize(),
    //                                                 cmd_size,
    //                                                 weight_size);
    // printf("ops addr: 0x%x data addr: 0x%x  tmp_mem: 0x%x cmd addr: 0x%x , weight addr: 0x%x\n",
    //                                                 snpu_task->ops,
    //                                                 snpu_task->data,
    //                                                 snpu_task->tmp_mem,
    //                                                 snpu_task->cmd,
    //                                                 snpu_task->weight);
    gx_uart_write(UART_BOOT_PORT, (const unsigned char *)"snpu ok", 9);
    if (0 != ret) {
        printf(LOG_TAG"UART Load Failed\n");
        return -1;
    }

#if 0
      unsigned char *cmd = snpu_task->cmd;
      unsigned int cmd_total = 0;
      for (int i = 0; i < cmd_size; i++) {
         cmd_total += cmd[i];
      }
      printf("cmd :%x, cmd_total:%x\n", cmd, cmd_total);
      printf("cmd head:\n");
      for (int i = 0; i < 32; i++) {
            printf("%x ", cmd[i]);
      }
      printf("\n");
      printf("cmd tail:\n");
      for (int i = 0; i < 32; i++) {
            printf("%x ", cmd[cmd_size - 1 - i]);
      }
      printf("\n");

      unsigned char *weight = snpu_task->weight;
      unsigned int weight_total = 0;
      for (int i = 0; i < weight_size; i++) {
         weight_total += weight[i];
      }
      printf("\nweight :%x, weight_total:%x\n", weight, weight_total);
      printf("weight head:\n");
      for (int i = 0; i < 70; i++) {
            printf("%x ", weight[i]);
            if ((i+1)%32 == 0) printf("\n");
      }
      printf("\n");
      printf("weight tail:\n");
      for (int i = 0; i < 70; i++) {
            printf("%x ", weight[weight_size - 1 - i]);
            if ((i+1)%32 == 0) printf("\n");
      }
      printf("\n");

      unsigned char *data = snpu_task->data;
      printf("data :%x\n", data);

# endif
    return 0;
}

void LvpLoadKwsNpuModle(void) // load npu mode
{
#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
    GX_SNPU_TASK snpu_task;
    int ret = 0;
    unsigned int cmd_size = LvpModelGetCmdSize();
    unsigned int weight_size = LvpModelGetWeightSize();

    snpu_task.ops = (void *)(NPU_SRAM_ADDR/4*4);
    snpu_task.data = (void *)((unsigned int)snpu_task.ops + (LvpModelGetOpsSize() + 3)/4*4);
    snpu_task.tmp_mem = (void *)((unsigned int)snpu_task.data + (LvpModelGetDataSize() + 3)/4*4);
    snpu_task.cmd     = (void *)((unsigned int)snpu_task.tmp_mem +(LvpModelGetTmpSize() +3)/4*4);
    snpu_task.weight  = (void *)((unsigned int)snpu_task.cmd + (cmd_size + 3)/4*4);

# if defined (CONFIG_BOOT_BY_FLASH)

#  ifdef CONFIG_NPU_RUN_IN_FLASH
#   ifdef CONFIG_ENABLE_CTC_KWS_AND_BUN_KWS_CASCADE
    ret = _LvpKwsLoadSnpuFromFlash(&snpu_task, (cmd_size+3) / 4*4, (weight_size+3) / 4*4);
#   endif
#  else
    ret = _LvpKwsLoadSnpuFromFlash(&snpu_task, (cmd_size+3) / 4*4, (weight_size+3) / 4*4);
#  endif

# elif defined (CONFIG_BOOT_BY_UART)
    ret = _LvpKwsLoadSnpuFromUart(&snpu_task, (cmd_size+3) / 4*4, (weight_size+3) / 4*4);
# endif
    if(ret == 0) {
        LvpSetSnpuTask(&snpu_task);
    }
#endif
}

//-------------------------------------------------------------------------------------------------
DRAM0_STAGE2_SRAM_ATTR int _LvpKwsSystemFrequencyResume(void *priv)
{
#ifdef CONFIG_KWS_TYPE_HYBRID
    if (LvpModelGetUseXipModelFlag()) {
        gx_disable_irq();
        LvpDynamiciallyAdjustSystemFrequency(SYSTEM_FREQUENCE_HIGH_SPEED);
        gx_enable_irq();
    }
#endif
    return 0;
}

DRAM0_STAGE2_SRAM_ATTR int LvpKwsSwitchModelByEnvNoise(LVP_CONTEXT *context)
{
#ifdef CONFIG_KWS_TYPE_HYBRID_SWITCH_BY_ENV_NOISE
    static int last_env_noise = ENV_LOW_NOISE;
    extern int LvpModelSetUseXipModelFlag(int flag);
    if (context->env_noise) {
        if (last_env_noise != context->env_noise) {
            LvpAudioInSuspend();
            while (gx_snpu_get_state() == GX_SNPU_BUSY);
            gx_disable_irq();
            last_env_noise = context->env_noise;
            LvpDynamiciallyAdjustSystemFrequency(SYSTEM_FREQUENCE_HIGH_SPEED);
            LvpModelSetUseXipModelFlag(1);
            gx_enable_irq();
            LvpAudioInResume();
            printf(LOG_TAG" ^o^ HIGH_SPEED, %d, %d, %d\n", context->ctx_index, last_env_noise , context->env_noise);
        }
    } else {
        if (last_env_noise != context->env_noise) {
            LvpAudioInSuspend();
            while (gx_snpu_get_state() == GX_SNPU_BUSY);
            gx_disable_irq();
            last_env_noise = context->env_noise;
            LvpLoadKwsNpuModle();
            LvpDynamiciallyAdjustSystemFrequency(SYSTEM_FREQUENCE_STANDARD_SPEED);
            LvpModelSetUseXipModelFlag(0);
            gx_enable_irq();
            LvpAudioInResume();
            printf(LOG_TAG" ^-^ STANDARD_SPEED, %d, %d, %d\n", context->ctx_index, last_env_noise , context->env_noise);
        }
    }
#endif
    return 0;
}

#ifdef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
DRAM0_STAGE2_SRAM_ATTR int LvpKwsSwitchModelByKwsState(VUI_KWS_STATE state)
{
#ifdef CONFIG_KWS_TYPE_HYBRID_SWITCH_BY_KWS_STATE
    extern int LvpModelSetUseXipModelFlag(int flag);
    if (state == VUI_KWS_ACTIVE_STATE) {
            LvpAudioInSuspend();
            while (gx_snpu_get_state() == GX_SNPU_BUSY);
            gx_disable_irq();
            LvpModelSetUseXipModelFlag(1);
            gx_enable_irq();
            LvpAudioInResume();
            printf(LOG_TAG" ^_^ big model, %d, %d\n", state, __LINE__);
    } else if (state == VUI_KWS_VAD_STATE) {
            LvpAudioInSuspend();
            while (gx_snpu_get_state() == GX_SNPU_BUSY);
            gx_disable_irq();
            LvpLoadKwsNpuModle();
            LvpModelSetUseXipModelFlag(0);
            gx_enable_irq();
            LvpAudioInResume();
            printf(LOG_TAG" ^.^ small model, %d, %d\n", state, __LINE__);
    }
#endif
    return 0;
}
#endif

int LvpKwsInit(GX_SNPU_CALLBACK callback, GX_WAKEUP_SOURCE start_mode)
{
    if (GX_WAKEUP_SOURCE_COLD == start_mode || GX_WAKEUP_SOURCE_WDT == start_mode) {
        LvpLoadKwsNpuModle();
    }
    gx_snpu_init();

#ifdef CONFIG_KWS_TYPE_HYBRID
    LVP_RESUME_INFO resume_info = {
        .resume_callback = _LvpKwsSystemFrequencyResume,
        .priv = NULL
    };
    LvpResumeInfoRegist(&resume_info);
#endif

    s_snpu_callback = callback;
    return 0;
}

int LvpKwsDone(void)
{
    gx_snpu_exit();
    s_snpu_callback = NULL;
    return 0;
}

//-------------------------------------------------------------------------------------------------
