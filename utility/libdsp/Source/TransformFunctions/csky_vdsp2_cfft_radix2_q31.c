/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_cfft_radix2_q31.c
* Description:  Radix-2 Decimation in Frequency in Frequency CFFT & CIFFT Fixed point processing function
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
* This file comes from arm_cfft_radix2_q31.c. It is renamed by replacing arm with csky_vdsp2.
*
*/

/******************************************************************************
 * @file     csky_vdsp2_cfft_radix2_q31.c
 * @brief    Radix-2 Decimation in Frequency CFFT & CIFFT
 *           Fixed point processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_vdsp2_math.h"

#define multAcc_32x32_keep32_R(a, x, y) \
    a = (q31_t) (((((q63_t) a) << 32) + ((q63_t) x * y) + 0x80000000LL ) >> 32)

/* SMMLSR */
#define multSub_32x32_keep32_R(a, x, y) \
    a = (q31_t) (((((q63_t) a) << 32) - ((q63_t) x * y) + 0x80000000LL ) >> 32)

/* SMMULR */
#define mult_32x32_keep32_R(a, x, y) \
    a = (q31_t) (((q63_t) x * y + 0x80000000LL ) >> 32)

void csky_vdsp2_radix2_butterfly_q31(
  q31_t * pSrc,
  uint32_t fftLen,
  q31_t * pCoef,
  uint16_t twidCoefModifier);

void csky_vdsp2_radix2_butterfly_inverse_q31(
  q31_t * pSrc,
  uint32_t fftLen,
  q31_t * pCoef,
  uint16_t twidCoefModifier);

void csky_vdsp2_bitreversal_q31(
  q31_t * pSrc,
  uint32_t fftLen,
  uint16_t bitRevFactor,
  uint16_t * pBitRevTab);

/**
* @details
* @brief Processing function for the fixed-point CFFT/CIFFT.
* @deprecated Do not use this function.  It has been superseded by \ref csky_vdsp2_cfft_q31 and will be removed
* @param[in]      *S    points to an instance of the fixed-point CFFT/CIFFT structure.
* @param[in, out] *pSrc points to the complex data buffer of size <code>2*fftLen</code>. Processing occurs in-place.
* @return none.
*/

void csky_vdsp2_cfft_radix2_q31(
const csky_vdsp2_cfft_radix2_instance_q31 * S,
q31_t * pSrc)
{

   if(S->ifftFlag == 1u)
   {
      csky_vdsp2_radix2_butterfly_inverse_q31(pSrc, S->fftLen,
      S->pTwiddle, S->twidCoefModifier);
   }
   else
   {
      csky_vdsp2_radix2_butterfly_q31(pSrc, S->fftLen,
      S->pTwiddle, S->twidCoefModifier);
   }

   csky_vdsp2_bitreversal_q31(pSrc, S->fftLen, S->bitRevFactor, S->pBitRevTable);
}

