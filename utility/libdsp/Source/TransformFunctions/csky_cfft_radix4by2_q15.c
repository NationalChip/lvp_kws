/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_cfft_q15.c
* Description:  Combined Radix Decimation in Q15 Frequency CFFT processing function
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
* This file comes from arm_cfft_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_cfft_radix4by2_q15.c
 * @brief    Combined Radix Decimation in Q15 Frequency CFFT processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

void csky_cfft_radix4by2_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    const q15_t * pCoef)
{
    uint32_t i;
    uint32_t n2;
    q15_t p0, p1, p2, p3;
    uint32_t ia, l;
    q15_t xt, yt, cosVal, sinVal;

    n2 = fftLen >> 1;

    ia = 0;
    for (i = 0; i < n2; i++)
    {
        cosVal = pCoef[ia * 2];
        sinVal = pCoef[(ia * 2) + 1];
        ia++;

        l = i + n2;

        xt = (pSrc[2 * i] >> 1u) - (pSrc[2 * l] >> 1u);
        pSrc[2 * i] = ((pSrc[2 * i] >> 1u) + (pSrc[2 * l] >> 1u)) >> 1u;

        yt = (pSrc[2 * i + 1] >> 1u) - (pSrc[2 * l + 1] >> 1u);
        pSrc[2 * i + 1] =
        ((pSrc[2 * l + 1] >> 1u) + (pSrc[2 * i + 1] >> 1u)) >> 1u;

        pSrc[2u * l] = (((int16_t) (((q31_t) xt * cosVal) >> 16)) +
                  ((int16_t) (((q31_t) yt * sinVal) >> 16)));

        pSrc[2u * l + 1u] = (((int16_t) (((q31_t) yt * cosVal) >> 16)) -
                       ((int16_t) (((q31_t) xt * sinVal) >> 16)));
    }


    // first col
    csky_radix4_butterfly_q15( pSrc, n2, (q15_t*)pCoef, 2u);
    // second col
    csky_radix4_butterfly_q15( pSrc + fftLen, n2, (q15_t*)pCoef, 2u);

    for (i = 0; i < fftLen >> 1; i++)
    {
        p0 = pSrc[4*i+0];
        p1 = pSrc[4*i+1];
        p2 = pSrc[4*i+2];
        p3 = pSrc[4*i+3];

        p0 <<= 1;
        p1 <<= 1;
        p2 <<= 1;
        p3 <<= 1;

        pSrc[4*i+0] = p0;
        pSrc[4*i+1] = p1;
        pSrc[4*i+2] = p2;
        pSrc[4*i+3] = p3;
    }
}

void csky_cfft_radix4by2_inverse_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    const q15_t * pCoef)
{
    uint32_t i;
    uint32_t n2;
    q15_t p0, p1, p2, p3;
    uint32_t ia, l;
    q15_t xt, yt, cosVal, sinVal;

    n2 = fftLen >> 1;


    ia = 0;
    for (i = 0; i < n2; i++)
    {
        cosVal = pCoef[ia * 2];
        sinVal = pCoef[(ia * 2) + 1];
        ia++;

        l = i + n2;
        xt = (pSrc[2 * i] >> 1u) - (pSrc[2 * l] >> 1u);
        pSrc[2 * i] = ((pSrc[2 * i] >> 1u) + (pSrc[2 * l] >> 1u)) >> 1u;

        yt = (pSrc[2 * i + 1] >> 1u) - (pSrc[2 * l + 1] >> 1u);
        pSrc[2 * i + 1] =
          ((pSrc[2 * l + 1] >> 1u) + (pSrc[2 * i + 1] >> 1u)) >> 1u;

        pSrc[2u * l] = (((int16_t) (((q31_t) xt * cosVal) >> 16)) -
                        ((int16_t) (((q31_t) yt * sinVal) >> 16)));

        pSrc[2u * l + 1u] = (((int16_t) (((q31_t) yt * cosVal) >> 16)) +
                           ((int16_t) (((q31_t) xt * sinVal) >> 16)));
    }

    // first col
    csky_radix4_butterfly_inverse_q15( pSrc, n2, (q15_t*)pCoef, 2u);
    // second col
    csky_radix4_butterfly_inverse_q15( pSrc + fftLen, n2, (q15_t*)pCoef, 2u);

    for (i = 0; i < fftLen >> 1; i++)
    {
        p0 = pSrc[4*i+0];
        p1 = pSrc[4*i+1];
        p2 = pSrc[4*i+2];
        p3 = pSrc[4*i+3];

        p0 <<= 1;
        p1 <<= 1;
        p2 <<= 1;
        p3 <<= 1;

        pSrc[4*i+0] = p0;
        pSrc[4*i+1] = p1;
        pSrc[4*i+2] = p2;
        pSrc[4*i+3] = p3;
    }
}
