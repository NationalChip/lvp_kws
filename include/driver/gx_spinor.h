#ifndef __GX_SPINOR_H__
#define __GX_SPINOR_H__

int gx_spi_nor_init(int bus_num,int cs, int speed, int mode);
void gx_spi_nor_protect_init(void);
int gx_spi_nor_fast_readdata(unsigned int from, unsigned char *buf,unsigned int len);
int gx_spi_nor_dual_readdata(unsigned int from, unsigned char *buf,unsigned int len);
int gx_spi_nor_readdata(unsigned int from, unsigned char *buf, unsigned int len);
int gx_spi_nor_pageprogram(unsigned int to,unsigned char *buf,unsigned int len);
int gx_spi_nor_chiperase(void);
int gx_spi_nor_erasedata(unsigned int addr, unsigned int len);
void gx_spi_nor_sync(void);

void gx_spi_nor_exit_4byte(void);
char *gx_spi_nor_gettype(void);
unsigned int gx_spi_nor_get_info(enum gx_flash_info flash_info);

int gx_spi_nor_write_protect_mode(void);
int gx_spi_nor_write_protect_status(unsigned int *len);
int gx_spi_nor_write_protect_lock(unsigned int protect_addr);
int gx_spi_nor_write_protect_unlock(void);
int gx_spi_nor_set_lock_reg(void);
int gx_spi_nor_get_lock_reg(int *lock);
void gx_spi_nor_calcblockrange(unsigned int addr, unsigned int len, unsigned int *pstart, unsigned int *pend);

int gx_spi_nor_otp_status(unsigned char *buf);
int gx_spi_nor_otp_lock(void);
int gx_spi_nor_otp_read(unsigned int addr, unsigned char *buf, unsigned int len);
int gx_spi_nor_otp_write(unsigned int addr, unsigned char *buf, unsigned int len);
int gx_spi_nor_otp_erase(void);
int gx_spi_nor_otp_get_regions(unsigned int *region);
int gx_spi_nor_otp_set_region(unsigned int region);

#endif
