/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_rfft_init_f32.c
* Description:  RFFT & RIFFT Floating point initialisation function
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
* This file comes from arm_rfft_init_f32.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_rfft_init_f32.c
 * @brief    RFFT & RIFFT Floating point initialisation function.
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

csky_status csky_cfft_radix4_init_f32(
  csky_cfft_radix4_instance_f32 * S,
  uint16_t fftLen,
  uint8_t ifftFlag,
  uint8_t bitReverseFlag);

/**
* @brief  Initialization function for the floating-point RFFT/RIFFT.
* @deprecated Do not use this function.  It has been superceded by \ref csky_rfft_fast_init_f32 and will be removed
* in the future.
* @param[in,out] *S             points to an instance of the floating-point RFFT/RIFFT structure.
* @param[in,out] *S_CFFT        points to an instance of the floating-point CFFT/CIFFT structure.
* @param[in]     fftLenReal     length of the FFT.
* @param[in]     ifftFlagR      flag that selects forward (ifftFlagR=0) or inverse (ifftFlagR=1) transform.
* @param[in]     bitReverseFlag flag that enables (bitReverseFlag=1) or disables (bitReverseFlag=0) bit reversal of output.
* @return		The function returns CSKY_MATH_SUCCESS if initialization is successful or CSKY_MATH_ARGUMENT_ERROR if <code>fftLenReal</code> is not a supported value.
*
* \par Description:
* \par
* The parameter <code>fftLenReal</code>	Specifies length of RFFT/RIFFT Process. Supported FFT Lengths are 128, 512, 2048.
* \par
* The parameter <code>ifftFlagR</code> controls whether a forward or inverse transform is computed.
* Set(=1) ifftFlagR to calculate RIFFT, otherwise RFFT is calculated.
* \par
* The parameter <code>bitReverseFlag</code> controls whether output is in normal order or bit reversed order.
* Set(=1) bitReverseFlag for output to be in normal order otherwise output is in bit reversed order.
* \par
* This function also initializes Twiddle factor table.
*/

csky_status csky_rfft_init_f32(
  csky_rfft_instance_f32 * S,
  csky_cfft_radix4_instance_f32 * S_CFFT,
  uint32_t fftLenReal,
  uint32_t ifftFlagR,
  uint32_t bitReverseFlag)
{

  /*  Initialise the default csky status */
  csky_status status = CSKY_MATH_SUCCESS;

  /*  Initialize the Real FFT length */
  S->fftLenReal = (uint16_t) fftLenReal;

  /*  Initialize the Complex FFT length */
  S->fftLenBy2 = (uint16_t) fftLenReal / 2u;

  /*  Initialize the Twiddle coefficientA pointer */
  S->pTwiddleAReal = (float32_t *) realCoefA;

  /*  Initialize the Twiddle coefficientB pointer */
  S->pTwiddleBReal = (float32_t *) realCoefB;

  /*  Initialize the Flag for selection of RFFT or RIFFT */
  S->ifftFlagR = (uint8_t) ifftFlagR;

  /*  Initialize the Flag for calculation Bit reversal or not */
  S->bitReverseFlagR = (uint8_t) bitReverseFlag;

  /*  Initializations of structure parameters depending on the FFT length */
  switch (S->fftLenReal)
  {
    /* Init table modifier value */
  case 8192u:
    S->twidCoefRModifier = 1u;
    break;
  case 2048u:
    S->twidCoefRModifier = 4u;
    break;
  case 512u:
    S->twidCoefRModifier = 16u;
    break;
  case 128u:
    S->twidCoefRModifier = 64u;
    break;
  default:
    /*  Reporting argument error if rfftSize is not valid value */
    status = CSKY_MATH_ARGUMENT_ERROR;
    break;
  }

  /* Init Complex FFT Instance */
  S->pCfft = S_CFFT;

  if(S->ifftFlagR)
  {
    /* Initializes the CIFFT Module for fftLenreal/2 length */
    csky_cfft_radix4_init_f32(S->pCfft, S->fftLenBy2, 1u, 0u);
  }
  else
  {
    /* Initializes the CFFT Module for fftLenreal/2 length */
    csky_cfft_radix4_init_f32(S->pCfft, S->fftLenBy2, 0u, 0u);
  }

  /* return the status of RFFT Init function */
  return (status);

}

  /**
   * @} end of RealFFT group
   */
