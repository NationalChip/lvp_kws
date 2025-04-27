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
 * @file     csky_rfft_q15_yv.c
 * @brief    RFFT Q15 process function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/*--------------------------------------------------------------------
*		Internal functions prototypes
--------------------------------------------------------------------*/

void csky_split_rfft_yv_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    const q15_t * pATable,
    q15_t * pDst,
    uint32_t modifier);

/**
* @ingroup FFTlibyv
*/

/**
* @defgroup RealFFTyv  Real FFT Functions
*
* \par Brief Introduction
* This is the real data part of the FFT library. Real FFT algorithms take advantage
* of the symmetry properties of the FFT and have a speed advantage over complex
* algorithms of the same length. Unlike the
* <a href=group___real_f_f_t.html> Original Method </a>
* the forward transform, the inverse transform are seperated,
* so the <code>ifftFlag</code>a is not needed.More details can be seen in
* <a href=group___real_f_f_t.html> Real FFT Functions </a>.
*
* \par Algorithm
* The algorithm used here is almost the same as the
* <a href=group___real_f_f_t.html> Original Method </a>.
*
* \par Initialization
* Just like the CFFT, these is no need to initialize any instance structure, all the
* parameters and constants are passed to the related functions straightforwardly.
*/

/**
* @addtogroup RealFFTyv
* @{
*/

/**
*
* @brief       Processing function for the Q15 real FFT.
* @param[in]      log_buf_len            log2 value of FFT size, the FFT size N is (1<<log2_buf_len)
* @param[in, out] *in_buf                point to the input and output memory
* @param[in]      *out_buf               not used
* @param[in]      *twi_table             point to the twi table
* @param[in]      *last_stage_twi_table  point to the real twi table
* @param[in]      *bitrev_tbl            point to the bit reversal table
* @param[in]      *temp_buf              not used
* @param[in]      *ScaleShift            not used
* @param[in]      br                     bit reversal flag, always set
* @return none.
*
* \note
* The time domin data and the frequency domain data are all in Q15 format, the forward transform
* is easy to overflow. So care to use it to avoid overflow.
*/

void csky_fft_lib_int16_fft(
    q31_t log2_buf_len,
    q15_t * in_buf,
    q15_t * out_buf,
    const q15_t * twi_table,
    const q15_t * last_stage_twi_table,
    const uint16_t * bitrev_tbl,
    q15_t * temp_buf,
    q7_t  * ScaleShift,
    q31_t br)
{
    log2_buf_len -= 1;
    uint32_t L    = 1 << (log2_buf_len);

    /* Complex FFT process */
    csky_fft_lib_cx16_fft(log2_buf_len, in_buf, out_buf, twi_table, bitrev_tbl, temp_buf, ScaleShift, br);

    /*  Real FFT core process */
    csky_split_rfft_yv_q15(in_buf, L, last_stage_twi_table, out_buf, 1);
}

/**
* @} end of RealFFTyv group
*/
