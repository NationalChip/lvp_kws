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
 * @file     csky_max_q7.S
 * @brief    Maximum value of a Q7 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_max_q7(
 *   q7_t * pSrc,
 *   uint32_t blockSize,
 *   q7_t * pResult,
 *   uint32_t * pIndex)
 */

    .file           "csky_max_q7.S"
    .section        .text.csky_max_q7,"ax",@progbits
    .align          2
    .global         csky_max_q7
    .type           csky_max_q7, @function
csky_max_q7:
    movi            t0, 0               // count = 0
    movi            t1, 0               // outIndex = 0
    ldbi.bs         t2, (a0)            // out = *pSrc++

    subi            a1, a1, 1           // blockSize - 1u
    lsri            t3, a1, 3
    bez             t3, .L0
.L1:
    pldbi.d         t4, (a0)            // maxVal1, maxVal2
    sext            t6, t4, 7, 0
    cmplt           t2, t6
    movt            t2, t6
    inct            t1, t0, 1           // outIndex = count + 1

    sext            t6, t4, 15, 8
    cmplt           t2, t6
    movt            t2, t6
    inct            t1, t0, 2           // outIndex = count + 2

    sext            t6, t4, 23, 16
    cmplt           t2, t6
    movt            t2, t6
    inct            t1, t0, 3           // outIndex = count + 3

    sext            t6, t4, 31, 24
    cmplt           t2, t6
    movt            t2, t6
    inct            t1, t0, 4           // outIndex = count + 4

    sext            t6, t5, 7, 0
    cmplt           t2, t6
    movt            t2, t6
    inct            t1, t0, 5           // outIndex = count + 5

    sext            t6, t5, 15, 8
    cmplt           t2, t6
    movt            t2, t6
    inct            t1, t0, 6           // outIndex = count + 6

    sext            t6, t5, 23, 16
    cmplt           t2, t6
    movt            t2, t6
    inct            t1, t0, 7           // outIndex = count + 7

    sext            t6, t5, 31, 24
    cmplt           t2, t6
    movt            t2, t6
    inct            t1, t0, 8           // outIndex = count + 8

.L2:
    addi            t0, t0, 8
    bloop           t3, .L1, .L2
.L0:

    andi            a1, a1, 7           // (blockSize - 1u) % 8
    bez             a1, .L3
.L4:
    ldbi.bs         t4, (a0)            // maxVal1
    cmplt           t2, t4
    movt            t2, t4
    inct            t1, t0, 1

.L5:
    addi            t0, t0, 1
    bloop           a1, .L4, .L5

.L3:
    st.b            t2, (a2, 0x0)       // *pResult = out
    st.w            t1, (a3, 0x0)       // *pIndex = outIndex
    rts
    .size           csky_max_q7, .-csky_max_q7
