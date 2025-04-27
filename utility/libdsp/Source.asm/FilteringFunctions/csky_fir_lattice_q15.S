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
 * @file     csky_fir_lattice_q15.S
 * @brief    Q15 FIR lattice filter processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_fir_lattice_q15(
 *   const csky_fir_lattice_instance_q15 * S,
 *   q15_t * pSrc,
 *   q15_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_fir_lattice_q15.S"
    .section        .text.csky_fir_lattice_q15,"ax",@progbits
    .align          2
    .global         csky_fir_lattice_q15
    .type           csky_fir_lattice_q15, @function
csky_fir_lattice_q15:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, lr

    ld.h            t0, (a0, 0x0)       // numStages
    ld.w            t1, (a0, 0x8)       // pCoeffs
    ld.w            a0, (a0, 0x4)       // pState

    lsri            t2, a3, 2           // blkCnt = blockSize >> 2
    bez             t2, .L0
.L1:
    ldbi.hs         l0, (a1)            // fcurnt1
    ldbi.hs         l1, (a1)            // fcurnt2

    mov             t3, t1              // pk = pCoeffs
    mov             t4, a0              // px = pState

    ld.hs           l2, (t4, 0x0)       // gcurnt1 = *px

    ldbi.hs         l3, (t3)            // k = *pk

    mult            t5, l2, l3
    asri            t5, t5, 15
    addu            l4, t5, l0          // fnext1 = gcurnt1 * k >> 15 + fcurnt1
    clipi.s32       l4, l4, 16

    mult            t5, l0, l3
    asri            t5, t5, 15
    addu            l5, t5, l2          // gnext1 = fcurnt1 * k >> 15 + gcurnt1
    clipi.s32       l5, l5, 16

    mult            t5, l0, l3
    asri            t5, t5, 15
    addu            l6, t5, l1          // fnext2 = fcurnt1 * k >> 15 + fcurnt2
    clipi.s32       l6, l6, 16

    mult            t5, l1, l3
    asri            t5, t5, 15
    addu            l7, t5, l0          // gnext2 = fcurnt2 * k >> 15 + fcurnt1
    clipi.s32       l7, l7, 16

    ldbi.hs         l8, (a1)            // fcurnt3
    ldbi.hs         l9, (a1)            // fcurnt4
    stbi.h          l9, (t4)            // *px++ = fcurnt4

    mult            t5, l1, l3
    asri            t5, t5, 15
    addu            t6, t5, l8          // fnext3 = fcurnt2 * k >> 15 + fcurnt3
    clipi.s32       t6, t6, 16

    mult            t5, l8, l3
    asri            t5, t5, 15
    addu            t7, t5, l1          // gnext3 = fcurnt3 * k >> 15 + fcurnt2
    clipi.s32       t7, t7, 16

    mult            t5, l8, l3
    asri            t5, t5, 15
    addu            t8, t5, l9          // fnext4 = fcurnt3 * k >> 15 + fcurnt4
    clipi.s32       t8, t8, 16

    mult            t5, l9, l3
    asri            t5, t5, 15
    addu            t9, t5, l8          // gnext4 = fcurnt4 * k >> 15 + fcurnt3
    clipi.s32       t9, t9, 16

    mov             l0, l4              // fcurnt1 = fnext1
    mov             l1, l6              // fcurnt2 = fnext2
    mov             l8, t6              // fcurnt3 = fnext3
    mov             l9, t8              // fcurnt4 = fnext4

    subi            lr, t0, 1
    lsri            lr, lr, 2           // stageCnt

    bez             lr, .L3
.L4:
    ld.hs           l2, (t4, 0x0)       // gcurnt1 = *px
    stbi.h          t9, (t4)            // *px++ = gnext4

    ldbi.hs         l3, (t3)            // k = *pk++

    mult            t5, l2, l3
    asri            t5, t5, 15
    addu            l4, t5, l0          // fnext1 = gcurnt1 * k >> 15 + fcurnt1
    clipi.s32       l4, l4, 16

    mult            t5, l5, l3
    asri            t5, t5, 15
    addu            l6, t5, l1          // fnext2 = gnext1 * k >> 15 + fcurnt2
    clipi.s32       l6, l6, 16

    mult            t5, l7, l3
    asri            t5, t5, 15
    addu            t6, t5, l8          // fnext3 = gnext2 * k >> 15 + fcurnt3
    clipi.s32       t6, t6, 16

    mult            t5, t7, l3
    asri            t5, t5, 15
    addu            t8, t5, l9          // fnext4 = gnext3 * k >> 15 + fcurnt4
    clipi.s32       t8, t8, 16

    mult            t5, l9, l3
    asri            t5, t5, 15
    addu            t9, t5, t7          // gnext4 = fcurnt4 * k >> 15 + gnext3
    clipi.s32       t9, t9, 16

    mult            t5, l8, l3
    asri            t5, t5, 15
    addu            t7, t5, l7          // gnext3 = fcurnt3 * k >> 15 + gnext2
    clipi.s32       t7, t7, 16

    mult            t5, l1, l3
    asri            t5, t5, 15
    addu            l7, t5, l5          // gnext2 = fcurnt2 * k >> 15 + genxt1
    clipi.s32       l7, l7, 16

    mult            t5, l0, l3
    asri            t5, t5, 15
    addu            l5, t5, l2          // gnext1 = fcurnt1 * k >> 15 + gcurnt1
    clipi.s32       l5, l5, 16

    ld.hs           l2, (t4, 0x0)       // gcurnt1 = *px
    stbi.h          t9, (t4)            // *px++ = gnext4
    ldbi.hs         l3, (t3)            // k = *pk++

    mult            t5, l2, l3
    asri            t5, t5, 15
    addu            l0, t5, l4          // fcurnt1 = gcurnt1 * k >> 15 + fnext1
    clipi.s32       l0, l0, 16

    mult            t5, l5, l3
    asri            t5, t5, 15
    addu            l1, t5, l6          // fcurnt2 = gnext1 * k >> 15 + fnext2
    clipi.s32       l1, l1, 16

    mult            t5, l7, l3
    asri            t5, t5, 15
    addu            l8, t5, t6          // fcurnt3 = gnext2 * k >> 15 + fnext3
    clipi.s32       l8, l8, 16

    mult            t5, t7, l3
    asri            t5, t5, 15
    addu            l9, t5, t8          // fcurnt4 = gnext3 * k >> 15 + fnext4
    clipi.s32       l9, l9, 16

    mult            t5, t8, l3
    asri            t5, t5, 15
    addu            t9, t5, t7          // gnext4 = fnext4 * k >> 15 + gnext3
    clipi.s32       t9, t9, 16

    mult            t5, t6, l3
    asri            t5, t5, 15
    addu            t7, t5, l7          // gnext3 = fnext3 * k >> 15 + gnext2
    clipi.s32       t7, t7, 16

    mult            t5, l6, l3
    asri            t5, t5, 15
    addu            l7, t5, l5          // gnext2 = fnext2 * k >> 15 + genxt1
    clipi.s32       l7, l7, 16

    mult            t5, l4, l3
    asri            t5, t5, 15
    addu            l5, t5, l2          // gnext1 = fnext1 * k >> 15 + gcurnt1
    clipi.s32       l5, l5, 16

    ld.hs           l2, (t4, 0x0)       // gcurnt1 = *px
    stbi.h          t9, (t4)            // *px++ = gnext4
    ldbi.hs         l3, (t3)            // k = *pk++

    mult            t5, l2, l3
    asri            t5, t5, 15
    addu            l4, t5, l0          // fnext1 = gcurnt1 * k >> 15 + fcurnt1
    clipi.s32       l4, l4, 16

    mult            t5, l5, l3
    asri            t5, t5, 15
    addu            l6, t5, l1          // fnext2 = gnext1 * k >> 15 + fcurnt2
    clipi.s32       l6, l6, 16

    mult            t5, l7, l3
    asri            t5, t5, 15
    addu            t6, t5, l8          // fnext3 = gnext2 * k >> 15 + fcurnt3
    clipi.s32       t6, t6, 16

    mult            t5, t7, l3
    asri            t5, t5, 15
    addu            t8, t5, l9          // fnext4 = gnext3 * k >> 15 + fcurnt4
    clipi.s32       t8, t8, 16

    mult            t5, l9, l3
    asri            t5, t5, 15
    addu            t9, t5, t7          // gnext4 = fcurnt4 * k >> 15 + gnext3
    clipi.s32       t9, t9, 16

    mult            t5, l8, l3
    asri            t5, t5, 15
    addu            t7, t5, l7          // gnext3 = fcurnt3 * k >> 15 + gnext2
    clipi.s32       t7, t7, 16

    mult            t5, l1, l3
    asri            t5, t5, 15
    addu            l7, t5, l5          // gnext2 = fcurnt2 * k >> 15 + genxt1
    clipi.s32       l7, l7, 16

    mult            t5, l0, l3
    asri            t5, t5, 15
    addu            l5, t5, l2          // gnext1 = fcurnt1 * k >> 15 + gcurnt1
    clipi.s32       l5, l5, 16

    ld.hs           l2, (t4, 0x0)       // gcurnt1 = *px
    stbi.h          t9, (t4)            // *px++ = gnext4
    ldbi.hs         l3, (t3)            // k = *pk++

    mult            t5, l2, l3
    asri            t5, t5, 15
    addu            l0, t5, l4          // fcurnt1 = gcurnt1 * k >> 15 + fnext1
    clipi.s32       l0, l0, 16

    mult            t5, l5, l3
    asri            t5, t5, 15
    addu            l1, t5, l6          // fcurnt2 = gnext1 * k >> 15 + fnext2
    clipi.s32       l1, l1, 16

    mult            t5, l7, l3
    asri            t5, t5, 15
    addu            l8, t5, t6          // fcurnt3 = gnext2 * k >> 15 + fnext3
    clipi.s32       l8, l8, 16

    mult            t5, t7, l3
    asri            t5, t5, 15
    addu            l9, t5, t8          // fcurnt4 = gnext3 * k >> 15 + fnext4
    clipi.s32       l9, l9, 16

    mult            t5, t8, l3
    asri            t5, t5, 15
    addu            t9, t5, t7          // gnext4 = fnext4 * k >> 15 + gnext3
    clipi.s32       t9, t9, 16

    mult            t5, t6, l3
    asri            t5, t5, 15
    addu            t7, t5, l7          // gnext3 = fnext3 * k >> 15 + gnext2
    clipi.s32       t7, t7, 16

    mult            t5, l6, l3
    asri            t5, t5, 15
    addu            l7, t5, l5          // gnext2 = fnext2 * k >> 15 + genxt1
    clipi.s32       l7, l7, 16

    mult            t5, l4, l3
    asri            t5, t5, 15
    addu            l5, t5, l2          // gnext1 = fnext1 * k >> 15 + gcurnt1
.L5:
    clipi.s32       l5, l5, 16

    bloop           lr, .L4, .L5        // stageCnt--
.L3:

    andi            lr, t0, 3           // stageCnt
    bez             lr, .L6

.L7:
    ld.hs           l2, (t4, 0x0)       // gcurnt1 = *px
    stbi.h          t9, (t4)            // *px++ = gnext4
    ldbi.hs         l3, (t3)            // k = *pk++

    mult            t5, l2, l3
    asri            t5, t5, 15
    addu            l4, t5, l0          // fnext1 = gcurnt1 * k >> 15 + fcurnt1
    clipi.s32       l4, l4, 16

    mult            t5, l5, l3
    asri            t5, t5, 15
    addu            l6, t5, l1          // fnext2 = gnext1 * k >> 15 + fcurnt2
    clipi.s32       l6, l6, 16

    mult            t5, l7, l3
    asri            t5, t5, 15
    addu            t6, t5, l8          // fnext3 = gnext2 * k >> 15 + fcurnt3
    clipi.s32       t6, t6, 16

    mult            t5, t7, l3
    asri            t5, t5, 15
    addu            t8, t5, l9          // fnext4 = gnext3 * k >> 15 + fcurnt4
    clipi.s32       t8, t8, 16

    mult            t5, l9, l3
    asri            t5, t5, 15
    addu            t9, t5, t7          // gnext4 = fcurnt4 * k >> 15 + gnext3
    clipi.s32       t9, t9, 16

    mult            t5, l8, l3
    asri            t5, t5, 15
    addu            t7, t5, l7          // gnext3 = fcurnt3 * k >> 15 + gnext2
    clipi.s32       t7, t7, 16

    mult            t5, l1, l3
    asri            t5, t5, 15
    addu            l7, t5, l5          // gnext2 = fcurnt2 * k >> 15 + genxt1
    clipi.s32       l7, l7, 16

    mult            t5, l0, l3
    asri            t5, t5, 15
    addu            l5, t5, l2          // gnext1 = fcurnt1 * k >> 15 + gcurnt1
    clipi.s32       l5, l5, 16


    mov             l0, l4              // fcurnt1 = fnext1
    mov             l1, l6              // fcurnt2 = fnext2
    mov             l8, t6              // fcurnt3 = fnext3
.L8:
    mov             l9, t8              // fcurnt4 = fnext4

    bloop           lr, .L7, .L8
.L6:

    pkgll           l0, l0, l1
    pkgll           l8, l8, l9
    stbi.w          l0, (a2)
    stbi.w          l8, (a2)

    subi            t2, t2, 1           // blkCnt--
    bnez            t2, .L1
.L0:

    andi            t2, a3, 3           // blkCnt
    bez             t2, .L9
.L10:
    ldbi.hs         l0, (a1)            // fcurnt1

    mov             t3, t1              // pk = pCoeffs
    mov             t4, a0              // px = pState

    ld.hs           l1, (t4, 0x0)       // gcurnt1 = *px
    ldbi.hs         l3, (t3)            // k = *pk++

    mult            l2, l1, l3
    asri            l2, l2, 15
    addu            l4, l2, l0          // fnext1 = gcurnt1 * k >> 15 + fcurnt1
    clipi.s32       l4, l4, 16

    mult            l2, l0, l3
    asri            l2, l2, 15
    addu            l5, l2, l1          // gnext1 = fcurnt1 * k >> 15 + gcurnt1
    clipi.s32       l5, l5, 16

    stbi.h          l0, (t4)            // *px++ = fcurnt1

    mov             l0, l4

    subi            t0, t0, 1
    bez             t0, .L12
.L13:
    ld.hs           l1, (t4, 0x0)       // gcurnt1 = *px
    stbi.h          l5, (t4)            // *px++ = gnext1
    ldbi.hs         l3, (t3)            // k = *pk++

    mult            l2, l1, l3
    asri            l2, l2, 15
    addu            l4, l2, l0          // fnext1 = gcurnt1 * k >> 15 + fcurnt1
    clipi.s32       l4, l4, 16

    mult            l2, l0, l3
    asri            l2, l2, 15
    addu            l5, l2, l1          // gnext1 = fcurnt1 * k >> 15 + gcurnt1
    clipi.s32       l5, l5, 16

.L14:
    mov             l0, l4

    bloop           t0, .L13, .L14
.L12:

    clipi.s32       l0, l0, 16
    stbi.h          l0, (a2)

    subi            t2, t2, 1
    bnez            t2, .L10            // blkCnt--
.L9:

    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, lr
    .size           csky_fir_lattice_q15, .-csky_fir_lattice_q15
