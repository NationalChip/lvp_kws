/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_fir_interpolate_q15.c
* Description:  Q15 FIR interpolation
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
* This file comes from arm_fir_interpolate_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_fir_interpolate_q15.c
 * @brief    Q15 FIR interpolation.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/


#include "csky_math.h"

/**
 * @ingroup groupFilters
 */

/**
 * @addtogroup FIR_Interpolate
 * @{
 */

/**
 * @brief Processing function for the Q15 FIR interpolator.
 * @param[in] *S        points to an instance of the Q15 FIR interpolator structure.
 * @param[in] *pSrc     points to the block of input data.
 * @param[out] *pDst    points to the block of output data.
 * @param[in] blockSize number of input samples to process per call.
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using a 64-bit internal accumulator.
 * Both coefficients and state variables are represented in 1.15 format and multiplications yield a 2.30 result.
 * The 2.30 intermediate results are accumulated in a 64-bit accumulator in 34.30 format.
 * There is no risk of internal overflow with this approach and the full precision of intermediate multiplications is preserved.
 * After all additions have been performed, the accumulator is truncated to 34.15 format by discarding low 15 bits.
 * Lastly, the accumulator is saturated to yield a result in 1.15 format.
 */

#ifndef CSKY_MATH_NO_SIMD


void csky_fir_interpolate_q15(
  const csky_fir_interpolate_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer                                            */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer                                      */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state                */
  q15_t *ptr1, *ptr2;                            /* Temporary pointers for state and coefficient buffers     */
  q63_t sum0;                                    /* Accumulators                                             */
  q15_t x0, c0;                                  /* Temporary variables to hold state and coefficient values */
  uint32_t i, blkCnt, j, tapCnt;                 /* Loop counters                                            */
  uint16_t phaseLen = S->phaseLength;            /* Length of each polyphase filter component */
  uint32_t blkCntN2;
  q63_t acc0, acc1;
  q15_t x1;

  /* S->pState buffer contains previous frame (phaseLen - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + ((q31_t) phaseLen - 1);

  /* Initialise  blkCnt */
  blkCnt = blockSize / 2;
  blkCntN2 = blockSize - (2 * blkCnt);

  /* Samples loop unrolled by 2 */
  while(blkCnt > 0u)
  {
    /* Copy new input sample into the state buffer */
    *pStateCurnt++ = *pSrc++;
    *pStateCurnt++ = *pSrc++;

    /* Address modifier index of coefficient buffer */
    j = 1u;

    /* Loop over the Interpolation factor. */
    i = (S->L);

    while(i > 0u)
    {
      /* Set accumulator to zero */
      acc0 = 0;
      acc1 = 0;

      /* Initialize state pointer */
      ptr1 = pState;

      /* Initialize coefficient pointer */
      ptr2 = pCoeffs + (S->L - j);

      /* Loop over the polyPhase length. Unroll by a factor of 4.
       ** Repeat until we've computed numTaps-(4*S->L) coefficients. */
      tapCnt = phaseLen >> 2u;

      x0 = *ptr1++;

      while(tapCnt > 0u)
      {

        /* Read the input sample */
        x1 = *ptr1++;

        /* Read the coefficient */
        c0 = *(ptr2);

        /* Perform the multiply-accumulate */
        acc0 += (q63_t) (x0 *c0);
        acc1 += (q63_t) (x1 *c0);


        /* Read the coefficient */
        c0 = *(ptr2 + S->L);

        /* Read the input sample */
        x0 = *ptr1++;

        /* Perform the multiply-accumulate */
        acc0 += (q63_t) (x1 *c0);
        acc1 += (q63_t) (x0 *c0);


        /* Read the coefficient */
        c0 = *(ptr2 + S->L * 2);

        /* Read the input sample */
        x1 = *ptr1++;

        /* Perform the multiply-accumulate */
        acc0 += (q63_t) (x0 *c0);
        acc1 += (q63_t) (x1 *c0);

        /* Read the coefficient */
        c0 = *(ptr2 + S->L * 3);

        /* Read the input sample */
        x0 = *ptr1++;

        /* Perform the multiply-accumulate */
        acc0 += (q63_t) (x1 *c0);
        acc1 += (q63_t) (x0 *c0);

        /* Upsampling is done by stuffing L-1 zeros between each sample.
         * So instead of multiplying zeros with coefficients,
         * Increment the coefficient pointer by interpolation factor times. */
        ptr2 += 4 * S->L;

        /* Decrement the loop counter */
        tapCnt--;
      }

      /* If the polyPhase length is not a multiple of 4, compute the remaining filter taps */
      tapCnt = phaseLen % 0x4u;

      while(tapCnt > 0u)
      {

        /* Read the input sample */
        x1 = *ptr1++;

        /* Read the coefficient */
        c0 = *(ptr2);

        /* Perform the multiply-accumulate */
        acc0 += (q63_t) (x0 *c0);
        acc1 += (q63_t) (x1 *c0);

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* update states for next sample processing */
        x0 = x1;

        /* Decrement the loop counter */
        tapCnt--;
      }

      /* The result is in the accumulator, store in the destination buffer. */
      *pDst = (q15_t) (__SSAT_16((acc0 >> 15)));
      *(pDst + S->L) = (q15_t) (__SSAT_16((acc1 >> 15)));

      pDst++;

      /* Increment the address modifier index of coefficient buffer */
      j++;

      /* Decrement the loop counter */
      i--;
    }

    /* Advance the state pointer by 1
     * to process the next group of interpolation factor number samples */
    pState = pState + 2;

    pDst += S->L;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 2, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blkCntN2;

  /* Loop over the blockSize. */
  while(blkCnt > 0u)
  {
    /* Copy new input sample into the state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Address modifier index of coefficient buffer */
    j = 1u;

    /* Loop over the Interpolation factor. */
    i = S->L;
    while(i > 0u)
    {
      /* Set accumulator to zero */
      sum0 = 0;

      /* Initialize state pointer */
      ptr1 = pState;

      /* Initialize coefficient pointer */
      ptr2 = pCoeffs + (S->L - j);

      /* Loop over the polyPhase length. Unroll by a factor of 4.
       ** Repeat until we've computed numTaps-(4*S->L) coefficients. */
      tapCnt = phaseLen >> 2;
      while(tapCnt > 0u)
      {

        /* Read the coefficient */
        c0 = *(ptr2);

        /* Upsampling is done by stuffing L-1 zeros between each sample.
         * So instead of multiplying zeros with coefficients,
         * Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *ptr1++;

        /* Perform the multiply-accumulate */
        sum0 += (q63_t) (x0 *c0);

        /* Read the coefficient */
        c0 = *(ptr2);

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *ptr1++;

        /* Perform the multiply-accumulate */
        sum0 += (q63_t) (x0 *c0);

        /* Read the coefficient */
        c0 = *(ptr2);

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *ptr1++;

        /* Perform the multiply-accumulate */
        sum0 += (q63_t) (x0 *c0);

        /* Read the coefficient */
        c0 = *(ptr2);

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *ptr1++;

        /* Perform the multiply-accumulate */
        sum0 += (q63_t) (x0 *c0);

        /* Decrement the loop counter */
        tapCnt--;
      }

      /* If the polyPhase length is not a multiple of 4, compute the remaining filter taps */
      tapCnt = phaseLen & 0x3u;

      while(tapCnt > 0u)
      {
        /* Read the coefficient */
        c0 = *(ptr2);

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *ptr1++;

        /* Perform the multiply-accumulate */
        sum0 += (q63_t) (x0 *c0);

        /* Decrement the loop counter */
        tapCnt--;
      }

      /* The result is in the accumulator, store in the destination buffer. */
      *pDst++ = (q15_t) (__SSAT_16((sum0 >> 15)));

      j++;

      /* Decrement the loop counter */
      i--;
    }

    /* Advance the state pointer by 1
     * to process the next group of interpolation factor number samples */
    pState = pState + 1;

    /* Decrement the loop counter */
    blkCnt--;
  }


  /* Processing is complete.
   ** Now copy the last phaseLen - 1 samples to the satrt of the state buffer.
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  i = ((uint32_t) phaseLen - 1u) >> 2u;

  /* copy data */
  while(i > 0u)
  {
#ifndef UNALIGNED_SUPPORT_DISABLE

    *__SIMD32(pStateCurnt)++ = *__SIMD32(pState)++;
    *__SIMD32(pStateCurnt)++ = *__SIMD32(pState)++;

#else

    *pStateCurnt++ = *pState++;
	*pStateCurnt++ = *pState++;
	*pStateCurnt++ = *pState++;
	*pStateCurnt++ = *pState++;

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE	*/

	/* Decrement the loop counter */
    i--;
  }

  i = ((uint32_t) phaseLen - 1u) % 0x04u;

  while(i > 0u)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    i--;
  }
}

