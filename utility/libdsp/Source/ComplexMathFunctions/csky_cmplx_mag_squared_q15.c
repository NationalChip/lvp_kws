/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_cmplx_mag_squared_q15.c
* Description:  Q15 complex magnitude squared
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
* This file comes from arm_cmplx_nag_squared_q15.c. It is renamed by replacing
* arm with csky.
*
*/

/******************************************************************************
 * @file     csky_cmplx_mag_squared_q15.c
 * @brief    Q15 complex magnitude squared.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupCmplxMath
 */

/**
 * @addtogroup cmplx_mag_squared
 * @{
 */

/**
 * @brief  Q15 complex magnitude squared
 * @param  *pSrc points to the complex input vector
 * @param  *pDst points to the real output vector
 * @param  numSamples number of complex samples in the input vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function implements 1.15 by 1.15 multiplications and finally output is converted into 3.13 format.
 */

void csky_cmplx_mag_squared_q15(
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t numSamples)
{
  q31_t acc0, acc1;                              /* Accumulators */

#ifndef CSKY_MATH_NO_SIMD

  uint32_t blkCnt;                               /* loop counter */
  q31_t in1, in2, in3, in4;
  q31_t acc2, acc3;

  /*loop Unrolling */
  blkCnt = numSamples >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C[0] = (A[0] * A[0] + A[1] * A[1]) */
    in1 = *__SIMD32(pSrc)++;
    in2 = *__SIMD32(pSrc)++;
    in3 = *__SIMD32(pSrc)++;
    in4 = *__SIMD32(pSrc)++;

    acc0 = __SMUAD(in1, in1);
    acc1 = __SMUAD(in2, in2);
    acc2 = __SMUAD(in3, in3);
    acc3 = __SMUAD(in4, in4);

    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ = (q15_t) (acc0 >> 17);
    *pDst++ = (q15_t) (acc1 >> 17);
    *pDst++ = (q15_t) (acc2 >> 17);
    *pDst++ = (q15_t) (acc3 >> 17);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the numSamples is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = numSamples % 0x4u;

  while(blkCnt > 0u)
  {
    /* C[0] = (A[0] * A[0] + A[1] * A[1]) */
    in1 = *__SIMD32(pSrc)++;
    acc0 = __SMUAD(in1, in1);

    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ = (q15_t) (acc0 >> 17);

    /* Decrement the loop counter */
    blkCnt--;
  }

#else

  q15_t real, imag;                              /* Temporary variables to store real and imaginary values */

  while(numSamples > 0u)
  {
    /* out = ((real * real) + (imag * imag)) */
    real = *pSrc++;
    imag = *pSrc++;
    acc0 = (real * real);
    acc1 = (imag * imag);
    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ = (q15_t) (((q63_t) acc0 + acc1) >> 17);

    /* Decrement the loop counter */
    numSamples--;
  }

#endif /* #ifndef CSKY_MATH_NO_SIMD */

}

/**
 * @} end of cmplx_mag_squared group
 */
