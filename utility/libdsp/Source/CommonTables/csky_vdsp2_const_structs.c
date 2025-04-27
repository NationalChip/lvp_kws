/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_const_structs.c
 * Description:  Constant structs that are initialized for user convenience.
 *               For example, some can be given as arguments to the arm_cfft_f32() or arm_rfft_f32() functions.
 *
 * $Date:        27. January 2017
 * $Revision:    V.1.5.1
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
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
 * This file comes from arm_const_structs.c. It initializes all the structures
 * used for cfft, rfft and dct4, including the structures with continuous
 * twiddle factors as well. And this file is specific for CPUs with vdsp2
 * instructions.
 *
 */

#include "csky_common_tables.h"
#include "csky_vdsp2_const_structs.h"

//Floating-point structs

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_sR_f32_len16 = {
	16, (float32_t *)twiddleCoef_16, cskyBitRevIndexTable16, CSKYBITREVINDEXTABLE__16_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_sR_f32_len32 = {
	32, (float32_t *)twiddleCoef_32, cskyBitRevIndexTable32, CSKYBITREVINDEXTABLE__32_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_sR_f32_len64 = {
	64, (float32_t *)twiddleCoef_64, cskyBitRevIndexTable64, CSKYBITREVINDEXTABLE__64_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_sR_f32_len128 = {
	128, (float32_t *)twiddleCoef_128, cskyBitRevIndexTable128, CSKYBITREVINDEXTABLE_128_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_sR_f32_len256 = {
	256, (float32_t *)twiddleCoef_256, cskyBitRevIndexTable256, CSKYBITREVINDEXTABLE_256_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_sR_f32_len512 = {
	512, (float32_t *)twiddleCoef_512, cskyBitRevIndexTable512, CSKYBITREVINDEXTABLE_512_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_sR_f32_len1024 = {
	1024, (float32_t *)twiddleCoef_1024, cskyBitRevIndexTable1024, CSKYBITREVINDEXTABLE1024_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_sR_f32_len2048 = {
	2048, (float32_t *)twiddleCoef_2048, cskyBitRevIndexTable2048, CSKYBITREVINDEXTABLE2048_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_sR_f32_len4096 = {
	4096, (float32_t *)twiddleCoef_4096, cskyBitRevIndexTable4096, CSKYBITREVINDEXTABLE4096_TABLE_LENGTH
};


//Floating-point structs radix4

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_sR_f32_len16 = {
	16, (float32_t *)twiddleCoef_16, cskyBitRevIndexTable_fixed_16, CSKYBITREVINDEXTABLE_FIXED___16_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_sR_f32_len32 = {
	32, (float32_t *)twiddleCoef_32, cskyBitRevIndexTable_fixed_32, CSKYBITREVINDEXTABLE_FIXED___32_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_sR_f32_len64 = {
	64, (float32_t *)twiddleCoef_64, cskyBitRevIndexTable_fixed_64, CSKYBITREVINDEXTABLE_FIXED___64_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_sR_f32_len128 = {
	128, (float32_t *)twiddleCoef_128, cskyBitRevIndexTable_fixed_128, CSKYBITREVINDEXTABLE_FIXED__128_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_sR_f32_len256 = {
	256, (float32_t *)twiddleCoef_256, cskyBitRevIndexTable_fixed_256, CSKYBITREVINDEXTABLE_FIXED__256_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_sR_f32_len512 = {
	512, (float32_t *)twiddleCoef_512, cskyBitRevIndexTable_fixed_512, CSKYBITREVINDEXTABLE_FIXED__512_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_sR_f32_len1024 = {
	1024, (float32_t *)twiddleCoef_1024, cskyBitRevIndexTable_fixed_1024, CSKYBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_sR_f32_len2048 = {
	2048, (float32_t *)twiddleCoef_2048, cskyBitRevIndexTable_fixed_2048, CSKYBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_sR_f32_len4096 = {
	4096, (float32_t *)twiddleCoef_4096, cskyBitRevIndexTable_fixed_4096, CSKYBITREVINDEXTABLE_FIXED_4096_TABLE_LENGTH
};

//Floating-point structs radix4 fast

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_fast_sR_f32_len16 = {
	16, (float32_t *)twiddleCoef_fast_16, cskyBitRevIndexTable_fixed_16, CSKYBITREVINDEXTABLE_FIXED___16_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_fast_sR_f32_len32 = {
	32, (float32_t *)twiddleCoef_fast_32, cskyBitRevIndexTable_fixed_32, CSKYBITREVINDEXTABLE_FIXED___32_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_fast_sR_f32_len64 = {
	64, (float32_t *)twiddleCoef_fast_64, cskyBitRevIndexTable_fixed_64, CSKYBITREVINDEXTABLE_FIXED___64_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_fast_sR_f32_len128 = {
	128, (float32_t *)twiddleCoef_fast_128, cskyBitRevIndexTable_fixed_128, CSKYBITREVINDEXTABLE_FIXED__128_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_fast_sR_f32_len256 = {
	256, (float32_t *)twiddleCoef_fast_256, cskyBitRevIndexTable_fixed_256, CSKYBITREVINDEXTABLE_FIXED__256_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_fast_sR_f32_len512 = {
	512, (float32_t *)twiddleCoef_fast_512, cskyBitRevIndexTable_fixed_512, CSKYBITREVINDEXTABLE_FIXED__512_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_fast_sR_f32_len1024 = {
	1024, (float32_t *)twiddleCoef_fast_1024, cskyBitRevIndexTable_fixed_1024, CSKYBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_fast_sR_f32_len2048 = {
	2048, (float32_t *)twiddleCoef_fast_2048, cskyBitRevIndexTable_fixed_2048, CSKYBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix4_fast_sR_f32_len4096 = {
	4096, (float32_t *)twiddleCoef_fast_4096, cskyBitRevIndexTable_fixed_4096, CSKYBITREVINDEXTABLE_FIXED_4096_TABLE_LENGTH
};

//Floating-point structs radix2
const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix2_sR_f32_len16 = {
	16, (float32_t *)twiddleCoef_16, cskyBitRevIndexTable_fixed_16, CSKYBITREVINDEXTABLE_FIXED___16_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix2_sR_f32_len32 = {
	32, (float32_t *)twiddleCoef_32, cskyBitRevIndexTable_fixed_32, CSKYBITREVINDEXTABLE_FIXED___32_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix2_sR_f32_len64 = {
	64, (float32_t *)twiddleCoef_64, cskyBitRevIndexTable_fixed_64, CSKYBITREVINDEXTABLE_FIXED___64_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix2_sR_f32_len128 = {
	128, (float32_t *)twiddleCoef_128, cskyBitRevIndexTable_fixed_128, CSKYBITREVINDEXTABLE_FIXED__128_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix2_sR_f32_len256 = {
	256, (float32_t *)twiddleCoef_256, cskyBitRevIndexTable_fixed_256, CSKYBITREVINDEXTABLE_FIXED__256_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix2_sR_f32_len512 = {
	512, (float32_t *)twiddleCoef_512, cskyBitRevIndexTable_fixed_512, CSKYBITREVINDEXTABLE_FIXED__512_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix2_sR_f32_len1024 = {
	1024, (float32_t *)twiddleCoef_1024, cskyBitRevIndexTable_fixed_1024, CSKYBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix2_sR_f32_len2048 = {
	2048, (float32_t *)twiddleCoef_2048, cskyBitRevIndexTable_fixed_2048, CSKYBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_f32 csky_vdsp2_cfft_radix2_sR_f32_len4096 = {
	4096, (float32_t *)twiddleCoef_4096, cskyBitRevIndexTable_fixed_4096, CSKYBITREVINDEXTABLE_FIXED_4096_TABLE_LENGTH
};

//Fixed-point structs

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_sR_q31_len16 = {
	16, twiddleCoef_16_q31, cskyBitRevIndexTable_fixed_16, CSKYBITREVINDEXTABLE_FIXED___16_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_sR_q31_len32 = {
	32, twiddleCoef_32_q31, cskyBitRevIndexTable_fixed_32, CSKYBITREVINDEXTABLE_FIXED___32_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_sR_q31_len64 = {
	64, twiddleCoef_64_q31, cskyBitRevIndexTable_fixed_64, CSKYBITREVINDEXTABLE_FIXED___64_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_sR_q31_len128 = {
	128, twiddleCoef_128_q31, cskyBitRevIndexTable_fixed_128, CSKYBITREVINDEXTABLE_FIXED__128_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_sR_q31_len256 = {
	256, twiddleCoef_256_q31, cskyBitRevIndexTable_fixed_256, CSKYBITREVINDEXTABLE_FIXED__256_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_sR_q31_len512 = {
	512, twiddleCoef_512_q31, cskyBitRevIndexTable_fixed_512, CSKYBITREVINDEXTABLE_FIXED__512_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_sR_q31_len1024 = {
	1024, twiddleCoef_1024_q31, cskyBitRevIndexTable_fixed_1024, CSKYBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_sR_q31_len2048 = {
	2048, twiddleCoef_2048_q31, cskyBitRevIndexTable_fixed_2048, CSKYBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_sR_q31_len4096 = {
	4096, twiddleCoef_4096_q31, cskyBitRevIndexTable_fixed_4096, CSKYBITREVINDEXTABLE_FIXED_4096_TABLE_LENGTH
};


const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_sR_q15_len16 = {
	16, twiddleCoef_16_q15, cskyBitRevIndexTable_fixed_16, CSKYBITREVINDEXTABLE_FIXED___16_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_sR_q15_len32 = {
	32, twiddleCoef_32_q15, cskyBitRevIndexTable_fixed_32, CSKYBITREVINDEXTABLE_FIXED___32_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_sR_q15_len64 = {
	64, twiddleCoef_64_q15, cskyBitRevIndexTable_fixed_64, CSKYBITREVINDEXTABLE_FIXED___64_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_sR_q15_len128 = {
	128, twiddleCoef_128_q15, cskyBitRevIndexTable_fixed_128, CSKYBITREVINDEXTABLE_FIXED__128_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_sR_q15_len256 = {
	256, twiddleCoef_256_q15, cskyBitRevIndexTable_fixed_256, CSKYBITREVINDEXTABLE_FIXED__256_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_sR_q15_len512 = {
	512, twiddleCoef_512_q15, cskyBitRevIndexTable_fixed_512, CSKYBITREVINDEXTABLE_FIXED__512_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_sR_q15_len1024 = {
	1024, twiddleCoef_1024_q15, cskyBitRevIndexTable_fixed_1024, CSKYBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_sR_q15_len2048 = {
	2048, twiddleCoef_2048_q15, cskyBitRevIndexTable_fixed_2048, CSKYBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_sR_q15_len4096 = {
	4096, twiddleCoef_4096_q15, cskyBitRevIndexTable_fixed_4096, CSKYBITREVINDEXTABLE_FIXED_4096_TABLE_LENGTH
};

/* fast version*/
const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_fast_sR_q31_len16 = {
	16, twiddleCoef_fast_16_q31, cskyBitRevIndexTable_fixed_16, CSKYBITREVINDEXTABLE_FIXED___16_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_fast_sR_q31_len32 = {
	32, twiddleCoef_fast_32_q31, cskyBitRevIndexTable_fixed_32, CSKYBITREVINDEXTABLE_FIXED___32_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_fast_sR_q31_len64 = {
	64, twiddleCoef_fast_64_q31, cskyBitRevIndexTable_fixed_64, CSKYBITREVINDEXTABLE_FIXED___64_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_fast_sR_q31_len128 = {
	128, twiddleCoef_fast_128_q31, cskyBitRevIndexTable_fixed_128, CSKYBITREVINDEXTABLE_FIXED__128_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_fast_sR_q31_len256 = {
	256, twiddleCoef_fast_256_q31, cskyBitRevIndexTable_fixed_256, CSKYBITREVINDEXTABLE_FIXED__256_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_fast_sR_q31_len512 = {
	512, twiddleCoef_fast_512_q31, cskyBitRevIndexTable_fixed_512, CSKYBITREVINDEXTABLE_FIXED__512_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_fast_sR_q31_len1024 = {
	1024, twiddleCoef_fast_1024_q31, cskyBitRevIndexTable_fixed_1024, CSKYBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_fast_sR_q31_len2048 = {
	2048, twiddleCoef_fast_2048_q31, cskyBitRevIndexTable_fixed_2048, CSKYBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q31 csky_vdsp2_cfft_fast_sR_q31_len4096 = {
	4096, twiddleCoef_fast_4096_q31, cskyBitRevIndexTable_fixed_4096, CSKYBITREVINDEXTABLE_FIXED_4096_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_fast_sR_q15_len16 = {
	16, twiddleCoef_fast_16_q15, cskyBitRevIndexTable_fixed_16, CSKYBITREVINDEXTABLE_FIXED___16_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_fast_sR_q15_len32 = {
	32, twiddleCoef_fast_32_q15, cskyBitRevIndexTable_fixed_32, CSKYBITREVINDEXTABLE_FIXED___32_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_fast_sR_q15_len64 = {
	64, twiddleCoef_fast_64_q15, cskyBitRevIndexTable_fixed_64, CSKYBITREVINDEXTABLE_FIXED___64_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_fast_sR_q15_len128 = {
	128, twiddleCoef_fast_128_q15, cskyBitRevIndexTable_fixed_128, CSKYBITREVINDEXTABLE_FIXED__128_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_fast_sR_q15_len256 = {
	256, twiddleCoef_fast_256_q15, cskyBitRevIndexTable_fixed_256, CSKYBITREVINDEXTABLE_FIXED__256_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_fast_sR_q15_len512 = {
	512, twiddleCoef_fast_512_q15, cskyBitRevIndexTable_fixed_512, CSKYBITREVINDEXTABLE_FIXED__512_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_fast_sR_q15_len1024 = {
	1024, twiddleCoef_fast_1024_q15, cskyBitRevIndexTable_fixed_1024, CSKYBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_fast_sR_q15_len2048 = {
	2048, twiddleCoef_fast_2048_q15, cskyBitRevIndexTable_fixed_2048, CSKYBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH
};

const csky_vdsp2_cfft_instance_q15 csky_vdsp2_cfft_fast_sR_q15_len4096 = {
	4096, twiddleCoef_fast_4096_q15, cskyBitRevIndexTable_fixed_4096, CSKYBITREVINDEXTABLE_FIXED_4096_TABLE_LENGTH
};

/* The instances for RFFT and IRFFT.*/

/***************************Q15 RFFT* **********************/
csky_vdsp2_rfft_instance_q15 csky_vdsp2_rfft_sR_q15_len32 = {
    32, 0, 1, 1, (q15_t *)realCoefAQ15_32, &csky_vdsp2_cfft_sR_q15_len16};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_rfft_sR_q15_len64 = {
    64, 0, 1, 1, (q15_t *)realCoefAQ15_64, &csky_vdsp2_cfft_sR_q15_len32};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_rfft_sR_q15_len128 = {
    128, 0, 1, 1, (q15_t *)realCoefAQ15_128, &csky_vdsp2_cfft_sR_q15_len64};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_rfft_sR_q15_len256 = {
    256, 0, 1, 1, (q15_t *)realCoefAQ15_256, &csky_vdsp2_cfft_sR_q15_len128};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_rfft_sR_q15_len512 = {
    512, 0, 1, 1, (q15_t *)realCoefAQ15_512, &csky_vdsp2_cfft_sR_q15_len256};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_rfft_sR_q15_len1024 = {
    1024, 0, 1, 1, (q15_t *)realCoefAQ15_1024, &csky_vdsp2_cfft_sR_q15_len512};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_rfft_sR_q15_len2048 = {
    2048, 0, 1, 1, (q15_t *)realCoefAQ15_2048, &csky_vdsp2_cfft_sR_q15_len1024};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_rfft_sR_q15_len4096 = {
    4096, 0, 1, 1, (q15_t *)realCoefAQ15_4096, &csky_vdsp2_cfft_sR_q15_len2048};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_rfft_sR_q15_len8192 = {
    8192, 0, 1, 1, (q15_t *)realCoefAQ15_8192, &csky_vdsp2_cfft_sR_q15_len4096};

/***************************Q15 IRFFT* **********************/
csky_vdsp2_rfft_instance_q15 csky_vdsp2_inv_rfft_sR_q15_len32 = {
    32, 1, 1, 1, (q15_t *)realCoefAQ15_32, &csky_vdsp2_cfft_sR_q15_len16};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_inv_rfft_sR_q15_len64 = {
    64, 1, 1, 1, (q15_t *)realCoefAQ15_64, &csky_vdsp2_cfft_sR_q15_len32};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_inv_rfft_sR_q15_len128 = {
    128, 1, 1, 1, (q15_t *)realCoefAQ15_128, &csky_vdsp2_cfft_sR_q15_len64};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_inv_rfft_sR_q15_len256 = {
    256, 1, 1, 1, (q15_t *)realCoefAQ15_256, &csky_vdsp2_cfft_sR_q15_len128};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_inv_rfft_sR_q15_len512 = {
    512, 1, 1, 1, (q15_t *)realCoefAQ15_512, &csky_vdsp2_cfft_sR_q15_len256};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_inv_rfft_sR_q15_len1024 = {
    1024, 1, 1, 1, (q15_t *)realCoefAQ15_1024, &csky_vdsp2_cfft_sR_q15_len512};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_inv_rfft_sR_q15_len2048 = {
    2048, 1, 1, 1, (q15_t *)realCoefAQ15_2048, &csky_vdsp2_cfft_sR_q15_len1024};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_inv_rfft_sR_q15_len4096 = {
    4096, 1, 1, 1, (q15_t *)realCoefAQ15_4096, &csky_vdsp2_cfft_sR_q15_len2048};

csky_vdsp2_rfft_instance_q15 csky_vdsp2_inv_rfft_sR_q15_len8192 = {
    8192, 1, 1, 1, (q15_t *)realCoefAQ15_8192, &csky_vdsp2_cfft_sR_q15_len4096};

/************************* Q15 RFFT FAST *********************/
csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_rfft_fast_sR_q15_len32 = {
    32, 0, 1, (q15_t *)realCoefAQ15_32, (q15_t *)realCoefBQ15_32, &csky_vdsp2_cfft_fast_sR_q15_len16};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_rfft_fast_sR_q15_len64 = {
    64, 0, 1, (q15_t *)realCoefAQ15_64, (q15_t *)realCoefBQ15_64, &csky_vdsp2_cfft_fast_sR_q15_len32};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_rfft_fast_sR_q15_len128 = {
    128, 0, 1, (q15_t *)realCoefAQ15_128, (q15_t *)realCoefBQ15_128, &csky_vdsp2_cfft_fast_sR_q15_len64};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_rfft_fast_sR_q15_len256 = {
    256, 0, 1, (q15_t *)realCoefAQ15_256, (q15_t *)realCoefBQ15_256, &csky_vdsp2_cfft_fast_sR_q15_len128};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_rfft_fast_sR_q15_len512 = {
    512, 0, 1, (q15_t *)realCoefAQ15_512, (q15_t *)realCoefBQ15_512, &csky_vdsp2_cfft_fast_sR_q15_len256};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_rfft_fast_sR_q15_len1024 = {
    1024, 0, 1, (q15_t *)realCoefAQ15_1024, (q15_t *)realCoefBQ15_1024, &csky_vdsp2_cfft_fast_sR_q15_len512};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_rfft_fast_sR_q15_len2048 = {
    2048, 0, 1, (q15_t *)realCoefAQ15_2048, (q15_t *)realCoefBQ15_2048, &csky_vdsp2_cfft_fast_sR_q15_len1024};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_rfft_fast_sR_q15_len4096 = {
    4096, 0, 1, (q15_t *)realCoefAQ15_4096, (q15_t *)realCoefBQ15_4096, &csky_vdsp2_cfft_fast_sR_q15_len2048};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_rfft_fast_sR_q15_len8192 = {
    8192, 0, 1, (q15_t *)realCoefAQ15_8192, (q15_t *)realCoefBQ15_8192, &csky_vdsp2_cfft_fast_sR_q15_len4096};

/***************************Q15 IRFFT Fast **********************/
csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_inv_rfft_fast_sR_q15_len32 = {
    32, 1, 1, (q15_t *)realCoefAQ15_32, (q15_t *)realCoefBQ15_32, &csky_vdsp2_cfft_fast_sR_q15_len16};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_inv_rfft_fast_sR_q15_len64 = {
    64, 1, 1, (q15_t *)realCoefAQ15_64, (q15_t *)realCoefBQ15_64, &csky_vdsp2_cfft_fast_sR_q15_len32};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_inv_rfft_fast_sR_q15_len128 = {
    128, 1, 1, (q15_t *)realCoefAQ15_128, (q15_t *)realCoefBQ15_128, &csky_vdsp2_cfft_fast_sR_q15_len64};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_inv_rfft_fast_sR_q15_len256 = {
    256, 1, 1, (q15_t *)realCoefAQ15_256, (q15_t *)realCoefBQ15_256, &csky_vdsp2_cfft_fast_sR_q15_len128};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_inv_rfft_fast_sR_q15_len512 = {
    512, 1, 1, (q15_t *)realCoefAQ15_512, (q15_t *)realCoefBQ15_512, &csky_vdsp2_cfft_fast_sR_q15_len256};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_inv_rfft_fast_sR_q15_len1024 = {
    1024, 1, 1, (q15_t *)realCoefAQ15_1024, (q15_t *)realCoefBQ15_1024, &csky_vdsp2_cfft_fast_sR_q15_len512};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_inv_rfft_fast_sR_q15_len2048 = {
    2048, 1, 1, (q15_t *)realCoefAQ15_2048, (q15_t *)realCoefBQ15_2048, &csky_vdsp2_cfft_fast_sR_q15_len1024};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_inv_rfft_fast_sR_q15_len4096 = {
    4096, 1, 1, (q15_t *)realCoefAQ15_4096, (q15_t *)realCoefBQ15_4096, &csky_vdsp2_cfft_fast_sR_q15_len2048};

csky_vdsp2_rfft_fast_instance_q15 csky_vdsp2_inv_rfft_fast_sR_q15_len8192 = {
    8192, 1, 1, (q15_t *)realCoefAQ15_8192, (q15_t *)realCoefBQ15_8192, &csky_vdsp2_cfft_fast_sR_q15_len4096};

/***************************Q31 RFFT* **********************/
csky_vdsp2_rfft_instance_q31 csky_vdsp2_rfft_sR_q31_len32 = {
    32, 0, 1, 1, (q31_t *)realCoefAQ31_32, &csky_vdsp2_cfft_sR_q31_len16};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_rfft_sR_q31_len64 = {
    64, 0, 1, 1, (q31_t *)realCoefAQ31_64, &csky_vdsp2_cfft_sR_q31_len32};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_rfft_sR_q31_len128 = {
    128, 0, 1, 1, (q31_t *)realCoefAQ31_128, &csky_vdsp2_cfft_sR_q31_len64};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_rfft_sR_q31_len256 = {
    256, 0, 1, 1, (q31_t *)realCoefAQ31_256, &csky_vdsp2_cfft_sR_q31_len128};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_rfft_sR_q31_len512 = {
    512, 0, 1, 1, (q31_t *)realCoefAQ31_512, &csky_vdsp2_cfft_sR_q31_len256};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_rfft_sR_q31_len1024 = {
    1024, 0, 1, 1, (q31_t *)realCoefAQ31_1024, &csky_vdsp2_cfft_sR_q31_len512};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_rfft_sR_q31_len2048 = {
    2048, 0, 1, 1, (q31_t *)realCoefAQ31_2048, &csky_vdsp2_cfft_sR_q31_len1024};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_rfft_sR_q31_len4096 = {
    4096, 0, 1, 1, (q31_t *)realCoefAQ31_4096, &csky_vdsp2_cfft_sR_q31_len2048};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_rfft_sR_q31_len8192 = {
    8192, 0, 1, 1, (q31_t *)realCoefAQ31_8192, &csky_vdsp2_cfft_sR_q31_len4096};

/***************************Q31 IRFFT* **********************/
csky_vdsp2_rfft_instance_q31 csky_vdsp2_inv_rfft_sR_q31_len32 = {
    32, 1, 1, 1, (q31_t *)realCoefAQ31_32, &csky_vdsp2_cfft_sR_q31_len16};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_inv_rfft_sR_q31_len64 = {
    64, 1, 1, 1, (q31_t *)realCoefAQ31_64, &csky_vdsp2_cfft_sR_q31_len32};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_inv_rfft_sR_q31_len128 = {
    128, 1, 1, 1, (q31_t *)realCoefAQ31_128, &csky_vdsp2_cfft_sR_q31_len64};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_inv_rfft_sR_q31_len256 = {
    256, 1, 1, 1, (q31_t *)realCoefAQ31_256, &csky_vdsp2_cfft_sR_q31_len128};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_inv_rfft_sR_q31_len512 = {
    512, 1, 1, 1, (q31_t *)realCoefAQ31_512, &csky_vdsp2_cfft_sR_q31_len256};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_inv_rfft_sR_q31_len1024 = {
    1024, 1, 1, 1, (q31_t *)realCoefAQ31_1024, &csky_vdsp2_cfft_sR_q31_len512};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_inv_rfft_sR_q31_len2048 = {
    2048, 1, 1, 1, (q31_t *)realCoefAQ31_2048, &csky_vdsp2_cfft_sR_q31_len1024};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_inv_rfft_sR_q31_len4096 = {
    4096, 1, 1, 1, (q31_t *)realCoefAQ31_4096, &csky_vdsp2_cfft_sR_q31_len2048};

csky_vdsp2_rfft_instance_q31 csky_vdsp2_inv_rfft_sR_q31_len8192 = {
    8192, 1, 1, 1, (q31_t *)realCoefAQ31_8192, &csky_vdsp2_cfft_sR_q31_len4096};

/***************************Q31 RFFT FAST **********************/
csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_rfft_fast_sR_q31_len32 = {
    32, 0, 1, (q31_t *)realCoefAQ31_32, (q31_t *)realCoefBQ31_32, &csky_vdsp2_cfft_fast_sR_q31_len16};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_rfft_fast_sR_q31_len64 = {
    64, 0, 1, (q31_t *)realCoefAQ31_64, (q31_t *)realCoefBQ31_64, &csky_vdsp2_cfft_fast_sR_q31_len32};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_rfft_fast_sR_q31_len128 = {
    128, 0, 1, (q31_t *)realCoefAQ31_128, (q31_t *)realCoefBQ31_128, &csky_vdsp2_cfft_fast_sR_q31_len64};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_rfft_fast_sR_q31_len256 = {
    256, 0, 1, (q31_t *)realCoefAQ31_256, (q31_t *)realCoefBQ31_256, &csky_vdsp2_cfft_fast_sR_q31_len128};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_rfft_fast_sR_q31_len512 = {
    512, 0, 1, (q31_t *)realCoefAQ31_512, (q31_t *)realCoefBQ31_512, &csky_vdsp2_cfft_fast_sR_q31_len256};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_rfft_fast_sR_q31_len1024 = {
    1024, 0, 1, (q31_t *)realCoefAQ31_1024, (q31_t *)realCoefBQ31_1024, &csky_vdsp2_cfft_fast_sR_q31_len512};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_rfft_fast_sR_q31_len2048 = {
    2048, 0, 1, (q31_t *)realCoefAQ31_2048, (q31_t *)realCoefBQ31_2048, &csky_vdsp2_cfft_fast_sR_q31_len1024};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_rfft_fast_sR_q31_len4096 = {
    4096, 0, 1, (q31_t *)realCoefAQ31_4096, (q31_t *)realCoefBQ31_4096, &csky_vdsp2_cfft_fast_sR_q31_len2048};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_rfft_fast_sR_q31_len8192 = {
    8192, 0, 1, (q31_t *)realCoefAQ31_8192, (q31_t *)realCoefBQ31_8192, &csky_vdsp2_cfft_fast_sR_q31_len4096};

/***************************Q31 IRFFT FAST **********************/
csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_inv_rfft_fast_sR_q31_len32 = {
    32, 1, 1, (q31_t *)realCoefAQ31_32, (q31_t *)realCoefBQ31_32, &csky_vdsp2_cfft_fast_sR_q31_len16};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_inv_rfft_fast_sR_q31_len64 = {
    64, 1, 1, (q31_t *)realCoefAQ31_64, (q31_t *)realCoefBQ31_64, &csky_vdsp2_cfft_fast_sR_q31_len32};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_inv_rfft_fast_sR_q31_len128 = {
    128, 1, 1, (q31_t *)realCoefAQ31_128, (q31_t *)realCoefBQ31_128, &csky_vdsp2_cfft_fast_sR_q31_len64};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_inv_rfft_fast_sR_q31_len256 = {
    256, 1, 1, (q31_t *)realCoefAQ31_256, (q31_t *)realCoefBQ31_256, &csky_vdsp2_cfft_fast_sR_q31_len128};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_inv_rfft_fast_sR_q31_len512 = {
    512, 1, 1, (q31_t *)realCoefAQ31_512, (q31_t *)realCoefBQ31_512, &csky_vdsp2_cfft_fast_sR_q31_len256};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_inv_rfft_fast_sR_q31_len1024 = {
    1024, 1, 1, (q31_t *)realCoefAQ31_1024, (q31_t *)realCoefBQ31_1024, &csky_vdsp2_cfft_fast_sR_q31_len512};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_inv_rfft_fast_sR_q31_len2048 = {
    2048, 1, 1, (q31_t *)realCoefAQ31_2048, (q31_t *)realCoefBQ31_2048, &csky_vdsp2_cfft_fast_sR_q31_len1024};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_inv_rfft_fast_sR_q31_len4096 = {
    4096, 1, 1, (q31_t *)realCoefAQ31_4096, (q31_t *)realCoefBQ31_4096, &csky_vdsp2_cfft_fast_sR_q31_len2048};

csky_vdsp2_rfft_fast_instance_q31 csky_vdsp2_inv_rfft_fast_sR_q31_len8192 = {
    8192, 1, 1, (q31_t *)realCoefAQ31_8192, (q31_t *)realCoefBQ31_8192, &csky_vdsp2_cfft_fast_sR_q31_len4096};

/***************************Float RFFT and IRFFT***********************/
csky_vdsp2_rfft_fast_instance_f32 csky_vdsp2_rfft_sR_f32_len32 = {
    {16, (float32_t *)twiddleCoef_16, cskyBitRevIndexTable16,
    CSKYBITREVINDEXTABLE__16_TABLE_LENGTH}, 32, (float32_t *)twiddleCoef_rfft_32};

csky_vdsp2_rfft_fast_instance_f32 csky_vdsp2_rfft_sR_f32_len64 = {
    {32, (float32_t *)twiddleCoef_32, cskyBitRevIndexTable32,
    CSKYBITREVINDEXTABLE__32_TABLE_LENGTH}, 64, (float32_t *)twiddleCoef_rfft_64};

csky_vdsp2_rfft_fast_instance_f32 csky_vdsp2_rfft_sR_f32_len128 = {
    {64, (float32_t *)twiddleCoef_64, cskyBitRevIndexTable64,
    CSKYBITREVINDEXTABLE__64_TABLE_LENGTH}, 128, (float32_t *)twiddleCoef_rfft_128};

csky_vdsp2_rfft_fast_instance_f32 csky_vdsp2_rfft_sR_f32_len256 = {
    {128, (float32_t *)twiddleCoef_128, cskyBitRevIndexTable128,
    CSKYBITREVINDEXTABLE_128_TABLE_LENGTH}, 256, (float32_t *)twiddleCoef_rfft_256};

csky_vdsp2_rfft_fast_instance_f32 csky_vdsp2_rfft_sR_f32_len512 = {
    {256, (float32_t *)twiddleCoef_256, cskyBitRevIndexTable256,
    CSKYBITREVINDEXTABLE_256_TABLE_LENGTH}, 512, (float32_t *)twiddleCoef_rfft_512};

csky_vdsp2_rfft_fast_instance_f32 csky_vdsp2_rfft_sR_f32_len1024 = {
    {512, (float32_t *)twiddleCoef_512, cskyBitRevIndexTable512,
    CSKYBITREVINDEXTABLE_512_TABLE_LENGTH}, 1024, (float32_t *)twiddleCoef_rfft_1024};

csky_vdsp2_rfft_fast_instance_f32 csky_vdsp2_rfft_sR_f32_len2048 = {
    {1024, (float32_t *)twiddleCoef_1024, cskyBitRevIndexTable1024,
    CSKYBITREVINDEXTABLE1024_TABLE_LENGTH}, 2048, (float32_t *)twiddleCoef_rfft_2048};

csky_vdsp2_rfft_fast_instance_f32 csky_vdsp2_rfft_sR_f32_len4096 = {
    {2048, (float32_t *)twiddleCoef_2048, cskyBitRevIndexTable2048,
    CSKYBITREVINDEXTABLE2048_TABLE_LENGTH}, 4096, (float32_t *)twiddleCoef_rfft_4096};

csky_vdsp2_rfft_fast_instance_f32 csky_vdsp2_rfft_sR_f32_len8192 = {
    {4096, (float32_t *)twiddleCoef_4096, cskyBitRevIndexTable4096,
    CSKYBITREVINDEXTABLE4096_TABLE_LENGTH}, 8192, (float32_t *)twiddleCoef_rfft_8192};

/**********************************Q15 DCT*******************************/
csky_vdsp2_cfft_radix4_instance_q15 pCfft_q15;

csky_vdsp2_dct4_instance_q15 csky_vdsp2_dct4_sR_q15_len128 = {
128, 64, 0x1000, (q15_t *)WeightsQ15_128, (q15_t *)cos_factorsQ15_128,
&csky_vdsp2_rfft_sR_q15_len128, &pCfft_q15};

csky_vdsp2_dct4_instance_q15 csky_vdsp2_dct4_sR_q15_len512 = {
512, 256, 0x800, (q15_t *)WeightsQ15_512, (q15_t *)cos_factorsQ15_512,
&csky_vdsp2_rfft_sR_q15_len512, &pCfft_q15};

csky_vdsp2_dct4_instance_q15 csky_vdsp2_dct4_sR_q15_len2048 = {
2048, 1024, 0x400, (q15_t *)WeightsQ15_2048, (q15_t *)cos_factorsQ15_2048,
&csky_vdsp2_rfft_sR_q15_len2048, &pCfft_q15};

csky_vdsp2_dct4_instance_q15 csky_vdsp2_dct4_sR_q15_len8192 = {
8192, 4096, 0x200, (q15_t *)WeightsQ15_8192, (q15_t *)cos_factorsQ15_8192,
&csky_vdsp2_rfft_sR_q15_len8192, &pCfft_q15};

/*******************************Q15 DCT FAST*****************************/

csky_vdsp2_dct4_fast_instance_q15 csky_vdsp2_dct4_fast_sR_q15_len128 = {
128, 64, 0x1000, (q15_t *)WeightsQ15_128, (q15_t *)cos_factorsQ15_128,
&csky_vdsp2_rfft_fast_sR_q15_len128, &pCfft_q15};

csky_vdsp2_dct4_fast_instance_q15 csky_vdsp2_dct4_fast_sR_q15_len512 = {
512, 256, 0x800, (q15_t *)WeightsQ15_512, (q15_t *)cos_factorsQ15_512,
&csky_vdsp2_rfft_fast_sR_q15_len512, &pCfft_q15};

csky_vdsp2_dct4_fast_instance_q15 csky_vdsp2_dct4_fast_sR_q15_len2048 = {
2048, 1024, 0x400, (q15_t *)WeightsQ15_2048, (q15_t *)cos_factorsQ15_2048,
&csky_vdsp2_rfft_fast_sR_q15_len2048, &pCfft_q15};

csky_vdsp2_dct4_fast_instance_q15 csky_vdsp2_dct4_fast_sR_q15_len8192 = {
8192, 4096, 0x200, (q15_t *)WeightsQ15_8192, (q15_t *)cos_factorsQ15_8192,
&csky_vdsp2_rfft_fast_sR_q15_len8192, &pCfft_q15};

/**********************************Q31 DCT*******************************/
csky_vdsp2_cfft_radix4_instance_q31 pCfft_q31;

csky_vdsp2_dct4_instance_q31 csky_vdsp2_dct4_sR_q31_len128 = {
128, 64, 0x10000000, (q31_t *)WeightsQ31_128, (q31_t *)cos_factorsQ31_128,
&csky_vdsp2_rfft_sR_q31_len128, &pCfft_q31};

csky_vdsp2_dct4_instance_q31 csky_vdsp2_dct4_sR_q31_len512 = {
512, 256, 0x8000000, (q31_t *)WeightsQ31_512, (q31_t *)cos_factorsQ31_512,
&csky_vdsp2_rfft_sR_q31_len512, &pCfft_q31};

csky_vdsp2_dct4_instance_q31 csky_vdsp2_dct4_sR_q31_len2048 = {
2048, 1024, 0x4000000, (q31_t *)WeightsQ31_2048, (q31_t *)cos_factorsQ31_2048,
&csky_vdsp2_rfft_sR_q31_len2048, &pCfft_q31};

csky_vdsp2_dct4_instance_q31 csky_vdsp2_dct4_sR_q31_len8192 = {
8192, 4096, 0x2000000, (q31_t *)WeightsQ31_8192, (q31_t *)cos_factorsQ31_8192,
&csky_vdsp2_rfft_sR_q31_len8192, &pCfft_q31};

/********************************Q31 DCT FAST*******************************/
csky_vdsp2_cfft_radix4_instance_q31 pCfft_q31;

csky_vdsp2_dct4_fast_instance_q31 csky_vdsp2_dct4_fast_sR_q31_len128 = {
128, 64, 0x10000000, (q31_t *)WeightsQ31_128, (q31_t *)cos_factorsQ31_128,
&csky_vdsp2_rfft_fast_sR_q31_len128, &pCfft_q31};

csky_vdsp2_dct4_fast_instance_q31 csky_vdsp2_dct4_fast_sR_q31_len512 = {
512, 256, 0x8000000, (q31_t *)WeightsQ31_512, (q31_t *)cos_factorsQ31_512,
&csky_vdsp2_rfft_fast_sR_q31_len512, &pCfft_q31};

csky_vdsp2_dct4_fast_instance_q31 csky_vdsp2_dct4_fast_sR_q31_len2048 = {
2048, 1024, 0x4000000, (q31_t *)WeightsQ31_2048, (q31_t *)cos_factorsQ31_2048,
&csky_vdsp2_rfft_fast_sR_q31_len2048, &pCfft_q31};

csky_vdsp2_dct4_fast_instance_q31 csky_vdsp2_dct4_fast_sR_q31_len8192 = {
8192, 4096, 0x2000000, (q31_t *)WeightsQ31_8192, (q31_t *)cos_factorsQ31_8192,
&csky_vdsp2_rfft_fast_sR_q31_len8192, &pCfft_q31};

/**********************************Float DCT*******************************/
csky_vdsp2_cfft_radix4_instance_f32 pCfft_f32;

csky_vdsp2_dct4_instance_f32 csky_vdsp2_dct4_sR_f32_len128 = {
128, 64, 0.125, (float32_t *)Weights_128, (float32_t *)cos_factors_128,
&csky_vdsp2_rfft_sR_f32_len128, &pCfft_f32};

csky_vdsp2_dct4_instance_f32 csky_vdsp2_dct4_sR_f32_len512 = {
512, 256, 0.0625, (float32_t *)Weights_512, (float32_t *)cos_factors_512,
&csky_vdsp2_rfft_sR_f32_len512, &pCfft_f32};

csky_vdsp2_dct4_instance_f32 csky_vdsp2_dct4_sR_f32_len2048 = {
2048, 1024, 0.03125, (float32_t *)Weights_2048, (float32_t *)cos_factors_2048,
&csky_vdsp2_rfft_sR_f32_len2048, &pCfft_f32};

csky_vdsp2_dct4_instance_f32 csky_vdsp2_dct4_sR_f32_len8192 = {
8192, 4096, 0.015625, (float32_t *)Weights_8192, (float32_t *)cos_factors_8192,
&csky_vdsp2_rfft_sR_f32_len8192, &pCfft_f32};
