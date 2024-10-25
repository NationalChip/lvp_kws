#include    "adpcm.h"
#include <printf.h>

static const signed char idxtbl[] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8};
static const unsigned short steptbl[] = {
    7,      8,      9,      10,     11,     12,     13,     14,     16,     17,
    19,     21,     23,     25,     28,     31,     34,     37,     41,     45,
    50,     55,     60,     66,     73,     80,     88,     97,     107,    118,
    130,    143,    157,    173,    190,    209,    230,    253,    279,    307,
    337,    371,    408,    449,    494,    544,    598,    658,    724,    796,
    876,    963,    1060,   1166,   1282,   1411,   1552,   1707,   1878,   2066,
    2272,   2499,   2749,   3024,   3327,   3660,   4026,   4428,   4871,   5358,
    5894,   6484,   7132,   7845,   8630,   9493,   10442,  11487,  12635,  13899,
    15289,  16818,  18500,  20350,  22385,  24623,  27086,  29794,  32767   };

static int de_predict = 0;
static int de_predict_idx = 0;

static int en_predict = 0;
static int en_predict_idx = 0;

void AdpcmClearEncode(void)
{
    en_predict = 0;
    en_predict_idx = 0;
}

void AdpcmClearDecode(void)
{
    de_predict = 0;
    de_predict_idx = 0;

}

void Pcm2Adpcm (short *in_pcm, short *out_adpcm, int simples)
{
    int i, j;
    unsigned short code=0;
    unsigned short code16=0;
    static signed short *out;
    out = out_adpcm;
    code = 0;

    for (i=0; i<simples; i++) {

        short di = in_pcm[i];
        int step = steptbl[en_predict_idx];
        int diff = di - en_predict;

        if (diff >=0 ) {
            code = 0;
        }
        else {
            diff = -diff;
            code = 0x8;
        }

        int diffq = step >> 3;

            if( diff >= step) {
                diff = diff - step;
                diffq = diffq + step;
                code = code + 0x4;
            }
            step = step >> 1;

            if( diff >= step) {
                diff = diff - step;
                diffq = diffq + step;
                code = code + 0x2;
            }
            step = step >> 1;

            if( diff >= step) {
                diff = diff - step;
                diffq = diffq + step;
                code = code + 0x1;
            }
//            step = step >> 1;


        code16 = (code16 >> 4) | (code << 12);
        if ( (i&3) == 3) {
//            code16 = ((code16&0x0f)<<4)|((code16&0xf0)>>4) | ((code16&0x0f00)<<4)|((code16&0xf000)>>4);
            *out++ = code16;
        }

        if(code >= 8) {
            en_predict = en_predict - diffq;

            if (en_predict < -32768) {
                en_predict = -32768;
            }
        }
        else {
            en_predict = en_predict + diffq;

            if (en_predict > 32766) {
                en_predict = 32766;
            }
        }


        en_predict_idx = en_predict_idx + idxtbl[code];
        if(en_predict_idx < 0) {
            en_predict_idx = 0;
        }
        else if(en_predict_idx > 88) {
            en_predict_idx = 88;
        }
    }
}

void Adpcm2Pcm (short *in_adpcm, short *out_pcm, int simples)
{
    int i;
    unsigned char *pcode = (unsigned char *) in_adpcm;
    unsigned char code;
    code = *pcode ++;
//    code = ((code>>4)&0x0f)|((code<<4) &0xf0);

    for (i = 0; i < simples; i++) {

        if (1) {
            int step = steptbl[de_predict_idx];

            int diffq = step >> 3;

            if (code & 4) {
                diffq = diffq + step;
            }
            step = step >> 1;
            if (code & 2) {
                diffq = diffq + step;
            }
            step = step >> 1;
            if (code & 1) {
                diffq = diffq + step;
            }

            if (code & 8) {
                de_predict = de_predict - diffq;
            }
            else {
                de_predict = de_predict + diffq;
            }

            if (de_predict > 32767) {
                de_predict = 32767;
            }
            else if (de_predict < -32768) {
                de_predict = -32768;
            }

            de_predict_idx = de_predict_idx + idxtbl[code & 15];

            if(de_predict_idx < 0) {
                de_predict_idx = 0;
            }
            else if(de_predict_idx > 88) {
                de_predict_idx = 88;
            }

            if (i&1) {
                code = *pcode ++;
//                code = ((code>>4)&0x0f)|((code<<4) &0xf0);
            }
            else {
                code = code >> 4;
            }
        }

        *out_pcm++ = de_predict;
    }
}

