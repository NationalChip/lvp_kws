/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_bitreversal.c
* Description:  Bitreversal functions
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
* This file comes from arm_bitreversal.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_bitreversal.c
 * @brief    This file has common tables like Bitreverse, reciprocal etc
 *           which are used across different functions.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"
#include "csky_common_tables.h"

#if FOR_X86_64
/*
* @brief  In-place bit reversal function.
* @param[in, out] *pSrc        points to the in-place buffer of unknown 32-bit data type.
* @param[in]      bitRevLen    bit reversal table length
* @param[in]      *pBitRevTab  points to bit reversal table.
* @return none.
*/
void csky_bitreversal_32(uint32_t *pSrc, int bitRevLen, uint16_t *pBitRevTab)
{
	int i = 0;
	int j = (bitRevLen + 1) >> 1;
	uint32_t tmp1, tmp2;
	uint8_t *base = (uint8_t *)pSrc;
	uint8_t *table = (uint8_t *)pBitRevTab;

	while (j > 0) {
		tmp1 = *(uint32_t *)(base + *(uint16_t *)(table + i));
		tmp2 = *(uint32_t *)(base + *(uint16_t *)(table + i + 2));
		*(uint32_t *)(base + *(uint16_t *)(table + i)) = tmp2;
		*(uint32_t *)(base + *(uint16_t *)(table + i + 2)) = tmp1;

		tmp1 = *(uint32_t *)(base + *(uint16_t *)(table + i) + 4);
		tmp2 = *(uint32_t *)(base + *(uint16_t *)(table + i + 2) + 4);
		*(uint32_t *)(base + *(uint16_t *)(table + i) + 4) = tmp2;
		*(uint32_t *)(base + *(uint16_t *)(table + i + 2) + 4) = tmp1;

		i += 4;
		j--;
	};
}

void csky_bitreversal_16(uint32_t *pSrc, int bitRevLen, uint16_t *pBitRevTab)
{
	int i = 0;
	int j = (bitRevLen + 1) >> 1;
	uint32_t tmp1, tmp2;
	uint8_t *base = (uint8_t *)pSrc;
	uint8_t *table = (uint8_t *)pBitRevTab;

	while (j > 0) {
		tmp1 = *(uint32_t *)(base + (*(uint16_t *)(table + i) >> 1));
		tmp2 = *(uint32_t *)(base + (*(uint16_t *)(table + i + 2) >> 1));
		*(uint32_t *)(base + (*(uint16_t *)(table + i) >> 1)) = tmp2;
		*(uint32_t *)(base + (*(uint16_t *)(table + i + 2) >> 1)) = tmp1;

		i += 4;
		j--;
	};
}
#endif


/*
* @brief  In-place bit reversal function.
* @param[in, out] *pSrc        points to the in-place buffer of floating-point data type.
* @param[in]      fftSize      length of the FFT.
* @param[in]      bitRevFactor bit reversal modifier that supports different size FFTs with the same bit reversal table.
* @param[in]      *pBitRevTab  points to the bit reversal table.
* @return none.
*/

void csky_bitreversal_f32(
float32_t * pSrc,
uint16_t fftSize,
uint16_t bitRevFactor,
uint16_t * pBitRevTab)
{
   uint16_t fftLenBy2, fftLenBy2p1;
   uint16_t i, j;
   float32_t in;

   /*  Initializations */
   j = 0u;
   fftLenBy2 = fftSize >> 1u;
   fftLenBy2p1 = (fftSize >> 1u) + 1u;

   /* Bit Reversal Implementation */
   for (i = 0u; i <= (fftLenBy2 - 2u); i += 2u)
   {
      if(i < j)
      {
         /*  pSrc[i] <-> pSrc[j]; */
         in = pSrc[2u * i];
         pSrc[2u * i] = pSrc[2u * j];
         pSrc[2u * j] = in;

         /*  pSrc[i+1u] <-> pSrc[j+1u] */
         in = pSrc[(2u * i) + 1u];
         pSrc[(2u * i) + 1u] = pSrc[(2u * j) + 1u];
         pSrc[(2u * j) + 1u] = in;

         /*  pSrc[i+fftLenBy2p1] <-> pSrc[j+fftLenBy2p1] */
         in = pSrc[2u * (i + fftLenBy2p1)];
         pSrc[2u * (i + fftLenBy2p1)] = pSrc[2u * (j + fftLenBy2p1)];
         pSrc[2u * (j + fftLenBy2p1)] = in;

         /*  pSrc[i+fftLenBy2p1+1u] <-> pSrc[j+fftLenBy2p1+1u] */
         in = pSrc[(2u * (i + fftLenBy2p1)) + 1u];
         pSrc[(2u * (i + fftLenBy2p1)) + 1u] =
         pSrc[(2u * (j + fftLenBy2p1)) + 1u];
         pSrc[(2u * (j + fftLenBy2p1)) + 1u] = in;

      }

      /*  pSrc[i+1u] <-> pSrc[j+1u] */
      in = pSrc[2u * (i + 1u)];
      pSrc[2u * (i + 1u)] = pSrc[2u * (j + fftLenBy2)];
      pSrc[2u * (j + fftLenBy2)] = in;

      /*  pSrc[i+2u] <-> pSrc[j+2u] */
      in = pSrc[(2u * (i + 1u)) + 1u];
      pSrc[(2u * (i + 1u)) + 1u] = pSrc[(2u * (j + fftLenBy2)) + 1u];
      pSrc[(2u * (j + fftLenBy2)) + 1u] = in;

      /*  Reading the index for the bit reversal */
      j = *pBitRevTab;

      /*  Updating the bit reversal index depending on the fft length  */
      pBitRevTab += bitRevFactor;
   }
}



