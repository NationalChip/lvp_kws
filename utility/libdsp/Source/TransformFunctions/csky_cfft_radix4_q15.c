/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_cfft_radix4_q15.c
* Description:  This file has function definition if Radix-4 FFT & IFFT function and
*               In-place bit reversal using bit reversal table
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
* This file comes from arm_cfft_radix4_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_cfft_radix4_q15.c
 * @brief    This file has function definition of Radix-4 FFT & IFFT function and
 *				   In-place bit reversal using bit reversal table.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"


void csky_radix4_butterfly_q15(
  q15_t * pSrc16,
  uint32_t fftLen,
  q15_t * pCoef16,
  uint32_t twidCoefModifier);

void csky_radix4_butterfly_inverse_q15(
  q15_t * pSrc16,
  uint32_t fftLen,
  q15_t * pCoef16,
  uint32_t twidCoefModifier);

void csky_bitreversal_q15(
  q15_t * pSrc,
  uint32_t fftLen,
  uint16_t bitRevFactor,
  uint16_t * pBitRevTab);


/**
 * @details
 * @brief Processing function for the Q15 CFFT/CIFFT.
 * @deprecated Do not use this function.  It has been superseded by \ref csky_cfft_q15 and will be removed
 * @param[in]      *S    points to an instance of the Q15 CFFT/CIFFT structure.
 * @param[in, out] *pSrc points to the complex data buffer. Processing occurs in-place.
 * @return none.
 *
 * \par Input and output formats:
 * \par
 * Internally input is downscaled by 2 for every stage to avoid saturations inside CFFT/CIFFT process.
 * Hence the output format is different for different FFT sizes.
 * The input and output formats for different FFT sizes and number of bits to upscale are mentioned in the tables below for CFFT and CIFFT:
 */

void csky_cfft_radix4_q15(
  const csky_cfft_radix4_instance_q15 * S,
  q15_t * pSrc)
{
  if(S->ifftFlag == 1u)
  {
    /*  Complex IFFT radix-4  */
    csky_radix4_butterfly_inverse_q15(pSrc, S->fftLen, S->pTwiddle,
                                     S->twidCoefModifier);
  }
  else
  {
    /*  Complex FFT radix-4  */
    csky_radix4_butterfly_q15(pSrc, S->fftLen, S->pTwiddle,
                             S->twidCoefModifier);
  }

  if(S->bitReverseFlag == 1u)
  {
    /*  Bit Reversal */
    csky_bitreversal_q15(pSrc, S->fftLen, S->bitRevFactor, S->pBitRevTable);
  }

}

