/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_cmplx_dot_prod_f32.c
* Description:  Floating-point complex dot product
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
* This file comes from arm_cmplx_dot_prod_f32.c. It is renamed by replacing arm
* with csky.
*
*/

/******************************************************************************
 * @file     csky_cmplx_dot_prod_f32.c
 * @brief    Floating-point complex dot product.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupCmplxMath
 */

/**
 * @defgroup cmplx_dot_prod Complex Dot Product
 *
 * Computes the dot product of two complex vectors.
 * The vectors are multiplied element-by-element and then summed.
 *
 * The <code>pSrcA</code> points to the first complex input vector and
 * <code>pSrcB</code> points to the second complex input vector.
 * <code>numSamples</code> specifies the number of complex samples
 * and the data in each array is stored in an interleaved fashion
 * (real, imag, real, imag, ...).
 * Each array has a total of <code>2*numSamples</code> values.
 *
 * The underlying algorithm is used:
 * <pre>
 * realResult=0;
 * imagResult=0;
 * for(n=0; n<numSamples; n++) {
 *     realResult += pSrcA[(2*n)+0]*pSrcB[(2*n)+0] - pSrcA[(2*n)+1]*pSrcB[(2*n)+1];
 *     imagResult += pSrcA[(2*n)+0]*pSrcB[(2*n)+1] + pSrcA[(2*n)+1]*pSrcB[(2*n)+0];
 * }
 * </pre>
 *
 * There are separate functions for floating-point, Q15, and Q31 data types.
 */

/**
 * @addtogroup cmplx_dot_prod
 * @{
 */

/**
 * @brief  Floating-point complex dot product
 * @param  *pSrcA points to the first input vector
 * @param  *pSrcB points to the second input vector
 * @param  numSamples number of complex samples in each vector
 * @param  *realResult real part of the result returned here
 * @param  *imagResult imaginary part of the result returned here
 * @return none.
 */

void csky_cmplx_dot_prod_f32(
  float32_t * pSrcA,
  float32_t * pSrcB,
  uint32_t numSamples,
  float32_t * realResult,
  float32_t * imagResult)
{
  float32_t real_sum = 0.0f, imag_sum = 0.0f;    /* Temporary result storage */
  float32_t a0,b0,c0,d0;

#ifndef CSKY_MATH_NO_SIMD

  uint32_t blkCnt;                               /* loop counter */

  /*loop Unrolling */
  blkCnt = numSamples >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      /* Decrement the loop counter */
      blkCnt--;
  }

  /* If the numSamples is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = numSamples & 0x3u;

  while(blkCnt > 0u)
  {
      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      /* Decrement the loop counter */
      blkCnt--;
  }

#else


  while(numSamples > 0u)
  {
      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      /* Decrement the loop counter */
      numSamples--;
  }

#endif /* #ifndef CSKY_MATH_NO_SIMD */

  /* Store the real and imaginary results in the destination buffers */
  *realResult = real_sum;
  *imagResult = imag_sum;
}

/**
 * @} end of cmplx_dot_prod group
 */
