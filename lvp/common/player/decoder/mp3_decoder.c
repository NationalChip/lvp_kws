#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mp3dec.h"
#include "mp3common.h"  /* includes mp3dec.h (public API) and internal, platform-independent API */

static HMP3Decoder hMP3Decoder;

int lvp_mp3_decoder_init(void)
{
    if((hMP3Decoder = MP3InitDecoder()) == 0)
        return -1;

    return 0;
}

int lvp_mp3_decode(unsigned char **in_data, int *in_len, unsigned char *out_data, int *out_len)
{
    MP3FrameInfo mp3FrameInfo;

    int err = MP3Decode(hMP3Decoder, in_data, in_len, (short *)out_data, 0);

    if(err)
    {
        printf("Error[%d %x] after MP3Decode\n", err, err);

        /* error occurred */
        switch (err)
        {
            case ERR_MP3_INDATA_UNDERFLOW:
                printf("ERR_MP3_INDATA_UNDERFLOW\n");
                break;
            case ERR_MP3_MAINDATA_UNDERFLOW:
                printf("ERR_MP3_MAINDATA_UNDERFLOW\n");
                /* do nothing - next call to decode will provide more mainData */
                break;
            case ERR_MP3_FREE_BITRATE_SYNC:
            default:
                printf("ERR_MP3_FREE_BITRATE_SYNC and Default error!\n");
                break;
        }
    }
    else
    {
        /* no error */
        MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
        *out_len = mp3FrameInfo.bitsPerSample / 8 * mp3FrameInfo.outputSamps;
    }

    return 0;
}

int lvp_mp3_getinfo(const unsigned char *mp3_data, int data_len, int *offset, MP3FrameInfo *info)
{
    int off;
    MP3FrameInfo inf;
    off = MP3FindSyncWord(mp3_data, data_len);
    *offset = off;
    if(offset < 0)
        return -1;
    MP3GetNextFrameInfo(hMP3Decoder, &inf, mp3_data + off);
    if(info)
        *info = inf;
#if 0
    printf("bitrate: %d\n", info.bitrate);
    printf("nChans: %d\n", info.nChans);
    printf("samprate: %d\n", info.samprate);
    printf("bitsPerSample: %d\n", info.bitsPerSample);
    printf("outputSamps: %d\n", info.outputSamps);
    printf("layer: %d\n", info.layer);
    printf("version: %d\n", info.version);
#endif
    return 0;
}
