#ifndef __OTP_H__
#define __OTP_H__

#define OTP_USER_DATA_LENGTH (64)
int gx_otp_read(unsigned int start_addr, char *data, int rd_num);
int gx_otp_write(unsigned int start_addr, char *data, int wr_num);
int otp_get_chipname(char *chip_name, unsigned int buf_len);
int otp_get_publicid(unsigned long long *public_id);
int otp_set_userdata(unsigned int index, char *data, int length);
int otp_get_userdata(unsigned int index, char *data, int length);
int otp_lock(void);
int otp_get_lock(unsigned int *lock);

#endif
