/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_rfft_init_q15.c
* Description:  RFFT & RIFFT Q15 initialisation function
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
* This file comes from arm_rfft_init_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_rfft_init_q15.c
 * @brief    RFFT & RIFFT Q15 initialisation function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/


#include "csky_math.h"
#include "csky_common_tables.h"
#include "csky_const_structs.h"

/**
* @ingroup groupTransforms
*/

/**
* @addtogroup RealFFT
* @{
*/

/**
* @brief  Initialization function for the Q15 RFFT/RIFFT.
* @param[in, out] *S             points to an instance of the Q15 RFFT/RIFFT structure.
* @param[in]      fftLenReal     length of the FFT.
* @param[in]      ifftFlagR      flag that selects forward (ifftFlagR=0) or inverse (ifftFlagR=1) transform.
* @param[in]      bitReverseFlag flag that enables (bitReverseFlag=1) or disables (bitReverseFlag=0) bit reversal of output.
* @return		The function returns CSKY_MATH_SUCCESS if initialization is successful or CSKY_MATH_ARGUMENT_ERROR if <code>fftLenReal</code> is not a supported value.
*
* \par Description:
* \par
* The parameter <code>fftLenReal</code>	Specifies length of RFFT/RIFFT Process. Supported FFT Lengths are 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192.
* \par
* The parameter <code>ifftFlagR</code> controls whether a forward or inverse transform is computed.
* Set(=1) ifftFlagR to calculate RIFFT, otherwise RFFT is calculated.
* \par
* The parameter <code>bitReverseFlag</code> controls whether output is in normal order or bit reversed order.
* Set(=1) bitReverseFlag for output to be in normal order otherwise output is in bit reversed order.
* \par
* This function also initializes Twiddle factor table.
*
* \note
* This initialization function is recommended not to used, the function csky_rfft_q15() can be used  straightforwardly by the manully initialized instance instructions for each length.
*/

csky_status csky_rfft_init_q15(
    csky_rfft_instance_q15 * S,
    uint32_t fftLenReal,
    uint32_t ifftFlagR,
    uint32_t bitReverseFlag)
{
    /*  Initialise the default csky status */
    csky_status status = CSKY_MATH_SUCCESS;

    /*  Initialize the Real FFT length */
    S->fftLenReal = (uint16_t) fftLenReal;

    /*  Initialize the Twiddle coefficientA pointer */
    S->pTwiddleAReal = (q15_t *) realCoefAQ15_8192;

    /*  Initialize the Flag for selection of RFFT or RIFFT */
    S->ifftFlagR = (uint8_t) ifftFlagR;

    /*  Initialize the Flag for calculation Bit reversal or not */
    S->bitReverseFlagR = (uint8_t) bitReverseFlag;

    /*  Initialization of coef modifier depending on the FFT length */
    switch (S->fftLenReal)
    {
    case 8192u:
        S->twidCoefRModifier = 1u;
        S->pCfft = &csky_cfft_sR_q15_len4096;
        break;
    case 4096u:
        S->twidCoefRModifier = 2u;
        S->pCfft = &csky_cfft_sR_q15_len2048;
        break;
    case 2048u:
        S->twidCoefRModifier = 4u;
        S->pCfft = &csky_cfft_sR_q15_len1024;
        break;
    case 1024u:
        S->twidCoefRModifier = 8u;
        S->pCfft = &csky_cfft_sR_q15_len512;
        break;
    case 512u:
        S->twidCoefRModifier = 16u;
        S->pCfft = &csky_cfft_sR_q15_len256;
        break;
    case 256u:
        S->twidCoefRModifier = 32u;
        S->pCfft = &csky_cfft_sR_q15_len128;
        break;
    case 128u:
        S->twidCoefRModifier = 64u;
        S->pCfft = &csky_cfft_sR_q15_len64;
        break;
    case 64u:
        S->twidCoefRModifier = 128u;
        S->pCfft = &csky_cfft_sR_q15_len32;
        break;
    case 32u:
        S->twidCoefRModifier = 256u;
        S->pCfft = &csky_cfft_sR_q15_len16;
        break;
    default:
        /*  Reporting argument error if rfftSize is not valid value */
        status = CSKY_MATH_ARGUMENT_ERROR;
        break;
    }

    /* return the status of RFFT Init function */
    return (status);
}

/**
* @} end of RealFFT group
*/
