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

#ifndef SILK_MAIN_H
#define SILK_MAIN_H

#include "SigProc_FIX.h"
#include "define.h"
#include "structs.h"
#include "tables.h"
#include "control.h"
#include "entdec.h"

#if defined(OPUS_X86_MAY_HAVE_SSE4_1)
#include "x86/main_sse.h"
#endif

#if (defined(OPUS_ARM_ASM) || defined(OPUS_ARM_MAY_HAVE_NEON_INTR))
#include "arm/NSQ_del_dec_arm.h"
#endif

/* Decodes signs of excitation */
void silk_decode_signs(
    ec_dec                      *psRangeDec,                        /* I/O  Compressor data structure               */
    opus_int16                  pulses[],                           /* I/O  pulse signal                            */
    opus_int                    length,                             /* I    length of input                         */
    const opus_int              signalType,                         /* I    Signal type                             */
    const opus_int              quantOffsetType,                    /* I    Quantization offset type                */
    const opus_int              sum_pulses[ MAX_NB_SHELL_BLOCKS ]   /* I    Sum of absolute pulses per block        */
);

/***************/
/* Shell coder */
/***************/

/* Shell decoder, operates on one shell code frame of 16 pulses */
void silk_shell_decoder(
    opus_int16                  *pulses0,                       /* O    data: nonnegative pulse amplitudes          */
    ec_dec                      *psRangeDec,                    /* I/O  Compressor data structure                   */
    const opus_int              pulses4                         /* I    number of pulses per pulse-subframe         */
);

/* Gains scalar dequantization, uniform on log scale */
void silk_gains_dequant(
    opus_int32                  gain_Q16[ MAX_NB_SUBFR ],       /* O    quantized gains                             */
    const opus_int8             ind[ MAX_NB_SUBFR ],            /* I    gain indices                                */
    opus_int8                   *prev_ind,                      /* I/O  last index in previous frame                */
    const opus_int              conditional,                    /* I    first gain is delta coded if 1              */
    const opus_int              nb_subfr                        /* I    number of subframes                          */
);

/************************************/
/* Noise shaping quantization (NSQ) */
/************************************/

#if !defined(OVERRIDE_silk_NSQ)
#endif

/* Noise shaping using delayed decision */

/************/
/* Silk VAD */
/************/

/* Unpack predictor values and indices for entropy coding tables */
void silk_NLSF_unpack(
          opus_int16            ec_ix[],                        /* O    Indices to entropy tables [ LPC_ORDER ]     */
          opus_uint8            pred_Q8[],                      /* O    LSF predictor [ LPC_ORDER ]                 */
    const silk_NLSF_CB_struct   *psNLSF_CB,                     /* I    Codebook object                             */
    const opus_int              CB1_index                       /* I    Index of vector in first LSF codebook       */
);

/***********************/
/* NLSF vector decoder */
/***********************/
void silk_NLSF_decode(
          opus_int16            *pNLSF_Q15,                     /* O    Quantized NLSF vector [ LPC_ORDER ]         */
          opus_int8             *NLSFIndices,                   /* I    Codebook path vector [ LPC_ORDER + 1 ]      */
    const silk_NLSF_CB_struct   *psNLSF_CB                      /* I    Codebook object                             */
);

/****************************************************/
/* Decoder Functions                                */
/****************************************************/
opus_int silk_init_decoder(
    silk_decoder_state          *psDec                          /* I/O  Decoder state pointer                       */
);

/* Set decoder sampling rate */
opus_int silk_decoder_set_fs(
    silk_decoder_state          *psDec,                         /* I/O  Decoder state pointer                       */
    opus_int                    fs_kHz,                         /* I    Sampling frequency (kHz)                    */
    opus_int32                  fs_API_Hz                       /* I    API Sampling frequency (Hz)                 */
);

/****************/
/* Decode frame */
/****************/
opus_int silk_decode_frame(
    silk_decoder_state          *psDec,                         /* I/O  Pointer to Silk decoder state               */
    ec_dec                      *psRangeDec,                    /* I/O  Compressor data structure                   */
    opus_int16                  pOut[],                         /* O    Pointer to output speech frame              */
    opus_int32                  *pN,                            /* O    Pointer to size of output frame             */
    opus_int                    lostFlag,                       /* I    0: no loss, 1 loss, 2 decode fec            */
    opus_int                    condCoding,                     /* I    The type of conditional coding to use       */
    int                         arch                            /* I    Run-time architecture                       */
);

/* Decode indices from bitstream */
void silk_decode_indices(
    silk_decoder_state          *psDec,                         /* I/O  State                                       */
    ec_dec                      *psRangeDec,                    /* I/O  Compressor data structure                   */
    opus_int                    FrameIndex,                     /* I    Frame number                                */
    opus_int                    decode_LBRR,                    /* I    Flag indicating LBRR data is being decoded  */
    opus_int                    condCoding                      /* I    The type of conditional coding to use       */
);

/* Decode parameters from payload */
void silk_decode_parameters(
    silk_decoder_state          *psDec,                         /* I/O  State                                       */
    silk_decoder_control        *psDecCtrl,                     /* I/O  Decoder control                             */
    opus_int                    condCoding                      /* I    The type of conditional coding to use       */
);

/* Core decoder. Performs inverse NSQ operation LTP + LPC */
void silk_decode_core(
    silk_decoder_state          *psDec,                         /* I/O  Decoder state                               */
    silk_decoder_control        *psDecCtrl,                     /* I    Decoder control                             */
    opus_int16                  xq[],                           /* O    Decoded speech                              */
    const opus_int16            pulses[ MAX_FRAME_LENGTH ],     /* I    Pulse signal                                */
    int                         arch                            /* I    Run-time architecture                       */
);

/* Decode quantization indices of excitation (Shell coding) */
void silk_decode_pulses(
    ec_dec                      *psRangeDec,                    /* I/O  Compressor data structure                   */
    opus_int16                  pulses[],                       /* O    Excitation signal                           */
    const opus_int              signalType,                     /* I    Sigtype                                     */
    const opus_int              quantOffsetType,                /* I    quantOffsetType                             */
    const opus_int              frame_length                    /* I    Frame length                                */
);

#endif
