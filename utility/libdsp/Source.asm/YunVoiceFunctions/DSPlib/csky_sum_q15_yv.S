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
 * @file     csky_sum_q15_yv.S
 * @brief    Q15 vector sum.
 * @version  V1.0
 * @date     14. Nov 2017
 ******************************************************************************/
/**
 * q63_t csky_dsp_lib_vec_sum16(
 * q15_t * A,
 * uint32_t N)
 *
**/

    .file           "csky_sum_q15_yv.S"
    .section        .text.csky_dsp_lib_vec_sum16,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_vec_sum16
    .type           csky_dsp_lib_vec_sum16, @function

csky_dsp_lib_vec_sum16:
    lsri            t0, a1, 2           // blkCnt = N >> 2u
    andi            t1, a1, 3           // blkCnt = N % 0x4u
    movi            t2, 1
    dup.16          t7, t2, 0
    movi            t8, 0
    movi            t9, 0
    bez             t0, .L2

.L4:
    pldbi.d         t2, (a0)            // load 4 q15_t data
    mulaca.s16.e    t8, t7, t2
.L5:
    mulaca.s16.e    t8, t7, t3

    bloop           t0, .L4, .L5        // while loop

.L2:
    bez             t1, .L0             // the condition to end

.L3:
    ldbi.h          t2, (a0)            // load 1 q15_t data
.L1:
    mulall.s16.e    t8, t7, t2

    bloop           t1, .L3, .L1        // while loop

.L0:
    mov             a0, t8
    mov             a1, t9
    rts                                 // end
    .size           csky_dsp_lib_vec_sum16, .-csky_dsp_lib_vec_sum16