void csky_vdsp2_radix2_butterfly_q31(
q31_t * pSrc,
uint32_t fftLen,
q31_t * pCoef,
uint16_t twidCoefModifier)
{

   unsigned i, j, k, l, m;
   unsigned n1, n2, ia;
   q31_t xt, yt, cosVal, sinVal;
   q31_t p0, p1;

   //N = fftLen;
   n2 = fftLen;

   n1 = n2;
   n2 = n2 >> 1;
   ia = 0;

   // loop for groups
   for (i = 0; i < n2; i++)
   {
      cosVal = pCoef[ia * 2];
      sinVal = pCoef[(ia * 2) + 1];
      ia = ia + twidCoefModifier;

      l = i + n2;
      xt = (pSrc[2 * i] >> 1u) - (pSrc[2 * l] >> 1u);
      pSrc[2 * i] = ((pSrc[2 * i] >> 1u) + (pSrc[2 * l] >> 1u)) >> 1u;

      yt = (pSrc[2 * i + 1] >> 1u) - (pSrc[2 * l + 1] >> 1u);
      pSrc[2 * i + 1] =
        ((pSrc[2 * l + 1] >> 1u) + (pSrc[2 * i + 1] >> 1u)) >> 1u;

#ifdef CSKY_SIMD
      p0 = mult_32x32_keep32_R(xt, cosVal);
      p1 = mult_32x32_keep32_R(yt, cosVal);
      p0 = multAcc_32x32_keep32_R(p0, yt, sinVal);
      p1 = multSub_32x32_keep32_R(p1, xt, sinVal);
#else
      mult_32x32_keep32_R(p0, xt, cosVal);
      mult_32x32_keep32_R(p1, yt, cosVal);
      multAcc_32x32_keep32_R(p0, yt, sinVal);
      multSub_32x32_keep32_R(p1, xt, sinVal);
#endif

      pSrc[2u * l] = p0;
      pSrc[2u * l + 1u] = p1;

   }                             // groups loop end

   twidCoefModifier <<= 1u;

   // loop for stage
   for (k = fftLen / 2; k > 2; k = k >> 1)
   {
      n1 = n2;
      n2 = n2 >> 1;
      ia = 0;

      // loop for groups
      for (j = 0; j < n2; j++)
      {
         cosVal = pCoef[ia * 2];
         sinVal = pCoef[(ia * 2) + 1];
         ia = ia + twidCoefModifier;

         // loop for butterfly
         i = j;
         m = fftLen / n1;
         do
         {
            l = i + n2;
            xt = pSrc[2 * i] - pSrc[2 * l];
            pSrc[2 * i] = (pSrc[2 * i] + pSrc[2 * l]) >> 1u;

            yt = pSrc[2 * i + 1] - pSrc[2 * l + 1];
            pSrc[2 * i + 1] = (pSrc[2 * l + 1] + pSrc[2 * i + 1]) >> 1u;

#ifdef CSKY_SIMD
            p0 = mult_32x32_keep32_R(xt, cosVal);
            p1 = mult_32x32_keep32_R(yt, cosVal);
            p0 = multAcc_32x32_keep32_R(p0, yt, sinVal);
            p1 = multSub_32x32_keep32_R(p1, xt, sinVal);
#else
            mult_32x32_keep32_R(p0, xt, cosVal);
            mult_32x32_keep32_R(p1, yt, cosVal);
            multAcc_32x32_keep32_R(p0, yt, sinVal);
            multSub_32x32_keep32_R(p1, xt, sinVal);
#endif

            pSrc[2u * l] = p0;
            pSrc[2u * l + 1u] = p1;
            i += n1;
            m--;
         } while( m > 0);                   // butterfly loop end

      }                           // groups loop end

      twidCoefModifier <<= 1u;
   }                             // stages loop end

   n1 = n2;
   n2 = n2 >> 1;
   ia = 0;

   cosVal = pCoef[ia * 2];
   sinVal = pCoef[(ia * 2) + 1];
   ia = ia + twidCoefModifier;

   // loop for butterfly
   for (i = 0; i < fftLen; i += n1)
   {
      l = i + n2;
      xt = pSrc[2 * i] - pSrc[2 * l];
      pSrc[2 * i] = (pSrc[2 * i] + pSrc[2 * l]);

      yt = pSrc[2 * i + 1] - pSrc[2 * l + 1];
      pSrc[2 * i + 1] = (pSrc[2 * l + 1] + pSrc[2 * i + 1]);

      pSrc[2u * l] = xt;

      pSrc[2u * l + 1u] = yt;

      i += n1;
      l = i + n2;

      xt = pSrc[2 * i] - pSrc[2 * l];
      pSrc[2 * i] = (pSrc[2 * i] + pSrc[2 * l]);

      yt = pSrc[2 * i + 1] - pSrc[2 * l + 1];
      pSrc[2 * i + 1] = (pSrc[2 * l + 1] + pSrc[2 * i + 1]);

      pSrc[2u * l] = xt;

      pSrc[2u * l + 1u] = yt;

   }                             // butterfly loop end

}


