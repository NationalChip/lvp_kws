/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_rfft_f32.c
* Description:  RFFT & RIFFT Floating point precess function
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
* This file comes from arm_rfft_f32.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_rfft_f32.c
 * @brief    RFFT & RIFFT Floating point process function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

extern void csky_radix4_butterfly_f32(
    float32_t * pSrc,
    uint16_t fftLen,
    float32_t * pCoef,
    uint16_t twidCoefModifier);

extern void csky_radix4_butterfly_inverse_f32(
    float32_t * pSrc,
    uint16_t fftLen,
    float32_t * pCoef,
    uint16_t twidCoefModifier,
    float32_t onebyfftLen);

extern void csky_bitreversal_f32(
    float32_t * pSrc,
    uint16_t fftSize,
    uint16_t bitRevFactor,
    uint16_t * pBitRevTab);

/**
 * @ingroup groupTransforms
 */

/*--------------------------------------------------------------------
 *		Internal functions prototypes
 *--------------------------------------------------------------------*/

void csky_split_rfft_f32(
  float32_t * pSrc,
  uint32_t fftLen,
  float32_t * pATable,
  float32_t * pBTable,
  float32_t * pDst,
  uint32_t modifier);
void csky_split_rifft_f32(
  float32_t * pSrc,
  uint32_t fftLen,
  float32_t * pATable,
  float32_t * pBTable,
  float32_t * pDst,
  uint32_t modifier);

/**
 * @brief Processing function for the floating-point RFFT/RIFFT.
 * @deprecated Do not use this function.  It has been superceded by \ref csky_rfft_fast_f32 and will be removed
 * in the future.
 * @param[in]  *S    points to an instance of the floating-point RFFT/RIFFT structure.
 * @param[in]  *pSrc points to the input buffer.
 * @param[out] *pDst points to the output buffer.
 * @return none.
 */

void csky_rfft_f32(
  const csky_rfft_instance_f32 * S,
  float32_t * pSrc,
  float32_t * pDst)
{
  const csky_cfft_radix4_instance_f32 *S_CFFT = S->pCfft;


  /* Calculation of Real IFFT of input */
  if(S->ifftFlagR == 1u)
  {
    /*  Real IFFT core process */
    csky_split_rifft_f32(pSrc, S->fftLenBy2, S->pTwiddleAReal,
                        S->pTwiddleBReal, pDst, S->twidCoefRModifier);


    /* Complex radix-4 IFFT process */
    csky_radix4_butterfly_inverse_f32(pDst, S_CFFT->fftLen,
                                     S_CFFT->pTwiddle,
                                     S_CFFT->twidCoefModifier,
                                     S_CFFT->onebyfftLen);

    /* Bit reversal process */
    if(S->bitReverseFlagR == 1u)
    {
      csky_bitreversal_f32(pDst, S_CFFT->fftLen,
                          S_CFFT->bitRevFactor, S_CFFT->pBitRevTable);
    }
  }
  else
  {

    /* Calculation of RFFT of input */

    /* Complex radix-4 FFT process */
    csky_radix4_butterfly_f32(pSrc, S_CFFT->fftLen,
                             S_CFFT->pTwiddle, S_CFFT->twidCoefModifier);

    /* Bit reversal process */
    if(S->bitReverseFlagR == 1u)
    {
      csky_bitreversal_f32(pSrc, S_CFFT->fftLen,
                          S_CFFT->bitRevFactor, S_CFFT->pBitRevTable);
    }


    /*  Real FFT core process */
    csky_split_rfft_f32(pSrc, S->fftLenBy2, S->pTwiddleAReal,
                       S->pTwiddleBReal, pDst, S->twidCoefRModifier);
  }

}

/**
 * @brief  Core Real FFT process
 * @param[in]   *pSrc 				points to the input buffer.
 * @param[in]   fftLen  			length of FFT.
 * @param[in]   *pATable 			points to the twiddle Coef A buffer.
 * @param[in]   *pBTable 			points to the twiddle Coef B buffer.
 * @param[out]  *pDst 				points to the output buffer.
 * @param[in]   modifier 	        twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
 * @return none.
 */

