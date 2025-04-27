/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_cfft_q31.c
* Description:  Combined Radix Decimation in Q31 Frequency CFFT processing function
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
* This file comes from arm_cfft_q31.c. It is renamed by replacing arm with csky_vdsp2.
* And move the radix decimation function to a specified file.
*
*/

/******************************************************************************
 * @file     csky_vdsp2_cfft_q31.c
 * @brief    Combined Radix Decimation in Frequency CFFT fixed point processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_vdsp2_math.h"

extern void csky_vdsp2_radix4_butterfly_q31(
    q31_t * pSrc,
    uint32_t fftLen,
    q31_t * pCoef,
    uint32_t twidCoefModifier);

extern void csky_vdsp2_radix4_butterfly_inverse_q31(
    q31_t * pSrc,
    uint32_t fftLen,
    q31_t * pCoef,
    uint32_t twidCoefModifier);

extern void csky_vdsp2_bitreversal_32(
    uint32_t * pSrc,
    const uint16_t bitRevLen,
    const uint16_t * pBitRevTable);

void csky_vdsp2_cfft_radix4by2_q31(
    q31_t * pSrc,
    uint32_t fftLen,
    const q31_t * pCoef);

void csky_vdsp2_cfft_radix4by2_inverse_q31(
    q31_t * pSrc,
    uint32_t fftLen,
    const q31_t * pCoef);

/**
* @ingroup groupTransforms
*/

/**
* @addtogroup ComplexFFT
* @{
*/

/**
* @details
* @brief       Processing function for the fixed-point complex FFT in Q31 format.
* @param[in]      *S    points to an instance of the fixed-point CFFT structure.
* @param[in, out] *p1   points to the complex data buffer of size <code>2*fftLen</code>. Processing occurs in-place.
* @param[in]     ifftFlag       flag that selects forward (ifftFlag=0) or inverse (ifftFlag=1) transform.
* @param[in]     bitReverseFlag flag that enables (bitReverseFlag=1) or disables (bitReverseFlag=0) bit reversal of output.
* @return none.
*/

void csky_vdsp2_cfft_q31(
    const csky_vdsp2_cfft_instance_q31 * S,
    q31_t * p1,
    uint8_t ifftFlag,
    uint8_t bitReverseFlag)
{
    uint32_t L = S->fftLen;

    if(ifftFlag == 1u)
    {
        switch (L)
        {
        case 16:
        case 64:
        case 256:
        case 1024:
        case 4096:
            csky_vdsp2_radix4_butterfly_inverse_q31  ( p1, L, (q31_t*)S->pTwiddle, 1 );
            break;

        case 32:
        case 128:
        case 512:
        case 2048:
            csky_vdsp2_cfft_radix4by2_inverse_q31  ( p1, L, S->pTwiddle );
            break;
        }
    }
    else
    {
        switch (L)
        {
        case 16:
        case 64:
        case 256:
        case 1024:
        case 4096:
            csky_vdsp2_radix4_butterfly_q31  ( p1, L, (q31_t*)S->pTwiddle, 1 );
            break;

        case 32:
        case 128:
        case 512:
        case 2048:
            csky_vdsp2_cfft_radix4by2_q31  ( p1, L, S->pTwiddle );
            break;
        }
    }

    if( bitReverseFlag )
        csky_vdsp2_bitreversal_32((uint32_t*)p1,S->bitRevLength,S->pBitRevTable);
}

/**
* @} end of ComplexFFT group
*/

