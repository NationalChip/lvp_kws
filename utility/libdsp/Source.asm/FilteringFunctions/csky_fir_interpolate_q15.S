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
 * @file     csky_fir_interpolate_q15.S
 * @brief    Q15 FIR interpolation.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_fir_interpolate_q15(
 *   const csky_fir_interpolate_instance_q15 * S,
 *   q15_t * pSrc,
 *   q15_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_fir_interpolate_q15.S"
    .section        .text.csky_fir_interpolate_q15,"ax",@progbits
    .align          2
    .global         csky_fir_interpolate_q15
    .type           csky_fir_interpolate_q15, @function
csky_fir_interpolate_q15:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9

    ld.b            t0, (a0, 0x0)       // S->L
    lsli            t9, t0, 1
    ld.h            t1, (a0, 0x2)       // phaseLen = S->phaseLength
    ld.w            t2, (a0, 0x4)       // pCoeffs = S->pCoeffs
    ld.w            t3, (a0, 0x8)       // pState = S->pState

    subi            t4, t1, 1
    lsli            t4, t4, 1
    addu            t4, t3, t4          // pStateCurnt

    lsri            t5, a3, 1           // blkCnt = blockSize / 2

    bez             t5, .L0
.L1:
    ldbi.w          l0, (a1)
    stbi.w          l0, (t4)

    movi            t6, 1               // j = 1
    mov             t7, t0              // i = S->L

    bez             t7, .L3
.L4:
    movi            l0, 0
    movi            l1, 0               // acc0
    movi            l2, 0
    movi            l3, 0               // acc1

    mov             l4, t3              // ptr1 = pState
    subu            l5, t0, t6          // S->L - j
    lsli            l5, l5, 1
    addu            l5, l5, t2          // ptr2

    lsri            t8, t1, 2           // tapCnt = phaseLen >> 2

    ldbi.hs         l6, (l4)            // x0 = *ptr1++

    bez             t8, .L6
.L7:
    ldbi.hs         l7, (l4)            // x1
    ldbir.hs        l8, (l5), t9        // c0
    mulall.s16.e    l0, l6, l8          // acc0
    mulall.s16.e    l2, l7, l8          // acc1

    ldbir.hs        l9, (l5), t9        // c0
    ldbi.hs         l6, (l4)            // x0
    mulall.s16.e    l0, l7, l9          // acc0
    mulall.s16.e    l2, l6, l9          // acc1

    ldbi.hs         l7, (l4)            // x1
    ldbir.hs        l8, (l5), t9        // c0
    mulall.s16.e    l0, l6, l8          // acc0
    mulall.s16.e    l2, l7, l8          // acc1

    ldbir.hs        l9, (l5), t9        // c0
    ldbi.hs         l6, (l4)            // x0
    mulall.s16.e    l0, l7, l9          // acc0
.L8:
    mulall.s16.e    l2, l6, l9          // acc1

    bloop           t8, .L7, .L8
.L6:

    andi            t8, t1, 3           // tapCnt = phaseLen % 4
    bez             t8, .L9
.L10:
    ldbi.hs         l7, (l4)            // x1
    ldbir.hs        l8, (l5), t9        // c0
    mulall.s16.e    l0, l6, l8          // acc0
    mulall.s16.e    l2, l7, l8          // acc1

.L11:
    mov             l6, l7

    bloop           t8, .L10, .L11
.L9:

    dexti           l0, l0, l1, 15
    dexti           l2, l2, l3, 15
    clipi.s32       l0, l0, 16
    clipi.s32       l2, l2, 16
    st.h            l0, (a2, 0x0)       // pDst
    str.h           l2, (a2, t9 << 0)   // pDst

    addi            a2, a2, 2           // pDst++
    addi            t6, t6, 1           // j++
    subi            t7, t7, 1           // i--
    bnez            t7, .L4
.L3:

    addu            a2, a2, t9          // pDst += S->L

    addi            t3, t3, 4           // pState += 2
    subi            t5, t5, 1           // blkCnt--
    bnez            t5, .L1
.L0:

    andi            t5, t5, 1           // blkCnt
    bez             t5, .L12            // no loop, at most one time

    ldbi.h          l0, (a1)            // *pStateCurnt++ = *pSrc++
    stbi.h          l0, (t4)

    movi            t6, 1               // j = 1
    mov             t7, t0              // i = S->L

    bez             t7, .L13
.L14:
    movi            l0, 0
    movi            l1, 0               // sum0 = 0

    mov             l2, t3              // ptr1 = pState
    subu            l5, t0, t6
    lsli            l5, l5, 1
    addu            l3, t2, l5          // ptr2 = pCoeffs + (S->L - j)

    lsri            t8, t1, 2           // tapCnt = phaseLen >> 2
    bez             t8, .L16
.L17:
    pldbir.d        l4, (l3), t9        // c0
    pldbi.d         l6, (l2)            // x0
    mulall.s16.e    l0, l4, l6          // sum0
    mulahh.s16.e    l0, l4, l6          // sum0

    mulall.s16.e    l0, l5, l7          // sum0
.L18:
    mulahh.s16.e    l0, l5, l7          // sum0

    bloop           t8, .L17, .L18
.L16:

    andi            t8, t1, 3           // tapCnt
    bez             t8, .L19
.L20:
    ldbir.hs        l4, (l3), t9        // c0
    ldbi.h          l5, (l2)            // x0
.L21:
    mulall.s16.e    l0, l4, l5          // sum0

    bloop           t8, .L20, .L21
.L19:

    dexti           l0, l0, l1, 15
    clipi.s32       l0, l0, 16
    stbi.h          l0, (a2)            // pDst

    addi            t6, t6, 1
    subi            t7, t7, 1           // i--
    bnez            t7, .L14
.L13:

.L12:

    ld.w            t6, (a0, 0x8)       // pStateCurnt = S->pState

    subi            t7, t1, 1
    lsri            t8, t7, 2           // tapCnt

    bez             t8, .L22
.L23:
    pldbi.d         l0, (t3)
    stbi.w          l0, (t6)
.L24:
    stbi.w          l1, (t6)

    bloop           t8, .L23, .L24
.L22:

    andi            t8, t7, 3           // tapCnt
    bez             t8, .L25
.L26:
    ldbi.h          l0, (t3)
.L27:
    stbi.h          l0, (t6)

    bloop           t8, .L26, .L27
.L25:

    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_fir_interpolate_q15, .-csky_fir_interpolate_q15
