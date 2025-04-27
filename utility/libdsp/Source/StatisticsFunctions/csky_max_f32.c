/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_max_f32.c
* Description:  Maixmum value of a floating-point vector
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
* This file comes from arm_max_f32.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_max_f32.c
 * @brief    Maximum value of a floating-point vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupStats
 */

/**
 * @defgroup Max Maximum
 *
 * Computes the maximum value of an array of data.
 * The function returns both the maximum value and its position within the array.
 * There are separate functions for floating-point, Q31, Q15, and Q7 data types.
 */

/**
 * @addtogroup Max
 * @{
 */


/**
 * @brief Maximum value of a floating-point vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult maximum value returned here
 * @param[out]      *pIndex index of maximum value returned here
 * @return none.
 */

void csky_max_f32(
  float32_t * pSrc,
  uint32_t blockSize,
  float32_t * pResult,
  uint32_t * pIndex)
{
#ifndef CSKY_MATH_NO_SIMD

  float32_t maxVal1, maxVal2, out;               /* Temporary variables to store the output value. */
  uint32_t blkCnt, outIndex, count;              /* loop counter */

  /* Initialise the count value. */
  count = 0u;
  /* Initialise the index value to zero. */
  outIndex = 0u;
  /* Load first input value that act as reference value for comparision */
  out = *pSrc++;

  /* Loop unrolling */
  blkCnt = (blockSize - 1u) >> 2u;

  while(blkCnt > 0u)
  {
    /* Initialize maxVal to the next consecutive values one by one */
    maxVal1 = *pSrc++;

    maxVal2 = *pSrc++;

    /* compare for the maximum value */
    if(out < maxVal1)
    {
      /* Update the maximum value and its index */
      out = maxVal1;
      outIndex = count + 1u;
    }

    maxVal1 = *pSrc++;

    /* compare for the maximum value */
    if(out < maxVal2)
    {
      /* Update the maximum value and its index */
      out = maxVal2;
      outIndex = count + 2u;
    }

    maxVal2 = *pSrc++;

    /* compare for the maximum value */
    if(out < maxVal1)
    {
      /* Update the maximum value and its index */
      out = maxVal1;
      outIndex = count + 3u;
    }

    /* compare for the maximum value */
    if(out < maxVal2)
    {
      /* Update the maximum value and its index */
      out = maxVal2;
      outIndex = count + 4u;
    }

    count += 4u;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* if (blockSize - 1u) is not multiple of 4 */
  blkCnt = (blockSize - 1u) % 4u;

#else

  float32_t maxVal1, out;                        /* Temporary variables to store the output value. */
  uint32_t blkCnt, outIndex;                     /* loop counter */

  /* Initialise the index value to zero. */
  outIndex = 0u;
  /* Load first input value that act as reference value for comparision */
  out = *pSrc++;

  blkCnt = (blockSize - 1u);

#endif /* #ifndef CSKY_MATH_NO_SIMD */

  while(blkCnt > 0u)
  {
    /* Initialize maxVal to the next consecutive values one by one */
    maxVal1 = *pSrc++;

    /* compare for the maximum value */
    if(out < maxVal1)
    {
      /* Update the maximum value and it's index */
      out = maxVal1;
      outIndex = blockSize - blkCnt;
    }


    /* Decrement the loop counter */
    blkCnt--;

  }

  /* Store the maximum value and it's index into destination pointers */
  *pResult = out;
  *pIndex = outIndex;
}

/**
 * @} end of Max group
 */
