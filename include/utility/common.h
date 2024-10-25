/*
 * (C) Copyright 2000-2020
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#ifndef __COMMON_H__
#define __COMMON_H__
//=================================================================================================

#include <types.h>

#ifndef __ASSEMBLY__        /* put C only stuff in this section */

# define cpu_to_le16(x)        (x)
# define cpu_to_le32(x)        (x)
# define cpu_to_le64(x)        (x)
# define le16_to_cpu(x)        (x)
# define le32_to_cpu(x)        (x)
# define le64_to_cpu(x)        (x)
# define cpu_to_be16(x)        uswap_16(x)
# define cpu_to_be32(x)        uswap_32(x)
# define cpu_to_be64(x)        uswap_64(x)
# define be16_to_cpu(x)        uswap_16(x)
# define be32_to_cpu(x)        uswap_32(x)
# define be64_to_cpu(x)        uswap_64(x)

//=================================================================================================

#include <stdio.h>
#include <board_config.h>
#include <types.h>
#include <autoconf.h>

#endif /* __ASSEMBLY__ */

/* crc32.c */
uint32_t crc32 (uint32_t crc, const unsigned char/*Bytef*/ *p, unsigned int/*uInt*/ len);

#define roundup(x, y)     ((((x) + ((y) - 1)) / (y)) * (y))

#define readw(addr) ({ unsigned short __v = (*(volatile unsigned short *) (addr)); __v; })

#endif    /* __COMMON_H__ */
