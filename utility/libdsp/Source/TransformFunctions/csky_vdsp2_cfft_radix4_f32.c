/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_cfft_radix4_f32.c
* Description:  Radix4 Decimation in Freqquency CFFT & CIFFT Floating point processing function
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
* This file comes from arm_cfft_radix4_f32.c. It is renamed by replacing arm with
* csky_vdsp2.
*
*/

/******************************************************************************
 * @file     csky_vdsp2_cfft_radix4_f32.c
 * @brief    Radix-4 Decimation in Frequency CFFT & CIFFT
 *           Floating point processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_vdsp2_math.h"

extern void csky_vdsp2_bitreversal_32(
uint32_t * pSrc,
int fftSize,
uint16_t * pBitRevTab);

extern void csky_vdsp2_cfft_radix4by2_f32(
float32_t * pSrc,
uint32_t fftLen,
const float32_t * pCoef);

extern void csky_vdsp2_cfft_radix4by2_inverse_f32(
float32_t * pSrc,
uint32_t fftLen,
const float32_t * pCoef,
float32_t onebyfftLen);


/* ----------------------------------------------------------------------
** Internal helper function used by the FFTs
** ------------------------------------------------------------------- */

/*
* @brief  Core function for the floating-point CFFT butterfly process.
* @param[in, out] *pSrc            points to the in-place buffer of floating-point data type.
* @param[in]      fftLen           length of the FFT.
* @param[in]      *pCoef           points to the twiddle coefficient buffer.
* @param[in]      twidCoefModifier twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
* @return none.
*/

void csky_vdsp2_radix4_butterfly_f32(
float32_t * pSrc,
uint16_t fftLen,
float32_t * pCoef,
uint16_t twidCoefModifier);

/*
* @brief  Core function for the floating-point CIFFT butterfly process.
* @param[in, out] *pSrc            points to the in-place buffer of floating-point data type.
* @param[in]      fftLen           length of the FFT.
* @param[in]      *pCoef           points to twiddle coefficient buffer.
* @param[in]      twidCoefModifier twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
* @param[in]      onebyfftLen      value of 1/fftLen.
* @return none.
*/

void csky_vdsp2_radix4_butterfly_inverse_f32(
float32_t * pSrc,
uint16_t fftLen,
float32_t * pCoef,
uint16_t twidCoefModifier,
float32_t onebyfftLen);
/**
* @details
* @brief Processing function for the floating-point Radix-4 CFFT/CIFFT.
* @deprecated Do not use this function.  It has been superseded by \ref csky_vdsp2_cfft_f32 and will be removed
* in the future.
* @param[in]      *S    points to an instance of the floating-point Radix-4 CFFT/CIFFT structure.
* @param[in, out] *pSrc points to the complex data buffer of size <code>2*fftLen</code>. Processing occurs in-place.
* @return none.
*/

void csky_vdsp2_cfft_radix4_f32(
    const csky_vdsp2_cfft_instance_f32 * S,
    float32_t * p1,
    uint8_t ifftFlag,
    uint8_t bitReverseFlag,
    float32_t onebyfftLen)
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
            csky_vdsp2_radix4_butterfly_inverse_f32  ( p1, L, (float32_t*)S->pTwiddle, 1 , onebyfftLen);
            break;

        case 32:
        case 128:
        case 512:
        case 2048:
            csky_vdsp2_cfft_radix4by2_inverse_f32  ( p1, L, S->pTwiddle, onebyfftLen);
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
            csky_vdsp2_radix4_butterfly_f32  ( p1, L, (float32_t*)S->pTwiddle, 1 );
            break;

        case 32:
        case 128:
        case 512:
        case 2048:
            csky_vdsp2_cfft_radix4by2_f32  ( p1, L, S->pTwiddle );
            break;
        }
    }

    if( bitReverseFlag)
        csky_vdsp2_bitreversal_32((uint32_t *)p1,S->bitRevLength, (uint16_t *)S->pBitRevTable);
}
