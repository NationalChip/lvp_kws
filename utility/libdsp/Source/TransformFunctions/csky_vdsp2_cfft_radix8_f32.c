/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_cfft_radix8_f32.c
* Description:  Radix-8 Decimation in Frequency & CIFFT Floating point processing function.
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
* This file comes from arm_cfft_radix-8_f32.c. It is renamed by replacing arm with csky_vdsp2.
* And the butterfly function is moved to a specified file.
*/

/******************************************************************************
 * @file     csky_vdsp2_cfft_f32.c
 * @brief    Combined Radix Decimation in Frequency CFFT Floating point processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_vdsp2_math.h"
#include "csky_common_tables.h"

extern void csky_vdsp2_radix8_butterfly_f32(
    float32_t * pSrc,
    uint16_t fftLen,
    const float32_t * pCoef,
    uint16_t twidCoefModifier);

extern void csky_vdsp2_bitreversal_32(
    uint32_t * pSrc,
    const uint16_t bitRevLen,
    const uint16_t * pBitRevTable);

void csky_vdsp2_cfft_radix8by2_f32(
    csky_vdsp2_cfft_instance_f32 * S,
    float32_t * p1);

void csky_vdsp2_cfft_radix8by4_f32(
    csky_vdsp2_cfft_instance_f32 * S,
    float32_t * p1);
/**
* @ingroup groupTransforms
*/

/**
* @defgroup ComplexFFT Complex FFT Functions
*
* \par
* The Fast Fourier Transform (FFT) is an efficient algorithm for computing the
* Discrete Fourier Transform (DFT).  The FFT can be orders of magnitude faster
* than the DFT, especially for long lengths.
* The algorithms described in this section
* operate on complex data.  A separate set of functions is devoted to handling
* of real sequences.
* \par
* There are separate algorithms for handling floating-point, Q15, and Q31 data
* types.  The algorithms available for each data type are described next.
* \par
* The FFT functions operate in-place.  That is, the array holding the input data
* will also be used to hold the corresponding result.  The input data is complex
* and contains <code>2*fftLen</code> interleaved values as shown below.
* <pre> {real[0], imag[0], real[1], imag[1],..} </pre>
* The FFT result will be contained in the same array and the frequency domain
* values will have the same interleaving.
*
* \par Floating-point
* The floating-point complex FFT uses a mixed-radix algorithm.  Multiple radix-8
* stages are performed along with a single radix-2 or radix-4 stage, as needed.
* The algorithm supports lengths of [16, 32, 64, ..., 4096] and each length uses
* a different twiddle factor table.
* \par
* The function uses the standard FFT definition and output values may grow by a
* factor of <code>fftLen</code> when computing the forward transform.  The
* inverse transform includes a scale of <code>1/fftLen</code> as part of the
* calculation and this matches the textbook definition of the inverse FFT.
* \par
* Pre-initialized data structures containing twiddle factors and bit reversal
* tables are provided and defined in <code>csky_vdsp2_const_structs.h</code>.  Include
* this header in your function and then pass one of the constant structures as
* an argument to csky_vdsp2_cfft_f32.  For example:
* \par
* <code>csky_vdsp2_cfft_f32(csky_vdsp2_cfft_sR_f32_len64, pSrc, 1, 1)</code>
* \par
* computes a 64-point inverse complex FFT including bit reversal.
* The data structures are treated as constant data and not modified during the
* calculation.  The same data structure can be reused for multiple transforms
* including mixing forward and inverse transforms.
* \par
* Earlier releases of the library provided separate radix-2 and radix-4
* algorithms that operated on floating-point data.  These functions are still
* provided but are deprecated.  The older functions are slower and less general
* than the new functions.
* \par
* An example of initialization of the constants for the csky_vdsp2_cfft_f32 function follows:
* \code
* const static csky_vdsp2_cfft_instance_f32 *S;
* ...
*   switch (length) {
*     case 16:
*       S = &csky_vdsp2_cfft_sR_f32_len16;
*       break;
*     case 32:
*       S = &csky_vdsp2_cfft_sR_f32_len32;
*       break;
*     case 64:
*       S = &csky_vdsp2_cfft_sR_f32_len64;
*       break;
*     case 128:
*       S = &csky_vdsp2_cfft_sR_f32_len128;
*       break;
*     case 256:
*       S = &csky_vdsp2_cfft_sR_f32_len256;
*       break;
*     case 512:
*       S = &csky_vdsp2_cfft_sR_f32_len512;
*       break;
*     case 1024:
*       S = &csky_vdsp2_cfft_sR_f32_len1024;
*       break;
*     case 2048:
*       S = &csky_vdsp2_cfft_sR_f32_len2048;
*       break;
*     case 4096:
*       S = &csky_vdsp2_cfft_sR_f32_len4096;
*       break;
*   }
* \endcode
* \par Q15 and Q31
* The floating-point complex FFT uses a mixed-radix algorithm.  Multiple radix-4
* stages are performed along with a single radix-2 stage, as needed.
* The algorithm supports lengths of [16, 32, 64, ..., 4096] and each length uses
* a different twiddle factor table.
* \par
* The function uses the standard FFT definition and output values may grow by a
* factor of <code>fftLen</code> when computing the forward transform.  The
* inverse transform includes a scale of <code>1/fftLen</code> as part of the
* calculation and this matches the textbook definition of the inverse FFT.
* \par
* Pre-initialized data structures containing twiddle factors and bit reversal
* tables are provided and defined in <code>csky_vdsp2_const_structs.h</code>.  Include
* this header in your function and then pass one of the constant structures as
* an argument to csky_vdsp2_cfft_q31.  For example:
* \par
* <code>csky_vdsp2_cfft_q31(csky_vdsp2_cfft_sR_q31_len64, pSrc, 1, 1)</code>
* \par
* computes a 64-point inverse complex FFT including bit reversal.
* The data structures are treated as constant data and not modified during the
* calculation.  The same data structure can be reused for multiple transforms
* including mixing forward and inverse transforms.
* \par
* Earlier releases of the library provided separate radix-2 and radix-4
* algorithms that operated on floating-point data.  These functions are still
* provided but are deprecated.  The older functions are slower and less general
* than the new functions.
* \par
* An example of initialization of the constants for the csky_vdsp2_cfft_q31 function follows:
* \code
* const static csky_vdsp2_cfft_instance_q31 *S;
* ...
*   switch (length) {
*     case 16:
*       S = &csky_vdsp2_cfft_sR_q31_len16;
*       break;
*     case 32:
*       S = &csky_vdsp2_cfft_sR_q31_len32;
*       break;
*     case 64:
*       S = &csky_vdsp2_cfft_sR_q31_len64;
*       break;
*     case 128:
*       S = &csky_vdsp2_cfft_sR_q31_len128;
*       break;
*     case 256:
*       S = &csky_vdsp2_cfft_sR_q31_len256;
*       break;
*     case 512:
*       S = &csky_vdsp2_cfft_sR_q31_len512;
*       break;
*     case 1024:
*       S = &csky_vdsp2_cfft_sR_q31_len1024;
*       break;
*     case 2048:
*       S = &csky_vdsp2_cfft_sR_q31_len2048;
*       break;
*     case 4096:
*       S = &csky_vdsp2_cfft_sR_q31_len4096;
*       break;
*   }
* \endcode
*
*/

