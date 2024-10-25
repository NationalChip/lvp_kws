/* Voice Signal Preprocess
 * Copyright (C) 1991-2019 Nationalchip Co., Ltd
 *
 * crc.h: crc32 utilities
 *
 */
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef _CRC_H_
#define _CRC_H_

#include <stdint.h>

uint32_t crc32 (uint32_t crc, const unsigned char/*Bytef*/ *p, unsigned int/*uInt*/ len);
#endif
