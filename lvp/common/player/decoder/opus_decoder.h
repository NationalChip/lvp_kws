#ifndef __OPUS_DECODER__
#define __OPUS_DECODER__

#define OPUS_FRAME_LEN 40

int lvp_opus_decode(const unsigned char *opus_data, int opus_len, char *pcm_data, int pcm_len);
int lvp_opus_decoder_init(int sample_rate, int channels);

#endif
