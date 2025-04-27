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
 * @file     csky_fir_decimate_q31.S
 * @brief    Q31 FIR filter processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_fir_decimate_q31(
 *   const csky_fir_decimate_instance_q31 * S,
 *   q31_t * pSrc,
 *   q31_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_fir_decimate_q31.S"
    .section        .text.csky_fir_decimate_q31,"ax",@progbits
    .align          2
    .global         csky_fir_decimate_q31
    .type           csky_fir_decimate_q31, @function
csky_fir_decimate_q31:
    push            l0, l1, l2, l3, l4, l5

    ld.w            t0, (a0, 0x8)       // pState = S->pState
    mov             t9, t0
    ld.w            t1, (a0, 0x4)       // pCoeffs = S->pCoeffs
    ld.h            t2, (a0, 0x2)       // numTaps = S->numTaps
    ld.b            t3, (a0, 0x0)       // S->M

    lsli            t4, t2, 2
    addu            t4, t0, t4
    subi            t4, t4, 4           // pStateCurnt
    divs            a0, a3, t3          // outBlockSize = blockSize / S->M

    bez             a0, .L0
.L1:
    mov             t5, t3              // i = S->M
.L3:
    ldbi.w          t6, (a1)
.L4:
    stbi.w          t6, (t4)

    bloop           t5, .L3, .L4

    movi            l0, 0
    movi            l1, 0               // sum0

    mov             t6, t0              // px = pState
    mov             t7, t1              // pb = pCoeffs

    lsri            t8, t2, 2           // tapCnt = numTaps >> 2
    bez             t8, .L5
.L6:
    pldbi.d         l2, (t6)
    pldbi.d         l4, (t7)
    mula.s32        l0, l2, l4
    mula.s32        l0, l3, l5

    pldbi.d         l2, (t6)
    pldbi.d         l4, (t7)
    mula.s32        l0, l2, l4
.L7:
    mula.s32        l0, l3, l5

    bloop           t8, .L6, .L7
.L5:

    andi            t8, t2, 3           // tapCnt = numTaps % 4
    bez             t8, .L8
.L9:
    ldbi.w          l2, (t6)
    ldbi.w          l4, (t7)
.L10:
    mula.s32        l0, l2, l4

    bloop           t8, .L9, .L10
.L8:

    lsli            t8, t3, 2
    addu            t0, t0, t8          // pState = pState + S->M
    dexti           l0, l0, l1, 31
    st.w            l0, (a2)

    addi            a2, a2, 4
    subi            a0, a0, 1           // blkCnt--
    bnez            a0, .L1
.L0:

    mov             t1, t9              // pStateCurnt = S->pState
    subi            t2, t2, 1           // numTaps - 1
    lsri            t8, t2, 2           // i

    bez             t8, .L11
.L12:
    pldbi.d         l0, (t0)
    stbi.w          l0, (t1)
    stbi.w          l1, (t1)
    pldbi.d         l0, (t0)
    stbi.w          l0, (t1)
.L13:
    stbi.w          l1, (t1)

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

    pop             l0, l1, l2, l3, l4, l5
    .size           csky_fir_decimate_q31, .-csky_fir_decimate_q31
