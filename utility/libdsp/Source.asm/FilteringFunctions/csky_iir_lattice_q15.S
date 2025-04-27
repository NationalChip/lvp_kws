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
 * @file     csky_iir_lattice_q15.S
 * @brief    Q15 IIR lattice filter processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_iir_lattice_q15(
 *   const csky_iir_lattice_instance_q15 * S,
 *   q15_t * pSrc,
 *   q15_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_iir_lattice_q15.S"
    .section        .text.csky_iir_lattice_q15,"ax",@progbits
    .align          2
    .global         csky_iir_lattice_q15
    .type           csky_iir_lattice_q15, @function
csky_iir_lattice_q15:
    push            l0, l1, l2, l3, l4, l5
    mov             t2, a3              // blkCnt = blockSize
    ld.w            t3, (a0, 0x4)       // pState = &S->pState[0]
    ld.hs           t4, (a0, 0x0)       // numStages = S->numStages
    subi            t4, t4, 1

    bez             t2, .L0
.L1:
    ldbi.hs         t6, (a1)            // fcurr = *pSrc++
    mov             t5, t3              // px1 = pState
    movi            l0, 0
    movi            l1, 0               // acc

    ld.w            t0, (a0, 0xc)       // pv
    ld.w            t1, (a0, 0x8)       // pk

    ld.hs           t7, (t5, 0x0)       // gcurr
    ldbi.hs         t8, (t1)
    mult            t9, t7, t8
    asri            t9, t9, 15
    sub.s32.s       l2, t6, t9
    clipi.s32       l2, l2, 16          // fnext
    mult            t9, l2, t8
    asri            t9, t9, 15
    add.s32.s       l3, t7, t9
    clipi.s32       l3, l3, 16          // gnext
    stbi.h          l3, (t5)
    ldbi.hs         t9, (t0)
    mulall.s16.e    l0, l3, t9          // acc

    mov             t6, l2              // fcurr = fnext

    lsri            l4, t4, 2           // tapCnt
    bez             l4, .L5
.L3:
    ld.hs           t7, (t5, 0x0)       // gcurr
    ldbi.hs         t8, (t1)
    mult            t9, t7, t8
    asri            t9, t9, 15
    sub.s32.s       l2, t6, t9          // fnext
    clipi.s32       l2, l2, 16
    mult            t9, l2, t8
    asri            t9, t9, 15
    add.s32.s       l3, t7, t9          // gnext1
    clipi.s32       l3, l3, 16
    stbi.h          l3, (t5)

    ld.hs           t7, (t5, 0x0)       // gcurr
    ldbi.hs         t8, (t1)
    mult            t9, t7, t8
    asri            t9, t9, 15
    sub.s32.s       t6, l2, t9          // fcurr
    clipi.s32       t6, t6, 16
    mult            t9, t6, t8
    asri            t9, t9, 15
    add.s32.s       l5, t7, t9          // gnext2
    clipi.s32       l5, l5, 16
    stbi.h          l5, (t5)

    ldbi.w          t9, (t0)
    pkgll           l3, l3, l5
    mulaca.s16.e    l0, l3, t9          // acc

    ld.hs           t7, (t5, 0x0)       // gcurr
    ldbi.hs         t8, (t1)
    mult            t9, t7, t8
    asri            t9, t9, 15
    sub.s32.s       l2, t6, t9          // fnext
    clipi.s32       l2, l2, 16
    mult            t9, l2, t8
    asri            t9, t9, 15
    add.s32.s       l3, t7, t9          // gnext
    clipi.s32       l3, l3, 16
    stbi.h          l3, (t5)

    ld.hs           t7, (t5, 0x0)       // gcurr
    ldbi.hs         t8, (t1)
    mult            t9, t7, t8
    asri            t9, t9, 15
    sub.s32.s       t6, l2, t9          // fcurr
    clipi.s32       t6, t6, 16
    mult            t9, t6, t8
    asri            t9, t9, 15
    add.s32.s       l5, t7, t9          // gnext
    clipi.s32       l5, l5, 16
    stbi.h          l5, (t5)

    ldbi.w          t9, (t0)
    pkgll           l3, l3, l5
.L4:
    mulaca.s16.e    l0, l3, t9          // acc

    bloop           l4, .L3, .L4        // tapCnt--
.L5:
    mov             l2, t6              // fnext = fcurr

    andi            l4, t4, 3           // tapCnt
    bez             l4, .L8
.L6:
    ld.hs           t7, (t5, 0x0)       // gcurr
    ldbi.hs         t8, (t1)
    mult            t9, t7, t8
    asri            t9, t9, 15
    subu            l2, t6, t9
    clipi.s32       l2, l2, 16          // fnext
    mult            t9, l2, t8
    asri            t9, t9, 15
    addu            l3, t7, t9
    clipi.s32       l3, l3, 16          // gnext
    ldbi.hs         t9, (t0)
    mulall.s16.e    l0, l3, t9          // acc
    stbi.h          l3, (t5)

.L7:
    mov             t6, l2              // fcurr = fnext
    bloop           l4, .L6, .L7
.L8:

    ldbi.hs         t9, (t0)
    mulall.s16.e    l0, l2, t9          // acc

    stbi.h          l2, (t5)            // *px2++ = fnext

    dexti           l0, l0, l1, 15
    stbi.h          l0, (a2)            // *pDst++

    addi            t3, t3, 2           // pState = pState + 1
    subi            t2, t2, 1           // blkCnt--
    bnez            t2, .L1
.L0:

    ld.w            t0, (a0, 0x4)       // pStateCurnt = &S->pState[0]
    lsli            t1, a3, 2
    addu            t1, t1, t0          // pState = &S->pState[blockSize]

    addi            t4, t4, 1
    lsri            t2, t4, 2           // stgCnt = numStages >> 2
    bez             t2, .L12
.L10:
    pldbi.d         l0, (t1)
    stbi.w          l0, (t0)
.L11:
    stbi.w          l1, (t0)

    bloop           t2, .L10, .L11
.L12:

    andi            t2, t4, 3           // stgCnt = (numStages) % 0x4
    bez             t2, .L14
.L22:
    ldbi.h          l0, (t1)
.L13:
    stbi.h          l0, (t0)

    bloop           t2, .L22, .L13
.L14:

    pop             l0, l1, l2, l3, l4, l5
    .size           csky_iir_lattice_q15, .-csky_iir_lattice_q15
