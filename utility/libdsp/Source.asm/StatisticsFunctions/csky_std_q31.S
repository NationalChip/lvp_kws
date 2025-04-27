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
 * @file     csky_std_q31.S
 * @brief    Root Mean Square of the elements of a Q31 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

.import csky_sqrt_q31

/*
 * void csky_std_q31(
 *   q31_t * pSrc,
 *   uint32_t blockSize,
 *   q31_t * pResult)
 */

    .file           "csky_std_q31.S"
    .section        .text.csky_std_q31,"ax",@progbits
    .align          2
    .global         csky_std_q31
    .type           csky_std_q31, @function
csky_std_q31:
    push            lr
    movi            t0, 0
    movi            t1, 0               // sumOfSquares
    movi            t2, 0
    movi            t3, 0               // sum
    movi            t8, 1
    lsri            a3, a1, 2           // blkCnt
    bez             a3, .L0
.L1:
    pldbi.d         t4, (a0)
    pldbi.d         t6, (a0)
    asri            t4, t4, 8
    asri            t5, t5, 8
    asri            t6, t6, 8
    asri            t7, t7, 8
    mula.s32        t0, t4, t4
    mula.s32        t2, t4, t8
    mula.s32        t0, t5, t5
    mula.s32        t2, t5, t8
    mula.s32        t0, t6, t6
    mula.s32        t2, t6, t8
    mula.s32        t0, t7, t7
.L2:
    mula.s32        t2, t7, t8

    bloop           a3, .L1, .L2
.L0:

    andi            a3, a1, 3
    bez             a3, .L3
.L4:
    ldbi.w          t4, (a0)
    asri            t4, t4, 8
    mula.s32        t0, t4, t4
.L5:
    mula.s32        t2, t4, t8

    bloop           a3, .L4, .L5

.L3:
    subi            a0, a1, 1
    divsl           t0, t0, a0          // meanOfSquares

    mul.u32         t4, t2, t2
    mult            t2, t2, t3
    addu            t5, t5, t2
    addu            t5, t5, t2

    divsl           t2, t4, a0
    divsl           t2, t2, a1          // squareOfMean

    sub.64          t0, t0, t2
    dexti           a0, t0, t1, 15
    mov             a1, a2
    jbsr            csky_sqrt_q31
    pop             lr
    .size           csky_std_q31, .-csky_std_q31
