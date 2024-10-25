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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SigProc_FIX.h"

/*******************************************/
/* LPC analysis filter                     */
/* NB! State is kept internally and the    */
/* filter always starts with zero state    */
/* first d output samples are set to zero  */
/*******************************************/

/* OPT: Using celt_fir() for this function should be faster, but it may cause
   integer overflows in intermediate values (not final results), which the
   current implementation silences by casting to unsigned. Enabling
   this should be safe in pretty much all cases, even though it is not technically
   C89-compliant. */
#define USE_CELT_FIR 0

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

DRAM0_STAGE2_SRAM_ATTR OPUS_EXPORT void silk_LPC_analysis_filter(
    opus_int16                  *out,               /* O    Output signal                                               */
    const opus_int16            *in,                /* I    Input signal                                                */
    const opus_int16            *B,                 /* I    MA prediction coefficients, Q12 [order]                     */
    const opus_int32            len,                /* I    Signal length                                               */
    const opus_int32            d,                  /* I    Filter order                                                */
    int                         arch                /* I    Run-time architecture                                       */
)
{
    opus_int   j;
#if defined(FIXED_POINT) && USE_CELT_FIR
    opus_int16 num[SILK_MAX_ORDER_LPC];
#else
    int ix;
    opus_int32       out32_Q12, out32;
    const opus_int16 *in_ptr;
#endif

    celt_assert( d >= 6 );
    celt_assert( (d & 1) == 0 );
    celt_assert( d <= len );

#if defined(FIXED_POINT) && USE_CELT_FIR
    celt_assert( d <= SILK_MAX_ORDER_LPC );
    for ( j = 0; j < d; j++ ) {
        num[ j ] = -B[ j ];
    }
    celt_fir( in + d, num, out + d, len - d, d, arch );
    for ( j = 0; j < d; j++ ) {
        out[ j ] = 0;
    }
#else

#if 0
    for( ix = d; ix < len; ix++ ) {
        in_ptr = &in[ ix - 1 ];

        out32_Q12 = silk_SMULBB( in_ptr[  0 ], B[ 0 ] );
        //printf("ix=0x%.4x  ", ix);
        //printf("in_ptr=0x%.8x  ", in_ptr);
        //printf("in_ptr[-1]=0x%.8x  ", in_ptr[-1]);
        //printf("in_ptr[0]=0x%.8x  ", in_ptr[0]);
        //printf("out32_Q12=0x%.8x  ", out32_Q12);
        //printf("B[ 0 ]=0x%.4x  ", B[ 0 ]);
        /* Allowing wrap around so that two wraps can cancel each other. The rare
           cases where the result wraps around can only be triggered by invalid streams*/
        out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -1 ], B[ 1 ] );

        out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -2 ], B[ 2 ] );
        out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -3 ], B[ 3 ] );
        out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -4 ], B[ 4 ] );
        out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -5 ], B[ 5 ] );
        for( j = 6; j < d; j += 2 ) {
            out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -j     ], B[ j     ] );
            out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -j - 1 ], B[ j + 1 ] );
        }
        /* Subtract prediction */
        out32_Q12 = silk_SUB32_ovflw( silk_LSHIFT( (opus_int32)in_ptr[ 1 ], 12 ), out32_Q12 );

        /* Scale to Q0 */
        out32 = silk_RSHIFT_ROUND( out32_Q12, 12 );
        //printf("out32_Q12=0x%.8x  ", out32_Q12);
        /* Saturate output */
        out[ ix ] = (opus_int16)silk_SAT16( out32 );
        //printf("out[ix]=0x%.4x\n", out[ix]);

    }
#else
        asm volatile(
        "lrw                    l1, 0xfffffffc     \n\t" // -4
        "mov                    t5, %0             \n\t" // ix
        "ixh                    l0, %4, %0         \n\t" // out + d * 2
        "0:                                        \n\t"
        "movi                   t0, 0              \n\t" // out32_Q12
        "movi                   t1, 0              \n\t"
        "mov                    t4, %2             \n\t" // B
        "ixh                    t9, %1, t5         \n\t"
        "subi                   t9, t9, 4          \n\t" // in_ptr - 2
        "mov                    t6, t9             \n\t"
        "lsri.u32.r             t8, %0, 1          \n\t" // d / 2

        "1:                                        \n\t"
        "ldbi.w                 t2, (t4)           \n\t" // B[0] B[1]
        "ldbir.w                t3, (t9), l1       \n\t" // in_ptr[-1] in_ptr[0]
        "mulacax.s16.e          t0, t2, t3         \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "nop                                       \n\t"
        "2:                                        \n\t"
        "bloop                  t8, 1b, 2b         \n\t"

        "ld.hs                  t7, (t6, 0x4)      \n\t" // in_ptr[ 1 ]
        "lsli                   t7, t7, 12         \n\t"
        "subu                   t0, t7, t0         \n\t"
        "asri.s32.r             t0, t0, 12         \n\t" // out32
        "clipi.s32              t0, t0, 16         \n\t"
        "stbi.h                 t0, (l0)           \n\t" // out[ ix ]
        "cmplt                  t5, %3             \n\t"
        "addi                   t5, 1              \n\t"
        "bt                     0b                 \n\t"
            :/* output */
            :/* input */
            "r"(d),                         // 0
            "r"(in),                        // 1
            "r"(B),                         // 2
            "r"(len),                       // 3
            "r"(out)                        // 4
            :"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9", "l0", "l1", "memory");
#endif


    /* Set first d output samples to zero */
    silk_memset( out, 0, d * sizeof( opus_int16 ) );
    //for (int i = d; i < len; i++)
    //    printf("%04x, ", (unsigned short)out[i]);
    //printf("\n");
#endif
}
#pragma GCC diagnostic pop
