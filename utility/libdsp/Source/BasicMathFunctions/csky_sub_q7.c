/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_sub_q7.c
* Description:  Q7 vector subtraction
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
* This file comes from arm_sub_q7.c. It is renamed b y replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_sub_q7.c
 * @brief    Q7 vector subtraction.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupMath
 */

/**
 * @addtogroup BasicSub
 * @{
 */

/**
 * @brief Q7 vector subtraction.
 * @param[in]       *pSrcA points to the first input vector
 * @param[in]       *pSrcB points to the second input vector
 * @param[out]      *pDst points to the output vector
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q7 range [0x80 0x7F] will be saturated.
 */

void csky_sub_q7(
  q7_t * pSrcA,
  q7_t * pSrcB,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

#ifndef CSKY_MATH_NO_SIMD
  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = A - B */
    /* Subtract and then store the results in the destination buffer 4 samples at a time. */
    *__SIMD32(pDst)++ = __QSUB8(*__SIMD32(pSrcA)++, *__SIMD32(pSrcB)++);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

  while(blkCnt > 0u)
  {
    /* C = A - B */
    /* Subtract and then store the result in the destination buffer. */
    *pDst++ = __SSAT_8(*pSrcA++ - *pSrcB++);

    /* Decrement the loop counter */
    blkCnt--;
  }
#else


  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A - B */
    /* Subtract and then store the result in the destination buffer. */
    *pDst++ = (q7_t) __SSAT_8((q15_t) * pSrcA++ - *pSrcB++);

    /* Decrement the loop counter */
    blkCnt--;
  }

#endif /* #ifndef CSKY_MATH_NO_SIMD */


}

/**
 * @} end of BasicSub group
 */
