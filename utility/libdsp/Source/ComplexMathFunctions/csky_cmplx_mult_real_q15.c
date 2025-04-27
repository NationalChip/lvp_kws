/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_const_structs.c
* Description:  Constant structs that are initialized for user convenience.
*               For example, some can be given as arguments to the arm_cfft_f32() or arm_rfft_f32() functions.
*
* $Date:        27. January 2017
* $Revision:    V.1.5.1
*
* Target Processor: Cortex-M cores
*-------------------------------------------------------------------- */
/*
* Copyright (C) 2010-2019 ARM Limited or its affiliates. All rights reserved.
*
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the License); you may
* not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an AS IS BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
* -------------------------------------------------------------------- */
/*
* Copyright (C) 2016-2020 T-HEAD Limited. All rights reserved.
*
* This file comes from arm_const_structs.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_cmplx_mult_real_q15.c
 * @brief    Q15 complex by real multiplication.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupCmplxMath
 */

/**
 * @addtogroup CmplxByRealMult
 * @{
 */


/**
 * @brief  Q15 complex-by-real multiplication
 * @param[in]  *pSrcCmplx points to the complex input vector
 * @param[in]  *pSrcReal points to the real input vector
 * @param[out]  *pCmplxDst points to the complex output vector
 * @param[in]  numSamples number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q15 range [0x8000 0x7FFF] will be saturated.
 */

void csky_cmplx_mult_real_q15(
  q15_t * pSrcCmplx,
  q15_t * pSrcReal,
  q15_t * pCmplxDst,
  uint32_t numSamples)
{
  q15_t in;                                      /* Temporary variable to store input value */

#ifndef CSKY_MATH_NO_SIMD

  uint32_t blkCnt;                               /* loop counters */
  q31_t inA1, inA2;                              /* Temporary variables to hold input data */
  q31_t inB1;                                    /* Temporary variables to hold input data */
  q15_t out1, out2, out3, out4;                  /* Temporary variables to hold output data */
  q31_t mul1, mul2, mul3, mul4;                  /* Temporary variables to hold intermediate data */

  /* loop Unrolling */
  blkCnt = numSamples >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C[2 * i] = A[2 * i] * B[i].            */
    /* C[2 * i + 1] = A[2 * i + 1] * B[i].        */
    /* read complex number both real and imaginary from complex input buffer */
    inA1 = *__SIMD32(pSrcCmplx)++;
    /* read two real values at a time from real input buffer */
    inB1 = *__SIMD32(pSrcReal)++;
    /* read complex number both real and imaginary from complex input buffer */
    inA2 = *__SIMD32(pSrcCmplx)++;

    /* multiply complex number with real numbers */
#ifndef CSKY_MATH_BIG_ENDIAN
    mul1 = (q31_t) ((q15_t) (inA1) * (q15_t) (inB1));
    mul2 = (q31_t) ((q15_t) (inA1 >> 16) * (q15_t) (inB1));
    mul3 = (q31_t) ((q15_t) (inA2) * (q15_t) (inB1 >> 16));
    mul4 = (q31_t) ((q15_t) (inA2 >> 16) * (q15_t) (inB1 >> 16));
#else
    mul2 = (q31_t) ((q15_t) (inA1 >> 16) * (q15_t) (inB1 >> 16));
    mul1 = (q31_t) ((q15_t) inA1 * (q15_t) (inB1 >> 16));
    mul4 = (q31_t) ((q15_t) (inA2 >> 16) * (q15_t) inB1);
    mul3 = (q31_t) ((q15_t) inA2 * (q15_t) inB1);
#endif /* #ifndef CSKY_MATH_BIG_ENDIAN */

    /* saturate the result */
    out1 = (q15_t) __SSAT_16(mul1 >> 15u);
    out2 = (q15_t) __SSAT_16(mul2 >> 15u);
    out3 = (q15_t) __SSAT_16(mul3 >> 15u);
    out4 = (q15_t) __SSAT_16(mul4 >> 15u);

    /* pack real and imaginary outputs and store them to destination */
    *__SIMD32(pCmplxDst)++ = __PKHBT(out1, out2, 16);
    *__SIMD32(pCmplxDst)++ = __PKHBT(out3, out4, 16);

    inA1 = *__SIMD32(pSrcCmplx)++;
    inB1 = *__SIMD32(pSrcReal)++;
    inA2 = *__SIMD32(pSrcCmplx)++;

#ifndef CSKY_MATH_BIG_ENDIAN
    mul1 = (q31_t) ((q15_t) (inA1) * (q15_t) (inB1));
    mul2 = (q31_t) ((q15_t) (inA1 >> 16) * (q15_t) (inB1));
    mul3 = (q31_t) ((q15_t) (inA2) * (q15_t) (inB1 >> 16));
    mul4 = (q31_t) ((q15_t) (inA2 >> 16) * (q15_t) (inB1 >> 16));
#else
    mul2 = (q31_t) ((q15_t) (inA1 >> 16) * (q15_t) (inB1 >> 16));
    mul1 = (q31_t) ((q15_t) inA1 * (q15_t) (inB1 >> 16));
    mul4 = (q31_t) ((q15_t) (inA2 >> 16) * (q15_t) inB1);
    mul3 = (q31_t) ((q15_t) inA2 * (q15_t) inB1);
#endif /* #ifndef CSKY_MATH_BIG_ENDIAN */

    out1 = (q15_t) __SSAT_16(mul1 >> 15u);
    out2 = (q15_t) __SSAT_16(mul2 >> 15u);
    out3 = (q15_t) __SSAT_16(mul3 >> 15u);
    out4 = (q15_t) __SSAT_16(mul4 >> 15u);

    *__SIMD32(pCmplxDst)++ = __PKHBT(out1, out2, 16);
    *__SIMD32(pCmplxDst)++ = __PKHBT(out3, out4, 16);

    /* Decrement the numSamples loop counter */
    blkCnt--;
  }

  /* If the numSamples is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = numSamples % 0x4u;

  while(blkCnt > 0u)
  {
    /* C[2 * i] = A[2 * i] * B[i].            */
    /* C[2 * i + 1] = A[2 * i + 1] * B[i].        */
    in = *pSrcReal++;
    /* store the result in the destination buffer. */
    *pCmplxDst++ =
      (q15_t) __SSAT_16((((q31_t) (*pSrcCmplx++) * (in)) >> 15));
    *pCmplxDst++ =
      (q15_t) __SSAT_16((((q31_t) (*pSrcCmplx++) * (in)) >> 15));

    /* Decrement the numSamples loop counter */
    blkCnt--;
  }

#else


  while(numSamples > 0u)
  {
    /* realOut = realA * realB.            */
    /* imagOut = imagA * realB.                */
    in = *pSrcReal++;
    /* store the result in the destination buffer. */
    *pCmplxDst++ =
      (q15_t) __SSAT_16((((q31_t) (*pSrcCmplx++) * (in)) >> 15));
    *pCmplxDst++ =
      (q15_t) __SSAT_16((((q31_t) (*pSrcCmplx++) * (in)) >> 15));

    /* Decrement the numSamples loop counter */
    numSamples--;
  }

#endif /* #ifndef CSKY_MATH_NO_SIMD */

}

/**
 * @} end of CmplxByRealMult group
 */
