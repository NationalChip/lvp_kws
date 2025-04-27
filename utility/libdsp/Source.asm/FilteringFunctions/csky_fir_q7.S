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
 * @file     csky_fir_q7.S
 * @brief    Q7 FIR filter processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_fir_q7(
 *   const csky_fir_instance_q7 * S,
 *   q7_t * pSrc,
 *   q7_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_fir_q7.S"
    .section        .text.csky_fir_q7,"ax",@progbits
    .align          2
    .global         csky_fir_q7
    .type           csky_fir_q7, @function
csky_fir_q7:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9

    ld.w            t0, (a0, 0x4)       // pState
    ld.w            t1, (a0, 0x8)       // pCoeffs
    ld.hs           t2, (a0, 0x0)       // numTaps

    addu            t3, t0, t2
    subi            t3, t3, 1           // pStateCurnt

    lsri            t4, a3, 2           // blkCnt = blockSize >> 2

    bez             t4, .L2
.L0:
    ldbi.w          t8, (a1)            // pSrc
    stbi.w          t8, (t3)            // pStateCurnt

    movi            l0, 0               // acc0
    movi            l1, 0               // acc1
    movi            l2, 0               // acc2
    movi            l3, 0               // acc3

    mov             l8, t0              // px = pState
    mov             l9, t1              // pb = pCoeffs

    ldbi.bs         l4, (l8)            // x0
    ldbi.bs         l5, (l8)            // x1
    ldbi.bs         l6, (l8)            // x2

    lsri            t5, t2, 2           // tapCnt = numTaps >> 2
    bez             t5, .L3
.L4:
    ldbi.bs         t6, (l9)            // c0
    ldbi.bs         l7, (l8)            // x3
    mulall.s16.s    l0, l4, t6          // acc0
    mulall.s16.s    l1, l5, t6          // acc1
    mulall.s16.s    l2, l6, t6          // acc2
    mulall.s16.s    l3, l7, t6          // acc3

    ldbi.bs         t6, (l9)            // c0
    ldbi.bs         l4, (l8)            // x0
    mulall.s16.s    l0, l5, t6          // acc0
    mulall.s16.s    l1, l6, t6          // acc1
    mulall.s16.s    l2, l7, t6          // acc2
    mulall.s16.s    l3, l4, t6          // acc3

    ldbi.bs         t6, (l9)            // c0
    ldbi.bs         l5, (l8)            // x1
    mulall.s16.s    l0, l6, t6          // acc0
    mulall.s16.s    l1, l7, t6          // acc1
    mulall.s16.s    l2, l4, t6          // acc2
    mulall.s16.s    l3, l5, t6          // acc3

    ldbi.bs         t6, (l9)            // c0
    ldbi.bs         l6, (l8)            // x2
    mulall.s16.s    l0, l7, t6          // acc0
    mulall.s16.s    l1, l4, t6          // acc1
    mulall.s16.s    l2, l5, t6          // acc2
.L5:
    mulall.s16.s    l3, l6, t6          // acc3

    bloop           t5, .L4, .L5
.L3:

    andi            t5, t2, 3           // numTaps & 0x3
    bez             t5, .L6
.L7:
    ldbi.bs         t6, (l9)            // c0
    ldbi.bs         l7, (l8)            // x3
    mulall.s16.s    l0, l4, t6          // acc0
    mulall.s16.s    l1, l5, t6          // acc1
    mulall.s16.s    l2, l6, t6          // acc2
    mulall.s16.s    l3, l7, t6          // acc3
    mov             l4, l5
    mov             l5, l6
.L8:
    mov             l6, l7

    bloop           t5, .L7, .L8
.L6:

    asri            l0, l0, 7
    asri            l1, l1, 7
    asri            l2, l2, 7
    asri            l3, l3, 7
    clipi.s32       l0, l0, 8
    clipi.s32       l1, l1, 8
    clipi.s32       l2, l2, 8
    clipi.s32       l3, l3, 8
    stbi.b          l0, (a2)
    stbi.b          l1, (a2)
    stbi.b          l2, (a2)
    stbi.b          l3, (a2)

    addi            t0, t0, 4           // pState = pState + 4
    subi            t4, t4, 1           // blkCnt--
    bnez            t4, .L0
.L2:

    andi            t4, a3, 3           // blkCnt = blockSize % 0x4
    bez             t4, .L10
.L11:
    ldbi.b          t7, (a1)
    stbi.b          t7, (t3)

    movi            l0, 0

    mov             l8, t0              // px1 = pState
    mov             l9, t1              // pb = pCoeffs

    mov             t5, t2              // i = numTaps

.L13:
    ldbi.bs         t7, (l8)
    ldbi.bs         t8, (l9)
.L14:
    mulall.s16.s    l0, t7, t8          // acc0
    bloop           t5, .L13, .L14

    asri            l0, l0, 7
    clipi.s32       l0, l0, 8
    stbi.b          l0, (a2)

.L12:
    addi            t0, t0, 1           // pState = pState + 1
    subi            t4, t4, 1
    bnez            t4, .L11
.L10:

    ld.w            t3, (a0, 0x4)       // pStateCurnt
    subi            t2, t2, 1
    lsri            a0, t2, 2           // tapCnt
    bez             a0, .L27
.L16:
    ldbi.w          l0, (t0)
.L17:
    stbi.w          l0, (t3)
    bloop           a0, .L16, .L17
.L27:

    andi            a0, t2, 3           // tapCnt
    bez             a0, .L18
.L19:
    ldbi.b          l0, (t0)
.L20:
    stbi.b          l0, (t3)
    bloop           a0, .L19, .L20
.L18:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_fir_q7, .-csky_fir_q7
