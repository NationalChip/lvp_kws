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
* This file comes from arm_rfft_q15.c. It is renamed by replacing arm with csky.
* And the split functions are moved to a specific file.
*
*/

/******************************************************************************
 * @file     csky_rfft_q15.c
 * @brief    RFFT & RIFFT Q15 process function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"


/*--------------------------------------------------------------------
*		Internal functions prototypes
--------------------------------------------------------------------*/

void csky_split_rfft_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    q15_t * pATable,
    q15_t * pDst,
    uint32_t modifier);

void csky_split_rifft_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    q15_t * pATable,
    q15_t * pDst,
    uint32_t modifier);

/**
* @addtogroup RealFFT
* @{
*/

/**
* @brief Processing function for the Q15 RFFT/RIFFT.
* @param[in]  *S    points to an instance of the Q15 RFFT/RIFFT structure.
* @param[in]  *pSrc points to the input buffer.
* @param[out] *pDst points to the output buffer.
* @return none.
*
* \par Recommended Usage
* For each length of input, the usage is as follows:
* \code
* csky_rfft_q15(&csky_rfft_sR_q15_len32, pSrc, pDst);
* csky_rfft_q15(&csky_rfft_sR_q15_len64, pSrc, pDst);
* ....
* csky_rfft_q15(&csky_rfft_sR_q15_len4096, pSrc, pDst);
* csky_rfft_q15(&csky_rfft_sR_q15_len8192, pSrc, pDst);
* \endcode
*
* \par Input an output formats:
* \par
* Internally input is downscaled by 2 for every stage to avoid saturations inside CFFT/CIFFT process.
* Hence the output format is different for different RFFT sizes.
* The input and output formats for different RFFT sizes and number of bits to upscale are mentioned in the tables below for RFFT and RIFFT:
*
* \par
* | RFFT Size | Input Format | Output Format | Number of bits to upscale |
* |-----------|--------------|---------------|---------------------------|
* | 32        | 1.15         | 5.11          | 4                         |
* | 64        | 1.15         | 6.10          | 5                         |
* | 128       | 1.15         | 7.9           | 6                         |
* | 256       | 1.15         | 8.8           | 7                         |
* | 512       | 1.15         | 9.7           | 8                         |
* | 1024      | 1.15         | 10.6          | 9                         |
* | 2048      | 1.15         | 11.5          | 10                        |
* | 4096      | 1.15         | 12.4          | 11                        |
* | 8192      | 1.15         | 13.3          | 12                        |
*
* \par
* | RIFFT Size | Input Format | Output Format | Number of bits to upscale |
* |------------|--------------|---------------|---------------------------|
* | 32         | 1.15         | 5.11          | 0                         |
* | 64         | 1.15         | 6.10          | 0                         |
* | 128        | 1.15         | 7.9           | 0                         |
* | 256        | 1.15         | 8.8           | 0                         |
* | 512        | 1.15         | 9.7           | 0                         |
* | 1024       | 1.15         | 10.6          | 0                         |
* | 2048       | 1.15         | 11.5          | 0                         |
* | 4096       | 1.15         | 12.4          | 0                         |
* | 8192       | 1.15         | 13.3          | 0                         |

*/

void csky_rfft_q15(
    const csky_rfft_instance_q15 * S,
    q15_t * pSrc,
    q15_t * pDst)
{
    const csky_cfft_instance_q15 *S_CFFT = S->pCfft;
    uint32_t i;
    uint32_t L2 = S->fftLenReal >> 1;

    /* Calculation of RIFFT of input */
    if(S->ifftFlagR == 1u)
    {
        /*  Real IFFT core process */
        csky_split_rifft_q15(pSrc, L2, S->pTwiddleAReal, pDst, S->twidCoefRModifier);

        /* Complex IFFT process */
        csky_cfft_q15(S_CFFT, pDst, S->ifftFlagR, S->bitReverseFlagR);

#ifndef CSKY_DSP3
        for(i=0;i<S->fftLenReal;i++)
        {
            pDst[i] = pDst[i] << 1;
        }
#endif

    }
    else
    {
        /* Calculation of RFFT of input */

        /* Complex FFT process */
        csky_cfft_q15(S_CFFT, pSrc, S->ifftFlagR, S->bitReverseFlagR);

        /*  Real FFT core process */
        csky_split_rfft_q15(pSrc, L2, S->pTwiddleAReal, pDst, S->twidCoefRModifier);
    }
}

/**
* @} end of RealFFT group
*/

