/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_rfft_q15.c
* Description:  RFFT & RIFFT Q15 process function
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
* This file comes from arm_rfft_q15.c. Only the split functions are placed here.
* And the name of the functions is started with csky. 
*
*/

/******************************************************************************
 * @file     csky_split_rfft_q15.c
 * @brief    RFFT & RIFFT Q15 process function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
  \brief   Dual 16-bit absolute value with saturation.
  \details This function enables you to compute absolute vlaue of two 16-bit data with
            saturation.
  \param [in]    x   input of two 16-bit data.
  \return        the absolute value of both high half and low half of the input data.
  \remark
                 res[15:0]  = abs(val[15:0])        \n
                 res[31:16] = abs(va1[31:16])
 */

__ALWAYS_STATIC_INLINE uint32_t __PABS16(uint32_t x)
{
#ifdef CSKY_SIMD
  uint32_t result;
  __ASM volatile("pabs.s16.s %0, %1\n\t"
                   :"=r" (result), "=r" (x): "0" (result), "1" (x));
  return result;

#else
    int32_t r, s;

    r = (int32_t)x >> 16;
    s = ((int32_t)x << 16) >> 16;

    if(r < 0)
    {
        r = __SSAT_16(-r);
    }
    if(s < 0)
    {
        s = __SSAT_16(-s);
    }

    return(uint32_t)((r << 16) | (s & 0xFFFF));
#endif
}


/**
* @brief  Core Real FFT process
* @param  *pSrc 				points to the input buffer.
* @param  fftLen  				length of FFT.
* @param  *pATable 			points to the A twiddle Coef buffer.
* @param  *pDst 				points to the output buffer.
* @param  modifier 	        twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
* @return none.
* The function implements a Real FFT
*/

