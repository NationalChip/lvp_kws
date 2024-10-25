#include "adpcm_decoder.h"
#include "adpcm.h"
#include <stdio.h>
#include <driver/gx_dcache.h>

int lvp_adpcm_decode(const unsigned char *adpcm_data, unsigned char *pcm_data, int adpcm_len)
{
    // printf("adpcm decode start!\n");
    Adpcm2Pcm((short *)adpcm_data, (short *)pcm_data, adpcm_len * 2);   // adpcm_len * 2 = 采样点数
    // printf("adpcm: %#x len:%d\n",adpcm_data, adpcm_len * 2);
    gx_dcache_clean_range((unsigned int*)pcm_data, adpcm_len * 4);      // adpcm_len * 4 = 解码的pcm长度
    // printf("adpcm decode over!\n");
    return 0;
}

int lvp_adpcm_decoder_init(int len)
{
    // printf("adpcm decode init!\n");
    return 0;
}