/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_dot_prod_q31.c
* Description:  Q31 dot product
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
* This file comes from arm_dot_prod_q15.c. It is renamed by replacing arm with
* csky.
*
*/

/******************************************************************************
 * @file     csky_dot_prod_q31.c
 * @brief    Q31 dot product.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/
#include "csky_math.h"

/**
 * @ingroup groupMath
 */

/**
 * @addtogroup dot_prod
 * @{
 */

/**
 * @brief Dot product of Q31 vectors.
 * @param[in]       *pSrcA points to the first input vector
 * @param[in]       *pSrcB points to the second input vector
 * @param[in]       blockSize number of samples in each vector
 * @param[out]      *result output result returned here
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The intermediate multiplications are in 1.31 x 1.31 = 2.62 format and these
 * are truncated to 2.48 format by discarding the lower 14 bits.
 * The 2.48 result is then added without saturation to a 64-bit accumulator in 16.48 format.
 * There are 15 guard bits in the accumulator and there is no risk of overflow as long as
 * the length of the vectors is less than 2^16 elements.
 * The return result is in 16.48 format.
 */

void csky_dot_prod_q31(
  q31_t * pSrcA,
  q31_t * pSrcB,
  uint32_t blockSize,
  q63_t * result)
{
  q63_t sum = 0;                                 /* Temporary result storage */
  uint32_t blkCnt;                               /* loop counter */


#ifndef CSKY_MATH_NO_SIMD

  q31_t inA1, inA2, inA3, inA4;
  q31_t inB1, inB2, inB3, inB4;

  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
    /* Calculate dot product and then store the result in a temporary buffer. */
    inA1 = *pSrcA++;
    inA2 = *pSrcA++;
    inA3 = *pSrcA++;
    inA4 = *pSrcA++;
    inB1 = *pSrcB++;
    inB2 = *pSrcB++;
    inB3 = *pSrcB++;
    inB4 = *pSrcB++;

    sum += ((q63_t) inA1 * inB1) >> 14u;
    sum += ((q63_t) inA2 * inB2) >> 14u;
    sum += ((q63_t) inA3 * inB3) >> 14u;
    sum += ((q63_t) inA4 * inB4) >> 14u;

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
    sum += ((q63_t) * pSrcA++ * *pSrcB++) >> 14u;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Store the result in the destination buffer in 16.48 format */
  *result = sum;
}

/**
 * @} end of dot_prod group
 */
