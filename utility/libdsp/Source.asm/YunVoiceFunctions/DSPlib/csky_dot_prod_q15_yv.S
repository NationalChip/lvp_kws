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
 * @file     csky_dot_prod_q15_yv.S
 * @brief    Q15 dot product.
 * @version  V1.0
 * @date     13. Nov 2017
 ******************************************************************************/
/**
 * q31_t csky_dsp_lib_vec_dot_q15(
 * q15_t * A,
 * q15_t * B,
 * uint32_t N)
 *
**/

    .file           "csky_dot_prod_q15_dot_yv.S"
    .section        .text.csky_dsp_lib_vec_dot_q15,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_vec_dot_q15
    .type           csky_dsp_lib_vec_dot_q15, @function

csky_dsp_lib_vec_dot_q15:
    movi            t0, 0               // sum = 0
    movi            t1, 0
    lsri            t2, a2, 2           // blkCnt = N >> 2u
    andi            t3, a2, 3           // blkCnt = N % 0x04u
    bez             t2, .L2

.L4:
    pldbi.d         t4, (a0)            // load 4 q15_t data from source a
    pldbi.d         t6, (a1)            // load 4 q15_t data from source b
    mulaca.s16.s    t0, t4, t6          // mul and add
.L5:
    mulaca.s16.s    t0, t5, t7

    bloop           t2, .L4, .L5        // while loop

.L2:
    bez             t3, .L0             // condition to end

.L3:
    ldbi.h          t4, (a0)            // load 1 q15_t data from source a
    ldbi.h          t6, (a1)            // load 1 q15_t data from source b
.L1:
    mulall.s16.s    t0, t4, t6          // mul and add

    bloop           t3, .L3, .L1        // while loop

.L0:
    lsli.s32.s      a0, t0, 1
    rts
    .size           csky_dsp_lib_vec_dot_q15, .-csky_dsp_lib_vec_dot_q15
