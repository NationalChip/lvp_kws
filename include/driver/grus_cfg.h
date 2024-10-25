#ifndef _GRUS_CFG_H_
#define _GRUS_CFG_H_

#define GRUS_CFG_ADDR 0xFF000UL
#define GRUS_CFG_XOR_LEN 1
#define GRUS_CFG_MAGIC 0x47525553 // GRUS

struct grus_cfg {
	unsigned int magic;
	unsigned int version;
	unsigned int trim_32k_value;
	unsigned int trim_1m_value;
	unsigned int trim_24m_value;
	unsigned char resv[43];
	unsigned char xor_result;
} __attribute__((packed));

#endif
