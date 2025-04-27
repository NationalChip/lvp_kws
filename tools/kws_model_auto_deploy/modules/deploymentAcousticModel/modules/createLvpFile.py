#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os, re
from datetime import datetime

class CreateLvpAcousticModelPackageFile:
    """ 这是一个创建模型包相关文件的类 """
    def __init__(self):
        self.keyword_regular_dict = {
                "cmd_content": [
                    {"index1": r"const\s*", "index2": ""},
                    {"index1": r"cmd_content", "index2": "kws_cmd_content"},
                    {"index1": r"\)\s*=\s*{", "index2": ") DRAM0_NPU_CMD_ATTR = {"},
                ],
                "weight_content": [
                    {"index1": r"const\s*", "index2": ""},
                    {"index1": r"weight_content", "index2": "kws_weight_content"},
                    {"index1": r"\)\s*=\s*{", "index2": ") DRAM0_NPU_WEIGHT_ATTR = {"},
                ],
                "ops_content": [
                    {"index1": r"const\s*", "index2": ""},
                    {"index1": r"ops_content", "index2": "kws_ops_content"},
                    {"index1": r"\)\s*=\s*{", "index2": ") DRAM0_NPU_ATTR = {"},
                ],
                "data_content": [
                    {"index1": r"const\s*", "index2": ""},
                    {"index1": r"data_content", "index2": "kws_data_content"},
                    {"index1": r"\)(\s)*;", "index2": ") DRAM0_NPU_ATTR;"},
                ],
                "tmp_content": [
                    {"index1": r"const\s*", "index2": ""},
                    {"index1": r"tmp_content", "index2": "kws_tmp_content"},
                ],
        }


    def create_kws_name(self, output_path, parsed_data_dict, internal_msg_bus):
        """
            功能:
                    * 生成kws.name文件
            参数:
                    * str:      文件路径 output_path
                    * dict:     解析后的数据字典 parsed_data_dict
                    * dict:     工具内部通信字典 internal_msg_bus
            返回值:
                    * /
        """
        #1、使用项目名
        project_name = parsed_data_dict['project name']
        #kws_name_filename = os.path.join(output_path, "kws.name")
        kws_name_filename = output_path
        kws_name_content = f"""# Voice Signal Preprocess
# Copyright (C) 2001-{datetime.now().year} NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Kconfig: One KeyWord Name For ../Kconfig
#---------------------------------------------------------------------------------#

config LVP_KWS_{project_name.upper()}
    bool "{project_name.replace('_', ' ').lower()}"

#---------------------------------------------------------------------------------#
"""
        with open(kws_name_filename, "w") as f:
            f.write(kws_name_content)



    def create_kws_list_h(self, output_path, parsed_data_dict, internal_msg_bus):
        """
            功能:
                    * 生成kws_list.h文件
                    * 用于第一基础部署，不适用分组后及自动获取阈值后
            参数:
                    * str:      文件路径 output_path
                    * dict:     解析后的数据字典 parsed_data_dict
                    * dict:     工具内部通信字典 internal_msg_bus
            返回值:
                    * /
        """
        #kws_list_filename = os.path.join(output_path, "kws_list.h")
        kws_list_filename = output_path
        kws_list_content = f"""/* Grus
 * Copyright (C) 1991-2024 Nationalchip Co., Ltd
 *
 * kws_list.h: a Circular Queue using array
 *
 */

#ifndef __KWS_LIST_H__
#define __KWS_LIST_H__

#include <lvp_param.h>

// 注： 阈值已经在此处固定，更改阈值请手动更改此文件

LVP_KWS_PARAM g_kws_param_list[] = {{
"""
        word_max_len = max([len(internal_msg_bus["词信息"][x]["词"]) for x in internal_msg_bus["词信息"]])
        #print(word_max_len)
        for index in internal_msg_bus["词信息"]:
            single_word_info = internal_msg_bus["词信息"][index]
            spaces_num = word_max_len + 1 - len(single_word_info["词"])
            kws_list_content += f'    {{"{single_word_info["词"]}", {" "*spaces_num} {{0}}, 1,'
            kws_list_content += f' {single_word_info["阈值"]}, {single_word_info["事件ID"]}, {single_word_info["是否主唤醒"]}}},\n'
        kws_list_content += f"""}};

#endif /* __KWS_LIST_H__ */
"""
        with open(kws_list_filename, "w") as f:
            f.write(kws_list_content)


    def create_kws_version_list(self, output_path, parsed_data_dict, internal_msg_bus):
        """
            功能:
                    * 生成kws_version.list文件
            参数:
                    * str:      文件路径 output_path
                    * dict:     解析后的数据字典 parsed_data_dict
                    * dict:     工具内部通信字典 internal_msg_bus
            返回值:
                    *
        """
        #1、使用项目名
        project_name = parsed_data_dict['project name']
        #2、使用版本
        merge_version = parsed_data_dict["acoustics model version"]
        #version_list_filename = os.path.join(output_path, "kws_version.list")
        version_list_filename = output_path
        version_list_content = f"""# Voice Signal Preprocess
# Copyright (C) 2001-{datetime.now().year} NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Kconfig: One KeyWord Name For ../Kconfig
#---------------------------------------------------------------------------------#

choice
    prompt "Model select:"
    default LVP_KWS_{project_name.upper()}_{merge_version.replace('.', 'DOT').upper()}_{datetime.now().strftime("%Y_%m%d")}
    depends on LVP_KWS_{project_name.upper()}

    source """

        version_list_content += '"lvp/vui/kws/models/'
        version_list_content += f'''{project_name.lower()}/*/kws_version.name"
endchoice

'''
        with open(version_list_filename, "w") as f:
            f.write(version_list_content)


    def create_model_h(self, output_path, parsed_data_dict, internal_msg_bus):
        """
            功能:
                    * 生成model.h文件""
            参数:
                    * str:      文件路径 output_path
                    * dict:     解析后的数据字典 parsed_data_dict
                    * dict:     工具内部通信字典 internal_msg_bus
            返回值:
                    *
        """

        #1、使用项目名
        project_name = parsed_data_dict['project name']
        #2、使用版本
        merge_version = parsed_data_dict["acoustics model version"]


        #model_h_fiename = os.path.join(output_path, "model.h")
        model_h_fiename = output_path
        model_h_content = f'''#include<lvp_attr.h>

const char *kws_version = "{project_name}_{merge_version}_{datetime.now().strftime('%Y_%m%d')}";
'''
        #修改model.h数据格式
        lines_data = parsed_data_dict["model_data"]["res"].split("\n")
        for keyword in self.keyword_regular_dict.keys():
            for x in range(len(lines_data)):
                if lines_data[x].find(keyword) >= 0:
                    for req in self.keyword_regular_dict[keyword]:
                        lines_data[x] = re.sub(req["index1"],req["index2"], lines_data[x])
                    #break
        lines_data = "\n".join(lines_data)

        lines_data = lines_data.replace("const unsigned int npu_size =", "const unsigned int npu_size DRAM0_NPU_KEEP_ATTR =")
        model_h_content += lines_data

        with open(model_h_fiename, "w") as f:
                f.write(model_h_content)


    def create_ctc_model_c(self, output_path, parsed_data_dict, internal_msg_bus):
        """
            功能:
                    * 生成ctc_model.c文件
            参数:
                    * str:      文件路径 output_path
                    * dict:     解析后的数据字典 parsed_data_dict
                    * dict:     工具内部通信字典 internal_msg_bus
            返回值:
                    *

        """
        #ctc_model_c_filename = os.path.join(output_path, "ctc_model.c")
        ctc_model_c_filename = output_path
        ctc_model_c_content = f"""/* Voice Signal Preprocess
* Copyright (C) 2001-{datetime.now().year} NationalChip Co., Ltd
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

#define LOG_TAG "[KWS]"

__attribute__((unused))  static GX_SNPU_TASK s_snpu_task;

int LvpModelGetCmdSize(void)
{{
  return sizeof(kws_cmd_content);
}}

int LvpModelGetWeightSize(void)
{{
  return sizeof(kws_weight_content);
}}

int LvpModelGetOpsSize(void)
{{
  return sizeof(kws_ops_content);
}}

int LvpModelGetDataSize(void)
{{
  return sizeof(kws_data_content);
}}

int LvpModelGetTmpSize(void)
{{
  return sizeof(kws_tmp_content);
}}

void LvpSetSnpuTask(GX_SNPU_TASK *snpu_task)
{{
  #ifdef CONFIG_NPU_RUN_IN_FLASH
  #else
    memcpy(&s_snpu_task, snpu_task, sizeof(GX_SNPU_TASK));
  #endif
}}

int LvpCTCModelInitSnpuTask(GX_SNPU_TASK *snpu_task)
{{
  if (CONFIG_KWS_SNPU_BUFFER_SIZE != sizeof(struct in_out)) {{
    printf ("snpu buffer size is need set:%d\\n", sizeof(struct in_out));
    return -1;
  }}

  snpu_task->module_id = 0x100;

   #ifdef CONFIG_NPU_RUN_IN_FLASH
      snpu_task->cmd       = (void *)MCU_TO_DEV((unsigned int)kws_cmd_content);
      snpu_task->weight    = (void *)MCU_TO_DEV((unsigned int)kws_weight_content);
      snpu_task->ops       = (void *)MCU_TO_DEV((unsigned int)(void *)(NPU_SRAM_ADDR/4*4));
      void *data = (void *)(((unsigned int)snpu_task->ops + LvpModelGetOpsSize() + 3)/4*4);
      snpu_task->data      = (void *)MCU_TO_DEV((unsigned int)data);
      snpu_task->tmp_mem   = (void *)MCU_TO_DEV((unsigned int)kws_tmp_content);
   #else
      snpu_task->cmd       = (void *)MCU_TO_DEV(s_snpu_task.cmd);
      snpu_task->weight    = (void *)MCU_TO_DEV(s_snpu_task.weight);
      snpu_task->ops       = (void *)MCU_TO_DEV(s_snpu_task.ops);
      snpu_task->data      = (void *)MCU_TO_DEV(s_snpu_task.data);
      snpu_task->tmp_mem   = (void *)MCU_TO_DEV(s_snpu_task.tmp_mem);
   #endif

   #if 0
      printf ("\\ncmd:\\n");
      unsigned char *cmd = (unsigned char *)s_snpu_task.cmd;
      for (int i = 0; i < 10; i++) {{
         printf ("%x, ", cmd[i]);
      }}
      printf ("\\n");
      for (int i = LvpModelGetCmdSize() - 10; i < LvpModelGetCmdSize(); i++) {{
         printf ("%x, ", cmd[i]);
      }}
      printf ("\\n");

      printf ("weight:\\n");
      unsigned char *weight = (unsigned char *)s_snpu_task.weight;
      for (int i = 0; i < 10; i++) {{
         printf ("%x, ", weight[i]);
      }}
      printf ("\\n");
      for (int i = LvpModelGetWeightSize() - 10; i < LvpModelGetWeightSize(); i++) {{
         printf ("%x, ", weight[i]);
      }}
      printf ("\\n");

      printf ("tmp:\\n");
      unsigned char *tmp_mem = (unsigned char *)s_snpu_task.tmp_mem;
      for (int i = 0; i < 10; i++) {{
         printf ("%x, ", tmp_mem[i]);
      }}
      printf ("\\n");
      for (int i = LvpModelGetTmpSize() - 10; i < LvpModelGetTmpSize(); i++) {{
         printf ("%x, ", tmp_mem[i]);
      }}
      printf ("\\n");
   #endif
      return 0;
   }}

   const char *LvpCTCModelGetKwsVersion(void)
   {{
      return kws_version;
   }}

   void *LvpCTCModelGetSnpuOutBuffer(void *snpu_buffer)
   {{
      return (void *)((struct in_out*)snpu_buffer)->"""
        ctc_model_c_content += f"""{parsed_data_dict['model_data']['info']['output_name']};"""
        ctc_model_c_content += """
   }

   void *LvpCTCModelGetSnpuFeatsBuffer(void *snpu_buffer)
   {
      return (void *)((struct in_out*)snpu_buffer)->"""
        ctc_model_c_content += f"""{parsed_data_dict['model_data']['info']['input_name']};"""
        ctc_model_c_content += """
   }

   void *LvpCTCModelGetSnpuStateBuffer(void *snpu_buffer)
   {
     return """
        if parsed_data_dict['model_data']['info']['isRnn']:
            ctc_model_c_content += """(void *)((struct in_out*)snpu_buffer)->"""
            if 'stateAddr' in parsed_data_dict['model_data']['info']:
                state_addr = parsed_data_dict['model_data']['info']['stateAddr']
            else:
                state_addr = 'State_c0'
            ctc_model_c_content += state_addr + ";"
        else:
            ctc_model_c_content += "NULL;"
        ctc_model_c_content += """
   }
   unsigned int LvpCTCModelGetSnpuFeatsDim(void)
   {
     return CONFIG_KWS_MODEL_INPUT_WIN_LENGTH * CONFIG_KWS_MODEL_FEATURES_DIM_PER_FRAME;
   }

  unsigned int LvpCTCModelGetSnpuStateDim(void)
  {"""
        if parsed_data_dict['model_data']['info']['isRnn']:
            ctc_model_c_content  += """
    return sizeof(struct input)/sizeof(short) - LvpCTCModelGetSnpuFeatsDim();
  }"""
        else:
            ctc_model_c_content += """"
       return 0;
   }"""

        with open(ctc_model_c_filename, "w") as f:
            f.write(ctc_model_c_content)


    def create_ctc_model_h(self, output_path, parsed_data_dict, internal_msg_bus):
        """
            功能:
                    * 生成ctc_model.h文件
            参数:
                    * str:      文件路径 output_path
                    * dict:     解析后的数据字典 parsed_data_dict
                    * dict:     工具内部通信字典 internal_msg_bus
            返回值:
                    *
        """
        #ctc_model_h_filename = os.path.join(output_path, "ctc_model.h")
        ctc_model_h_filename = output_path
        with open(ctc_model_h_filename, "w") as f:
            f.write(f"""/* Grus
* Copyright (C) 2001-{datetime.now().year} NationalChip Co., Ltd
* ALL RIGHTS RESERVED!
*
* ctc_model.h: Kws Model
*
*/

#ifndef __CTC_MODEL_H__
#define __CTC_MODEL_H__

#include <driver/gx_snpu.h>

int LvpModelGetOpsSize(void);
int LvpModelGetDataSize(void);
int LvpModelGetTmpSize(void);
int LvpModelGetCmdSize(void);
int LvpModelGetWeightSize(void);
void LvpSetSnpuTask(GX_SNPU_TASK* snpu_task);
int LvpCTCModelInitSnpuTask(GX_SNPU_TASK *snpu_task);
const char *LvpCTCModelGetKwsVersion(void);
void *LvpCTCModelGetSnpuOutBuffer(void *snpu_buffer);
void *LvpCTCModelGetSnpuFeatsBuffer(void *snpu_buffer);
void *LvpCTCModelGetSnpuStateBuffer(void *snpu_buffer);
unsigned int LvpCTCModelGetSnpuFeatsDim(void);
unsigned int LvpCTCModelGetSnpuStateDim(void);

#endif /* __CTC_MODEL_H__ */""")

    def create_kconfig(self, output_path, parsed_data_dict, internal_msg_bus,
                                decoder_stride_len=1,
                                decoder_window_len=25):
        """
            功能:
                    * 生成Kconfig文件
            参数:
                    * str:      文件路径 output_path
                    * dict:     解析后的数据字典 parsed_data_dict
                    * dict:     工具内部通信字典 internal_msg_bus
                    * int 分组数 decoder_stride_len= outout_groups_dict["分组数"] = group_num
                    * int 打分窗长 decoder_window_len self.viva_config.decoder_window_len
            返回值:
                    *
        """

        #1、使用项目名
        project_name = parsed_data_dict['project name']
        #2、使用版本
        merge_version = parsed_data_dict["acoustics model version"]



        #kconfig_filename = os.path.join(output_path, "Kconfig")
        kconfig_filename = output_path
        kconfig_content = f"""
menu "Model Param Setting:"
    depends on LVP_KWS_{project_name.upper()}_{merge_version.replace('.', 'DOT').upper()}_{datetime.now().strftime("%Y_%m%d")}
    config NORMAL_CTC_SCORE
      default {'y' if parsed_data_dict['normal_ctc'] else 'n'}

    config KWS_MODEL_SUPPORT_SOFTMAX
      default {'y' if parsed_data_dict['supportSoftMax'] else 'n'}
    # Byte
    config KWS_SNPU_BUFFER_SIZE
        default {parsed_data_dict['model_data']['info']['in_out_size']}

    # Frames
    config KWS_MODEL_FEATURES_DIM_PER_FRAME
        default {parsed_data_dict['model_data']['info']['feats_dim']}

    config KWS_MODEL_INPUT_STRIDE_LENGTH
        default {parsed_data_dict['stride']}

    config KWS_MODEL_INPUT_WIN_LENGTH
        default {parsed_data_dict['windowLength']}

    config KWS_MODEL_OUTPUT_LENGTH
        default {parsed_data_dict['model_data']['info']['output_length']}

    config KWS_MODEL_DECODER_STRIDE_LENGTH
        int "KWS Lantency (unit of Context)"
        default {decoder_stride_len}
        range 1 4

    config KWS_MODEL_DECODER_WIN_LENGTH
        int "KWS Model Decoder Window Length (unit of context)"
        default {decoder_window_len}
endmenu

"""
        with open(kconfig_filename, "w") as f:
            f.write(kconfig_content)

    def create_kws_version_name(self, output_path,  parsed_data_dict, internal_msg_bus):
        """
            功能:
                    * 生成kws_version.name文件
            参数:
                    * str:      文件路径 output_path
                    * dict:     解析后的数据字典 parsed_data_dict
                    * dict:     工具内部通信字典 internal_msg_bus
            返回值:
        """


        #1、使用项目名
        project_name = parsed_data_dict['project name']
        #2、使用版本
        merge_version = parsed_data_dict["acoustics model version"]

        #path = os.path.join(output_path, "kws_version.name")
        path = output_path
        with open(path, "w") as f:
            f.write(f"""# Voice Signal Preprocess
# Copyright (C) 2001-{datetime.now().year} NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Kconfig: Model Name
#---------------------------------------------------------------------------------#

config LVP_KWS_{project_name.upper()}_{merge_version.replace('.', 'DOT').upper()}_{datetime.now().strftime("%Y_%m%d")}
    bool "LVP_KWS_{project_name.upper()}_{merge_version.replace('.', 'DOT').upper()}_{datetime.now().strftime("%Y_%m%d")}"

#---------------------------------------------------------------------------------#
""")

    def create_makefile(self, output_path, parsed_data_dict, internal_msg_bus):
        """
            功能:
                    * 生成Makefile文件
            参数:
                    * str:      文件路径 output_path
                    * dict:     解析后的数据字典 parsed_data_dict
                    * dict:     工具内部通信字典 internal_msg_bus
            返回值:
        """

        #1、使用项目名
        project_name = parsed_data_dict['project name']
        #2、使用版本
        merge_version = parsed_data_dict["acoustics model version"]


        #makefile_filename = os.path.join(output_path, "Makefile")
        makefile_filename = output_path
        makefile_content = f"""#
# Voice Signal Preprocess
# Copyright (C) 2001-{datetime.now().year} NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: source list in dsp/vpa/arach_lib/src/ctc/decoder
#
#=================================================================================#

ifeq ($(CONFIG_LVP_KWS_{project_name.upper()}_{merge_version.replace('.', 'DOT').upper()}_{datetime.now().strftime("%Y_%m%d")}), y)

MODEL_DIR_PRE  = lvp/vui/kws/models/{project_name}/{merge_version}

INCLUDE_DIR    += $(MODEL_DIR_PRE)/
INCLUDE_DIR    += $(MODEL_DIR_PRE)/../
kws_objs       += $(MODEL_DIR_PRE)/ctc_model.o

endif"""

        with open(makefile_filename, "w") as f:
            f.write(makefile_content)



if __name__=="__main__":
    pass
