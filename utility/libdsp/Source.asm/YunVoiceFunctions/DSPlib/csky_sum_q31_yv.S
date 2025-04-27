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
 * q63_t csky_dsp_lib_vec_sum32(
 * q31_t * A,
 * uint32_t N)
 *
**/

    .file           "csky_sum_q31_yv.S"
    .section        .text.csky_dsp_lib_vec_sum32,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_vec_sum32
    .type           csky_dsp_lib_vec_sum32, @function

csky_dsp_lib_vec_sum32:
    lsri            t0, a1, 2           // blkCnt = N >> 2u
    andi            t1, a1, 3           // blkCnt = N % 0x4u
    movi            t8, 0
    movi            t9, 0
    movi            t6, 1
    bez             t0, .L2

.L4:
    pldbi.d         t2, (a0)            // load 4 q31_t data
    pldbi.d         t4, (a0)
    mula.s32.s      t8, t6, t2
    mula.s32.s      t8, t6, t3
    mula.s32.s      t8, t6, t4
.L5:
    mula.s32.s      t8, t6, t5
 
    bloop           t0, .L4, .L5        // while loop

.L2:
    bez             t1, .L0             // the condition to end

.L3:
    ldbi.w          t2, (a0)            // load 1 q31_t data
.L1:
    mula.s32.s      t8, t6, t2

    bloop           t1, .L3, .L1        // while loop

.L0:
    mov             a0, t8
    mov             a1, t9
    rts                                 // end
    .size           csky_dsp_lib_vec_sum32, .-csky_dsp_lib_vec_sum32
