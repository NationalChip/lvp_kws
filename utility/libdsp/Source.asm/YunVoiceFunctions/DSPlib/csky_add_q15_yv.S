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
 * @file     csky_add_q15_yv.S
 * @brief    Q15 vector addition.
 * @version  V1.0
 * @date     13. Nov 2017
 ******************************************************************************/
/**
 * void csky_dsp_lib_vec_add16(
 * q15_t * A,
 * q15_t * B,
 * uint32_t N,
 * q15_t * C)
 *
**/

    .file           "csky_add_q15_yv.S"
    .section        .text.csky_dsp_lib_vec_add16,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_vec_add16
    .type           csky_dsp_lib_vec_add16, @function

csky_dsp_lib_vec_add16:
    lsri            t0, a2, 2           // blkCnt = N >> 2u
    andi            t1, a2, 3           // blkCnt = N % 0x4u
    bez             t0, .L2

.L4:
    pldbi.d         t2, (a0)            // load 4 q15_t data from source a
    pldbi.d         t4, (a1)            // load 4 q15_t data from source b
    padd.s16.s      t6, t2, t4          // addition
    padd.s16.s      t7, t3, t5
    st.w            t6, (a3, 0x0)
    st.w            t7, (a3, 0x4)
.L5:
    addi            a3, a3, 8

    bloop           t0, .L4, .L5        // while loop

.L2:
    bez             t1, .L0             // the condition to end

.L3:
    ldbi.hs         t2, (a0)            // load 1 q15_t data from source a
    ldbi.hs         t4, (a1)            // load 1 q15_t data from source b
    padd.s16.s      t6, t2, t4          // addition
.L1:
    stbi.h          t6, (a3)            // store the result

    bloop           t1, .L3, .L1        // while loop

.L0:
    rts                                 // end
    .size           csky_dsp_lib_vec_add16, .-csky_dsp_lib_vec_add16
