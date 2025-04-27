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
 * @file     csky_cmplx_mult_cmplx_re_f32.c
 * @brief    Floating-point complex-by-complex multiplication in revesal manner.
 * @version  V1.0
 * @date     15. Jan 2018
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupCmplxMath
 */

/**
 * @defgroup CmplxByCmplxMultRe Complex-by-Complex Multiplication in reversal manner.
 *
 * Multiplies a complex vector by another complex vector and generates a complex result.
 * The data in the complex arrays is stored in an interleaved fashion
 * (real, imag, real, imag, ...).
 * The parameter <code>numSamples</code> represents the number of complex
 * samples processed.  The complex arrays have a total of <code>2*numSamples</code>
 * real values.
 *
 * The underlying algorithm is used:
 *
 * <pre>
 * for(n=0; n<numSamples; n++) {
 *     pDst[(2*n)+0] = pSrcA[(2*n)+0] * pSrcB[(2*(numSample-n))-2] - pSrcA[(2*n)+1] * pSrcB[(2*(numSample-n))-1];
 *     pDst[(2*n)+1] = pSrcA[(2*n)+0] * pSrcB[(2*(numSample-n))-1] + pSrcA[(2*n)+1] * pSrcB[(2*(numSample-n))-2];
 * }
 * </pre>
 *
 * There are separate functions for floating-point, Q15, and Q31 data types.
 */

/**
 * @addtogroup CmplxByCmplxMultRe
 * @{
 */


/**
 * @brief  Floating-point complex-by-complex multiplication in reversal manner
 * @param[in]  *pSrcA points to the first input vector
 * @param[in]  *pSrcB points to the end of the secod input vector
 * @param[out]  *pDst  points to the output vector
 * @param[in]  numSamples number of complex samples in each vector
 * @return none.
 */

void csky_cmplx_mult_cmplx_re_f32(
  float32_t * pSrcA,
  float32_t * pSrcB,
  float32_t * pDst,
  uint32_t numSamples)
{
  float32_t a1, b1, c1, d1;                      /* Temporary variables to store real and imaginary values */
  uint32_t blkCnt;                               /* loop counters */
  uint32_t N;

#ifndef CSKY_MATH_NO_SIMD

  float32_t a2, b2, c2, d2;                      /* Temporary variables to store real and imaginary values */
  float32_t acc1, acc2, acc3, acc4;


  /* loop Unrolling */
  blkCnt = numSamples >> 2u;
  N      = numSamples;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    pSrcB -= 8u;
    /* C[2 * i] = A[2 * i] * B[2*(N - i) - 1] - A[2 * i + 1] * B[2*(N -i) - 2].  */
    /* C[2 * i + 1] = A[2 * i] * B[2*(N- i) -2] + A[2 * i + 1] * B[2*(N-i)-1].  */
    a1 = *pSrcA;                /* A[2 * i] */
    c1 = -*(pSrcB + 7);         /* B[2*(N-i)-1] */

    b1 = *(pSrcA + 1);          /* A[2 * i + 1] */
    acc1 = a1 * c1;             /* acc1 = A[2 * i] * B[2*(N-i)-1] */

    a2 = *(pSrcA + 2);          /* A[2 * i + 2] */
    acc2 = (b1 * c1);           /* acc2 = A[2 * i + 1] * B[2*(N-i)-1] */

    d1 = -*(pSrcB + 6);         /* B[2*(N-i)-2] */
    c2 = -*(pSrcB + 5);         /* B[2*(N-i)-3] */
    acc1 -= b1 * d1;            /* acc1 = A[2 * i] * B[2*(N-i)-1] - A[2 * i + 1] * B[2*(N-i)-2] */

    d2 = -*(pSrcB + 4);         /* B[2*(N-i)-4] */
    acc3 = a2 * c2;             /* acc3 = A[2 * i + 2] * B[2*(N-i)-3] */

    b2 = *(pSrcA + 3);          /* A[2 * i + 3] */
    acc2 += (a1 * d1);          /* acc2 = A[2 * i + 1] * B[2*(N-i)-1] + A[2 * i] * B[2*(N-i)-2] */

    a1 = *(pSrcA + 4);          /* A[2 * i + 4] */
    acc4 = (a2 * d2);           /* acc4 = A[2 * i + 2] * B[2*(N-i)-4] */

    c1 = -*(pSrcB + 3);         /* B[2*(N-i)-5] */
    acc3 -= (b2 * d2);          /* acc3 = A[2 * i + 2] * B[2*(N-i)-3] - A[2 * i + 3] * B[2*(N-3)-5] */
    *pDst = acc1;               /* C[2 * i] = A[2 * i] * B[2*(N-i)-1] - A[2 * i + 1] * B[2*(N-i)-2] */

    b1 = *(pSrcA + 5);          /* A[2 * i + 5] */
    acc4 += b2 * c2;            /* acc4 =   A[2 * i + 2] * B[2(N-i)-4] + A[2 * i + 3] * B[2*(N-i)-3] */

    *(pDst + 1) = acc2;         /* C[2 * i + 1] = A[2 * i + 1] * B[2*(N-i)-1] + A[2 * i] * B[2(N-i)-2]  */
    acc1 = (a1 * c1);

    d1 = -*(pSrcB + 2);
    acc2 = (b1 * c1);

    *(pDst + 2) = acc3;
    *(pDst + 3) = acc4;

    a2 = *(pSrcA + 6);
    acc1 -= (b1 * d1);

    c2 = -*(pSrcB + 1);
    acc2 += (a1 * d1);

    b2 = *(pSrcA + 7);
    acc3 = (a2 * c2);

    d2 = -*pSrcB;
    acc4 = (b2 * c2);

    *(pDst + 4) = acc1;
    pSrcA += 8u;

    acc3 -= (b2 * d2);
    acc4 += (a2 * d2);

    *(pDst + 5) = acc2;

    *(pDst + 6) = acc3;
    *(pDst + 7) = acc4;

    pDst += 8u;

    /* Decrement the numSamples loop counter */
    blkCnt--;
  }

  /* If the numSamples is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = numSamples % 0x4u;

#else

  blkCnt = numSamples;

#endif /* #ifndef CSKY_MATH_NO_SIMD */

  while(blkCnt > 0u)
  {
    pSrcB -= 2u;
    /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
    /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
    a1 = *pSrcA++;
    b1 = *pSrcA++;
    c1 = -*(pSrcB+1);
    d1 = -*pSrcB;

    /* store the result in the destination buffer. */
    *pDst++ = (a1 * c1) - (b1 * d1);
    *pDst++ = (a1 * d1) + (b1 * c1);

    /* Decrement the numSamples loop counter */
    blkCnt--;
  }
}

/**
 * @} end of CmplxByCmplxMult group
 */
