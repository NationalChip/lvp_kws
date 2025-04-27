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
 * @file     csky_fir_lattice_q31.S
 * @brief    Q31 FIR lattice filter processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_fir_lattice_q31(
 *   const csky_fir_lattice_instance_q31 * S,
 *   q31_t * pSrc,
 *   q31_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_fir_lattice_q31.S"
    .section        .text.csky_fir_lattice_q31,"ax",@progbits
    .align          2
    .global         csky_fir_lattice_q31
    .type           csky_fir_lattice_q31, @function
csky_fir_lattice_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9

    ld.hs           t0, (a0, 0x0)       // numStages
    ld.w            t1, (a0, 0x8)       // pCoeffs
    ld.w            a0, (a0, 0x4)       // pState

    lsri            t2, a3, 1           // blkCnt
    bez             t2, .L2
.L0:
    pldbi.d         l0, (a1)            // fcurr1, fcurr2
    mov             t4, t1              // pk = pCoeffs
    mov             t5, a0              // px = pState

    ld.w            t3, (t5, 0x0)       // gcurr1 = *px
    ldbi.w          t6, (t4)            // k = *pk++

    rmul.s32.h      t8, t3, t6          // fnext1 = gcurr1 * k
    rmul.s32.h      t9, l0, t6          // gnext1 = fcurr1 * k
    addu            t8, l0, t8          // fnext1 = fcurr1 + fnext1
    addu            t9, t3, t9          // gnext1 = gcurr1 + gnext1

    rmul.s32.h      l2, l0, t6          // fnext2 = fcurr1 * k
    rmul.s32.h      l3, l1, t6          // gnext2 = fcurr2 * k
    addu            l2, l1, l2          // fnext2 = fcurr2 + fnext2
    addu            l3, l0, l3          // gnext2 = fcurr1 + gnext2

    stbi.w          l1, (t5)            // *px++ = fcurr2
    mov             l0, t8              // fcurr1 = fnext1
    mov             l1, l2              // fcurr2 = fnext2

    subi            t7, t0, 1           // stageCnt = numStages - 1

    bez             t7, .L3
.L4:
    ldbi.w          t6, (t4)            // k = *pk++
    ld.w            t3, (t5, 0x0)       // gcurr1 = *px
    stbi.w          l3, (t5)            // *px++ = gnext2

    rmul.s32.h      t8, t3, t6          // fnext1 = gcurr1 * k
    rmul.s32.h      l2, t9, t6          // fnext2 = gnext1 * k
    addu            t8, l0, t8          // fnext1 = fcurr1 + fnext1
    addu            l2, l1, l2          // fnext2 = fcurr2 + fnext2

    rmul.s32.h      l3, l1, t6          // gnext2 = fcurr2 * k
    addu            l3, t9, l3          // gnext2 = gnext1 + gnext2
    rmul.s32.h      t9, l0, t6          // gnext1 = fcurr1 * k
    addu            t9, t3, t9          // gnext1 = gcurr1 + gnext1

    mov             l0, t8
.L5:
    mov             l1, l2

    bloop           t7, .L4, .L5
.L3:

    stbi.w          l0, (a2)            // *pDst++ = fcurr1
    stbi.w          l1, (a2)

    subi            t2, t2, 1           // blkcnt--
    bnez            t2, .L0
.L2:

    andi            t2, a3, 1           // blkCnt = blockSize % 2
    bnez            t2, .L7             // one time at most, no loop

    ldbi.w          l0, (a1)            // fcurr1 = *pSrc++
    mov             t4, t1              // pk = pCoeffs
    mov             t5, a0              // px = pState

    ld.w            t3, (t5, 0x0)       // gcurr1 = *px
    ldbi.w          t6, (t4)            // k = *pk++

    rmul.s32.h      t8, t3, t6          // fnext1 = gcurr1 * k
    addu            t8, l0, t8          // fnext1 = fcurr1 + fnext1
    rmul.s32.h      t9, l0, t6          // gnext1 = fcurr1 * k
    addu            t9, t3, t9          // gnext1 = gcurr1 + gnext1

    stbi.w          l0, (t5)            // *px++ = fcurr1

    mov             l0, t8              // fcurr1 = fnext1

    subi            t7, t0, 1           // stageCnt = numStages - 1

    bez             t7, .L8
.L9:
    ldbi.w          t6, (t4)            // k = *pk++
    ld.w            t3, (t5, 0x0)       // gcurr1 = *px
    stbi.w          t9, (t5)            // *px++ = gnext1

    rmul.s32.h      t8, t3, t6          // fnext1 = gcurr1 * k
    addu            t8, l0, t8          // fnext1 = fcurr1 + fnext1
    rmul.s32.h      t9, l0, t6          // gnext1 = fcurr1 * k
    addu            t9, t3, t9          // gnext1 = gcurr1 + gnext1

.L10:
    mov             l0, t8              // fcurr1 = fnext1

    bloop           t7, .L9, .L10
.L8:

    stbi.w          l0, (a2)            // *pDst++ = fcurr1
.L7:

    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_fir_lattice_q31, .-csky_fir_lattice_q31
