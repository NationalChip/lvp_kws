/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_scale_q7.c
* Description:  Multiplies a Q7 vector by a scalar
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
* This file comes from arm_scale_q7.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_scale_q7.c
 * @brief    Multiplies a Q7 vector by a scalar.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupMath
 */

/**
 * @addtogroup scale
 * @{
 */

/**
 * @brief Multiplies a Q7 vector by a scalar.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       scaleFract fractional portion of the scale value
 * @param[in]       shift number of bits to shift the result by
 * @param[out]      *pDst points to the output vector
 * @param[in]       blockSize number of samples in the vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The input data <code>*pSrc</code> and <code>scaleFract</code> are in 1.7 format.
 * These are multiplied to yield a 2.14 intermediate result and this is shifted with saturation to 1.7 format.
 */

void csky_scale_q7(
  q7_t * pSrc,
  q7_t scaleFract,
  int8_t shift,
  q7_t * pDst,
  uint32_t blockSize)
{
  int8_t kShift = 7 - shift;                     /* shift to apply after scaling */
  uint32_t blkCnt;                               /* loop counter */

#ifndef CSKY_MATH_NO_SIMD

  q7_t in1, in2, in3, in4, out1, out2, out3, out4;      /* Temporary variables to store input & output */


  /*loop Unrolling */
  blkCnt = blockSize >> 2u;


  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* Reading 4 inputs from memory */
    in1 = *pSrc++;
    in2 = *pSrc++;
    in3 = *pSrc++;
    in4 = *pSrc++;

    /* C = A * scale */
    /* Scale the inputs and then store the results in the temporary variables. */
    out1 = (q7_t) (__SSAT_8(((in1) * scaleFract) >> kShift));
    out2 = (q7_t) (__SSAT_8(((in2) * scaleFract) >> kShift));
    out3 = (q7_t) (__SSAT_8(((in3) * scaleFract) >> kShift));
    out4 = (q7_t) (__SSAT_8(((in4) * scaleFract) >> kShift));

    /* Packing the individual outputs into 32bit and storing in
     * destination buffer in single write */
    *__SIMD32(pDst)++ = __PACKq7(out1, out2, out3, out4);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

  while(blkCnt > 0u)
  {
    /* C = A * scale */
    /* Scale the input and then store the result in the destination buffer. */
    *pDst++ = (q7_t) (__SSAT_8(((*pSrc++) * scaleFract) >> kShift));

    /* Decrement the loop counter */
    blkCnt--;
  }

#else


  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A * scale */
    /* Scale the input and then store the result in the destination buffer. */
    *pDst++ = (q7_t) (__SSAT((((q15_t) * pSrc++ * scaleFract) >> kShift), 8));

    /* Decrement the loop counter */
    blkCnt--;
  }

#endif /* #ifndef CSKY_MATH_NO_SIMD */

}

/**
 * @} end of scale group
 */
