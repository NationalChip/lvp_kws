/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_power_q31.c
* Description:  Sum of the squares of the elements of a Q31 vector
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
* This file comes from arm_power_q31.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_power_q31.c
 * @brief    Sum of the squares of the elements of a Q31 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupStats
 */

/**
 * @addtogroup power
 * @{
 */

/**
 * @brief Sum of the squares of the elements of a Q31 vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult sum of the squares value returned here
 * @return none.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * The function is implemented using a 64-bit internal accumulator.
 * The input is represented in 1.31 format.
 * Intermediate multiplication yields a 2.62 format, and this
 * result is truncated to 2.48 format by discarding the lower 14 bits.
 * The 2.48 result is then added without saturation to a 64-bit accumulator in 16.48 format.
 * With 15 guard bits in the accumulator, there is no risk of overflow, and the
 * full precision of the intermediate multiplication is preserved.
 * Finally, the return result is in 16.48 format.
 *
 */

void csky_power_q31(
  q31_t * pSrc,
  uint32_t blockSize,
  q63_t * pResult)
{
  q63_t sum = 0;                                 /* Temporary result storage */
  q31_t in;
  uint32_t blkCnt;                               /* loop counter */


#ifndef CSKY_MATH_NO_SIMD
  q63_t tmp;

  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = A[0] * A[0] + A[1] * A[1] + A[2] * A[2] + ... + A[blockSize-1] * A[blockSize-1] */
    /* Compute Power then shift intermediate results by 14 bits to maintain 16.48 format and then store the result in a temporary variable sum, providing 15 guard bits. */

    in = *pSrc++;
    sum += ((q63_t) in * in) >> 14u;

    in = *pSrc++;
    sum += ((q63_t) in * in) >> 14u;

    in = *pSrc++;
    sum += ((q63_t) in * in) >> 14u;

    in = *pSrc++;
    sum += ((q63_t) in * in) >> 14u;

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
    /* Compute Power and then store the result in a temporary variable, sum. */
    in = *pSrc++;
    sum += ((q63_t) in * in) >> 14u;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Store the results in 16.48 format  */
  *pResult = sum;
}

/**
 * @} end of power group
 */
