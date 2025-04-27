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
 * @file     csky_min_q31.S
 * @brief    Minimum value of a Q31 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_min_q31(
 *   q31_t * pSrc,
 *   uint32_t blockSize,
 *   q31_t * pResult,
 *   uint32_t * pIndex)
 */

    .file           "csky_min_q31.S"
    .section        .text.csky_min_q31,"ax",@progbits
    .align          2
    .global         csky_min_q31
    .type           csky_min_q31, @function
csky_min_q31:
    movi            t0, 0               // count = 0
    movi            t1, 0               // outIndex = 0
    ldbi.w          t2, (a0)            // out = *pSrc++

    subi            a1, a1, 1           // blockSize - 1u
    lsri            t3, a1, 2
    bez             t3, .L0
.L1:
    pldbi.d         t4, (a0)            // minVal1, minVal2
    cmplt           t4, t2
    movt            t2, t4
    inct            t1, t0, 1           // outIndex = count + 1

    cmplt           t5, t2
    movt            t2, t5
    inct            t1, t0, 2           // outIndex = count + 2

    pldbi.d         t4, (a0)            // minVal1, minVal2
    cmplt           t4, t2
    movt            t2, t4
    inct            t1, t0, 3           // outIndex = count + 3

    cmplt           t5, t2
    movt            t2, t5
    inct            t1, t0, 4           // outIndex = count + 4

.L2:
    addi            t0, t0, 4
    bloop           t3, .L1, .L2
.L0:

    andi            a1, a1, 3           // (blockSize - 1u) % 4
    bez             a1, .L3
.L4:
    ldbi.w          t4, (a0)            // minVal1
    cmplt           t4, t2
    movt            t2, t4
    inct            t1, t0, 1

.L5:
    addi            t0, t0, 1
    bloop           a1, .L4, .L5

.L3:
    st.w            t2, (a2, 0x0)       // *pResult = out
    st.w            t1, (a3, 0x0)       // *pIndex = outIndex
    rts
    .size           csky_min_q31, .-csky_min_q31
