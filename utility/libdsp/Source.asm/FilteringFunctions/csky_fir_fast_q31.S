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
 * @file     csky_fir_fast_q31.S
 * @brief    Processing function for the Q31 Fast FIR filter.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_fir_fast_q31(
 *   const csky_fir_instance_q31 * S,
 *   q31_t * pSrc,
 *   q31_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_fir_fast_q31.S"
    .section        .text.csky_fir_fast_q31,"ax",@progbits
    .align          2
    .global         csky_fir_fast_q31
    .type           csky_fir_fast_q31, @function
csky_fir_fast_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9

    ld.w            t0, (a0, 0x4)       // pState
    ld.w            t1, (a0, 0x8)       // pCoeffs
    ld.hs           t2, (a0, 0x0)       // numTaps

    lsli            t3, t2, 2
    addu            t3, t3, t0
    subi            t3, t3, 4           // pStateCurnt

    lsri            t4, a3, 2           // blkCnt = blockSize >> 2
    bez             t4, .L2

.L0:
    pldbi.d         t7, (a1)            // pSrc
    stbi.w          t7, (t3)            // pStateCurnt
    stbi.w          t8, (t3)
    pldbi.d         t7, (a1)            // pSrc
    stbi.w          t7, (t3)            // pStateCurnt
    stbi.w          t8, (t3)

    movi            l0, 0               // acc0
    movi            l1, 0               // acc1
    movi            l2, 0               // acc2
    movi            l3, 0               // acc3

    mov             l8, t0              // px = pState
    mov             l9, t1              // pb = pCoeffs

    ldbi.w          t7, (l8)            // x0
    ldbi.w          t8, (l8)            // x1
    ldbi.w          l4, (l8)            // x2

    lsri            t9, t2, 2           // i = tapCnt
    bez             t9, .L3
.L4:
    ldbi.w          l6, (l9)            // c0
    ldbi.w          l5, (l8)            // x3

    mula.s32.rhs    l0, t7, l6
    mula.s32.rhs    l1, t8, l6
    mula.s32.rhs    l2, l4, l6
    mula.s32.rhs    l3, l5, l6

    ldbi.w          l6, (l9)            // c0
    ldbi.w          t7, (l8)            // x0

    mula.s32.rhs    l0, t8, l6
    mula.s32.rhs    l1, l4, l6
    mula.s32.rhs    l2, l5, l6
    mula.s32.rhs    l3, t7, l6

    ldbi.w          l6, (l9)            // c0
    ldbi.w          t8, (l8)            // x1

    mula.s32.rhs    l0, l4, l6
    mula.s32.rhs    l1, l5, l6
    mula.s32.rhs    l2, t7, l6
    mula.s32.rhs    l3, t8, l6

    ldbi.w          l6, (l9)            // c0
    ldbi.w          l4, (l8)            // x2

    mula.s32.rhs    l0, l5, l6
    mula.s32.rhs    l1, t7, l6
    mula.s32.rhs    l2, t8, l6
.L5:
    mula.s32.rhs    l3, l4, l6

    bloop           t9, .L4, .L5
.L3:

    andi            t9, t2, 3
    bez             t9, .L6
.L7:
    ldbi.w          l6, (l9)            // c0
    ldbi.w          l5, (l8)            // x3

    mula.s32.rhs    l0, t7, l6
    mula.s32.rhs    l1, t8, l6
    mula.s32.rhs    l2, l4, l6
    mula.s32.rhs    l3, l5, l6

    mov             t7, t8
    mov             t8, l4
.L8:
    mov             l4, l5

    bloop           t9, .L7, .L8
.L6:

    lsli            l0, l0, 1
    lsli            l1, l1, 1
    lsli            l2, l2, 1
    lsli            l3, l3, 1
    stbi.w          l0, (a2)
    stbi.w          l1, (a2)
    stbi.w          l2, (a2)
    stbi.w          l3, (a2)

    addi            t0, t0, 16          // pState = pState + 4
    subi            t4, t4, 1           // blkCnt--
    bnez            t4, .L0
.L2:

    andi            t9, a3, 3
    bez             t9, .L10
.L11:
    ldbi.w          t7, (a1)
    stbi.w          t7, (t3)

    movi            l0, 0

    mov             l8, t0              // px = pState
    mov             l9, t1              // pb = pCoeffs

    mov             a0, t2              // i = numTaps

.L13:
    ldbi.w          t7, (l8)
    ldbi.w          t8, (l9)
.L14:
    mula.s32.rhs    l0, t7, t8          // acc0

    bloop           a0, .L13, .L14

    lsli            l0, l0, 1
    stbi.w          l0, (a2)

    addi            t0, t0, 4           // pState = pState + 1
    subi            t9, t9, 1
    bnez            t9, .L11
.L10:

    ld.w            t3, (a0, 0x4)       // pStateCurnt
    subi            t2, t2, 1
    lsri            a0, t2, 2           // tapCnt
    bez             a0, .L15
.L16:
    pldbi.d         l0, (t0)
    stbi.w          l0, (t3)
    stbi.w          l1, (t3)

    pldbi.d         l0, (t0)
    stbi.w          l0, (t3)
.L17:
    stbi.w          l1, (t3)
    bloop           a0, .L16, .L17
.L15:


    andi            a0, t2, 3           // tapCnt
    bez             a0, .L18
.L19:
    ldbi.w          l0, (t0)
.L20:
    stbi.w          l0, (t3)
    bloop           a0, .L19, .L20
.L18:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_fir_fast_q31, .-csky_fir_fast_q31
