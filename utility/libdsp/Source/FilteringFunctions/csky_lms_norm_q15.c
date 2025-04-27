/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_lms_norm_q15.c
* Description:  Processing function for Q15 normalized LMS filter
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
* This file comes from arm_lms_norm_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_lms_norm_q15.c
 * @brief    Q15 NLMS filter.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupFilters
 */

/**
 * @brief Function to Calculates 1/in (reciprocal) value of Q15 Data type.
 */
static uint32_t csky_recip_q15(
q15_t in,
q15_t * dst,
q15_t * pRecipTable)
{
  q15_t out = 0;
  uint32_t tempVal = 0;
  uint32_t index = 0, i = 0;
  uint32_t signBits = 0;
  if(in > 0)
  {
    signBits = ((uint32_t)(__CLZ( in) - 17));
  }
  else
  {
    signBits = ((uint32_t)(__CLZ(-in) - 17));
  }
  /* Convert input sample to 1.15 format */
  in = (in << signBits);
  /* calculation of index for initial approximated Val */
  index = (uint32_t)(in >>  8);
  index = (index & INDEX_MASK);
  /*      1.15 with exp 1  */
  out = pRecipTable[index];
  /* calculation of reciprocal value */
  /* running approximation for two iterations */
  for (i = 0u; i < 2u; i++)
  {
    tempVal = (uint32_t) (((q31_t) in * out) >> 15);
    tempVal = 0x7FFFu - tempVal;
    /*      1.15 with exp 1 */
    out = (q15_t) (((q31_t) out * tempVal) >> 14);
  }
  /* write output */
  *dst = out;
  /* return num of signbits of out = 1/in value */
  return (signBits + 1);
}

/**
 * @addtogroup LMS_NORM
 * @{
 */

/**
* @brief Processing function for Q15 normalized LMS filter.
* @param[in] *S points to an instance of the Q15 normalized LMS filter structure.
* @param[in] *pSrc points to the block of input data.
* @param[in] *pRef points to the block of reference data.
* @param[out] *pOut points to the block of output data.
* @param[out] *pErr points to the block of error data.
* @param[in] blockSize number of samples to process.
* @return none.
*
* <b>Scaling and Overflow Behavior:</b>
* \par
* The function is implemented using a 64-bit internal accumulator.
* Both coefficients and state variables are represented in 1.15 format and
* multiplications yield a 2.30 result. The 2.30 intermediate results are
* accumulated in a 64-bit accumulator in 34.30 format.
* There is no risk of internal overflow with this approach and the full
* precision of intermediate multiplications is preserved. After all additions
* have been performed, the accumulator is truncated to 34.15 format by
* discarding low 15 bits. Lastly, the accumulator is saturated to yield a
* result in 1.15 format.
*
* \par
* 	In this filter, filter coefficients are updated for each sample and the updation of filter cofficients are saturted.
*
 */

