/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_add_f32.c
* Description:  Floating-point vector addition
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
* This file comes from arm_add_f32.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_add_f32.c
 * @brief    Floating-point vector addition.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/
#include "csky_math.h"

/**
 * @ingroup groupMath
 */

/**
 * @defgroup BasicAdd Vector Addition
 *
 * Element-by-element addition of two vectors.
 *
 * <pre>
 *     pDst[n] = pSrcA[n] + pSrcB[n],   0 <= n < blockSize.
 * </pre>
 *
 * There are separate functions for floating-point, Q7, Q15, and Q31 data types.
 */

/**
 * @addtogroup BasicAdd
 * @{
 */

/**
 * @brief Floating-point vector addition.
 * @param[in]       *pSrcA points to the first input vector
 * @param[in]       *pSrcB points to the second input vector
 * @param[out]      *pDst points to the output vector
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 */

void csky_add_f32(
  float32_t * pSrcA,
  float32_t * pSrcB,
  float32_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

#ifndef CSKY_MATH_NO_SIMD

  float32_t inA1, inA2, inA3, inA4;              /* temporary input variabels */
  float32_t inB1, inB2, inB3, inB4;              /* temporary input variables */

  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = A + B */
    /* Add and then store the results in the destination buffer. */

    /* read four inputs from sourceA and four inputs from sourceB */
    inA1 = *pSrcA;
    inB1 = *pSrcB;
    inA2 = *(pSrcA + 1);
    inB2 = *(pSrcB + 1);
    inA3 = *(pSrcA + 2);
    inB3 = *(pSrcB + 2);
    inA4 = *(pSrcA + 3);
    inB4 = *(pSrcB + 3);

    /* C = A + B */
    /* add and store result to destination */
    *pDst = inA1 + inB1;
    *(pDst + 1) = inA2 + inB2;
    *(pDst + 2) = inA3 + inB3;
    *(pDst + 3) = inA4 + inB4;

    /* update pointers to process next samples */
    pSrcA += 4u;
    pSrcB += 4u;
    pDst += 4u;


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
    /* C = A + B */
    /* Add and then store the results in the destination buffer. */
    *pDst++ = (*pSrcA++) + (*pSrcB++);

    /* Decrement the loop counter */
    blkCnt--;
  }
}

/**
 * @} end of BasicAdd group
 */
