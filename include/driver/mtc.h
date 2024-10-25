#ifndef __MTC_H__
#define __MTC_H__

int gx_mtc_open(void);
int gx_mtc_close(void);
int gx_mtc_ioctl(uint32_t key, void *buf);

#endif
