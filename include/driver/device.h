
#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <driver/spi.h>

void device_list_init(void);
int spi_register_master(struct spi_master *master);
int spi_flash_register_master(struct sflash_master *master);

#endif