void csky_split_rfft_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    q15_t * pATable,
    q15_t * pDst,
    uint32_t modifier)
{
    uint32_t i;                                    /* Loop Counter */
    uint32_t Correction;
#ifndef CSKY_MATH_BIG_ENDIAN
    Correction = 0x00008000;
#else
    Correction = 0x80000000;
#endif
    q31_t outR, outI;                              /* Temporary variables for output */
    q15_t *pCoefA;                                 /* Temporary pointers for twiddle factors */
    q31_t CoefB, tmp;
    q15_t *pSrc1, *pSrc2;
#ifndef CSKY_MATH_NO_SIMD
    q15_t *pD1;
#endif

    pCoefA = &pATable[modifier * 2u];

    pSrc1 = &pSrc[2];
    pSrc2 = &pSrc[(2u * fftLen) - 2u];

#ifndef CSKY_MATH_NO_SIMD

    i = 1u;
    pD1 = pDst + 2;

    for(i = fftLen - 1; i > 0; i--)
    {
        /*
        outR = (pSrc[2 * i] * pATable[2 * i] - pSrc[2 * i + 1] * pATable[2 * i + 1]
        + pSrc[2 * n - 2 * i] * pBTable[2 * i] +
        pSrc[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);
        */

        /* outI = (pIn[2 * i + 1] * pATable[2 * i] + pIn[2 * i] * pATable[2 * i + 1] +
        pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i]); */


#ifndef CSKY_MATH_BIG_ENDIAN

        /* pSrc[2 * i] * pATable[2 * i] - pSrc[2 * i + 1] * pATable[2 * i + 1] */
        outR  = __SMUSD(*__SIMD32(pSrc1), *__SIMD32(pCoefA));
        CoefB = __USUB16(Correction, *__SIMD32(pCoefA));
        tmp   = __PABS16(CoefB);
        CoefB = __PKHBT(tmp, CoefB, 0);
#else

        /* -(pSrc[2 * i + 1] * pATable[2 * i + 1] - pSrc[2 * i] * pATable[2 * i]) */
        outR = -(__SMUSD(*__SIMD32(pSrc1), *__SIMD32(pCoefA)));

        /* compute the CoefB.*/
        CoefB = __USUB16(Correction, *__SIMD32(pCoefA));
        tmp   = __PABS16(CoefB);
        CoefB = __PKHBT(tmp, CoefB, 0);

#endif /*      #ifndef CSKY_MATH_BIG_ENDIAN     */

        /* pSrc[2 * n - 2 * i] * pBTable[2 * i] +
        pSrc[2 * n - 2 * i + 1] * pBTable[2 * i + 1]) */
        outR = __SMLAD(*__SIMD32(pSrc2), CoefB, outR) >> 16u;

        /* pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i] */

#ifndef CSKY_MATH_BIG_ENDIAN

        outI = __SMUSDX(*__SIMD32(pSrc2)--, CoefB);

#else

        outI = __SMUSDX(CoefB, *__SIMD32(pSrc2)--);

#endif /*      #ifndef CSKY_MATH_BIG_ENDIAN     */

        /* (pIn[2 * i + 1] * pATable[2 * i] + pIn[2 * i] * pATable[2 * i + 1] */
        outI = __SMLADX(*__SIMD32(pSrc1)++, *__SIMD32(pCoefA), outI);

        /* write output */
        *pD1++ = (q15_t) outR;
        *pD1++ = outI >> 16u;

        /* update coefficient pointer */
        //pCoefB = pCoefB + (2u * modifier);
        pCoefA = pCoefA + (2u * modifier);
    }

    pDst[2u * fftLen] = (pSrc[0] - pSrc[1]) >> 1;
    pDst[(2u * fftLen) + 1u] = 0;

    pDst[0] = (pSrc[0] + pSrc[1]) >> 1;
    pDst[1] = 0;

#else

    i = 1u;

    while(i < fftLen)
    {
        /*
        outR = (pSrc[2 * i] * pATable[2 * i] - pSrc[2 * i + 1] * pATable[2 * i + 1]
        + pSrc[2 * n - 2 * i] * pBTable[2 * i] +
        pSrc[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);
        */

        outR = *pSrc1 * *pCoefA;
        outR = outR - (*(pSrc1 + 1) * *(pCoefA + 1));
        outR = outR + (*pSrc2 * *pCoefB);
        outR = (outR + (*(pSrc2 + 1) * *(pCoefB + 1))) >> 16;


        /* outI = (pIn[2 * i + 1] * pATable[2 * i] + pIn[2 * i] * pATable[2 * i + 1] +
        pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i]);
        */

        outI = *pSrc2 * *(pCoefB + 1);
        outI = outI - (*(pSrc2 + 1) * *pCoefB);
        outI = outI + (*(pSrc1 + 1) * *pCoefA);
        outI = outI + (*pSrc1 * *(pCoefA + 1));

        /* update input pointers */
        pSrc1 += 2u;
        pSrc2 -= 2u;

        /* write output */
        pDst[2u * i] = (q15_t) outR;
        pDst[(2u * i) + 1u] = outI >> 16u;

        /* write complex conjugate output */
        pDst[(4u * fftLen) - (2u * i)] = (q15_t) outR;
        pDst[((4u * fftLen) - (2u * i)) + 1u] = -(outI >> 16u);

        /* update coefficient pointer */
        pCoefB = pCoefB + (2u * modifier);
        pCoefA = pCoefA + (2u * modifier);

        i++;
    }

    pDst[2u * fftLen] = (pSrc[0] - pSrc[1]) >> 1;
    pDst[(2u * fftLen) + 1u] = 0;

    pDst[0] = (pSrc[0] + pSrc[1]) >> 1;
    pDst[1] = 0;

#endif /* #ifndef CSKY_MATH_NO_SIMD */
}


