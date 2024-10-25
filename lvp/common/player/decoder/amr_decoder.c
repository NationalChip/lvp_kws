#include <interf_dec.h>
#include <string.h>
#include <driver/gx_timer.h>
#include <stdio.h>
#include "base_decoder.h"
#include "amr.h"
#include "amr_decoder.h"

typedef struct {
    int enc_len;
    int dec_len;
}amrcodec_len;

static amrcodec_len codec_len;

int lvp_amr_decode(const unsigned char *amr_data, int amr_len, unsigned char *pcm_data, int pcm_len)
{
    int ret = 0;
    if (codec_len.enc_len <= 0)
        return ret;
    for (int i = 0; i < amr_len/codec_len.enc_len && i < pcm_len/codec_len.dec_len; i++) {
        Amr_Decode((unsigned char *)(amr_data + i * codec_len.enc_len), codec_len.enc_len,
                (short *)(pcm_data + i * codec_len.dec_len), codec_len.dec_len);
        ret += codec_len.dec_len;
    }
    return ret;
}

int lvp_amr_decoder_init(int sample_rate, int channels, int bytes_rate)
{
    if (sample_rate == 8000)
    {
        codec_len.enc_len = AMRNB_FRAME_LEN;
        codec_len.dec_len = PCM_8K_FRAME_LEN;
    }
    else if (sample_rate == 16000)
    {
        codec_len.enc_len = AMRWB_FRAME_LEN;
        codec_len.dec_len = PCM_16K_FRAME_LEN;
    }
    else
    {
        memset(&codec_len, 0, sizeof(codec_len));
        return -1;
    }
    Amr_Decoder_Init(sample_rate, channels, bytes_rate);
    return 0;
}
