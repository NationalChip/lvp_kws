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

#include "main.h"
#define MAX_LOOPS 20

DRAM0_STAGE2_SRAM_ATTR static void silk_insertion_sort_increasing_all_values_int16(
     opus_int16                 *a,                 /* I/O   Unsorted / Sorted vector                                   */
     const opus_int             L                   /* I     Vector length                                              */
)
{
    opus_int    value;
    opus_int    i, j;

    /* Safety checks */
    celt_assert( L >  0 );

    /* Sort vector elements by value, increasing order */
    for( i = 1; i < L; i++ ) {
        value = a[ i ];
        for( j = i - 1; ( j >= 0 ) && ( value < a[ j ] ); j-- ) {
            a[ j + 1 ] = a[ j ]; /* Shift value */
        }
        a[ j + 1 ] = value; /* Write value */
    }
}
DRAM0_STAGE2_SRAM_ATTR static void silk_NLSF_stabilize(
          opus_int16            *NLSF_Q15,          /* I/O   Unstable/stabilized normalized LSF vector in Q15 [L]       */
    const opus_int16            *NDeltaMin_Q15,     /* I     Min distance vector, NDeltaMin_Q15[L] must be >= 1 [L+1]   */
    const opus_int              L                   /* I     Number of NLSF parameters in the input vector              */
)
{
    opus_int   i, I=0, k, loops;
    opus_int16 center_freq_Q15;
    opus_int32 diff_Q15, min_diff_Q15, min_center_Q15, max_center_Q15;

    /* This is necessary to ensure an output within range of a opus_int16 */
    silk_assert( NDeltaMin_Q15[L] >= 1 );

    for( loops = 0; loops < MAX_LOOPS; loops++ ) {
        /**************************/
        /* Find smallest distance */
        /**************************/
        /* First element */
        min_diff_Q15 = NLSF_Q15[0] - NDeltaMin_Q15[0];
        I = 0;
        /* Middle elements */
        for( i = 1; i <= L-1; i++ ) {
            diff_Q15 = NLSF_Q15[i] - ( NLSF_Q15[i-1] + NDeltaMin_Q15[i] );
            if( diff_Q15 < min_diff_Q15 ) {
                min_diff_Q15 = diff_Q15;
                I = i;
            }
        }
        /* Last element */
        diff_Q15 = ( 1 << 15 ) - ( NLSF_Q15[L-1] + NDeltaMin_Q15[L] );
        if( diff_Q15 < min_diff_Q15 ) {
            min_diff_Q15 = diff_Q15;
            I = L;
        }

        /***************************************************/
        /* Now check if the smallest distance non-negative */
        /***************************************************/
        if( min_diff_Q15 >= 0 ) {
            return;
        }

        if( I == 0 ) {
            /* Move away from lower limit */
            NLSF_Q15[0] = NDeltaMin_Q15[0];

        } else if( I == L) {
            /* Move away from higher limit */
            NLSF_Q15[L-1] = ( 1 << 15 ) - NDeltaMin_Q15[L];

        } else {
            /* Find the lower extreme for the location of the current center frequency */
            min_center_Q15 = 0;
            for( k = 0; k < I; k++ ) {
                min_center_Q15 += NDeltaMin_Q15[k];
            }
            min_center_Q15 += silk_RSHIFT( NDeltaMin_Q15[I], 1 );

            /* Find the upper extreme for the location of the current center frequency */
            max_center_Q15 = 1 << 15;
            for( k = L; k > I; k-- ) {
                max_center_Q15 -= NDeltaMin_Q15[k];
            }
            max_center_Q15 -= silk_RSHIFT( NDeltaMin_Q15[I], 1 );

            /* Move apart, sorted by value, keeping the same center frequency */
            center_freq_Q15 = (opus_int16)silk_LIMIT_32( silk_RSHIFT_ROUND( (opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I], 1 ),
                min_center_Q15, max_center_Q15 );
            NLSF_Q15[I-1] = center_freq_Q15 - silk_RSHIFT( NDeltaMin_Q15[I], 1 );
            NLSF_Q15[I] = NLSF_Q15[I-1] + NDeltaMin_Q15[I];
        }
    }

    /* Safe and simple fall back method, which is less ideal than the above */
    if( loops == MAX_LOOPS )
    {
        /* Insertion sort (fast for already almost sorted arrays):   */
        /* Best case:  O(n)   for an already sorted array            */
        /* Worst case: O(n^2) for an inversely sorted array          */
        silk_insertion_sort_increasing_all_values_int16( &NLSF_Q15[0], L );

        /* First NLSF should be no less than NDeltaMin[0] */
        NLSF_Q15[0] = silk_max_int( NLSF_Q15[0], NDeltaMin_Q15[0] );

        /* Keep delta_min distance between the NLSFs */
        for( i = 1; i < L; i++ )
            NLSF_Q15[i] = silk_max_int( NLSF_Q15[i], silk_ADD_SAT16( NLSF_Q15[i-1], NDeltaMin_Q15[i] ) );

        /* Last NLSF should be no higher than 1 - NDeltaMin[L] */
        NLSF_Q15[L-1] = silk_min_int( NLSF_Q15[L-1], (1<<15) - NDeltaMin_Q15[L] );

        /* Keep NDeltaMin distance between the NLSFs */
        for( i = L-2; i >= 0; i-- )
            NLSF_Q15[i] = silk_min_int( NLSF_Q15[i], NLSF_Q15[i+1] - NDeltaMin_Q15[i+1] );
    }
}

