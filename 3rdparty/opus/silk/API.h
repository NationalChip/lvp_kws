/***********************************************************************
Copyright (c) 2006-2011, Skype Limited. All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
- Neither the name of Internet Society, IETF or IETF Trust, nor the
names of specific contributors, may be used to endorse or promote
products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#ifndef SILK_API_H
#define SILK_API_H

#include "control.h"
#include "typedef.h"
#include "errors.h"
#include "entdec.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SILK_MAX_FRAMES_PER_PACKET  3

/* Struct for TOC (Table of Contents) */
typedef struct {
    opus_int    VADFlag;                                /* Voice activity for packet                            */
    opus_int    VADFlags[ SILK_MAX_FRAMES_PER_PACKET ]; /* Voice activity for each frame in packet              */
    opus_int    inbandFECFlag;                          /* Flag indicating if packet contains in-band FEC       */
} silk_TOC_struct;

/****************************************/
/* Decoder functions                    */
/****************************************/

/*************************/
/* Init or Reset decoder */
/*************************/
opus_int silk_InitDecoder(void);

/******************/
/* Decode a frame */
/******************/
opus_int silk_Decode(                                   /* O    Returns error code                              */
    silk_DecControlStruct*          decControl,         /* I/O  Control Structure                               */
    opus_int                        lostFlag,           /* I    0: no loss, 1 loss, 2 decode fec                */
    opus_int                        newPacketFlag,      /* I    Indicates first decoder call for this packet    */
    ec_dec                          *psRangeDec,        /* I/O  Compressor data structure                       */
    opus_int16                      *samplesOut,        /* O    Decoded output speech vector                    */
    opus_int32                      *nSamplesOut,       /* O    Number of samples decoded                       */
    int                             arch                /* I    Run-time architecture                           */
);

#if 0
/**************************************/
/* Get table of contents for a packet */
/**************************************/
opus_int silk_get_TOC(
    const opus_uint8                *payload,           /* I    Payload data                                */
    const opus_int                  nBytesIn,           /* I    Number of input bytes                       */
    const opus_int                  nFramesPerPayload,  /* I    Number of SILK frames per payload           */
    silk_TOC_struct                 *Silk_TOC           /* O    Type of content                             */
);
#endif

#ifdef __cplusplus
}
#endif

#endif
