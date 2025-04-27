/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_rfft_q31.c
* Description:  RFFT & RIFFT Q31 process function
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
* This file comes from arm_rfft_q31.c. Only the split functions are placed here.
* And the name of the functions is started with csky. 
*
*/

/******************************************************************************
 * @file     csky_split_rfft_q31.c
 * @brief    RFFT & RIFFT Q31 process function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
* @brief  Core Real FFT process
* @param[in]   *pSrc 				points to the input buffer.
* @param[in]   fftLen  			    length of FFT.
* @param[in]   *pATable 			points to the twiddle Coef A buffer.
* @param[out]  *pDst 				points to the output buffer.
* @param[in]   modifier 	        twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
* @return none.
*/
void csky_split_rfft_q31(
    q31_t * pSrc,
    uint32_t fftLen,
    q31_t * pATable,
    q31_t * pDst,
    uint32_t modifier)
{
    uint32_t i;                                    /* Loop Counter */
    uint32_t Correction = 0x80000000;
    q31_t outR, outI;                              /* Temporary variables for output */
    q31_t *pCoefA;                                 /* Temporary pointers for twiddle factors */
    q31_t CoefA1, CoefA2, CoefB1;                  /* Temporary variables for twiddle coefficients */
    q31_t *pOut1 = &pDst[2];
    q31_t *pIn1 = &pSrc[2], *pIn2 = &pSrc[(2u * fftLen) - 1u];

    /* Init coefficient pointers */
    pCoefA = &pATable[modifier * 2u];

    i = fftLen - 1u;

    while(i > 0u)
    {
        /*
        outR = (pSrc[2 * i] * pATable[2 * i] - pSrc[2 * i + 1] * pATable[2 * i + 1]
        + pSrc[2 * n - 2 * i] * pBTable[2 * i] +
        pSrc[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);
        */

        /* outI = (pIn[2 * i + 1] * pATable[2 * i] + pIn[2 * i] * pATable[2 * i + 1] +
        pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i]); */

        CoefA1 = *pCoefA++;
        CoefA2 = *pCoefA;

        /* outR = (pSrc[2 * i] * pATable[2 * i] */
        mult_32x32_keep32_R(outR, *pIn1, CoefA1);

        /* outI = pIn[2 * i] * pATable[2 * i + 1] */
        mult_32x32_keep32_R(outI, *pIn1++, CoefA2);

        /* - pSrc[2 * i + 1] * pATable[2 * i + 1] */
        multSub_32x32_keep32_R(outR, *pIn1, CoefA2);

        /* (pIn[2 * i + 1] * pATable[2 * i] */
        multAcc_32x32_keep32_R(outI, *pIn1++, CoefA1);

        /* pSrc[2 * n - 2 * i] * pBTable[2 * i]  */
        multSub_32x32_keep32_R(outR, *pIn2, CoefA2);

        /*Compute the CoeffB1*/
        CoefB1 = Correction - (uint32_t)CoefA1;
        if((uint32_t)CoefB1 == 0x80000000)
        {
            CoefB1 = 0x7fffffff;
        }

        /* pIn[2 * n - 2 * i] * pBTable[2 * i + 1] */
        multSub_32x32_keep32_R(outI, *pIn2--, CoefB1);

        /* pSrc[2 * n - 2 * i + 1] * pBTable[2 * i + 1] */
        multAcc_32x32_keep32_R(outR, *pIn2, CoefB1);

        /* pIn[2 * n - 2 * i + 1] * pBTable[2 * i] */
        multSub_32x32_keep32_R(outI, *pIn2--, CoefA2);

        /* write output */
        *pOut1++ = outR;
        *pOut1++ = outI;

        /* update coefficient pointer */
        pCoefA = pCoefA + ((modifier * 2u) - 1u);

        i--;
    }
    pDst[2u * fftLen] = (pSrc[0] - pSrc[1]) >> 1;
    pDst[(2u * fftLen) + 1u] = 0;

    pDst[0] = (pSrc[0] + pSrc[1]) >> 1;
    pDst[1] = 0;
}

/**
* @brief  Core Real IFFT process
* @param[in]   *pSrc 				points to the input buffer.
* @param[in]   fftLen  			    length of FFT.
* @param[in]   *pATable 			points to the twiddle Coef A buffer.
* @param[out]  *pDst 				points to the output buffer.
* @param[in]   modifier 	        twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
* @return none.
*/
void csky_split_rifft_q31(
    q31_t * pSrc,
    uint32_t fftLen,
    q31_t * pATable,
    q31_t * pDst,
    uint32_t modifier)
{
    uint32_t Correction = 0x80000000;
    q31_t outR, outI;                              /* Temporary variables for output */
    q31_t *pCoefA;                                 /* Temporary pointers for twiddle factors */
    q31_t CoefA1, CoefA2, CoefB1;                  /* Temporary variables for twiddle coefficients */
    q31_t tmp;
    q31_t *pIn1 = &pSrc[0], *pIn2 = &pSrc[(2u * fftLen) + 1u];

    pCoefA = &pATable[0];

    while(fftLen > 0u)
    {
        /*
        outR = (pIn[2 * i] * pATable[2 * i] + pIn[2 * i + 1] * pATable[2 * i + 1] +
        pIn[2 * n - 2 * i] * pBTable[2 * i] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);

        outI = (pIn[2 * i + 1] * pATable[2 * i] - pIn[2 * i] * pATable[2 * i + 1] -
        pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i]);
        */
        CoefA1 = *pCoefA++;
        CoefA2 = *pCoefA;
        tmp = -CoefA2;

        /* outR = (pIn[2 * i] * pATable[2 * i] */
        mult_32x32_keep32_R(outR, *pIn1, CoefA1);

        /* - pIn[2 * i] * pATable[2 * i + 1] */
        mult_32x32_keep32_R(outI, *pIn1++, tmp);

        /* pIn[2 * i + 1] * pATable[2 * i + 1] */
        multAcc_32x32_keep32_R(outR, *pIn1, CoefA2);

        /* pIn[2 * i + 1] * pATable[2 * i] */
        multAcc_32x32_keep32_R(outI, *pIn1++, CoefA1);

        /* pIn[2 * n - 2 * i] * pBTable[2 * i] */
        multAcc_32x32_keep32_R(outR, *pIn2, CoefA2);

        /*Compute the CoeffB1*/
        CoefB1 = Correction - (uint32_t)CoefA1;
        if((uint32_t)CoefB1 == 0x80000000)
        {
            CoefB1 = 0x7fffffff;
        }

        /* pIn[2 * n - 2 * i] * pBTable[2 * i + 1] */
        multSub_32x32_keep32_R(outI, *pIn2--, CoefB1);

        /* pIn[2 * n - 2 * i + 1] * pBTable[2 * i + 1] */
        multAcc_32x32_keep32_R(outR, *pIn2, CoefB1);

        /* pIn[2 * n - 2 * i + 1] * pBTable[2 * i] */
        multAcc_32x32_keep32_R(outI, *pIn2--, CoefA2);

        /* write output */
        *pDst++ = outR;
        *pDst++ = outI;

        /* update coefficient pointer */
        pCoefA = pCoefA + ((modifier * 2u) - 1u);

        /* Decrement loop count */
        fftLen--;
    }
}
