#include <opus.h>
#include <stdio.h>
#include "base_decoder.h"
#include "opus_decoder.h"

typedef struct {
    int sampleRate;
    int channels;
    int duration;
    int opusBitRate;
    int pcmBitRate;
    int pcmFrameSize;
    int opusFrameSize;
}opus_dec;

static opus_dec decoder;

int lvp_opus_decode(const unsigned char *opus_data, int opus_len, char *pcm_data, int pcm_len)
{
    int ret = -1;
    int opusFrameSize = decoder.opusFrameSize;
    int pcmFrameSize = decoder.pcmFrameSize;

    int decodedSize = 0;
    int outLen = 0;

    if (opus_len < opusFrameSize || pcm_len < pcmFrameSize)
        return -1;


    opus_int16 *pcmBuf = (opus_int16 *)pcm_data;
    memset(pcmBuf, 0, pcm_len);
    while (decodedSize < (opus_len / opusFrameSize)) {
        outLen = opus_decode((const unsigned char *)(opus_data + decodedSize * opusFrameSize), opusFrameSize, pcmBuf, pcmFrameSize, 0);
        if (outLen < 0) {
            printf("opus decode len(%d) opus_len(%d) %s\n", outLen, opusFrameSize, opus_strerror(outLen));
            break;
        } else if (outLen != (pcmFrameSize/sizeof(short))) {
            printf("VBS not support!! outLen(%d) pcmFrameSize(%d)\n", outLen, pcmFrameSize);
            break;
        }
        decodedSize++;
        ret += outLen;
    }
    return ret;
}

int lvp_opus_decoder_init(int sample_rate, int channels)
{
    int bitRate = 16000;

    decoder.channels = channels;
    decoder.sampleRate = sample_rate;
    decoder.opusBitRate = bitRate;
    decoder.duration = 20;
    decoder.pcmBitRate = decoder.sampleRate * decoder.channels * sizeof(opus_int16) * 8;
    //decoder.pcmFrameSize = decoder.duration * decoder.sampleRate / 1000;
    //decoder.opusFrameSize = sizeof(opus_int16) * decoder.pcmFrameSize * bitRate / decoder.pcmBitRate;
    decoder.opusFrameSize = OPUS_FRAME_LEN;
    if (sample_rate == 8000)
        decoder.pcmFrameSize = PCM_8K_FRAME_LEN;
    else
        decoder.pcmFrameSize = PCM_16K_FRAME_LEN;

    if (opus_decoder_init(decoder.sampleRate, decoder.channels) != 0)
        return -1;
    opus_decoder_ctl(OPUS_SET_BITRATE(bitRate));
    opus_decoder_ctl(OPUS_SET_VBR(0));
    return 0;
}
