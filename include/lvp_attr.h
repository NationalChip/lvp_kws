/* Low-power Voice Process
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_attr.h:
 *
 */

#ifndef __LVP_ATTR_H__
#define __LVP_ATTR_H__

//=================================================================================================

#define ALIGNED_ATTR(x)         __attribute__((aligned(x)))

#define DRAM0_STAGE2_SRAM_ATTR  __attribute__((section(".sram_text")))

#define XIP_RODATA_ATTR         __attribute__((section(".xip.rodata*")))

#ifdef CONFIG_DL
# define SECTION_EXTERNAL_FLASH_TEXT     __attribute__((section(".external_flash.text")))
# define SECTION_EXTERNAL_FLASH_RODATA   __attribute__((section(".external_flash.rodata")))
#else
# define SECTION_EXTERNAL_FLASH_TEXT
# define SECTION_EXTERNAL_FLASH_RODATA
#endif

#define XIP_NPU_ATTR          __attribute__((section(".xip_npu_section")))
#define XIP_NPU_CMD_ATTR      __attribute__((section(".xip_cmd")))
#define XIP_NPU_WEIGHT_ATTR   __attribute__((section(".xip_weight")))

#define DRAM0_NPU_ATTR          __attribute__((section(".npu_section")))
#define DRAM0_NPU_CMD_ATTR      __attribute__((section(".cmd")))
#define DRAM0_NPU_WEIGHT_ATTR   __attribute__((section(".weight")))

#define DRAM0_NC_NPU_ATTR          __attribute__((section(".nc_npu_section")))
#define DRAM0_NC_NPU_CMD_ATTR      __attribute__((section(".nc_cmd")))
#define DRAM0_NC_NPU_WEIGHT_ATTR   __attribute__((section(".nc_weight")))

#define DRAM0_VP_NPU_ATTR          __attribute__((section(".vp_npu_section")))
#define DRAM0_VP_NPU_CMD_ATTR      __attribute__((section(".vp_cmd")))
#define DRAM0_VP_NPU_WEIGHT_ATTR   __attribute__((section(".vp_weight")))

#define DRAM0_AUDIO_IN_ATTR     __attribute__((section(".audio_in,\"aw\",@nobits#")))

#define DRAM0_NPU_KEEP_ATTR   __attribute__((section(".npu_data_keep")))

#define NPU_SRAM_ADDR           0x20000000

//=================================================================================================
#endif /* __LVP_ATTR_H__ */
