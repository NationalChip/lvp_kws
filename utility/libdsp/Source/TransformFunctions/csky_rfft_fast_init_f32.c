/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_rfft_fast_init_f32.c
* Description:  Split Radix Decimation in Frequency CFFT Floating pint processing function
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
* This file comes from arm_rfft_fast_init_f32.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_cfft_init_f32.c
 * @brief    Split Radix Decimation in Frequency CFFT Floating point processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"
#include "csky_common_tables.h"

/**
 * @ingroup groupTransforms
 */

/**
 * @addtogroup RealFFT
 * @{
 */

/**
* @brief  Initialization function for the floating-point real FFT.
* @param[in,out] *S             points to an csky_rfft_fast_instance_f32 structure.
* @param[in]     fftLen         length of the Real Sequence.
* @return        The function returns CSKY_MATH_SUCCESS if initialization is successful or CSKY_MATH_ARGUMENT_ERROR if <code>fftLen</code> is not a supported value.
*
* \par Description:
* \par
* The parameter <code>fftLen</code>	Specifies length of RFFT/CIFFT process. Supported FFT Lengths are 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192.
* \par
* This Function also initializes Twiddle factor table pointer and Bit reversal table pointer.
*
* \note
* This initialization function is recommended not to used, the function csky_rfft_fast_f32() can be used  straightforwardly by the manully initialized instance instructions for each length.
*/

csky_status csky_rfft_fast_init_f32(
  csky_rfft_fast_instance_f32 * S,
  uint16_t fftLen)
{
  csky_cfft_instance_f32 * Sint;
  /*  Initialise the default csky status */
  csky_status status = CSKY_MATH_SUCCESS;
  /*  Initialise the FFT length */
  Sint = &(S->Sint);
  Sint->fftLen = fftLen/2;
  S->fftLenRFFT = fftLen;

  /*  Initializations of structure parameters depending on the FFT length */
  switch (Sint->fftLen)
  {
  case 512u:
    Sint->bitRevLength = CSKYBITREVINDEXTABLE_512_TABLE_LENGTH;
    Sint->pBitRevTable = (uint16_t *)cskyBitRevIndexTable512;
    Sint->pTwiddle     = (float32_t *) twiddleCoef_512;
    S->pTwiddleRFFT    = (float32_t *) twiddleCoef_rfft_1024;
    break;
  case 256u:
    Sint->bitRevLength = CSKYBITREVINDEXTABLE_256_TABLE_LENGTH;
    Sint->pBitRevTable = (uint16_t *)cskyBitRevIndexTable256;
    Sint->pTwiddle     = (float32_t *) twiddleCoef_256;
    S->pTwiddleRFFT    = (float32_t *) twiddleCoef_rfft_512;
    break;
  case 128u:
    Sint->bitRevLength = CSKYBITREVINDEXTABLE_128_TABLE_LENGTH;
    Sint->pBitRevTable = (uint16_t *)cskyBitRevIndexTable128;
    Sint->pTwiddle     = (float32_t *) twiddleCoef_128;
    S->pTwiddleRFFT    = (float32_t *) twiddleCoef_rfft_256;
    break;
  case 64u:
    Sint->bitRevLength = CSKYBITREVINDEXTABLE__64_TABLE_LENGTH;
    Sint->pBitRevTable = (uint16_t *)cskyBitRevIndexTable64;
    Sint->pTwiddle     = (float32_t *) twiddleCoef_64;
    S->pTwiddleRFFT    = (float32_t *) twiddleCoef_rfft_128;
    break;
  case 32u:
    Sint->bitRevLength = CSKYBITREVINDEXTABLE__32_TABLE_LENGTH;
    Sint->pBitRevTable = (uint16_t *)cskyBitRevIndexTable32;
    Sint->pTwiddle     = (float32_t *) twiddleCoef_32;
    S->pTwiddleRFFT    = (float32_t *) twiddleCoef_rfft_64;
    break;
  case 16u:
    Sint->bitRevLength = CSKYBITREVINDEXTABLE__16_TABLE_LENGTH;
    Sint->pBitRevTable = (uint16_t *)cskyBitRevIndexTable16;
    Sint->pTwiddle     = (float32_t *) twiddleCoef_16;
    S->pTwiddleRFFT    = (float32_t *) twiddleCoef_rfft_32;
    break;
  default:
    /*  Reporting argument error if fftSize is not valid value */
    status = CSKY_MATH_ARGUMENT_ERROR;
    break;
  }

  return (status);
}

/**
 * @} end of RealFFT group
 */
