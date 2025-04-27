/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_var_f32.c
* Description:  Variance of the elements od a floating-point vector
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
* This file comes from arm_var_f32.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_var_f32.c
 * @brief    Variance of the elements of a floating-point vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupStats
 */

/**
 * @defgroup variance  Variance
 *
 * Calculates the variance of the elements in the input vector.
 * The underlying algorithm is used:
 *
 * <pre>
 * 	Result = (sumOfSquares - sum<sup>2</sup> / blockSize) / (blockSize - 1)
 *
 *	   where, sumOfSquares = pSrc[0] * pSrc[0] + pSrc[1] * pSrc[1] + ... + pSrc[blockSize-1] * pSrc[blockSize-1]
 *
 *	                   sum = pSrc[0] + pSrc[1] + pSrc[2] + ... + pSrc[blockSize-1]
 * </pre>
 *
 * There are separate functions for floating point, Q31, and Q15 data types.
 */

/**
 * @addtogroup variance
 * @{
 */


/**
 * @brief Variance of the elements of a floating-point vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult variance value returned here
 * @return none.
 *
 */


void csky_var_f32(
  float32_t * pSrc,
  uint32_t blockSize,
  float32_t * pResult)
{

  float32_t sum = 0.0f;                          /* Temporary result storage */
  float32_t sumOfSquares = 0.0f;                 /* Sum of squares */
  float32_t in;                                  /* input value */
  uint32_t blkCnt;                               /* loop counter */

#ifndef CSKY_MATH_NO_SIMD


  float32_t meanOfSquares, mean, squareOfMean;   /* Temporary variables */

	if(blockSize == 1)
	{
		*pResult = 0;
		return;
	}

  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1])  */
    /* Compute Sum of squares of the input samples
     * and then store the result in a temporary variable, sum. */
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute Sum of squares of the input samples
     * and then store the result in a temporary variable, sum. */
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Compute Mean of squares of the input samples
   * and then store the result in a temporary variable, meanOfSquares. */
  meanOfSquares = sumOfSquares / ((float32_t) blockSize - 1.0f);

  /* Compute mean of all input values */
  mean = sum / (float32_t) blockSize;

  /* Compute square of mean */
  squareOfMean = (mean * mean) * (((float32_t) blockSize) /
                                  ((float32_t) blockSize - 1.0f));

  /* Compute variance and then store the result to the destination */
  *pResult = meanOfSquares - squareOfMean;

#else

  float32_t squareOfSum;                         /* Square of Sum */

	if(blockSize == 1)
	{
		*pResult = 0;
		return;
	}

  /* Loop over blockSize number of values */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute Sum of squares of the input samples
     * and then store the result in a temporary variable, sumOfSquares. */
    in = *pSrc++;
    sumOfSquares += in * in;

    /* C = (A[0] + A[1] + ... + A[blockSize-1]) */
    /* Compute Sum of the input samples
     * and then store the result in a temporary variable, sum. */
    sum += in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Compute the square of sum */
  squareOfSum = ((sum * sum) / (float32_t) blockSize);

  /* Compute the variance */
  *pResult = ((sumOfSquares - squareOfSum) / (float32_t) (blockSize - 1.0f));

#endif /* #ifndef CSKY_MATH_NO_SIMD */

}

/**
 * @} end of variance group
 */
