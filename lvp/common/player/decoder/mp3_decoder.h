#ifndef __MP3_DECODER_H__
#define __MP3_DECODER_H__
#include <mp3dec.h>

int lvp_mp3_decoder_init(void);
int lvp_mp3_decode(unsigned char **in_data, int *in_len, unsigned char *out_data, int *out_len);
int lvp_mp3_getinfo(const unsigned char *mp3_data, int data_len, int *offset, MP3FrameInfo *info);

#endif
