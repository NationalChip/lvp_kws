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
 * @file     csky_cfft_q31_yv.c
 * @brief    Radix Decimation in Frequency CFFT fixed point processing function.
 * @version  V1.0
 * @date     15. Nov 2017
 ******************************************************************************/

#include "csky_math.h"
#include "csky_const_structs.h"

extern void csky_radix4_butterfly_yv_q31(
    q31_t * pSrc,
    uint32_t fftLen,
    const q31_t * pCoef,
    uint32_t twidCoefModifier);

extern void csky_bitreversal_32(
    uint32_t * pSrc,
    const uint16_t bitRevLen,
    const uint16_t * pBitRevTable);

void csky_cfft_radix4by2_yv_q31(
    q31_t * pSrc,
    uint32_t fftLen,
    const q31_t * pCoef);

/**
* @addtogroup ComplexFFTyv
* @{
*/

/**
*
* @brief       Processing function for the Q31 complex FFT.
* @param[in]      log_buf_len  log2 value of FFT size, the FFT size N is (1<<log2_buf_len)
* @param[in, out] *in_buf     point to the input and output memory
* @param[in]      *out_buf    not used
* @param[in]      *twi_table  point to the twi table
* @param[in]      *bitrev_tbl point to the bit reversal table
* @param[in]      *temp_buf   not used
* @param[in]      br           bit reversal flag, always set
* @return none.
*
* \note
* The time domin data and the frequency domain data are all in Q31 format, the forward transform
* is easy to overflow. So care to use it to avoid overflow.
*/

void csky_fft_lib_cx32_fft(
    q31_t log2_buf_len,
    q31_t * in_buf,
    q31_t * out_buf,
    const q31_t * twi_table,
    const uint16_t * bitrev_tbl,
    q31_t * temp_buf,
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
        csky_radix4_butterfly_yv_q31(in_buf, L, twi_table, 1 );
        break;

    case 32:
    case 128:
    case 512:
    case 2048:
        csky_cfft_radix4by2_yv_q31(in_buf, L, twi_table );
        break;
    }

    if( br )
        csky_bitreversal_32((uint32_t*)in_buf, bitRevLen, bitrev_tbl);
}

/**
* @} end of ComplexFFTyv group
*/
