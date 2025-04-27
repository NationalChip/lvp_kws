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
 * @file     csky_var_q15.S
 * @brief    Variance of an array of Q15 type.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

.import csky_sqrt_q15

/*
 * void csky_var_q15(
 *   q15_t * pSrc,
 *   uint32_t blockSize,
 *   q15_t * pResult)
 */

    .file           "csky_var_q15.S"
    .section        .text.csky_var_q15,"ax",@progbits
    .align          2
    .global         csky_var_q15
    .type           csky_var_q15, @function
csky_var_q15:
    movi            t0, 0
    movi            t1, 0               // sumOfSquares
    movi            t2, 0               // sum
    movi            t3, 1
    dup.16          t3, t3, 0           // 0x00010001
    lsri            a3, a1, 2           // blkCnt
    bez             a3, .L0
.L1:
    pldbi.d         t4, (a0)
    mulaca.s16.e    t0, t4, t4
    mulaca.s16.s    t2, t4, t3
    mulaca.s16.e    t0, t5, t5
.L2:
    mulaca.s16.s    t2, t5, t3

    bloop           a3, .L1, .L2
.L0:

    andi            a3, a1, 3
    bez             a3, .L3
.L4:
    ldbi.hs         t4, (a0)
    mulall.s16.e    t0, t4, t4
.L5:
    mulall.s16.s    t2, t4, t3

    bloop           a3, .L4, .L5

.L3:
    subi            a0, a1, 1
    divsl           t0, t0, a0          // meanOfSquares

    divs            t4, t2, a0
    divs            t5, t2, a1
    mul.s32         t4, t4, t5          // squareOfMean

    subu            t0, t0, t4
    lsri            a0, t0, 15
    st.h            a0, (a2, 0x0)
    rts
    .size           csky_var_q15, .-csky_var_q15