/*
* @brief  In-place bit reversal function.
* @param[in, out] *pSrc        points to the in-place buffer of Q31 data type.
* @param[in]      fftLen       length of the FFT.
* @param[in]      bitRevFactor bit reversal modifier that supports different size FFTs with the same bit reversal table
* @param[in]      *pBitRevTab  points to bit reversal table.
* @return none.
*/

void csky_bitreversal_q31(
q31_t * pSrc,
uint32_t fftLen,
uint16_t bitRevFactor,
uint16_t * pBitRevTable)
{
   uint32_t fftLenBy2, fftLenBy2p1, i, j;
   q31_t in;

   /*  Initializations      */
   j = 0u;
   fftLenBy2 = fftLen / 2u;
   fftLenBy2p1 = (fftLen / 2u) + 1u;

   /* Bit Reversal Implementation */
   for (i = 0u; i <= (fftLenBy2 - 2u); i += 2u)
   {
      if(i < j)
      {
         /*  pSrc[i] <-> pSrc[j]; */
         in = pSrc[2u * i];
         pSrc[2u * i] = pSrc[2u * j];
         pSrc[2u * j] = in;

         /*  pSrc[i+1u] <-> pSrc[j+1u] */
         in = pSrc[(2u * i) + 1u];
         pSrc[(2u * i) + 1u] = pSrc[(2u * j) + 1u];
         pSrc[(2u * j) + 1u] = in;

         /*  pSrc[i+fftLenBy2p1] <-> pSrc[j+fftLenBy2p1] */
         in = pSrc[2u * (i + fftLenBy2p1)];
         pSrc[2u * (i + fftLenBy2p1)] = pSrc[2u * (j + fftLenBy2p1)];
         pSrc[2u * (j + fftLenBy2p1)] = in;

         /*  pSrc[i+fftLenBy2p1+1u] <-> pSrc[j+fftLenBy2p1+1u] */
         in = pSrc[(2u * (i + fftLenBy2p1)) + 1u];
         pSrc[(2u * (i + fftLenBy2p1)) + 1u] =
         pSrc[(2u * (j + fftLenBy2p1)) + 1u];
         pSrc[(2u * (j + fftLenBy2p1)) + 1u] = in;

      }

      /*  pSrc[i+1u] <-> pSrc[j+1u] */
      in = pSrc[2u * (i + 1u)];
      pSrc[2u * (i + 1u)] = pSrc[2u * (j + fftLenBy2)];
      pSrc[2u * (j + fftLenBy2)] = in;

      /*  pSrc[i+2u] <-> pSrc[j+2u] */
      in = pSrc[(2u * (i + 1u)) + 1u];
      pSrc[(2u * (i + 1u)) + 1u] = pSrc[(2u * (j + fftLenBy2)) + 1u];
      pSrc[(2u * (j + fftLenBy2)) + 1u] = in;

      /*  Reading the index for the bit reversal */
      j = *pBitRevTable;

      /*  Updating the bit reversal index depending on the fft length */
      pBitRevTable += bitRevFactor;
   }
}



/*
   * @brief  In-place bit reversal function.
   * @param[in, out] *pSrc        points to the in-place buffer of Q15 data type.
   * @param[in]      fftLen       length of the FFT.
   * @param[in]      bitRevFactor bit reversal modifier that supports different size FFTs with the same bit reversal table
   * @param[in]      *pBitRevTab  points to bit reversal table.
   * @return none.
*/

void csky_bitreversal_q15(
q15_t * pSrc16,
uint32_t fftLen,
uint16_t bitRevFactor,
uint16_t * pBitRevTab)
{
   q31_t *pSrc = (q31_t *) pSrc16;
   q31_t in;
   uint32_t fftLenBy2, fftLenBy2p1;
   uint32_t i, j;

   /*  Initializations */
   j = 0u;
   fftLenBy2 = fftLen / 2u;
   fftLenBy2p1 = (fftLen / 2u) + 1u;

   /* Bit Reversal Implementation */
   for (i = 0u; i <= (fftLenBy2 - 2u); i += 2u)
   {
      if(i < j)
      {
         /*  pSrc[i] <-> pSrc[j]; */
         /*  pSrc[i+1u] <-> pSrc[j+1u] */
         in = pSrc[i];
         pSrc[i] = pSrc[j];
         pSrc[j] = in;

         /*  pSrc[i + fftLenBy2p1] <-> pSrc[j + fftLenBy2p1];  */
         /*  pSrc[i + fftLenBy2p1+1u] <-> pSrc[j + fftLenBy2p1+1u] */
         in = pSrc[i + fftLenBy2p1];
         pSrc[i + fftLenBy2p1] = pSrc[j + fftLenBy2p1];
         pSrc[j + fftLenBy2p1] = in;
      }

      /*  pSrc[i+1u] <-> pSrc[j+fftLenBy2];         */
      /*  pSrc[i+2] <-> pSrc[j+fftLenBy2+1u]  */
      in = pSrc[i + 1u];
      pSrc[i + 1u] = pSrc[j + fftLenBy2];
      pSrc[j + fftLenBy2] = in;

      /*  Reading the index for the bit reversal */
      j = *pBitRevTab;

      /*  Updating the bit reversal index depending on the fft length  */
      pBitRevTab += bitRevFactor;
   }
}