void csky_split_rfft_f32(
  float32_t * pSrc,
  uint32_t fftLen,
  float32_t * pATable,
  float32_t * pBTable,
  float32_t * pDst,
  uint32_t modifier)
{
  uint32_t i;                                    /* Loop Counter */
  float32_t outR, outI;                          /* Temporary variables for output */
  float32_t *pCoefA, *pCoefB;                    /* Temporary pointers for twiddle factors */
  float32_t CoefA1, CoefA2, CoefB1;              /* Temporary variables for twiddle coefficients */
  float32_t *pDst1 = &pDst[2], *pDst2 = &pDst[(4u * fftLen) - 1u];      /* temp pointers for output buffer */
  float32_t *pSrc1 = &pSrc[2], *pSrc2 = &pSrc[(2u * fftLen) - 1u];      /* temp pointers for input buffer */

  /* Init coefficient pointers */
  pCoefA = &pATable[modifier * 2u];
  pCoefB = &pBTable[modifier * 2u];

  i = fftLen - 1u;

  while(i > 0u)
  {
    /*
       outR = (pSrc[2 * i] * pATable[2 * i] - pSrc[2 * i + 1] * pATable[2 * i + 1]
       + pSrc[2 * n - 2 * i] * pBTable[2 * i] +
       pSrc[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);
     */

    /* outI = (pIn[2 * i + 1] * pATable[2 * i] + pIn[2 * i] * pATable[2 * i + 1] +
       pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
       pIn[2 * n - 2 * i + 1] * pBTable[2 * i]); */

    /* read pATable[2 * i] */
    CoefA1 = *pCoefA++;
    /* pATable[2 * i + 1] */
    CoefA2 = *pCoefA;

    /* pSrc[2 * i] * pATable[2 * i] */
    outR = *pSrc1 * CoefA1;
    /* pSrc[2 * i] * CoefA2 */
    outI = *pSrc1++ * CoefA2;

    /* (pSrc[2 * i + 1] + pSrc[2 * fftLen - 2 * i + 1]) * CoefA2 */
    outR -= (*pSrc1 + *pSrc2) * CoefA2;
    /* pSrc[2 * i + 1] * CoefA1 */
    outI += *pSrc1++ * CoefA1;

    CoefB1 = *pCoefB;

    /* pSrc[2 * fftLen - 2 * i + 1] * CoefB1 */
    outI -= *pSrc2-- * CoefB1;
    /* pSrc[2 * fftLen - 2 * i] * CoefA2 */
    outI -= *pSrc2 * CoefA2;

    /* pSrc[2 * fftLen - 2 * i] * CoefB1 */
    outR += *pSrc2-- * CoefB1;

    /* write output */
    *pDst1++ = outR;
    *pDst1++ = outI;

    /* write complex conjugate output */
    *pDst2-- = -outI;
    *pDst2-- = outR;

    /* update coefficient pointer */
    pCoefB = pCoefB + (modifier * 2u);
    pCoefA = pCoefA + ((modifier * 2u) - 1u);

    i--;

  }

  pDst[2u * fftLen] = pSrc[0] - pSrc[1];
  pDst[(2u * fftLen) + 1u] = 0.0f;

  pDst[0] = pSrc[0] + pSrc[1];
  pDst[1] = 0.0f;

}


/**
 * @brief  Core Real IFFT process
 * @param[in]   *pSrc 				points to the input buffer.
 * @param[in]   fftLen  			length of FFT.
 * @param[in]   *pATable 			points to the twiddle Coef A buffer.
 * @param[in]   *pBTable 			points to the twiddle Coef B buffer.
 * @param[out]  *pDst 				points to the output buffer.
 * @param[in]   modifier 	        twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
 * @return none.
 */

void csky_split_rifft_f32(
  float32_t * pSrc,
  uint32_t fftLen,
  float32_t * pATable,
  float32_t * pBTable,
  float32_t * pDst,
  uint32_t modifier)
{
  float32_t outR, outI;                          /* Temporary variables for output */
  float32_t *pCoefA, *pCoefB;                    /* Temporary pointers for twiddle factors */
  float32_t CoefA1, CoefA2, CoefB1;              /* Temporary variables for twiddle coefficients */
  float32_t *pSrc1 = &pSrc[0], *pSrc2 = &pSrc[(2u * fftLen) + 1u];

  pCoefA = &pATable[0];
  pCoefB = &pBTable[0];

  while(fftLen > 0u)
  {
    /*
       outR = (pIn[2 * i] * pATable[2 * i] + pIn[2 * i + 1] * pATable[2 * i + 1] +
       pIn[2 * n - 2 * i] * pBTable[2 * i] -
       pIn[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);

       outI = (pIn[2 * i + 1] * pATable[2 * i] - pIn[2 * i] * pATable[2 * i + 1] -
       pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
       pIn[2 * n - 2 * i + 1] * pBTable[2 * i]);

     */

    CoefA1 = *pCoefA++;
    CoefA2 = *pCoefA;

    /* outR = (pSrc[2 * i] * CoefA1 */
    outR = *pSrc1 * CoefA1;

    /* - pSrc[2 * i] * CoefA2 */
    outI = -(*pSrc1++) * CoefA2;

    /* (pSrc[2 * i + 1] + pSrc[2 * fftLen - 2 * i + 1]) * CoefA2 */
    outR += (*pSrc1 + *pSrc2) * CoefA2;

    /* pSrc[2 * i + 1] * CoefA1 */
    outI += (*pSrc1++) * CoefA1;

    CoefB1 = *pCoefB;

    /* - pSrc[2 * fftLen - 2 * i + 1] * CoefB1 */
    outI -= *pSrc2-- * CoefB1;

    /* pSrc[2 * fftLen - 2 * i] * CoefB1 */
    outR += *pSrc2 * CoefB1;

    /* pSrc[2 * fftLen - 2 * i] * CoefA2 */
    outI += *pSrc2-- * CoefA2;

    /* write output */
    *pDst++ = outR;
    *pDst++ = outI;

    /* update coefficient pointer */
    pCoefB = pCoefB + (modifier * 2u);
    pCoefA = pCoefA + ((modifier * 2u) - 1u);

    /* Decrement loop count */
    fftLen--;
  }

}
