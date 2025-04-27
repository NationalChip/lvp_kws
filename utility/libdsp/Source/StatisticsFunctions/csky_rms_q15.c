/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_rms_q15.c
* Description:  Root Mean Square of the elements of a Q15 vector
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
* This file comes from arm_rms_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_rms_q15.c
 * @brief    Root Mean Square of the elements of a Q15 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @addtogroup RMS
 * @{
 */

/**
 * @brief Root Mean Square of the elements of a Q15 vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult rms value returned here
 * @return none.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * The function is implemented using a 64-bit internal accumulator.
 * The input is represented in 1.15 format.
 * Intermediate multiplication yields a 2.30 format, and this
 * result is added without saturation to a 64-bit accumulator in 34.30 format.
 * With 33 guard bits in the accumulator, there is no risk of overflow, and the
 * full precision of the intermediate multiplication is preserved.
 * Finally, the 34.30 result is truncated to 34.15 format by discarding the lower
 * 15 bits, and then saturated to yield a result in 1.15 format.
 *
 */

void csky_rms_q15(
  q15_t * pSrc,
  uint32_t blockSize,
  q15_t * pResult)
{
  q63_t sum = 0;                                 /* accumulator */

#ifndef CSKY_MATH_NO_SIMD


  q31_t in;                                      /* temporary variable to store the input value */
  q15_t in1;                                     /* temporary variable to store the input value */
  uint32_t blkCnt;                               /* loop counter */

  /* loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute sum of the squares and then store the results in a temporary variable, sum */
    in = *__SIMD32(pSrc)++;
    sum = __SMLALD(in, in, sum);
    in = *__SIMD32(pSrc)++;
    sum = __SMLALD(in, in, sum);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute sum of the squares and then store the results in a temporary variable, sum */
    in1 = *pSrc++;
    sum = __SMLALD(in1, in1, sum);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Truncating and saturating the accumulator to 1.15 format */
  /* Store the result in the destination */
  csky_sqrt_q15(__SSAT_16((sum / (q63_t)blockSize) >> 15), pResult);

#else


  q15_t in;                                      /* temporary variable to store the input value */
  uint32_t blkCnt;                               /* loop counter */

  /* Loop over blockSize number of values */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute sum of the squares and then store the results in a temporary variable, sum */
    in = *pSrc++;
    sum += ((q31_t) in * in);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Truncating and saturating the accumulator to 1.15 format */
  /* Store the result in the destination */
  csky_sqrt_q15(__SSAT_16((sum / (q63_t)blockSize) >> 15), pResult);

#endif /* #ifndef CSKY_MATH_NO_SIMD */

}

/**
 * @} end of RMS group
 */
