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
 * @file     csky_fir_q15.S
 * @brief    Q15 FIR filter processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_fir_q15(
 *   const csky_fir_instance_q15 * S,
 *   q15_t * pSrc,
 *   q15_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_fir_q15.S"
    .section        .text.csky_fir_q15,"ax",@progbits
    .align          2
    .global         csky_fir_q15
    .type           csky_fir_q15, @function
csky_fir_q15:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9

    subi            sp, sp, 4

    ld.w            t0, (a0, 0x4)       // pState
    st.w            t0, (sp, 0x0)
    ld.w            t1, (a0, 0x8)       // pCoeffs
    ld.hs           t2, (a0, 0x0)       // numTaps

    lsli            t3, t2, 1
    addu            t3, t3, t0
    subi            t3, t3, 2           // pStateCurnt

    lsri            t4, a3, 2           // blkCnt = blockSize >> 2

    bez             t4, .L2
.L0:
    pldbi.d         t8, (a1)            // pSrc
    stbi.w          t8, (t3)            // pStateCurnt
    stbi.w          t9, (t3)

    movi            l0, 0
    movi            l1, 0               // acc0
    movi            l2, 0
    movi            l3, 0               // acc1
    movi            l4, 0
    movi            l5, 0               // acc2
    movi            l6, 0
    movi            l7, 0               // acc3

    mov             l8, t0              // px1 = pState
    mov             l9, t1              // pb = pCoeffs

    ld.w            t7, (l8, 0x0)       // x0
    addi            l8, l8, 2
    ld.w            t8, (l8, 0x0)       // x1
    addi            l8, l8, 2           // px1 += 2

    lsri            t5, t2, 2           // tapCnt = numTaps >> 2
    bez             t5, .L3
.L4:
    ldbi.w          t6, (l9)            // c0
    mulaca.s16.e    l0, t7, t6          // acc0
    mulaca.s16.e    l2, t8, t6          // acc1
    ld.w            t9, (l8, 0x0)       // x2
    addi            l8, l8, 2
    ld.w            a0, (l8, 0x0)       // x3
    addi            l8, l8, 2
    mulaca.s16.e    l4, t9, t6          // acc2
    mulaca.s16.e    l6, a0, t6          // acc3

    ldbi.w          t6, (l9)            // c0
    mulaca.s16.e    l0, t9, t6          // acc0
    mulaca.s16.e    l2, a0, t6          // acc1
    ld.w            t7, (l8, 0x0)       // x0
    addi            l8, l8, 2
    ld.w            t8, (l8, 0x0)       // x1
    addi            l8, l8, 2
    mulaca.s16.e    l4, t7, t6          // acc2
.L5:
    mulaca.s16.e    l6, t8, t6          // acc3

    bloop           t5, .L4, .L5
.L3:

    andi            t5, t2, 3           // numTaps & 0x3
    bez             t5, .L6
.L7:
    ldbi.w          t6, (l9)            // c0
    ld.w            t9, (l8, 0x0)       // x2
    addi            l8, l8, 2
    ld.w            a0, (l8, 0x0)       // x3
    addi            l8, l8, 2

    mulaca.s16.e    l0, t7, t6          // acc0
    mulaca.s16.e    l2, t8, t6          // acc1
    mulaca.s16.e    l4, t9, t6          // acc2
.L8:
    mulaca.s16.e    l6, a0, t6          // acc3

    bloop           t5, .L7, .L8
.L6:

    dexti           l0, l0, l1, 15
    dexti           l2, l2, l3, 15
    dexti           l4, l4, l5, 15
    dexti           l6, l6, l7, 15
    clipi.s32       l0, l0, 16
    clipi.s32       l2, l2, 16
    clipi.s32       l4, l4, 16
    clipi.s32       l6, l6, 16
    pkgll           l0, l0, l2
    pkgll           l4, l4, l6
    stbi.w          l0, (a2)
    stbi.w          l4, (a2)
    addi            t0, t0, 8           // pState = pState + 4
    subi            t4, t4, 1           // blkCnt--
    bnez            t4, .L0
.L2:

    andi            t4, a3, 3           // blkCnt = blockSize % 0x4
    bez             t4, .L10
.L11:
    ldbi.h          t7, (a1)
    stbi.h          t7, (t3)

    movi            l0, 0
    movi            l1, 0

    mov             l8, t0              // px1 = pState
    mov             l9, t1              // pb = pCoeffs

    lsri            t5, t2, 1           // numTaps >> 1

.L13:
    ldbi.w          t7, (l8)
    ldbi.w          t8, (l9)
.L14:
    mulaca.s16.e    l0, t7, t8          // acc0

    bloop           t5, .L13, .L14

    dexti           l0, l0, l1, 15
    clipi.s32       l0, l0, 16
    stbi.h          l0, (a2)

    addi            t0, t0, 2           // pState = pState + 1
    subi            t4, t4, 1
    bnez            t4, .L11
.L10:

    ld.w            t3, (sp, 0x0)       // pStateCurnt
    subi            t2, t2, 1
    lsri            a0, t2, 2           // tapCnt
    bez             a0, .L15
.L16:
    pldbi.d         l0, (t0)
    stbi.w          l0, (t3)
.L17:
    stbi.w          l1, (t3)
    bloop           a0, .L16, .L17
.L15:

    andi            a0, t2, 3           // tapCnt
    bez             a0, .L18
.L19:
    ldbi.h          l0, (t0)
.L20:
    stbi.h          l0, (t3)
    bloop           a0, .L19, .L20
.L18:
    addi            sp, sp, 4
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_fir_q15, .-csky_fir_q15
