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
 * @file     csky_dot_prod_yv_q31.S
 * @brief    Q31 dot product.
 * @version  V1.0
 * @date     13. Nov 2017
 ******************************************************************************/

/**
 * q31_t csky_dsp_lib_vec_dot_q31(
 * q31_t * A,
 * q31_t * B,
 * uint32_t N)
 *
**/


    .file           "csky_dot_prod_q31_yv.S"
    .section        .text.csky_dsp_lib_vec_dot_q31,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_vec_dot_q31
    .type           csky_dsp_lib_vec_dot_q31, @function

csky_dsp_lib_vec_dot_q31:
    movi            t0, 0               // sum = 0
    movi            t1, 0
    lsri            t2, a2, 2           // blkCnt = N >> 2u
    andi            t3, a2, 3           // blkCnt = N % 0x04u
    bez             t2, .L2

.L4:
    pldbi.d         t4, (a0)            // load 2 q31_t data from source a
    pldbi.d         t8, (a1)            // load 2 q31_t data from source b
    mula.s32.s      t0, t4, t8          // multiply and add

    mula.s32.s      t0, t5, t9          // another data

    pldbi.d         t4, (a0)            // load 2 q31_t data from source a
    pldbi.d         t8, (a1)            // load 2 q31_t data from source b
    mula.s32.s      t0, t4, t8          // multiply
.L5:
    mula.s32.s      t0, t5, t9          // another data

    bloop           t2, .L4, .L5        // while loop

.L2:
    bez             t3, .L0             // the condition to end

.L3:
    ldbi.w          t4, (a0)            // same as .L4 loop
    ldbi.w          t5, (a1)
.L1:
    mula.s32.s      t0, t4, t5          // another data

    bloop           t3, .L3, .L1        // while loop

.L0:
    asri            t2, t1, 30
    bmaski          t4, 31
    bmaski          t6, 0
    bgeni           t5, 31

    bhz             t2, .L6
    cmplt           t2, t6
    bt              .L8
    dexti           t5, t0, t1, 31
    br              .L8

.L6:
    mov             a0, t4
    br              .L9

.L8:
    mov             a0, t5             // result = sum

.L9:
    rts                                // end
    .size           csky_dsp_lib_vec_dot_q31, .-csky_dsp_lib_vec_dot_q31
