/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_q7_to_q31.c
* Description:  Converts the elements of the Q7 vector to Q31 vector
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
* This file comes from arm_q7_to_q31.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_q7_to_q31.c
 * @brief    Converts the elements of the Q7 vector to Q31 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupSupport
 */

/**
 * @addtogroup q7_to_x
 * @{
 */

/**
 * @brief Converts the elements of the Q7 vector to Q31 vector.
 * @param[in]       *pSrc points to the Q7 input vector
 * @param[out]      *pDst points to the Q31 output vector
 * @param[in]       blockSize length of the input vector
 * @return none.
 *
 * \par Description:
 *
 * The equation used for the conversion process is:
 *
 * <pre>
 * 	pDst[n] = (q31_t) pSrc[n] << 24;   0 <= n < blockSize.
 * </pre>
 *
 */


void csky_q7_to_q31(
  q7_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q7_t *pIn = pSrc;                              /* Src pointer */
  uint32_t blkCnt;                               /* loop counter */

#ifndef CSKY_MATH_NO_SIMD

  q31_t in;


  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = (q31_t) A << 24 */
    /* convert from q7 to q31 and then store the results in the destination buffer */
    in = *__SIMD32(pIn)++;

#ifndef CSKY_MATH_BIG_ENDIAN

    *pDst++ = (__ROR(in, 8)) & 0xFF000000;
    *pDst++ = (__ROR(in, 16)) & 0xFF000000;
    *pDst++ = (__ROR(in, 24)) & 0xFF000000;
    *pDst++ = (in & 0xFF000000);

#else

    *pDst++ = (in & 0xFF000000);
    *pDst++ = (__ROR(in, 24)) & 0xFF000000;
    *pDst++ = (__ROR(in, 16)) & 0xFF000000;
    *pDst++ = (__ROR(in, 8)) & 0xFF000000;

#endif //              #ifndef CSKY_MATH_BIG_ENDIAN

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

#else


  /* Loop over blockSize number of values */
  blkCnt = blockSize;

#endif /* #ifndef CSKY_MATH_NO_SIMD */

  while(blkCnt > 0u)
  {
    /* C = (q31_t) A << 24 */
    /* convert from q7 to q31 and then store the results in the destination buffer */
    *pDst++ = (q31_t) * pIn++ << 24;

    /* Decrement the loop counter */
    blkCnt--;
  }

}

/**
 * @} end of q7_to_x group
 */