void csky_vdsp2_radix2_butterfly_inverse_q31(
q31_t * pSrc,
uint32_t fftLen,
q31_t * pCoef,
uint16_t twidCoefModifier)
{

   unsigned i, j, k, l;
   unsigned n1, n2, ia;
   q31_t xt, yt, cosVal, sinVal;
   q31_t p0, p1;

   //N = fftLen;
   n2 = fftLen;

   n1 = n2;
   n2 = n2 >> 1;
   ia = 0;

   // loop for groups
   for (i = 0; i < n2; i++)
   {
      cosVal = pCoef[ia * 2];
      sinVal = pCoef[(ia * 2) + 1];
      ia = ia + twidCoefModifier;

      l = i + n2;
      xt = (pSrc[2 * i] >> 1u) - (pSrc[2 * l] >> 1u);
      pSrc[2 * i] = ((pSrc[2 * i] >> 1u) + (pSrc[2 * l] >> 1u)) >> 1u;

      yt = (pSrc[2 * i + 1] >> 1u) - (pSrc[2 * l + 1] >> 1u);
      pSrc[2 * i + 1] =
        ((pSrc[2 * l + 1] >> 1u) + (pSrc[2 * i + 1] >> 1u)) >> 1u;

#ifdef CSKY_SIMD
      p0 = mult_32x32_keep32_R(xt, cosVal);
      p1 = mult_32x32_keep32_R(yt, cosVal);
      p0 = multSub_32x32_keep32_R(p0, yt, sinVal);
      p1 = multAcc_32x32_keep32_R(p1, xt, sinVal);
#else
      mult_32x32_keep32_R(p0, xt, cosVal);
      mult_32x32_keep32_R(p1, yt, cosVal);
      multSub_32x32_keep32_R(p0, yt, sinVal);
      multAcc_32x32_keep32_R(p1, xt, sinVal);
#endif

      pSrc[2u * l] = p0;
      pSrc[2u * l + 1u] = p1;
   }                             // groups loop end

   twidCoefModifier = twidCoefModifier << 1u;

   // loop for stage
   for (k = fftLen / 2; k > 2; k = k >> 1)
   {
      n1 = n2;
      n2 = n2 >> 1;
      ia = 0;

      // loop for groups
      for (j = 0; j < n2; j++)
      {
         cosVal = pCoef[ia * 2];
         sinVal = pCoef[(ia * 2) + 1];
         ia = ia + twidCoefModifier;

         // loop for butterfly
         for (i = j; i < fftLen; i += n1)
         {
            l = i + n2;
            xt = pSrc[2 * i] - pSrc[2 * l];
            pSrc[2 * i] = (pSrc[2 * i] + pSrc[2 * l]) >> 1u;

            yt = pSrc[2 * i + 1] - pSrc[2 * l + 1];
            pSrc[2 * i + 1] = (pSrc[2 * l + 1] + pSrc[2 * i + 1]) >> 1u;

#ifdef CSKY_SIMD
            p0 = mult_32x32_keep32_R(xt, cosVal);
            p1 = mult_32x32_keep32_R(yt, cosVal);
            p0 = multSub_32x32_keep32_R(p0, yt, sinVal);
            p1 = multAcc_32x32_keep32_R(p1, xt, sinVal);
#else
            mult_32x32_keep32_R(p0, xt, cosVal);
            mult_32x32_keep32_R(p1, yt, cosVal);
            multSub_32x32_keep32_R(p0, yt, sinVal);
            multAcc_32x32_keep32_R(p1, xt, sinVal);
#endif

            pSrc[2u * l] = p0;
            pSrc[2u * l + 1u] = p1;
         }                         // butterfly loop end

      }                           // groups loop end

      twidCoefModifier = twidCoefModifier << 1u;
   }                             // stages loop end

   n1 = n2;
   n2 = n2 >> 1;
   ia = 0;

   cosVal = pCoef[ia * 2];
   sinVal = pCoef[(ia * 2) + 1];
   ia = ia + twidCoefModifier;

   // loop for butterfly
   for (i = 0; i < fftLen; i += n1)
   {
      l = i + n2;
      xt = pSrc[2 * i] - pSrc[2 * l];
      pSrc[2 * i] = (pSrc[2 * i] + pSrc[2 * l]);

      yt = pSrc[2 * i + 1] - pSrc[2 * l + 1];
      pSrc[2 * i + 1] = (pSrc[2 * l + 1] + pSrc[2 * i + 1]);

      pSrc[2u * l] = xt;

      pSrc[2u * l + 1u] = yt;

      i += n1;
      l = i + n2;

      xt = pSrc[2 * i] - pSrc[2 * l];
      pSrc[2 * i] = (pSrc[2 * i] + pSrc[2 * l]);

      yt = pSrc[2 * i + 1] - pSrc[2 * l + 1];
      pSrc[2 * i + 1] = (pSrc[2 * l + 1] + pSrc[2 * i + 1]);

      pSrc[2u * l] = xt;

      pSrc[2u * l + 1u] = yt;

   }                             // butterfly loop end

}
