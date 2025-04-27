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
 * @file     csky_dct4_q15.S
 * @brief    Processing function of DCT4 & IDCT4 Q15.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

.import csky_mult_q15
.import csky_rfft_q15
.import csky_cmplx_mult_cmplx_q15
.import csky_cmplx_mult_cmplx_re_q15
.import csky_shift_q15

/*
 * void csky_dct4_q15(
 *   const csky_dct4_instance_q15 * S,
 *   q15_t * pState,
 *   q15_t * pInlineBuffer)
 */
    .file           "csky_dct4_q15.S"
    .section        .text.csky_dct4_q15,"ax",@progbits
    .align          2
    .global         csky_dct4_q15
    .type           csky_dct4_q15, @function
csky_dct4_q15:
    push            l0, l1, l2, l3, lr
    mov             l0, a0
    mov             l1, a1
    mov             l2, a2
    ld.h            l3, (l0, 0x0)

    mov             a0, a2
    ld.w            a1, (l0, 0xc)
    mov             a3, l3
    jbsr            csky_mult_q15

    mov             a0, l2
    movi            a1, 1
    mov             a2, l2
    mov             a3, l3
    jbsr            csky_shift_q15

    mov             a0, l1              // pS1 = pState
    subi            a1, l3, 1
    lsli            a1, a1, 1
    addu            a1, a1, l1          // pS2 = pState + (S->N - 1u)

    mov             a2, l2              // pbuff = pInlineBuffer

    ld.h            t0, (l0, 0x2)       // S->Nby2
    lsri            t0, t0, 2           // i
    movi            t1, 4
    neg.s32.s       t1, t1
    subi            a1, a1, 2
.L0:
    pldbi.d         t2, (a2)
    pldbi.d         t4, (a2)
    pkgll           t6, t2, t3
    pkghh           t7, t3, t2
    stbi.w          t6, (a0)
    stbir.w         t7, (a1), t1        // pS2--
    pkgll           t6, t4, t5
    pkghh           t7, t5, t4
    stbi.w          t6, (a0)
.L1:
    stbir.w         t7, (a1), t1

    bloop           t0, .L0, .L1

    mov             a2, l2              // pbuff
    mov             a0, l1              // pS1 = pState
    lsri            t0, l3, 2           // i
.L2:
    pldbi.d         t2, (a0)
    stbi.w          t2, (a2)
.L3:
    stbi.w          t3, (a2)

    bloop           t0, .L2, .L3

    ld.w            a0, (l0, 0x10)
    mov             a1, l2
    mov             a2, l1
    jbsr            csky_rfft_q15

    addi            t0, l3, 2
    lsli            t0, t0, 1
    addu            a0, l1, t0          // pS1 = &pState[N+2]
    subi            t0, t0, 12
    addu            a1, l1, t0          // pS2 = &pState[N-2]

    lsri            a2, l3, 1
    subi            a2, a2, 1
    lsri            t0, a2, 2

.L11:
    pldbi.d         t2, (a1)
    pneg.s16.s      t4, t3
    pkg             t3, t3, 0, t4, 16
    stbi.w          t3, (a0)
    pneg.s16.s      t4, t2
    pkg             t2, t2, 0, t4, 16
    stbi.w          t2, (a0)
    subi            a1, a1, 16

    pldbi.d         t2, (a1)
    pneg.s16.s      t4, t3
    pkg             t3, t3, 0, t4, 16
    stbi.w          t3, (a0)
    pneg.s16.s      t4, t2
    pkg             t2, t2, 0, t4, 16
    stbi.w          t2, (a0)

.L12:
    subi            a1, a1, 16
    bloop           t0, .L11, .L12

    andi            t0, a2, 3
    addi            a1, a1, 4

.L13:
    ldbi.w          t2, (a1)
    pneg.s16.s      t4, t2
    pkg             t2, t2, 0, t4, 16
    stbi.w          t2, (a0)

.L14:
    subi            a1, a1, 4
    bloop           t0, .L13, .L14

    mov             a0, l1
    ld.w            a1, (l0, 0x8)
    mov             a2, l1
    lsri            a3, l3, 1
    addi            a3, a3, 1
    jbsr            csky_cmplx_mult_cmplx_q15

    addi            a0, l3, 2
    lsli            a0, a0, 1
    add             a0, a0, l1
    subi            a2, l3, 2
    lsli            a2, a2, 1
    ld.w            a1, (l0, 0x8)
    add             a1, a1, a2
    mov             a2, a0
    lsri            a3, l3, 1
    subi            a3, a3, 1
    jbsr            csky_cmplx_mult_cmplx_re_q15


    mov             a0, l1
    movi            a1, 2
    mov             a2, l1
    lsli            a3, l3, 1           // s->N * 2
    jbsr            csky_shift_q15

    subi            t0, l3, 1
    lsri            t0, t0, 2           // i

    mov             a0, l1              // pS1 = pState
    mov             a2, l2              // pbuff = pInlineBuffer

    ldbi.hs         t1, (a0)            // in
    lsri            t1, t1, 1
    stbi.h          t1, (a2)            // *pbuff++ = in

    addi            a0, 2               // pS1++

.L4:
    pldbi.d         t2, (a0)
    pldbi.d         t4, (a0)
    psub.16         t2, t2, t1
    stbi.h          t2, (a2)
    psub.16         t3, t3, t2
    stbi.h          t3, (a2)
    psub.16         t4, t4, t3
    stbi.h          t4, (a2)
    psub.16         t1, t5, t4
.L5:
    stbi.h          t1, (a2)

    bloop           t0, .L4, .L5

    subi            t0, l3, 1
    andi            t0, t0, 3           // (S->N - 1u) % 0x4
    bez             t0, .L6
.L7:
    ldbi.w          t2, (a0)
    psub.16         t1, t2, t1
.L8:
    stbi.h          t1, (a2)

    bloop           t0, .L7, .L8
.L6:

    lsri            t0, l3, 2           // i
    mov             a2, l2
    ld.h            t1, (l0, 0x4)       // s->normalize
    dup.16          t1, t1, 0

.L9:
    pldbi.d         t2, (l2)
    prmul.s16.h     t4, t2, t1
    prmul.s16.h     t5, t3, t1
    stbi.w          t4, (a2)
.L10:
    stbi.w          t5, (a2)

    bloop           t0, .L9, .L10

    pop             l0, l1, l2, l3, lr
    .size           csky_dct4_q15, .-csky_dct4_q15