void csky_lms_norm_q15(
  csky_lms_norm_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pRef,
  q15_t * pOut,
  q15_t * pErr,
  uint32_t blockSize)
{
  q15_t *pState = S->pState;                     /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *pStateCurnt;                            /* Points to the current sample of the state */
  q15_t *px, *pb;                                /* Temporary pointers for state and coefficient buffers */
  q15_t mu = S->mu;                              /* Adaptive factor */
  uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
  uint32_t tapCnt, blkCnt;                       /* Loop counters */
  q31_t energy;                                  /* Energy of the input */
  q63_t acc;                                     /* Accumulator */
  q15_t e = 0, d = 0;                            /* error, reference data sample */
  q15_t w = 0, in;                               /* weight factor and state */
  q15_t x0;                                      /* temporary variable to hold input sample */
  //uint32_t shift = (uint32_t) S->postShift + 1u; /* Shift to be applied to the output */
  q15_t errorXmu, oneByEnergy;                   /* Temporary variables to store error and mu product and reciprocal of energy */
  q15_t postShift;                               /* Post shift to be applied to weight after reciprocal calculation */
  q31_t coef;                                    /* Teporary variable for coefficient */
  q31_t acc_l, acc_h;
  int32_t lShift = (15 - (int32_t) S->postShift);       /*  Post shift  */
  int32_t uShift = (32 - lShift);

  energy = S->energy;
  x0 = S->x0;

  /* S->pState points to buffer which contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = &(S->pState[(numTaps - 1u)]);

  /* Loop over blockSize number of values */
  blkCnt = blockSize;


#ifndef CSKY_MATH_NO_SIMD


  while(blkCnt > 0u)
  {
    /* Copy the new input sample into the state buffer */
    *pStateCurnt++ = *pSrc;

    /* Initialize pState pointer */
    px = pState;

    /* Initialize coeff pointer */
    pb = (pCoeffs);

    /* Read the sample from input buffer */
    in = *pSrc++;

    /* Update the energy calculation */
    energy -= (((q31_t) x0 * (x0)) >> 15);
    energy += (((q31_t) in * (in)) >> 15);

    /* Set the accumulator to zero */
    acc = 0;

    /* Loop unrolling.  Process 4 taps at a time. */
    tapCnt = numTaps >> 2;

    while(tapCnt > 0u)
    {

      /* Perform the multiply-accumulate */
#ifndef UNALIGNED_SUPPORT_DISABLE

      acc = __SMLALD(*__SIMD32(px)++, (*__SIMD32(pb)++), acc);
      acc = __SMLALD(*__SIMD32(px)++, (*__SIMD32(pb)++), acc);

#else

      acc += (((q31_t) * px++ * (*pb++)));
      acc += (((q31_t) * px++ * (*pb++)));
      acc += (((q31_t) * px++ * (*pb++)));
      acc += (((q31_t) * px++ * (*pb++)));

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE	*/

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* If the filter length is not a multiple of 4, compute the remaining filter taps */
    tapCnt = numTaps % 0x4u;

    while(tapCnt > 0u)
    {
      /* Perform the multiply-accumulate */
      acc += (((q31_t) * px++ * (*pb++)));

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Calc lower part of acc */
    acc_l = acc & 0xffffffff;

    /* Calc upper part of acc */
    acc_h = (acc >> 32) & 0xffffffff;

    /* Apply shift for lower part of acc and upper part of acc */
    acc = (uint32_t) acc_l >> lShift | acc_h << uShift;

    /* Converting the result to 1.15 format and saturate the output */
    acc = __SSAT_16(acc);

    /* Store the result from accumulator into the destination buffer. */
    *pOut++ = (q15_t) acc;

    /* Compute and store error */
    d = *pRef++;
    e = d - (q15_t) acc;
    *pErr++ = e;

    /* Calculation of 1/energy */
    postShift = csky_recip_q15((q15_t) energy + DELTA_Q15,
                              &oneByEnergy, S->recipTable);

    /* Calculation of e * mu value */
    errorXmu = (q15_t) (((q31_t) e * mu) >> 15);

    /* Calculation of (e * mu) * (1/energy) value */
    acc = (((q31_t) errorXmu * oneByEnergy) >> (15 - postShift));

    /* Weighting factor for the normalized version */
    w = (q15_t) __SSAT_16((q31_t) acc);

    /* Initialize pState pointer */
    px = pState;

    /* Initialize coeff pointer */
    pb = (pCoeffs);

    /* Loop unrolling.  Process 4 taps at a time. */
    tapCnt = numTaps >> 2;

    /* Update filter coefficients */
    while(tapCnt > 0u)
    {
      coef = *pb + (((q31_t) w * (*px++)) >> 15);
      *pb++ = (q15_t) __SSAT_16((coef));
      coef = *pb + (((q31_t) w * (*px++)) >> 15);
      *pb++ = (q15_t) __SSAT_16((coef));
      coef = *pb + (((q31_t) w * (*px++)) >> 15);
      *pb++ = (q15_t) __SSAT_16((coef));
      coef = *pb + (((q31_t) w * (*px++)) >> 15);
      *pb++ = (q15_t) __SSAT_16((coef));

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* If the filter length is not a multiple of 4, compute the remaining filter taps */
    tapCnt = numTaps % 0x4u;

    while(tapCnt > 0u)
    {
      /* Perform the multiply-accumulate */
      coef = *pb + (((q31_t) w * (*px++)) >> 15);
      *pb++ = (q15_t) __SSAT_16((coef));

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Read the sample from state buffer */
    x0 = *pState;

    /* Advance state pointer by 1 for the next sample */
    pState = pState + 1u;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Save energy and x0 values for the next frame */
  S->energy = (q15_t) energy;
  S->x0 = x0;

  /* Processing is complete. Now copy the last numTaps - 1 samples to the
     satrt of the state buffer. This prepares the state buffer for the
     next function call. */

  /* Points to the start of the pState buffer */
  pStateCurnt = S->pState;

  /* Calculation of count for copying integer writes */
  tapCnt = (numTaps - 1u) >> 2;

  while(tapCnt > 0u)
  {

#ifndef UNALIGNED_SUPPORT_DISABLE

    *__SIMD32(pStateCurnt)++ = *__SIMD32(pState)++;
    *__SIMD32(pStateCurnt)++ = *__SIMD32(pState)++;

#else

    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;

#endif

    tapCnt--;

  }

  /* Calculation of count for remaining q15_t data */
  tapCnt = (numTaps - 1u) % 0x4u;

  /* copy data */
  while(tapCnt > 0u)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }

#else


  while(blkCnt > 0u)
  {
    /* Copy the new input sample into the state buffer */
    *pStateCurnt++ = *pSrc;

    /* Initialize pState pointer */
    px = pState;

    /* Initialize pCoeffs pointer */
    pb = pCoeffs;

    /* Read the sample from input buffer */
    in = *pSrc++;

    /* Update the energy calculation */
    energy -= (((q31_t) x0 * (x0)) >> 15);
    energy += (((q31_t) in * (in)) >> 15);

    /* Set the accumulator to zero */
    acc = 0;

    /* Loop over numTaps number of values */
    tapCnt = numTaps;

    while(tapCnt > 0u)
    {
      /* Perform the multiply-accumulate */
      acc += (((q31_t) * px++ * (*pb++)));

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Calc lower part of acc */
    acc_l = acc & 0xffffffff;

    /* Calc upper part of acc */
    acc_h = (acc >> 32) & 0xffffffff;

    /* Apply shift for lower part of acc and upper part of acc */
    acc = (uint32_t) acc_l >> lShift | acc_h << uShift;

    /* Converting the result to 1.15 format and saturate the output */
    acc = __SSAT_16(acc);

    /* Converting the result to 1.15 format */
    //acc = __SSAT_16((acc >> (16u - shift)), 16u);

    /* Store the result from accumulator into the destination buffer. */
    *pOut++ = (q15_t) acc;

    /* Compute and store error */
    d = *pRef++;
    e = d - (q15_t) acc;
    *pErr++ = e;

    /* Calculation of 1/energy */
    postShift = csky_recip_q15((q15_t) energy + DELTA_Q15,
                              &oneByEnergy, S->recipTable);

    /* Calculation of e * mu value */
    errorXmu = (q15_t) (((q31_t) e * mu) >> 15);

    /* Calculation of (e * mu) * (1/energy) value */
    acc = (((q31_t) errorXmu * oneByEnergy) >> (15 - postShift));

    /* Weighting factor for the normalized version */
    w = (q15_t) __SSAT_16((q31_t) acc);

    /* Initialize pState pointer */
    px = pState;

    /* Initialize coeff pointer */
    pb = (pCoeffs);

    /* Loop over numTaps number of values */
    tapCnt = numTaps;

    while(tapCnt > 0u)
    {
      /* Perform the multiply-accumulate */
      coef = *pb + (((q31_t) w * (*px++)) >> 15);
      *pb++ = (q15_t) __SSAT_16((coef));

      /* Decrement the loop counter */
      tapCnt--;
    }

    /* Read the sample from state buffer */
    x0 = *pState;

    /* Advance state pointer by 1 for the next sample */
    pState = pState + 1u;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Save energy and x0 values for the next frame */
  S->energy = (q15_t) energy;
  S->x0 = x0;

  /* Processing is complete. Now copy the last numTaps - 1 samples to the
     satrt of the state buffer. This prepares the state buffer for the
     next function call. */

  /* Points to the start of the pState buffer */
  pStateCurnt = S->pState;

  /* copy (numTaps - 1u) data */
  tapCnt = (numTaps - 1u);

  /* copy data */
  while(tapCnt > 0u)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }

#endif /*   #ifndef CSKY_MATH_NO_SIMD */

}


/**
   * @} end of LMS_NORM group
   */
