/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_rms_f32.c
* Description:  Root mean square value of the elements of a floating-point vector
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
* This file comes from arm_rms_f32.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_rms_f32.c
 * @brief    Root mean square value of an array of F32 type.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupStats
 */

/**
 * @defgroup RMS Root mean square (RMS)
 *
 *
 * Calculates the Root Mean Sqaure of the elements in the input vector.
 * The underlying algorithm is used:
 *
 * <pre>
 * 	Result = sqrt(((pSrc[0] * pSrc[0] + pSrc[1] * pSrc[1] + ... + pSrc[blockSize-1] * pSrc[blockSize-1]) / blockSize));
 * </pre>
 *
 * There are separate functions for floating point, Q31, and Q15 data types.
 */

/**
 * @addtogroup RMS
 * @{
 */


/**
 * @brief Root Mean Square of the elements of a floating-point vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult rms value returned here
 * @return none.
 *
 */

void csky_rms_f32(
  float32_t * pSrc,
  uint32_t blockSize,
  float32_t * pResult)
{
  float32_t sum = 0.0f;                          /* Accumulator */
  float32_t in;                                  /* Tempoprary variable to store input value */
  uint32_t blkCnt;                               /* loop counter */

#ifndef CSKY_MATH_NO_SIMD


  /* loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = A[0] * A[0] + A[1] * A[1] + A[2] * A[2] + ... + A[blockSize-1] * A[blockSize-1] */
    /* Compute sum of the squares and then store the result in a temporary variable, sum  */
    in = *pSrc++;
    sum += in * in;
    in = *pSrc++;
    sum += in * in;
    in = *pSrc++;
    sum += in * in;
    in = *pSrc++;
    sum += in * in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

#else


  /* Loop over blockSize number of values */
  blkCnt = blockSize;

#endif /* #ifndef CSKY_MATH_NO_SIMD */

  while(blkCnt > 0u)
  {
    /* C = A[0] * A[0] + A[1] * A[1] + A[2] * A[2] + ... + A[blockSize-1] * A[blockSize-1] */
    /* Compute sum of the squares and then store the results in a temporary variable, sum  */
    in = *pSrc++;
    sum += in * in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Compute Rms and store the result in the destination */
  csky_sqrt_f32(sum / (float32_t) blockSize, pResult);
}

/**
 * @} end of RMS group
 */
