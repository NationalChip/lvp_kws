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
 * @file     csky_rfft_q31_yv.c
 * @brief    RFFT  Q31 process function.
 * @version  V1.0
 * @date     14. Nov 2017
 ******************************************************************************/

#include "csky_math.h"

/*--------------------------------------------------------------------
*		Internal functions prototypes
--------------------------------------------------------------------*/

void csky_split_rfft_yv_q31(
    q31_t * pSrc,
    uint32_t fftLen,
    const q31_t * pATable,
    q31_t * pDst,
    uint32_t modifier);
/**
* @addtogroup RealFFTyv
* @{
*/

/**
*
* @brief       Processing function for the Q31 real FFT.
* @param[in]      log_buf_len            log2 value of FFT size, the FFT size N is (1<<log2_buf_len)
* @param[in, out] *in_buf                point to the input and output memory
* @param[in]      *out_buf               not used
* @param[in]      *twi_table             point to the twi table
* @param[in]      *last_stage_twi_table  point to the real twi table
* @param[in]      *bitrev_tbl            point to the bit reversal table
* @param[in]      *temp_buf              not used
* @param[in]      br                     bit reversal flag, always set
* @return none.
*
* \note
* The time domin data and the frequency domain data are all in Q31 format, the forward transform
* is easy to overflow. So care to use it to avoid overflow.
*/


void csky_fft_lib_int32_fft(
    q31_t log2_buf_len,
    q31_t * in_buf,
    q31_t * out_buf,
    const q31_t * twi_table,
    const q31_t * last_stage_twi_table,
    const uint16_t * bitrev_tbl,
    q31_t * temp_buf,
    q31_t br)
{
    log2_buf_len -= 1;
    uint32_t L    = 1 << log2_buf_len;

    /* Calculation of RFFT of input */

    /* Complex FFT process */
    csky_fft_lib_cx32_fft(log2_buf_len, in_buf, out_buf, twi_table, bitrev_tbl, temp_buf, br);

    /*  Real FFT core process */
    csky_split_rfft_yv_q31(in_buf, L, last_stage_twi_table, out_buf, 1);
}

/**
* @} end of RealFFTyv group
*/
