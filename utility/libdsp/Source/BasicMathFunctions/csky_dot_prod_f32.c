/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_dot_prod_f32.c
* Description:  Floating-point dot product
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
* This file comes from arm_dot_prod_f32.c. It is renamed by replacing arm with
* csky.
*
*/

/******************************************************************************
 * @file     csky_dot_prod_f32.c
 * @brief    Floating-point dot product.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/
#include "csky_math.h"

/**
 * @ingroup groupMath
 */

/**
 * @defgroup dot_prod Vector Dot Product
 *
 * Computes the dot product of two vectors.
 * The vectors are multiplied element-by-element and then summed.
 *
 * <pre>
 *     sum = pSrcA[0]*pSrcB[0] + pSrcA[1]*pSrcB[1] + ... + pSrcA[blockSize-1]*pSrcB[blockSize-1]
 * </pre>
 *
 * There are separate functions for floating-point, Q7, Q15, and Q31 data types.
 */

/**
 * @addtogroup dot_prod
 * @{
 */

/**
 * @brief Dot product of floating-point vectors.
 * @param[in]       *pSrcA points to the first input vector
 * @param[in]       *pSrcB points to the second input vector
 * @param[in]       blockSize number of samples in each vector
 * @param[out]      *result output result returned here
 * @return none.
 */


void csky_dot_prod_f32(
  float32_t * pSrcA,
  float32_t * pSrcB,
  uint32_t blockSize,
  float32_t * result)
{
  float32_t sum = 0.0f;                          /* Temporary result storage */
  uint32_t blkCnt;                               /* loop counter */


#ifndef CSKY_MATH_NO_SIMD

  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
    /* Calculate dot product and then store the result in a temporary buffer */
    sum += (*pSrcA++) * (*pSrcB++);
    sum += (*pSrcA++) * (*pSrcB++);
    sum += (*pSrcA++) * (*pSrcB++);
    sum += (*pSrcA++) * (*pSrcB++);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

#else


  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

#endif /* #ifndef CSKY_MATH_NO_SIMD */


  while(blkCnt > 0u)
  {
    /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
    /* Calculate dot product and then store the result in a temporary buffer. */
    sum += (*pSrcA++) * (*pSrcB++);

    /* Decrement the loop counter */
    blkCnt--;
  }
  /* Store the result back in the destination buffer */
  *result = sum;
}

/**
 * @} end of dot_prod group
 */
