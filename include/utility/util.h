/* Low-power Voice Process
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * util.h: mcu
 *
 */

#ifndef __MCU_UTIL_H__
#define __MCU_UTIL_H__

#define min(X, Y)				\
	({ typeof (X) __x = (X), __y = (Y);	\
	 (__x < __y) ? __x : __y; })

#define max(X, Y)				\
	({ typeof (X) __x = (X), __y = (Y);	\
	 (__x > __y) ? __x : __y; })

#define MIN(x, y)  min(x, y)
#define MAX(x, y)  max(x, y)	

#ifndef NULL
#define NULL    (void *) 0
#endif

// ctype
int isspace(int ch);
int isdigit(int ch);
int isupper(int ch);
int toupper(int ch);
int tolower(int ch);

#if 0
// stdlib
unsigned int atoi(const char *str);
unsigned int htoi(const char *str);
unsigned long long simple_strtoull(const char *cp,char **endp,unsigned int base);
#endif

// numerical
#define ABS(x)  (((x) >= 0) ? (x) : -(x))

void unsigned_divide(unsigned int dividened, unsigned int divisor, unsigned int *pquotient, unsigned int *premainder);
void signed_divide(int dividened, int divisor, int *pquotient, int *premainder);
int idivide(int dividened, int divisor);
int imodulus(int dividened, int divisor);

long get_ram_size(volatile long *base, long maxsize);

// network
int parse_netaddr(char *str, unsigned char *addr, int len);
int parse_ipaddr(char *str, unsigned int *ipaddr);
char *ipaddr_to_str(unsigned int ipaddr);

// byte ordering
static __inline__ unsigned short swab16(unsigned short x)
{
	return ((unsigned short)((((unsigned short)(x) & 0x00ff) << 8) | (((unsigned short)(x) & 0xff00) >> 8)));
}

static __inline__ unsigned int swab32(volatile unsigned int x)
{
	return ((unsigned long int)((((unsigned long int)(x) & 0x000000ffU) << 24) |
				(((unsigned long int)(x) & 0x0000ff00U) << 8) |
				(((unsigned long int)(x) & 0x00ff0000U) >> 8) |
				(((unsigned long int)(x) & 0xff000000U) >> 24)));
}

// for networking
static __inline__ unsigned short htons(unsigned short x)
{
	return swab16(x);
}

static __inline__ unsigned int htonl(unsigned int x)
{
	return swab32(x);
}

static __inline__ unsigned short ntohs(unsigned short x)
{
	return htons(x);
}

static __inline__ unsigned int ntohl(unsigned int x)
{
	return htonl(x);
}

unsigned binfile_crc_check(void *startptr, unsigned int size);
#ifdef CONFIG_ENABLE_BITMAPS
int bmp_inflate(unsigned char *in_buf, unsigned char *out_buf, unsigned int len);
#endif

#endif
