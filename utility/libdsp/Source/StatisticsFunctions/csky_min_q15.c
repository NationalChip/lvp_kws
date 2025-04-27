/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_min_q15.c
* Description:  Minimum value of a Q15 vector
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
* This file comes from arm_min_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_min_q15.c
 * @brief    Minimum value of a Q15 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupStats
 */


/**
 * @addtogroup Min
 * @{
 */


/**
 * @brief Minimum value of a Q15 vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult minimum value returned here
 * @param[out]      *pIndex index of minimum value returned here
 * @return none.
 *
 */

void csky_min_q15(
  q15_t * pSrc,
  uint32_t blockSize,
  q15_t * pResult,
  uint32_t * pIndex)
{
#ifndef CSKY_MATH_NO_SIMD

  q15_t minVal1, minVal2, out;                   /* Temporary variables to store the output value. */
  uint32_t blkCnt, outIndex, count;              /* loop counter */

  /* Initialise the count value. */
  count = 0u;
  /* Initialise the index value to zero. */
  outIndex = 0u;
  /* Load first input value that act as reference value for comparision */
  out = *pSrc++;

  /* Loop unrolling */
  blkCnt = (blockSize - 1u) >> 2u;

  while(blkCnt > 0)
  {
    /* Initialize minVal to the next consecutive values one by one */
    minVal1 = *pSrc++;
    minVal2 = *pSrc++;

    /* compare for the minimum value */
    if(out > minVal1)
    {
      /* Update the minimum value and its index */
      out = minVal1;
      outIndex = count + 1u;
    }

    minVal1 = *pSrc++;

    /* compare for the minimum value */
    if(out > minVal2)
    {
      /* Update the minimum value and its index */
      out = minVal2;
      outIndex = count + 2u;
    }

    minVal2 = *pSrc++;

    /* compare for the minimum value */
    if(out > minVal1)
    {
      /* Update the minimum value and its index */
      out = minVal1;
      outIndex = count + 3u;
    }

    /* compare for the minimum value */
    if(out > minVal2)
    {
      /* Update the minimum value and its index */
      out = minVal2;
      outIndex = count + 4u;
    }

    count += 4u;

    blkCnt--;
  }

  /* if (blockSize - 1u ) is not multiple of 4 */
  blkCnt = (blockSize - 1u) % 4u;

#else

  q15_t minVal1, out;                            /* Temporary variables to store the output value. */
  uint32_t blkCnt, outIndex;                     /* loop counter */

  blkCnt = (blockSize - 1u);

  /* Initialise the index value to zero. */
  outIndex = 0u;
  /* Load first input value that act as reference value for comparision */
  out = *pSrc++;

#endif //      #ifndef CSKY_MATH_NO_SIMD

  while(blkCnt > 0)
  {
    /* Initialize minVal to the next consecutive values one by one */
    minVal1 = *pSrc++;

    /* compare for the minimum value */
    if(out > minVal1)
    {
      /* Update the minimum value and it's index */
      out = minVal1;
      outIndex = blockSize - blkCnt;
    }

    blkCnt--;

  }



  /* Store the minimum value and its index into destination pointers */
  *pResult = out;
  *pIndex = outIndex;
}

/**
 * @} end of Min group
 */