/* Predictive dequantizer for NLSF residuals */
DRAM0_STAGE2_SRAM_ATTR static OPUS_INLINE void silk_NLSF_residual_dequant(          /* O    Returns RD value in Q30                     */
          opus_int16         x_Q10[],                        /* O    Output [ order ]                            */
    const opus_int8          indices[],                      /* I    Quantization indices [ order ]              */
    const opus_uint8         pred_coef_Q8[],                 /* I    Backward predictor coefs [ order ]          */
    const opus_int           quant_step_size_Q16,            /* I    Quantization step size                      */
    const opus_int16         order                           /* I    Number of input values                      */
)
{
    opus_int     i, out_Q10, pred_Q10;

    out_Q10 = 0;
    for( i = order-1; i >= 0; i-- ) {
        pred_Q10 = silk_RSHIFT( silk_SMULBB( out_Q10, (opus_int16)pred_coef_Q8[ i ] ), 8 );
        out_Q10  = silk_LSHIFT( indices[ i ], 10 );
        if( out_Q10 > 0 ) {
            out_Q10 = silk_SUB16( out_Q10, SILK_FIX_CONST( NLSF_QUANT_LEVEL_ADJ, 10 ) );
        } else if( out_Q10 < 0 ) {
            out_Q10 = silk_ADD16( out_Q10, SILK_FIX_CONST( NLSF_QUANT_LEVEL_ADJ, 10 ) );
        }
        out_Q10  = silk_SMLAWB( pred_Q10, (opus_int32)out_Q10, quant_step_size_Q16 );
        x_Q10[ i ] = out_Q10;
    }
}


/***********************/
/* NLSF vector decoder */
/***********************/
DRAM0_STAGE2_SRAM_ATTR void silk_NLSF_decode(
          opus_int16            *pNLSF_Q15,                     /* O    Quantized NLSF vector [ LPC_ORDER ]         */
          opus_int8             *NLSFIndices,                   /* I    Codebook path vector [ LPC_ORDER + 1 ]      */
    const silk_NLSF_CB_struct   *psNLSF_CB                      /* I    Codebook object                             */
)
{
    opus_int         i;
    opus_uint8       pred_Q8[  MAX_LPC_ORDER ];
    opus_int16       ec_ix[    MAX_LPC_ORDER ];
    opus_int16       res_Q10[  MAX_LPC_ORDER ];
    opus_int32       NLSF_Q15_tmp;
    const opus_uint8 *pCB_element;
    const opus_int16 *pCB_Wght_Q9;

    /* Unpack entropy table indices and predictor for current CB1 index */
    silk_NLSF_unpack( ec_ix, pred_Q8, psNLSF_CB, NLSFIndices[ 0 ] );

    /* Predictive residual dequantizer */
    silk_NLSF_residual_dequant( res_Q10, &NLSFIndices[ 1 ], pred_Q8, psNLSF_CB->quantStepSize_Q16, psNLSF_CB->order );

    /* Apply inverse square-rooted weights to first stage and add to output */
    pCB_element = &psNLSF_CB->CB1_NLSF_Q8[ NLSFIndices[ 0 ] * psNLSF_CB->order ];
    pCB_Wght_Q9 = &psNLSF_CB->CB1_Wght_Q9[ NLSFIndices[ 0 ] * psNLSF_CB->order ];
    for( i = 0; i < psNLSF_CB->order; i++ ) {
        NLSF_Q15_tmp = silk_ADD_LSHIFT32( silk_DIV32_16( silk_LSHIFT( (opus_int32)res_Q10[ i ], 14 ), pCB_Wght_Q9[ i ] ), (opus_int16)pCB_element[ i ], 7 );
        pNLSF_Q15[ i ] = (opus_int16)silk_LIMIT( NLSF_Q15_tmp, 0, 32767 );
    }
    silk_NLSF_stabilize(pNLSF_Q15, psNLSF_CB->deltaMin_Q15, psNLSF_CB->order);
}
