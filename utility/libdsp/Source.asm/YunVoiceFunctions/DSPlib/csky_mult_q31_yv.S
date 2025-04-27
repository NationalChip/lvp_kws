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
 * @file     csky_mult_q31_yv.S
 * @brief    Q31 vector multiplication.
 * @version  V1.0
 * @date     13. Nov 2017
 ******************************************************************************/

/**
 * void csky_dsp_lib_vec_mul_q31(
 * q31_t * A,
 * q31_t * B,
 * uint32_t N,
 * q31_t * C)
 *
**/

    .file           "csky_mult_q31_yv.S"
    .section        .text.csky_dsp_lib_vec_mul_q31,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_vec_mul_q31
    .type           csky_dsp_lib_vec_mul_q31, @function

csky_dsp_lib_vec_mul_q31:
    lsri            t0, a2, 2           // blkCnt = N >> 2u
    andi            t1, a2, 3           // blkCnt = N % 0x04
    bez             t0, .L2

.L4:
    pldbi.d         t2, (a0)            // load 2 q31_t data from source a
    pldbi.d         t4, (a1)            // load 2 q31_t data from source b
    rmul.s32.h      t6, t2, t4          // multiply
    rmul.s32.h      t7, t3, t5
    st.w            t6, (a3, 0x0)
    st.w            t7, (a3, 0x4)

    pldbi.d         t2, (a0)            // repeat
    pldbi.d         t4, (a1)
    rmul.s32.h      t6, t2, t4
    rmul.s32.h      t7, t3, t5
    st.w            t6, (a3, 0x8)
    st.w            t7, (a3, 0xc)
.L5:
    addi            a3, a3, 16

    bloop           t0, .L4, .L5        // while loop

.L2:
    bez             t1, .L0             // condition to end

.L3:
    ldbi.w          t2, (a0)            // load 1 q31_t data from source a
    ldbi.w          t4, (a1)            // load 1 q31_t data from source b
    rmul.s32.h      t6, t2, t4          // multiply
.L1:
    stbi.w          t6, (a3)            // store the result

    bloop           t1, .L3, .L1        // while loop

.L0:
    rts                                 // end
    .size           csky_dsp_lib_vec_mul_q31, .-csky_dsp_lib_vec_mul_q31
