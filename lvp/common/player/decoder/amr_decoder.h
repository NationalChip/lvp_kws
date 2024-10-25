#ifndef __AMR_DECODER_H__
#define __AMR_DECODER_H__

#define AMRNB_FRAME_LEN 13

#define AMRWB_FRAME_LEN 18

int lvp_amr_decode(const unsigned char *amr_data, int amr_len, unsigned char *pcm_data, int pcm_len);
int lvp_amr_decoder_init(int sample_rate, int channels, int bytes_rate);

#endif