/**
* @addtogroup ComplexFFT
* @{
*/

/**
* @details
* @brief       Processing function for the floating-point complex FFT.
* @param[in]      *S    points to an instance of the floating-point CFFT structure.
* @param[in, out] *p1   points to the complex data buffer of size <code>2*fftLen</code>. Processing occurs in-place.
* @param[in]     ifftFlag       flag that selects forward (ifftFlag=0) or inverse (ifftFlag=1) transform.
* @param[in]     bitReverseFlag flag that enables (bitReverseFlag=1) or disables (bitReverseFlag=0) bit reversal of output.
* @return none.
*/

void csky_vdsp2_cfft_f32(
    const csky_vdsp2_cfft_instance_f32 * S,
    float32_t * p1,
    uint8_t ifftFlag,
    uint8_t bitReverseFlag)
{
    uint32_t  L = S->fftLen, l;
    float32_t invL, * pSrc;

    if(ifftFlag == 1u)
    {
        /*  Conjugate input data  */
        pSrc = p1 + 1;
        for(l=0; l<L; l++)
        {
            *pSrc = -*pSrc;
            pSrc += 2;
        }
    }

    switch (L)
    {
    case 16:
    case 128:
    case 1024:
        csky_vdsp2_cfft_radix8by2_f32  ( (csky_vdsp2_cfft_instance_f32 *) S, p1);
        break;
    case 32:
    case 256:
    case 2048:
        csky_vdsp2_cfft_radix8by4_f32  ( (csky_vdsp2_cfft_instance_f32 *) S, p1);
        break;
    case 64:
    case 512:
    case 4096:
        csky_vdsp2_radix8_butterfly_f32( p1, L, (float32_t *) S->pTwiddle, 1);
        break;
    }

    if( bitReverseFlag )
        csky_vdsp2_bitreversal_32((uint32_t*)p1,S->bitRevLength,S->pBitRevTable);

    if(ifftFlag == 1u)
    {
        invL = 1.0f/(float32_t)L;
        /*  Conjugate and scale output data */
        pSrc = p1;
        for(l=0; l<L; l++)
        {
            *pSrc++ *=   invL ;
            *pSrc  = -(*pSrc) * invL;
            pSrc++;
        }
    }
}

/**
* @} end of ComplexFFT group
*/
