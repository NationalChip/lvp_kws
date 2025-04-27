/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_dct4_init_q15.c
* Description:  Initialization function of DCT-4 & IDCT4 Q15
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
* This file comes from arm_dct4_init_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_dct4_init_q15.c
 * @brief    Initialization function of DCT-4 & IDCT4 Q15.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"
#include "csky_common_tables.h"

/**
 * @ingroup groupTransforms
 */

/**
 * @addtogroup DCT4_IDCT4
 * @{
 */

/**
 * @brief  Initialization function for the Q15 DCT4/IDCT4.
 * @param[in,out] *S         points to an instance of Q15 DCT4/IDCT4 structure.
 * @param[in]     *S_RFFT    points to an instance of Q15 RFFT/RIFFT structure.
 * @param[in]     *S_CFFT    points to an instance of Q15 CFFT/CIFFT structure.
 * @param[in]     N          length of the DCT4.
 * @param[in]     Nby2       half of the length of the DCT4.
 * @param[in]     normalize  normalizing factor.
 * @return  	  csky_status function returns CSKY_MATH_SUCCESS if initialization is successful or CSKY_MATH_ARGUMENT_ERROR if <code>N</code> is not a supported transform length.
 * \par Normalizing factor:
 * The normalizing factor is <code>sqrt(2/N)</code>, which depends on the size of transform <code>N</code>.
 * Normalizing factors in 1.15 format are mentioned in the table below for different DCT sizes:
 *
 * \par
 * | DCT Size | Normalizing factor value(hexadecimal) |
 * |----------|---------------------------------------|
 * | 8192     | 0x200                                 |
 * | 2048     | 0x400                                 |
 * | 512      | 0x800                                 |
 * | 128      | 0x1000                                |
 *
 * \note
 * This initialization function is recommended not to used, the function csky_dct4_q15() can be used  straightforwardly by the manully initialized instance instructions for each length.
 */


csky_status csky_dct4_init_q15(
  csky_dct4_instance_q15 * S,
  csky_rfft_instance_q15 * S_RFFT,
  csky_cfft_radix4_instance_q15 * S_CFFT,
  uint16_t N,
  uint16_t Nby2,
  q15_t normalize)
{
  /*  Initialise the default csky status */
  csky_status status = CSKY_MATH_SUCCESS;

  /* Initializing the pointer array with the weight table base addresses of different lengths */
  q15_t *twiddlePtr[4] = { (q15_t *) WeightsQ15_128, (q15_t *) WeightsQ15_512,
    (q15_t *) WeightsQ15_2048, (q15_t *) WeightsQ15_8192
  };

  /* Initializing the pointer array with the cos factor table base addresses of different lengths */
  q15_t *pCosFactor[4] =
    { (q15_t *) cos_factorsQ15_128, (q15_t *) cos_factorsQ15_512,
    (q15_t *) cos_factorsQ15_2048, (q15_t *) cos_factorsQ15_8192
  };

  /* Initialize the DCT4 length */
  S->N = N;

  /* Initialize the half of DCT4 length */
  S->Nby2 = Nby2;

  /* Initialize the DCT4 Normalizing factor */
  S->normalize = normalize;

  /* Initialize Real FFT Instance */
  S->pRfft = S_RFFT;

  /* Initialize Complex FFT Instance */
  S->pCfft = S_CFFT;

  switch (N)
  {
    /* Initialize the table modifier values */
  case 8192u:
    S->pTwiddle = twiddlePtr[3];
    S->pCosFactor = pCosFactor[3];
    break;
  case 2048u:
    S->pTwiddle = twiddlePtr[2];
    S->pCosFactor = pCosFactor[2];
    break;
  case 512u:
    S->pTwiddle = twiddlePtr[1];
    S->pCosFactor = pCosFactor[1];
    break;
  case 128u:
    S->pTwiddle = twiddlePtr[0];
    S->pCosFactor = pCosFactor[0];
    break;
  default:
    status = CSKY_MATH_ARGUMENT_ERROR;
  }

  /* Initialize the RFFT/RIFFT */
  csky_rfft_init_q15(S->pRfft, S->N, 0u, 1u);

  /* return the status of DCT4 Init function */
  return (status);
}

/**
   * @} end of DCT4_IDCT4 group
   */
