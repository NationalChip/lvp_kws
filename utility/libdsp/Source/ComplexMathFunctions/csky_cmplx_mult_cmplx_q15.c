/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_cmplx_mult_cmplx_q15.c
* Description:  Q15 complex-by-complex multiplication
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
* This file comes from arm_cmplx_mult_cmplx_q15.c. It is renamed by replacing
* arm with csky.
*
*/

/******************************************************************************
 * @file     csky_cmplx_mult_cmplx_q15.c
 * @brief    Q15 complex-by-complex multiplication.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupCmplxMath
 */

/**
 * @addtogroup CmplxByCmplxMult
 * @{
 */

/**
 * @brief  Q15 complex-by-complex multiplication
 * @param[in]  *pSrcA points to the first input vector
 * @param[in]  *pSrcB points to the second input vector
 * @param[out]  *pDst  points to the output vector
 * @param[in]  numSamples number of complex samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function implements 1.15 by 1.15 multiplications and finally output is converted into 3.13 format.
 */

void csky_cmplx_mult_cmplx_q15(
  q15_t * pSrcA,
  q15_t * pSrcB,
  q15_t * pDst,
  uint32_t numSamples)
{
  q15_t a, b, c, d;                              /* Temporary variables to store real and imaginary values */

#ifndef CSKY_MATH_NO_SIMD

  uint32_t blkCnt;                               /* loop counters */

  /* loop Unrolling */
  blkCnt = numSamples >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
    /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * c) >> 17) - (((q31_t) b * d) >> 17);
    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * d) >> 17) + (((q31_t) b * c) >> 17);

    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * c) >> 17) - (((q31_t) b * d) >> 17);
    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * d) >> 17) + (((q31_t) b * c) >> 17);

    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * c) >> 17) - (((q31_t) b * d) >> 17);
    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * d) >> 17) + (((q31_t) b * c) >> 17);

    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * c) >> 17) - (((q31_t) b * d) >> 17);
    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * d) >> 17) + (((q31_t) b * c) >> 17);

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = numSamples % 0x4u;

  while(blkCnt > 0u)
  {
    /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
    /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * c) >> 17) - (((q31_t) b * d) >> 17);
    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * d) >> 17) + (((q31_t) b * c) >> 17);

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

#else


  while(numSamples > 0u)
  {
    /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
    /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * c) >> 17) - (((q31_t) b * d) >> 17);
    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * d) >> 17) + (((q31_t) b * c) >> 17);

    /* Decrement the blockSize loop counter */
    numSamples--;
  }

#endif /* #ifndef CSKY_MATH_NO_SIMD */

}

/**
 * @} end of CmplxByCmplxMult group
 */
