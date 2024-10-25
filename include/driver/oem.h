#ifndef __OEM_H__
#define __OEM_H__

#include <common.h>

#define OEM_SIZE (64*1024)

int oem_get_hardware_version(uint32_t *version);
int oem_get_software_version(uint32_t *version);
int oem_get_software_crc32(uint32_t *crc32);
int oem_get_vender_id(uint16_t *vender_id);
int oem_get_product_id(uint16_t *product_id);
int oem_get_sn(uint8_t *sn);
int oem_init(uint32_t start);

#endif
