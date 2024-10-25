/* Voice Signal Preprocess
 * Copyright (C) 1991-2020 Nationalchip Co., Ltd
 *
 * types.h: MCU libc types definition
 *
 */

#ifndef __MCU_TYPES_H__
#define __MCU_TYPES_H__

#include "util.h"

#ifndef NULL
#define NULL    0
#endif

#ifndef bool
typedef enum { false = 0, true = 1 } bool;
#endif

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef unsigned char        uchar;

/* bsd */
typedef unsigned char        u_char;
typedef unsigned short        u_short;
typedef unsigned int        u_int;
typedef unsigned long        u_long;

/* sysv */
typedef unsigned char unchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;


typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;


typedef        __u8        u_int8_t;
typedef        __s8        int8_t;
typedef        __u16        u_int16_t;
typedef        __s16        int16_t;
typedef        __u32        u_int32_t;
typedef        __s32        int32_t;

typedef        __u8        uint8_t;
typedef        __u16        uint16_t;
typedef        __u32        uint32_t;

typedef        __u64        uint64_t;
typedef        __u64        u_int64_t;
typedef        __s64        int64_t;

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#ifndef size_t
typedef unsigned int size_t;
#endif

#ifndef ssize_t
typedef int ssize_t;
#endif

#endif              /* __MCU_TYPES_H__ */