/**
* @brief  Core Real IFFT process
* @param[in]   *pSrc 				points to the input buffer.
* @param[in]   fftLen  		    length of FFT.
* @param[in]   *pATable 			points to the twiddle Coef A buffer.
* @param[out]  *pDst 				points to the output buffer.
* @param[in]   modifier 	        twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
* @return none.
* The function implements a Real IFFT
*/
void csky_split_rifft_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    q15_t * pATable,
    q15_t * pDst,
    uint32_t modifier)
{
    uint32_t i;                                    /* Loop Counter */
    uint32_t Correction;
#ifndef CSKY_MATH_BIG_ENDIAN
    Correction = 0x00008000;
#else
    Correction = 0x80000000;
#endif
    q31_t outR, outI;                              /* Temporary variables for output */
    q15_t *pCoefA;                                 /* Temporary pointers for twiddle factors */
    q15_t *pSrc1, *pSrc2;
    q31_t CoefB, tmp;
    q15_t *pDst1 = &pDst[0];

    pCoefA = &pATable[0];

    pSrc1 = &pSrc[0];
    pSrc2 = &pSrc[2u * fftLen];

#ifndef CSKY_MATH_NO_SIMD

    i = fftLen;

    while(i > 0u)
    {
        /*
        outR = (pIn[2 * i] * pATable[2 * i] + pIn[2 * i + 1] * pATable[2 * i + 1] +
        pIn[2 * n - 2 * i] * pBTable[2 * i] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);

        outI = (pIn[2 * i + 1] * pATable[2 * i] - pIn[2 * i] * pATable[2 * i + 1] -
        pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i]);
        */


#ifndef CSKY_MATH_BIG_ENDIAN

        /* pIn[2 * n - 2 * i] * pBTable[2 * i] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i + 1]) */

        /* compute the CoefB.*/
        CoefB = __USUB16(Correction, *__SIMD32(pCoefA));
        tmp   = __PABS16(CoefB);
        CoefB = __PKHBT(tmp, CoefB, 0);

        outR = __SMUSD(*__SIMD32(pSrc2), CoefB);

#else

        /* -(-pIn[2 * n - 2 * i] * pBTable[2 * i] +
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i + 1])) */

        /* compute the CoefB.*/
        CoefB = __USUB16(Correction, *__SIMD32(pCoefA));
        tmp   = __PABS16(CoefB);
        CoefB = __PKHBT(tmp, CoefB, 0);

        outR = -(__SMUSD(*__SIMD32(pSrc2), CoefB));

#endif /*      #ifndef CSKY_MATH_BIG_ENDIAN     */

        /* pIn[2 * i] * pATable[2 * i] + pIn[2 * i + 1] * pATable[2 * i + 1] +
        pIn[2 * n - 2 * i] * pBTable[2 * i] */
        outR = __SMLAD(*__SIMD32(pSrc1), *__SIMD32(pCoefA), outR) >> 16u;

        /*
        -pIn[2 * n - 2 * i] * pBTable[2 * i + 1] +
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i] */
        outI = __SMUADX(*__SIMD32(pSrc2)--, CoefB);

        /* pIn[2 * i + 1] * pATable[2 * i] - pIn[2 * i] * pATable[2 * i + 1] */

#ifndef CSKY_MATH_BIG_ENDIAN

        outI = __SMLSDX(*__SIMD32(pCoefA), *__SIMD32(pSrc1)++, -outI);

#else

        outI = __SMLSDX(*__SIMD32(pSrc1)++, *__SIMD32(pCoefA), -outI);

#endif /*      #ifndef CSKY_MATH_BIG_ENDIAN     */
        /* write output */

#ifndef CSKY_MATH_BIG_ENDIAN

        *__SIMD32(pDst1)++ = __PKHBT(outR, (outI >> 16u), 16);

#else

        *__SIMD32(pDst1)++ = __PKHBT((outI >> 16u), outR, 16);

#endif /*      #ifndef CSKY_MATH_BIG_ENDIAN     */

        /* update coefficient pointer */
        pCoefA = pCoefA + (2u * modifier);

        i--;
    }
#else
    i = fftLen;

    while(i > 0u)
    {
        /*
        outR = (pIn[2 * i] * pATable[2 * i] + pIn[2 * i + 1] * pATable[2 * i + 1] +
        pIn[2 * n - 2 * i] * pBTable[2 * i] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);
        */

        outR = *pSrc2 * *pCoefB;
        outR = outR - (*(pSrc2 + 1) * *(pCoefB + 1));
        outR = outR + (*pSrc1 * *pCoefA);
        outR = (outR + (*(pSrc1 + 1) * *(pCoefA + 1))) >> 16;

        /*
        outI = (pIn[2 * i + 1] * pATable[2 * i] - pIn[2 * i] * pATable[2 * i + 1] -
        pIn[2 * n - 2 * i] * pBTable[2 * i + 1] -
        pIn[2 * n - 2 * i + 1] * pBTable[2 * i]);
        */

        outI = *(pSrc1 + 1) * *pCoefA;
        outI = outI - (*pSrc1 * *(pCoefA + 1));
        outI = outI - (*pSrc2 * *(pCoefB + 1));
        outI = outI - (*(pSrc2 + 1) * *(pCoefB));

        /* update input pointers */
        pSrc1 += 2u;
        pSrc2 -= 2u;

        /* write output */
        *pDst1++ = (q15_t) outR;
        *pDst1++ = (q15_t) (outI >> 16);

        /* update coefficient pointer */
        pCoefB = pCoefB + (2u * modifier);
        pCoefA = pCoefA + (2u * modifier);

        i--;
    }
#endif /* #ifndef CSKY_MATH_NO_SIMD */
}
