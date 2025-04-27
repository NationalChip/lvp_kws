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
 * @file     csky_cfft_q15_yv.c
 * @brief    Radix Decimation in Q15 Frequency CFFT processing function.
 * @version  V1.0
 * @date     14. Nov 2017
 ******************************************************************************/

#include "csky_math.h"
#include "csky_const_structs.h"

extern void csky_radix4_butterfly_yv_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    const q15_t * pCoef,
    uint32_t twidCoefModifier);

extern void csky_bitreversal_16(
    uint16_t * pSrc,
    const uint16_t bitRevLen,
    const uint16_t * pBitRevTable);

void csky_cfft_radix4by2_yv_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    const q15_t * pCoef);

/**
* @ingroup groupYunvoice
*/

/**
* @defgroup FFTlibyv DSP FFT Library
*
* The yun voice DSP FFT library is specified for YunVoice project, which is
* derived from FFT functions in <a href=group__group_transforms.html>
* Transform Functions</a>. So, the introductions here will be brief, and
* will aim at the difference from the original version. More details can
* be seen in <a href=group__group_transforms.html> Transform Functions</a>.
*
* This library includes rfft(irfft) and cfft(icfft) functions, which are used to handle
* real data and complex data respectively. And there are separate algorithms
* to handle Q15 and Q31 data as well. Finally, the lengths of data supported in
* this library are[32, 64, 128, 256, 512, 1024].
*
*/

/**
* @ingroup FFTlibyv
*/

/**
* @defgroup ComplexFFTyv  Complex FFT Functions
*
* \par Brief Introduction
* Only Q15 and Q31 data is designed in this library. The FFT functions operate
* in-place as the original ones. The input data is complex
* and contains <code>2*fftLen</code> interleaved values as shown below.
* <pre> {real[0], imag[0], real[1], imag[1],..} </pre>
* The FFT result will be contained in the same array and the frequency domain
* values will have the same interleaving. Unlike the
* <a href=group___complex_f_f_t.html> Original Method </a>
* the forward transform, the inverse transform are
* seperated, so the <code>ifftFlag</code>a is not needed.
*
* \par Algorithm
* For the lengths of [64, 256, 1024], the algorithm is the combination of multiple
* radix-4 stages. While, for the lengths of [32, 128, 512], the algorithm is
* multiple radix-4 stages  along with a single radix-2 stage.
*
* \par
* The function uses the standard FFT definition, the inverse transform includes
* a scale of <code>1/fftLen</code>, which realized by multiply <code>1/2</code>
* at every stage. And this matches the textbook definition of the inverse FFT.
*
* \par Initialization
* Different from the <a href=group___complex_f_f_t.html> Original Method </a>,
* there is no initialization needed among this library. All the parameters and
* constants are passed to each function straightforwardly.
*
*/

/**
* @addtogroup ComplexFFTyv
* @{
*/

/**
*
* @brief       Processing function for the Q15 complex FFT.
* @param[in]      log_buf_len  log2 value of FFT size, the FFT size N is (1<<log2_buf_len)
* @param[in, out] *in_buf     point to the input and output memory
* @param[in]      *out_buf    not used
* @param[in]      *twi_table  point to the twi table
* @param[in]      *bitrev_tbl point to the bit reversal table
* @param[in]      *temp_buf   not used
* @param[in]      *ScaleShift not used
* @param[in]      br           bit reversal flag, always set
* @return none.
*
* \note
* The time domin data and the frequency domain data are all in Q15 format, the forward transform
* is easy to overflow. So care to use it to avoid overflow.
*/

void csky_fft_lib_cx16_fft(
    q31_t log2_buf_len,
    q15_t * in_buf,
    q15_t * out_buf,
    const q15_t * twi_table,
    const uint16_t * bitrev_tbl,
    q15_t * temp_buf,
    q7_t  * ScaleShift,
    q31_t br)
{
    uint32_t L;
    uint16_t bitRevLen;

    L = (1 << log2_buf_len);
    switch(L)
    {

    case 16:
        bitRevLen = CSKYBITREVINDEXTABLE_FIXED___16_TABLE_LENGTH;
        break;
    case 32:
        bitRevLen = CSKYBITREVINDEXTABLE_FIXED___32_TABLE_LENGTH;
        break;
    case 64:
        bitRevLen = CSKYBITREVINDEXTABLE_FIXED___64_TABLE_LENGTH;
        break;
    case 128:
        bitRevLen = CSKYBITREVINDEXTABLE_FIXED__128_TABLE_LENGTH;
        break;
    case 256:
        bitRevLen = CSKYBITREVINDEXTABLE_FIXED__256_TABLE_LENGTH;
        break;
    case 512:
        bitRevLen = CSKYBITREVINDEXTABLE_FIXED__512_TABLE_LENGTH;
        break;
    case 1024:
        bitRevLen = CSKYBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH;
        break;
    case 2048:
        bitRevLen = CSKYBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH;
        break;
    case 4096:
        bitRevLen = CSKYBITREVINDEXTABLE_FIXED_4096_TABLE_LENGTH;
        break;
    }

    switch (L)
    {
    case 16:
    case 64:
    case 256:
    case 1024:
    case 4096:
        csky_radix4_butterfly_yv_q15(in_buf, L, twi_table, 1 );
        break;

    case 32:
    case 128:
    case 512:
    case 2048:
        csky_cfft_radix4by2_yv_q15(in_buf, L, twi_table );
        break;
    }

    if(br)
    {
        csky_bitreversal_16((uint16_t*)in_buf, bitRevLen, bitrev_tbl);
    }
}

/**
* @} end of ComplexFFTyv group
*/
