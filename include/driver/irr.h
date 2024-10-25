#ifndef __IRR_H__
#define __IRR_H__

/* Cooperation with C and C++ */
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define IRR_READ_ROW_DATA (1)

unsigned char  irr_init(void);
void irr_poll(void);
int irr_close(void);
int irr_read(void *buffer, uint32_t *len);
int irr_ioctl(uint32_t key,void *buf);
void gx_irr_send_init(unsigned int gpio);
void gx_irr_send_code(unsigned char addr, unsigned char code);

#ifdef __cplusplus
}
#endif

#endif

