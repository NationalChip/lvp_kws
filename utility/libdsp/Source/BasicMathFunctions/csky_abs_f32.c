/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_abs_f32.c
* Description:  Floating-point vector absoulte value
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
* This file comes from arm_abs_f32.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_abs_f32.c
 * @brief    Vector absolute value.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"
#include <math.h>

/**
 * @ingroup groupMath
 */

/**
 * @defgroup BasicAbs Vector Absolute Value
 *
 * Computes the absolute value of a vector on an element-by-element basis.
 *
 * <pre>
 *     pDst[n] = abs(pSrc[n]),   0 <= n < blockSize.
 * </pre>
 *
 * The functions support in-place computation allowing the source and
 * destination pointers to reference the same memory buffer.
 * There are separate functions for floating-point, Q7, Q15, and Q31 data types.
 */

/**
 * @addtogroup BasicAbs
 * @{
 */

/**
 * @brief Floating-point vector absolute value.
 * @param[in]       *pSrc points to the input buffer
 * @param[out]      *pDst points to the output buffer
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 */

void csky_abs_f32(
  float32_t * pSrc,
  float32_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

#ifndef CSKY_MATH_NO_SIMD

  float32_t in1, in2, in3, in4;                  /* temporary variables */

  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = |A| */
    /* Calculate absolute and then store the results in the destination buffer. */
    /* read sample from source */
    in1 = *pSrc;
    in2 = *(pSrc + 1);
    in3 = *(pSrc + 2);

    /* find absolute value */
    in1 = fabs((double)in1);

    /* read sample from source */
    in4 = *(pSrc + 3);

    /* find absolute value */
    in2 = fabs((double)in2);

    /* read sample from source */
    *pDst = in1;

    /* find absolute value */
    in3 = fabs((double)in3);

    /* find absolute value */
    in4 = fabs((double)in4);

    /* store result to destination */
    *(pDst + 1) = in2;

    /* store result to destination */
    *(pDst + 2) = in3;

    /* store result to destination */
    *(pDst + 3) = in4;


    /* Update source pointer to process next sampels */
    pSrc += 4u;

    /* Update destination pointer to process next sampels */
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

#endif /*   #ifndef CSKY_MATH_NO_SIMD   */

  while(blkCnt > 0u)
  {
    /* C = |A| */
    /* Calculate absolute and then store the results in the destination buffer. */
    *pDst++ = fabs((double)(*pSrc++));

    /* Decrement the loop counter */
    blkCnt--;
  }
}

/**
 * @} end of BasicAbs group
 */
