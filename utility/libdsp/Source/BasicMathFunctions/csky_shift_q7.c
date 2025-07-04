/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_shift_q7.c
* Description:  Processing function for the Q7 Shifting
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
* This file comes from arm_shift_q7.c. It is renamed  by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_shift_q7.c
 * @brief    Shifts the elements of a Q7 vector by a specified number of bits.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupMath
 */

/**
 * @addtogroup shift
 * @{
 */


/**
 * @brief  Shifts the elements of a Q7 vector a specified number of bits.
 * @param[in]  *pSrc points to the input vector
 * @param[in]  shiftBits number of bits to shift.  A positive value shifts left; a negative value shifts right.
 * @param[out]  *pDst points to the output vector
 * @param[in]  blockSize number of samples in the vector
 * @return none.
 *
 * \par Conditions for optimum performance
 *  Input and output buffers should be aligned by 32-bit
 *
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q7 range [0x80 0x7F] will be saturated.
 */

void csky_shift_q7(
  q7_t * pSrc,
  int8_t shiftBits,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  uint8_t sign;                                  /* Sign of shiftBits */

#ifndef CSKY_MATH_NO_SIMD

  q7_t in1;                                      /* Input value1 */
  q7_t in2;                                      /* Input value2 */
  q7_t in3;                                      /* Input value3 */
  q7_t in4;                                      /* Input value4 */


  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* Getting the sign of shiftBits */
  sign = (shiftBits & 0x80);

  /* If the shift value is positive then do right shift else left shift */
  if(sign == 0u)
  {
    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while(blkCnt > 0u)
    {
      /* C = A << shiftBits */
      /* Read 4 inputs */
      in1 = *pSrc;
      in2 = *(pSrc + 1);
      in3 = *(pSrc + 2);
      in4 = *(pSrc + 3);

      /* Store the Shifted result in the destination buffer in single cycle by packing the outputs */
      *__SIMD32(pDst)++ = __PACKq7(__SSAT_8((in1 << shiftBits)),
                                   __SSAT_8((in2 << shiftBits)),
                                   __SSAT_8((in3 << shiftBits)),
                                   __SSAT_8((in4 << shiftBits)));
      /* Update source pointer to process next sampels */
      pSrc += 4u;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = blockSize % 0x4u;

    while(blkCnt > 0u)
    {
      /* C = A << shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      *pDst++ = (q7_t) __SSAT_8((*pSrc++ << shiftBits));

      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  else
  {
    shiftBits = -shiftBits;
    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while(blkCnt > 0u)
    {
      /* C = A >> shiftBits */
      /* Read 4 inputs */
      in1 = *pSrc;
      in2 = *(pSrc + 1);
      in3 = *(pSrc + 2);
      in4 = *(pSrc + 3);

      /* Store the Shifted result in the destination buffer in single cycle by packing the outputs */
      *__SIMD32(pDst)++ = __PACKq7((in1 >> shiftBits), (in2 >> shiftBits),
                                   (in3 >> shiftBits), (in4 >> shiftBits));


      pSrc += 4u;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = blockSize % 0x4u;

    while(blkCnt > 0u)
    {
      /* C = A >> shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      in1 = *pSrc++;
      *pDst++ = (in1 >> shiftBits);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }

#else


  /* Getting the sign of shiftBits */
  sign = (shiftBits & 0x80);

  /* If the shift value is positive then do right shift else left shift */
  if(sign == 0u)
  {
    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;

    while(blkCnt > 0u)
    {
      /* C = A << shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      *pDst++ = (q7_t) __SSAT_8(((q15_t) * pSrc++ << shiftBits));

      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  else
  {
    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;

    while(blkCnt > 0u)
    {
      /* C = A >> shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      *pDst++ = (*pSrc++ >> -shiftBits);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }

#endif /* #ifndef CSKY_MATH_NO_SIMD */
}

/**
 * @} end of shift group
 */
