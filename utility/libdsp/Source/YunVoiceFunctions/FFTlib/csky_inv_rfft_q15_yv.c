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
 * @file     csky_inv_rfft_q15_yv.c
 * @brief    RIFFT Q15 process function.
 * @version  V1.0
 * @date     15. Nov 2016
 ******************************************************************************/

#include "csky_math.h"

/*--------------------------------------------------------------------
*		Internal functions prototypes
--------------------------------------------------------------------*/

void csky_split_rifft_yv_q15(
    q15_t * pSrc,
    uint32_t fftLen,
    const q15_t * pATable,
    q15_t * pDst,
    uint32_t modifier);

/**
* @addtogroup RealFFTyv
* @{
*/

/**
*
* @brief       Processing function for the Q15 real inverse FFT.
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
*/

void csky_fft_lib_int16_ifft(
    q31_t  log2_buf_len,
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

    /*  Real IFFT core process */
    csky_split_rifft_yv_q15(in_buf, L, last_stage_twi_table, out_buf, 1);

    /* Complex IFFT process */
    csky_fft_lib_cx16_ifft(log2_buf_len, out_buf, out_buf, twi_table, bitrev_tbl, temp_buf, ScaleShift, br);

}

/**
* @} end of RealFFTyv group
*/
