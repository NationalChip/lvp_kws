#ifndef __ADPCM_DECODER_H__
#define __ADPCM_DECODER_H__

int lvp_adpcm_decode(const unsigned char *adpcm_data, unsigned char *pcm_data, int adpcm_len);
int lvp_adpcm_decoder_init(int len);

#endif