#else


void csky_fir_interpolate_q15(
  const csky_fir_interpolate_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer                                            */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer                                      */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state                */
  q15_t *ptr1, *ptr2;                            /* Temporary pointers for state and coefficient buffers     */
  q63_t sum;                                     /* Accumulator */
  q15_t x0, c0;                                  /* Temporary variables to hold state and coefficient values */
  uint32_t i, blkCnt, tapCnt;                    /* Loop counters                                            */
  uint16_t phaseLen = S->phaseLength;            /* Length of each polyphase filter component */


  /* S->pState buffer contains previous frame (phaseLen - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + (phaseLen - 1u);

  /* Total number of intput samples */
  blkCnt = blockSize;

  /* Loop over the blockSize. */
  while(blkCnt > 0u)
  {
    /* Copy new input sample into the state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Loop over the Interpolation factor. */
    i = S->L;

    while(i > 0u)
    {
      /* Set accumulator to zero */
      sum = 0;

      /* Initialize state pointer */
      ptr1 = pState;

      /* Initialize coefficient pointer */
      ptr2 = pCoeffs + (i - 1u);

      /* Loop over the polyPhase length */
      tapCnt = (uint32_t) phaseLen;

      while(tapCnt > 0u)
      {
        /* Read the coefficient */
        c0 = *ptr2;

        /* Increment the coefficient pointer by interpolation factor times. */
        ptr2 += S->L;

        /* Read the input sample */
        x0 = *ptr1++;

        /* Perform the multiply-accumulate */
        sum += ((q31_t) x0 * c0);

        /* Decrement the loop counter */
        tapCnt--;
      }

      /* Store the result after converting to 1.15 format in the destination buffer */
      *pDst++ = (q15_t) (__SSAT_16((sum >> 15)));

      /* Decrement the loop counter */
      i--;
    }

    /* Advance the state pointer by 1
     * to process the next group of interpolation factor number samples */
    pState = pState + 1;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Processing is complete.
   ** Now copy the last phaseLen - 1 samples to the start of the state buffer.
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  i = (uint32_t) phaseLen - 1u;

  while(i > 0u)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    i--;
  }

}

#endif /*   #ifndef CSKY_MATH_NO_SIMD */


 /**
  * @} end of FIR_Interpolate group
  */
