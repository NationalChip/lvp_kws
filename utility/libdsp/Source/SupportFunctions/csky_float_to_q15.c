/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_float_to_q15.c
* Description:  Converts the elements of the floating-point vector to Q15 vector
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
* This file comes from arm_float_to_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_float_to_q15.c
 * @brief    Converts the elements of the floating-point vector to Q15 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupSupport
 */

/**
 * @addtogroup float_to_x
 * @{
 */

/**
 * @brief Converts the elements of the floating-point vector to Q15 vector.
 * @param[in]       *pSrc points to the floating-point input vector
 * @param[out]      *pDst points to the Q15 output vector
 * @param[in]       blockSize length of the input vector
 * @return none.
 *
 * \par Description:
 * \par
 * The equation used for the conversion process is:
 * <pre>
 * 	pDst[n] = (q15_t)(pSrc[n] * 32768);   0 <= n < blockSize.
 * </pre>
 * \par Scaling and Overflow Behavior:
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q15 range [0x8000 0x7FFF] will be saturated.
 * \note
 * In order to apply rounding, the library should be rebuilt with the ROUNDING macro
 * defined in the preprocessor section of project options.
 *
 */


void csky_float_to_q15(
  float32_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  float32_t *pIn = pSrc;                         /* Src pointer */
  uint32_t blkCnt;                               /* loop counter */

#ifdef CSKY_MATH_ROUNDING

  float32_t in;

#endif /*      #ifdef CSKY_MATH_ROUNDING        */

#ifndef CSKY_MATH_NO_SIMD


  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {

#ifdef CSKY_MATH_ROUNDING
    /* C = A * 32768 */
    /* convert from float to q15 and then store the results in the destination buffer */
    in = *pIn++;
    in = (in * 32768.0f);
    in += in > 0.0f ? 0.5f : -0.5f;
    *pDst++ = (q15_t) (__SSAT_16((q31_t) (in)));

    in = *pIn++;
    in = (in * 32768.0f);
    in += in > 0.0f ? 0.5f : -0.5f;
    *pDst++ = (q15_t) (__SSAT_16((q31_t) (in)));

    in = *pIn++;
    in = (in * 32768.0f);
    in += in > 0.0f ? 0.5f : -0.5f;
    *pDst++ = (q15_t) (__SSAT_16((q31_t) (in)));

    in = *pIn++;
    in = (in * 32768.0f);
    in += in > 0.0f ? 0.5f : -0.5f;
    *pDst++ = (q15_t) (__SSAT_16((q31_t) (in)));

#else

    /* C = A * 32768 */
    /* convert from float to q15 and then store the results in the destination buffer */
    *pDst++ = (q15_t) __SSAT_16((q31_t) (*pIn++ * 32768.0f));
    *pDst++ = (q15_t) __SSAT_16((q31_t) (*pIn++ * 32768.0f));
    *pDst++ = (q15_t) __SSAT_16((q31_t) (*pIn++ * 32768.0f));
    *pDst++ = (q15_t) __SSAT_16((q31_t) (*pIn++ * 32768.0f));

#endif /*      #ifdef CSKY_MATH_ROUNDING        */

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

  while(blkCnt > 0u)
  {

#ifdef CSKY_MATH_ROUNDING
    /* C = A * 32768 */
    /* convert from float to q15 and then store the results in the destination buffer */
    in = *pIn++;
    in = (in * 32768.0f);
    in += in > 0.0f ? 0.5f : -0.5f;
    *pDst++ = (q15_t) (__SSAT_16((q31_t) (in)));

#else

    /* C = A * 32768 */
    /* convert from float to q15 and then store the results in the destination buffer */
    *pDst++ = (q15_t) __SSAT_16((q31_t) (*pIn++ * 32768.0f));

#endif /*      #ifdef CSKY_MATH_ROUNDING        */

    /* Decrement the loop counter */
    blkCnt--;
  }


#else


  /* Loop over blockSize number of values */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {

#ifdef CSKY_MATH_ROUNDING
    /* C = A * 32768 */
    /* convert from float to q15 and then store the results in the destination buffer */
    in = *pIn++;
    in = (in * 32768.0f);
    in += in > 0 ? 0.5f : -0.5f;
    *pDst++ = (q15_t) (__SSAT_16((q31_t) (in)));

#else

    /* C = A * 32768 */
    /* convert from float to q15 and then store the results in the destination buffer */
    *pDst++ = (q15_t) __SSAT_16((q31_t) (*pIn++ * 32768.0f));

#endif /*      #ifdef CSKY_MATH_ROUNDING        */

    /* Decrement the loop counter */
    blkCnt--;
  }

#endif /* #ifndef CSKY_MATH_NO_SIMD */

}

/**
 * @} end of float_to_x group
 */
