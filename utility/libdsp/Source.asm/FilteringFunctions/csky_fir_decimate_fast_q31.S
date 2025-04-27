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
 * @file     csky_fir_decimate_fast_q31.S
 * @brief    Q31 FIR filter processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_fir_decimate_fast_q31(
 *   const csky_fir_decimate_instance_q31 * S,
 *   q31_t * pSrc,
 *   q31_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_fir_decimate_fast_q31.S"
    .section        .text.csky_fir_decimate_fast_q31,"ax",@progbits
    .align          2
    .global         csky_fir_decimate_fast_q31
    .type           csky_fir_decimate_fast_q31, @function
csky_fir_decimate_fast_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7

    ld.w            t0, (a0, 0x8)       // pState = S->pState
    mov             t9, t0
    ld.w            t1, (a0, 0x4)       // pCoeffs = S->pCoeffs
    ld.h            t2, (a0, 0x2)       // numTaps = S->numTaps
    ld.b            t3, (a0, 0x0)       // S->M

    lsli            t4, t2, 2
    addu            t4, t0, t4
    subi            t4, t4, 4           // pStateCurnt
    divs            l6, a3, t3          // outBlockSize = blockSize / S->M

    lsri            a0, l6, 1           // blkCnt

    bez             a0, .L0
.L1:
    lsli            t5, t3, 1           // i = S->M * 2
.L3:
    ldbi.w          t6, (a1)
.L4:
    stbi.w          t6, (t4)

    bloop           t5, .L3, .L4

    movi            l0, 0               // acc0
    movi            l1, 0               // acc1

    mov             t6, t0              // px0 = pState
    lsli            l7, t3, 2
    addu            l7, t0, l7          // px1
    mov             t7, t1              // pb = pCoeffs

    lsri            t8, t2, 2           // tapCnt = numTaps >> 2
    bez             t8, .L5
.L6:
    pldbi.d         l2, (t6)            // x0
    pldbi.d         l4, (t7)            // c0
    mula.s32.hs     l0, l2, l4
    mula.s32.hs     l0, l3, l5          // acc0

    ldbi.w          l2, (l7)            // x1
    mula.s32.hs     l1, l2, l4
    ldbi.w          l3, (l7)
    mula.s32.hs     l1, l3, l5          // acc1

    pldbi.d         l2, (t6)            // x0
    pldbi.d         l4, (t7)            // c0
    mula.s32.hs     l0, l2, l4
    mula.s32.hs     l0, l3, l5          // acc0

    ldbi.w          l2, (l7)            // x1
    mula.s32.hs     l1, l2, l4
    ldbi.w          l3, (l7)
.L7:
    mula.s32.hs     l1, l3, l5          // acc1

    bloop           t8, .L6, .L7
.L5:

    andi            t8, t2, 3           // tapCnt = numTaps % 4
    bez             t8, .L8
.L9:
    ldbi.w          l2, (t6)
    ldbi.w          l4, (t7)
    mula.s32.hs     l0, l2, l4
    ldbi.w          l2, (l7)
.L10:
    mula.s32.hs     l1, l2, l4

    bloop           t8, .L9, .L10
.L8:

    lsli            t8, t3, 3
    addu            t0, t0, t8          // pState = pState + S->M * 2
    lsli            l0, l0, 1
    lsli            l1, l1, 1
    st.w            l0, (a2, 0x0)
    st.w            l1, (a2, 0x4)
    addi            a2, a2, 8

    subi            a0, a0, 1           // blkCnt--
    bnez            a0, .L1
.L0:


    andi            a0, l6, 1           // blkCnt

    bez             a0, .L20            // never loop
    mov             t5, t3              // i = S->M
.L23:
    ldbi.w          t6, (a1)
.L24:
    stbi.w          t6, (t4)

    bloop           t5, .L23, .L24

    movi            l0, 0               // acc0

    mov             t6, t0              // px = pState
    mov             t7, t1              // pb = pCoeffs

    lsri            t8, t2, 2           // tapCnt = numTaps >> 2
    bez             t8, .L25
.L26:
    pldbi.d         l2, (t6)            // c0
    pldbi.d         l4, (t7)            // x0
    mula.s32.hs     l0, l2, l4
    mula.s32.hs     l0, l3, l5          // acc0

    pldbi.d         l2, (t6)            // c0
    pldbi.d         l4, (t7)            // x0
    mula.s32.hs     l0, l2, l4
.L27:
    mula.s32.hs     l0, l3, l5          // acc0

    bloop           t8, .L26, .L27
.L25:

    andi            t8, t2, 3           // tapCnt = numTaps % 4
    bez             t8, .L28
.L29:
    ldbi.w          l2, (t6)
    ldbi.w          l4, (t7)
.L30:
    mula.s32.hs     l0, l2, l4

    bloop           t8, .L29, .L30
.L28:

    lsli            t8, t3, 2
    addu            t0, t0, t8          // pState = pState + S->M
    lsli            l0, l0, 1
    stbi.w          l0, (a2)


.L20:


    mov             t1, t9              // pStateCurnt = S->pState
    subi            t2, t2, 1           // numTaps - 1
    lsri            t8, t2, 2           // i

    bez             t8, .L11
.L12:
    pldbi.d         l0, (t0)
    st.w            l0, (t1, 0x0)
    st.w            l1, (t1, 0x4)
    pldbi.d         l0, (t0)
    st.w            l0, (t1, 0x8)
    st.w            l1, (t1, 0xc)
.L13:
    addi            t1, t1, 16
    bloop           t8, .L12, .L13
.L11:

    andi            t8, t2, 3           // i
    bez             t8, .L14
.L15:
    ldbi.w          l0, (t0)
.L16:
    stbi.w          l0, (t1)

    bloop           t8, .L15, .L16
.L14:

    pop             l0, l1, l2, l3, l4, l5, l6, l7
    .size           csky_fir_decimate_fast_q31, .-csky_fir_decimate_fast_q31
