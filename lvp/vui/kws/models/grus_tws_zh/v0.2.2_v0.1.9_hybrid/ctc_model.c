/* Voice Signal Preprocess
* Copyright (C) 2001-2021 NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
* ctc_model.c: Kws Model
*
*/

#include <autoconf.h>
#include <stdio.h>
#include <string.h>
#include <lvp_buffer.h>

#include <driver/gx_snpu.h>

#include "ctc_model.h"
#include "model.h"
#include "model_xip.h"

#define LOG_TAG "[KWS]"

__attribute__((unused))  static GX_SNPU_TASK s_snpu_task;

static int use_xip_model_flag = 0;

int LvpModelSetUseXipModelFlag(int flag)
{
   use_xip_model_flag = flag;
   return 0;
}

int LvpModelGetUseXipModelFlag(void)
{
   return use_xip_model_flag;
}

int LvpModelGetCmdSize(void)
{
   if (use_xip_model_flag) return sizeof(kws_cmd_content);
   else return sizeof(kws_cmd_content);
}

int LvpModelGetWeightSize(void)
{
   if (use_xip_model_flag) return sizeof(kws_weight_content);
   else return sizeof(kws_weight_content);
}

int LvpModelGetOpsSize(void)
{
   if (use_xip_model_flag) return sizeof(kws_ops_content);
   else return sizeof(kws_ops_content);

  return sizeof(kws_ops_content);
}

int LvpModelGetDataSize(void)
{
   if (use_xip_model_flag) return sizeof(kws_data_content);
   else return sizeof(kws_data_content);
}

int LvpModelGetTmpSize(void)
{
   if (use_xip_model_flag) return sizeof(kws_tmp_content);
   else return sizeof(kws_tmp_content);
}

void LvpSetSnpuTask(GX_SNPU_TASK *snpu_task)
{
   if (use_xip_model_flag) {

   } else {
      memcpy(&s_snpu_task, snpu_task, sizeof(GX_SNPU_TASK));
   }
}

int LvpCTCModelInitSnpuTask(GX_SNPU_TASK *snpu_task)
{
  if (CONFIG_KWS_SNPU_BUFFER_SIZE != sizeof(struct in_out)) {
    printf ("snpu buffer size is need set:%d\n", sizeof(struct in_out));
    return -1;
  }

  snpu_task->module_id = 0x100;

   if (use_xip_model_flag) {
      // printf ("xip_kws_cmd_content: %x\n", xip_kws_cmd_content);
      // printf ("xip_kws_weight_content: %x\n", xip_kws_weight_content);
      snpu_task->cmd       = (void *)MCU_TO_DEV((unsigned int)xip_kws_cmd_content);
      snpu_task->weight    = (void *)MCU_TO_DEV((unsigned int)xip_kws_weight_content);
      snpu_task->ops       = (void *)MCU_TO_DEV((unsigned int)(void *)(NPU_SRAM_ADDR/4*4));
      void *data = (void *)(((unsigned int)snpu_task->ops + LvpModelGetOpsSize() + 3)/4*4);
      snpu_task->data      = (void *)MCU_TO_DEV((unsigned int)data);
      snpu_task->tmp_mem   = (void *)MCU_TO_DEV((unsigned int)xip_kws_tmp_content);
   } else {
      snpu_task->cmd       = (void *)MCU_TO_DEV(s_snpu_task.cmd);
      snpu_task->weight    = (void *)MCU_TO_DEV(s_snpu_task.weight);
      snpu_task->ops       = (void *)MCU_TO_DEV(s_snpu_task.ops);
      snpu_task->data      = (void *)MCU_TO_DEV(s_snpu_task.data);
      snpu_task->tmp_mem   = (void *)MCU_TO_DEV(s_snpu_task.tmp_mem);
   }

   #if 0
      printf ("\nuse_xip_model_flag: %x\n", use_xip_model_flag);
      unsigned char *cmd = (unsigned char *)DEV_TO_MCU(snpu_task->cmd);
      printf ("\ncmd: %x\n", cmd);
      for (int i = 0; i < 10; i++) {
         printf ("%x, ", cmd[i]);
      }
      printf ("\n");
      for (int i = LvpModelGetCmdSize() - 10; i < LvpModelGetCmdSize(); i++) {
         printf ("%x, ", cmd[i]);
      }
      printf ("\n");

      unsigned char *weight = (unsigned char *)DEV_TO_MCU(snpu_task->weight);
      printf ("weight:%x\n", weight);
      for (int i = 0; i < 10; i++) {
         printf ("%x, ", weight[i]);
      }
      printf ("\n");
      for (int i = LvpModelGetWeightSize() - 10; i < LvpModelGetWeightSize(); i++) {
         printf ("%x, ", weight[i]);
      }
      printf ("\n");

      printf ("tmp:\n");
      unsigned char *tmp_mem = (unsigned char *)DEV_TO_MCU(snpu_task->tmp_mem);
      for (int i = 0; i < 10; i++) {
         printf ("%x, ", tmp_mem[i]);
      }
      printf ("\n");
      for (int i = LvpModelGetTmpSize() - 10; i < LvpModelGetTmpSize(); i++) {
         printf ("%x, ", tmp_mem[i]);
      }
      printf ("\n");
   #endif
      return 0;
   }

   const char *LvpCTCModelGetKwsVersion(void)
   {
      return kws_version;
   }

   void *LvpCTCModelGetSnpuOutBuffer(void *snpu_buffer)
   {
      return (void *)((struct in_out*)snpu_buffer)->phone_prob;
   }

   void *LvpCTCModelGetSnpuFeatsBuffer(void *snpu_buffer)
   {
      return (void *)((struct in_out*)snpu_buffer)->Feats;
   }

   void *LvpCTCModelGetSnpuStateBuffer(void *snpu_buffer)
   {
     return (void *)((struct in_out*)snpu_buffer)->State_c0;
   }
   unsigned int LvpCTCModelGetSnpuFeatsDim(void)
   {
     return CONFIG_KWS_MODEL_INPUT_WIN_LENGTH * CONFIG_KWS_MODEL_FEATURES_DIM_PER_FRAME;
   }

  unsigned int LvpCTCModelGetSnpuStateDim(void)
  {
    return sizeof(struct input)/sizeof(short) - LvpCTCModelGetSnpuFeatsDim();
  }
