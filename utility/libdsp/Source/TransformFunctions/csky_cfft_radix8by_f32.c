/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_cfft_f32.c
* Description:  Combined Radix Decimation is Frequency CFFT Floating point processing function
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
* This file comes from arm_cfft_f32. And only the radix decimation functions are moved
* into this file.
*
*/

/******************************************************************************
 * @file     csky_vdsp2_cfft_f32.c
 * @brief    Combined Radix Decimation in Frequency CFFT Floating point processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_vdsp2_math.h"
#include "csky_common_tables.h"

void csky_vdsp2_cfft_radix8by2_f32( csky_vdsp2_cfft_instance_f32 * S, float32_t * p1)
{
    uint32_t    L  = S->fftLen;
    float32_t * pCol1, * pCol2, * pMid1, * pMid2;
    float32_t * p2 = p1 + L;
    const float32_t * tw = (float32_t *) S->pTwiddle;
    float32_t t1[4], t2[4], t3[4], t4[4], twR, twI;
    float32_t m0, m1, m2, m3;
    uint32_t l;

    pCol1 = p1;
    pCol2 = p2;

    //    Define new length
    L >>= 1;
    //    Initialize mid pointers
    pMid1 = p1 + L;
    pMid2 = p2 + L;

    // do two dot Fourier transform
    for ( l = L >> 2; l > 0; l-- )
    {
        t1[0] = p1[0];
        t1[1] = p1[1];
        t1[2] = p1[2];
        t1[3] = p1[3];

        t2[0] = p2[0];
        t2[1] = p2[1];
        t2[2] = p2[2];
        t2[3] = p2[3];

        t3[0] = pMid1[0];
        t3[1] = pMid1[1];
        t3[2] = pMid1[2];
        t3[3] = pMid1[3];

        t4[0] = pMid2[0];
        t4[1] = pMid2[1];
        t4[2] = pMid2[2];
        t4[3] = pMid2[3];

        *p1++ = t1[0] + t2[0];
        *p1++ = t1[1] + t2[1];
        *p1++ = t1[2] + t2[2];
        *p1++ = t1[3] + t2[3];    // col 1

        t2[0] = t1[0] - t2[0];
        t2[1] = t1[1] - t2[1];
        t2[2] = t1[2] - t2[2];
        t2[3] = t1[3] - t2[3];    // for col 2

        *pMid1++ = t3[0] + t4[0];
        *pMid1++ = t3[1] + t4[1];
        *pMid1++ = t3[2] + t4[2];
        *pMid1++ = t3[3] + t4[3]; // col 1

        t4[0] = t4[0] - t3[0];
        t4[1] = t4[1] - t3[1];
        t4[2] = t4[2] - t3[2];
        t4[3] = t4[3] - t3[3];    // for col 2

        twR = *tw++;
        twI = *tw++;

        // multiply by twiddle factors
        m0 = t2[0] * twR;
        m1 = t2[1] * twI;
        m2 = t2[1] * twR;
        m3 = t2[0] * twI;

        // R  =  R  *  Tr - I * Ti
        *p2++ = m0 + m1;
        // I  =  I  *  Tr + R * Ti
        *p2++ = m2 - m3;

        // use vertical symmetry
        //  0.9988 - 0.0491i <==> -0.0491 - 0.9988i
        m0 = t4[0] * twI;
        m1 = t4[1] * twR;
        m2 = t4[1] * twI;
        m3 = t4[0] * twR;

        *pMid2++ = m0 - m1;
        *pMid2++ = m2 + m3;

        twR = *tw++;
        twI = *tw++;

        m0 = t2[2] * twR;
        m1 = t2[3] * twI;
        m2 = t2[3] * twR;
        m3 = t2[2] * twI;

        *p2++ = m0 + m1;
        *p2++ = m2 - m3;

        m0 = t4[2] * twI;
        m1 = t4[3] * twR;
        m2 = t4[3] * twI;
        m3 = t4[2] * twR;

        *pMid2++ = m0 - m1;
        *pMid2++ = m2 + m3;
    }

    // first col
    csky_vdsp2_radix8_butterfly_f32( pCol1, L, (float32_t *) S->pTwiddle, 2u);
    // second col
    csky_vdsp2_radix8_butterfly_f32( pCol2, L, (float32_t *) S->pTwiddle, 2u);
}

void csky_vdsp2_cfft_radix8by4_f32( csky_vdsp2_cfft_instance_f32 * S, float32_t * p1)
{
    uint32_t    L  = S->fftLen >> 1;
    float32_t * pCol1, *pCol2, *pCol3, *pCol4, *pEnd1, *pEnd2, *pEnd3, *pEnd4;
    const float32_t *tw2, *tw3, *tw4;
    float32_t * p2 = p1 + L;
    float32_t * p3 = p2 + L;
    float32_t * p4 = p3 + L;
    float32_t t2[4], t3[4], t4[4], twR, twI;
    float32_t p1ap3_0, p1sp3_0, p1ap3_1, p1sp3_1;
    float32_t m0, m1, m2, m3;
    uint32_t l, twMod2, twMod3, twMod4;

    pCol1 = p1;         // points to real values by default
    pCol2 = p2;
    pCol3 = p3;
    pCol4 = p4;
    pEnd1 = p2 - 1;     // points to imaginary values by default
    pEnd2 = p3 - 1;
    pEnd3 = p4 - 1;
    pEnd4 = pEnd3 + L;

    tw2 = tw3 = tw4 = (float32_t *) S->pTwiddle;

    L >>= 1;

    // do four dot Fourier transform

    twMod2 = 2;
    twMod3 = 4;
    twMod4 = 6;

    // TOP
    p1ap3_0 = p1[0] + p3[0];
    p1sp3_0 = p1[0] - p3[0];
    p1ap3_1 = p1[1] + p3[1];
    p1sp3_1 = p1[1] - p3[1];

    // col 2
    t2[0] = p1sp3_0 + p2[1] - p4[1];
    t2[1] = p1sp3_1 - p2[0] + p4[0];
    // col 3
    t3[0] = p1ap3_0 - p2[0] - p4[0];
    t3[1] = p1ap3_1 - p2[1] - p4[1];
    // col 4
    t4[0] = p1sp3_0 - p2[1] + p4[1];
    t4[1] = p1sp3_1 + p2[0] - p4[0];
    // col 1
    *p1++ = p1ap3_0 + p2[0] + p4[0];
    *p1++ = p1ap3_1 + p2[1] + p4[1];

    // Twiddle factors are ones
    *p2++ = t2[0];
    *p2++ = t2[1];
    *p3++ = t3[0];
    *p3++ = t3[1];
    *p4++ = t4[0];
    *p4++ = t4[1];

    tw2 += twMod2;
    tw3 += twMod3;
    tw4 += twMod4;

    for (l = (L - 2) >> 1; l > 0; l-- )
    {
        // TOP
        p1ap3_0 = p1[0] + p3[0];
        p1sp3_0 = p1[0] - p3[0];
        p1ap3_1 = p1[1] + p3[1];
        p1sp3_1 = p1[1] - p3[1];
        // col 2
        t2[0] = p1sp3_0 + p2[1] - p4[1];
        t2[1] = p1sp3_1 - p2[0] + p4[0];
        // col 3
        t3[0] = p1ap3_0 - p2[0] - p4[0];
        t3[1] = p1ap3_1 - p2[1] - p4[1];
        // col 4
        t4[0] = p1sp3_0 - p2[1] + p4[1];
        t4[1] = p1sp3_1 + p2[0] - p4[0];
        // col 1 - top
        *p1++ = p1ap3_0 + p2[0] + p4[0];
        *p1++ = p1ap3_1 + p2[1] + p4[1];

        // BOTTOM
        p1ap3_1 = pEnd1[-1] + pEnd3[-1];
        p1sp3_1 = pEnd1[-1] - pEnd3[-1];
        p1ap3_0 = pEnd1[0] + pEnd3[0];
        p1sp3_0 = pEnd1[0] - pEnd3[0];
        // col 2
        t2[2] = pEnd2[0]  - pEnd4[0] + p1sp3_1;
        t2[3] = pEnd1[0] - pEnd3[0] - pEnd2[-1] + pEnd4[-1];
        // col 3
        t3[2] = p1ap3_1 - pEnd2[-1] - pEnd4[-1];
        t3[3] = p1ap3_0 - pEnd2[0]  - pEnd4[0];
        // col 4
        t4[2] = pEnd2[0]  - pEnd4[0]  - p1sp3_1;
        t4[3] = pEnd4[-1] - pEnd2[-1] - p1sp3_0;
        // col 1 - Bottom
        *pEnd1-- = p1ap3_0 + pEnd2[0] + pEnd4[0];
        *pEnd1-- = p1ap3_1 + pEnd2[-1] + pEnd4[-1];

        // COL 2
        // read twiddle factors
        twR = *tw2++;
        twI = *tw2++;
        // multiply by twiddle factors
        //  let    Z1 = a + i(b),   Z2 = c + i(d)
        //   =>  Z1 * Z2  =  (a*c - b*d) + i(b*c + a*d)

        // Top
        m0 = t2[0] * twR;
        m1 = t2[1] * twI;
        m2 = t2[1] * twR;
        m3 = t2[0] * twI;

        *p2++ = m0 + m1;
        *p2++ = m2 - m3;
        // use vertical symmetry col 2
        // 0.9997 - 0.0245i  <==>  0.0245 - 0.9997i
        // Bottom
        m0 = t2[3] * twI;
        m1 = t2[2] * twR;
        m2 = t2[2] * twI;
        m3 = t2[3] * twR;

        *pEnd2-- = m0 - m1;
        *pEnd2-- = m2 + m3;

        // COL 3
        twR = tw3[0];
        twI = tw3[1];
        tw3 += twMod3;
        // Top
        m0 = t3[0] * twR;
        m1 = t3[1] * twI;
        m2 = t3[1] * twR;
        m3 = t3[0] * twI;

        *p3++ = m0 + m1;
        *p3++ = m2 - m3;
        // use vertical symmetry col 3
        // 0.9988 - 0.0491i  <==>  -0.9988 - 0.0491i
        // Bottom
        m0 = -t3[3] * twR;
        m1 = t3[2] * twI;
        m2 = t3[2] * twR;
        m3 = t3[3] * twI;

        *pEnd3-- = m0 - m1;
        *pEnd3-- = m3 - m2;

        // COL 4
        twR = tw4[0];
        twI = tw4[1];
        tw4 += twMod4;
        // Top
        m0 = t4[0] * twR;
        m1 = t4[1] * twI;
        m2 = t4[1] * twR;
        m3 = t4[0] * twI;

        *p4++ = m0 + m1;
        *p4++ = m2 - m3;
        // use vertical symmetry col 4
        // 0.9973 - 0.0736i  <==>  -0.0736 + 0.9973i
        // Bottom
        m0 = t4[3] * twI;
        m1 = t4[2] * twR;
        m2 = t4[2] * twI;
        m3 = t4[3] * twR;

        *pEnd4-- = m0 - m1;
        *pEnd4-- = m2 + m3;
    }

    //MIDDLE
    // Twiddle factors are
    //  1.0000  0.7071-0.7071i  -1.0000i  -0.7071-0.7071i
    p1ap3_0 = p1[0] + p3[0];
    p1sp3_0 = p1[0] - p3[0];
    p1ap3_1 = p1[1] + p3[1];
    p1sp3_1 = p1[1] - p3[1];

    // col 2
    t2[0] = p1sp3_0 + p2[1] - p4[1];
    t2[1] = p1sp3_1 - p2[0] + p4[0];
    // col 3
    t3[0] = p1ap3_0 - p2[0] - p4[0];
    t3[1] = p1ap3_1 - p2[1] - p4[1];
    // col 4
    t4[0] = p1sp3_0 - p2[1] + p4[1];
    t4[1] = p1sp3_1 + p2[0] - p4[0];
    // col 1 - Top
    *p1++ = p1ap3_0 + p2[0] + p4[0];
    *p1++ = p1ap3_1 + p2[1] + p4[1];

    // COL 2
    twR = tw2[0];
    twI = tw2[1];

    m0 = t2[0] * twR;
    m1 = t2[1] * twI;
    m2 = t2[1] * twR;
    m3 = t2[0] * twI;

    *p2++ = m0 + m1;
    *p2++ = m2 - m3;
    // COL 3
    twR = tw3[0];
    twI = tw3[1];

    m0 = t3[0] * twR;
    m1 = t3[1] * twI;
    m2 = t3[1] * twR;
    m3 = t3[0] * twI;

    *p3++ = m0 + m1;
    *p3++ = m2 - m3;
    // COL 4
    twR = tw4[0];
    twI = tw4[1];

    m0 = t4[0] * twR;
    m1 = t4[1] * twI;
    m2 = t4[1] * twR;
    m3 = t4[0] * twI;

    *p4++ = m0 + m1;
    *p4++ = m2 - m3;

    // first col
    csky_vdsp2_radix8_butterfly_f32( pCol1, L, (float32_t *) S->pTwiddle, 4u);
    // second col
    csky_vdsp2_radix8_butterfly_f32( pCol2, L, (float32_t *) S->pTwiddle, 4u);
    // third col
    csky_vdsp2_radix8_butterfly_f32( pCol3, L, (float32_t *) S->pTwiddle, 4u);
    // fourth col
    csky_vdsp2_radix8_butterfly_f32( pCol4, L, (float32_t *) S->pTwiddle, 4u);
}

