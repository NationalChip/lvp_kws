/*
 * Copyright (C) 2016-2019 C-SKY Limited. All rights reserved.
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
 */

/******************************************************************************
 * @file     csky_cfft_radix4by2_q31.c
 * @brief    Combined Radix Decimation in Frequency CFFT fixed point processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

void csky_cfft_radix4by2_q31(
    q31_t * pSrc,
    uint32_t fftLen,
    const q31_t * pCoef)
{
    uint32_t i, l;
    uint32_t n2, ia;
    q31_t xt, yt, cosVal, sinVal;
    q31_t p0, p1;

    n2 = fftLen >> 1;
    ia = 0;
    for (i = 0; i < n2; i++)
    {
        cosVal = pCoef[2*ia];
        sinVal = pCoef[2*ia + 1];
        ia++;

        l = i + n2;
        xt = (pSrc[2 * i] >> 2) - (pSrc[2 * l] >> 2);
        pSrc[2 * i] = (pSrc[2 * i] >> 2) + (pSrc[2 * l] >> 2);

        yt = (pSrc[2 * i + 1] >> 2) - (pSrc[2 * l + 1] >> 2);
        pSrc[2 * i + 1] = (pSrc[2 * l + 1] >> 2) + (pSrc[2 * i + 1] >> 2);

        mult_32x32_keep32_R(p0, xt, cosVal);
        mult_32x32_keep32_R(p1, yt, cosVal);
        multAcc_32x32_keep32_R(p0, yt, sinVal);
        multSub_32x32_keep32_R(p1, xt, sinVal);

        pSrc[2u * l] = p0 << 1;
        pSrc[2u * l + 1u] = p1 << 1;

    }

    // first col
    csky_radix4_butterfly_q31( pSrc, n2, (q31_t*)pCoef, 2u);
    // second col
    csky_radix4_butterfly_q31( pSrc + fftLen, n2, (q31_t*)pCoef, 2u);

    for (i = 0; i < fftLen >> 1; i++)
    {
        p0 = pSrc[4*i+0];
        p1 = pSrc[4*i+1];
        xt = pSrc[4*i+2];
        yt = pSrc[4*i+3];

        p0 <<= 1;
        p1 <<= 1;
        xt <<= 1;
        yt <<= 1;

        pSrc[4*i+0] = p0;
        pSrc[4*i+1] = p1;
        pSrc[4*i+2] = xt;
        pSrc[4*i+3] = yt;
    }

}

void csky_cfft_radix4by2_inverse_q31(
    q31_t * pSrc,
    uint32_t fftLen,
    const q31_t * pCoef)
{
    uint32_t i, l;
    uint32_t n2, ia;
    q31_t xt, yt, cosVal, sinVal;
    q31_t p0, p1;

    n2 = fftLen >> 1;
    ia = 0;
    for (i = 0; i < n2; i++)
    {
        cosVal = pCoef[2*ia];
        sinVal = pCoef[2*ia + 1];
        ia++;

        l = i + n2;
        xt = (pSrc[2 * i] >> 2) - (pSrc[2 * l] >> 2);
        pSrc[2 * i] = (pSrc[2 * i] >> 2) + (pSrc[2 * l] >> 2);

        yt = (pSrc[2 * i + 1] >> 2) - (pSrc[2 * l + 1] >> 2);
        pSrc[2 * i + 1] = (pSrc[2 * l + 1] >> 2) + (pSrc[2 * i + 1] >> 2);

        mult_32x32_keep32_R(p0, xt, cosVal);
        mult_32x32_keep32_R(p1, yt, cosVal);
        multSub_32x32_keep32_R(p0, yt, sinVal);
        multAcc_32x32_keep32_R(p1, xt, sinVal);

        pSrc[2u * l] = p0 << 1;
        pSrc[2u * l + 1u] = p1 << 1;

    }

    // first col
    csky_radix4_butterfly_inverse_q31( pSrc, n2, (q31_t*)pCoef, 2u);
    // second col
    csky_radix4_butterfly_inverse_q31( pSrc + fftLen, n2, (q31_t*)pCoef, 2u);

    for (i = 0; i < fftLen >> 1; i++)
    {
        p0 = pSrc[4*i+0];
        p1 = pSrc[4*i+1];
        xt = pSrc[4*i+2];
        yt = pSrc[4*i+3];

        p0 <<= 1;
        p1 <<= 1;
        xt <<= 1;
        yt <<= 1;

        pSrc[4*i+0] = p0;
        pSrc[4*i+1] = p1;
        pSrc[4*i+2] = xt;
        pSrc[4*i+3] = yt;
    }
}
