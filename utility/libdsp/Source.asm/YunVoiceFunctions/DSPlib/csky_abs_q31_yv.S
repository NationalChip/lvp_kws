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
 * @file     csky_abs_q31_yv.S
 * @brief    Q31 Vector absolute value.
 * @version  V1.0
 * @date     13. Nov 2017
 ******************************************************************************/

/**
 * void csky_dsp_lib_vec_abs32(
 * q31_t * A,
 * uint32_t N,
 * q31_t * C)
 *
**/

    .file           "csky_abs_q31_yv.S"
    .section        .text.csky_dsp_lib_vec_abs32,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_vec_abs32
    .type           csky_dsp_lib_vec_abs32, @function

csky_dsp_lib_vec_abs32:
    lsri            a3, a1, 2           // blkCnt = N >> 2u
    andi            a1, a1, 3           // blkCnt = N % 0x4u
    bez             a3, .L2

.L4:
    pldbi.d         t2, (a0)            // load q31_t data
    pldbi.d         t4, (a0)
    abs.s32.s       t2, t2              // compute the abs data
    abs.s32.s       t3, t3
    abs.s32.s       t4, t4
    abs.s32.s       t5, t5
    st.w            t2, (a2, 0x0)
    st.w            t3, (a2, 0x4)
    st.w            t4, (a2, 0x8)
    st.w            t5, (a2, 0xc)
.L5:
    addi            a2, a2, 16
    bloop           a3, .L4, .L5        // while loop

.L2:
    bez             a1, .L0             // the condition to end

.L3:
    ldbi.w          t1, (a0)            // load 1 q31_t data
    abs.s32.s       t1, t1              // compute the abs data
.L1:
    stbi.w          t1, (a2)            // store the results

    bloop           a1, .L3, .L1        // while loop

.L0:
    rts                                 //end
    .size           csky_dsp_lib_vec_abs32, .-csky_dsp_lib_vec_abs32
