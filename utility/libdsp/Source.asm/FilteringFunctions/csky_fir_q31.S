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
 * @file     csky_fir_q31.S
 * @brief    Q31 FIR filter processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_fir_q31(
 *   const csky_fir_instance_q31 * S,
 *   q31_t * pSrc,
 *   q31_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_fir_q31.S"
    .section        .text.csky_fir_q31,"ax",@progbits
    .align          2
    .global         csky_fir_q31
    .type           csky_fir_q31, @function
csky_fir_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9

    subi            sp, sp, 4

    ld.w            t0, (a0, 0x4)       // pState
    st.w            t0, (sp, 0x0)
    ld.w            t1, (a0, 0x8)       // pCoeffs
    ld.hs           t2, (a0, 0x0)       // numTaps

    lsli            t3, t2, 2
    addu            t3, t3, t0
    subi            t3, t3, 4           // pStateCurnt

    movi            l7, 3               // 3
    divs            t4, a3, l7          // blkCnt
    mult            t5, t4, l7
    subu            a3, a3, t5          // blockSize

    divs            t5, t2, l7          // tapCnt
    mult            t6, t5, l7
    subu            t6, t2, t6          // tapCntN3

    bez             t4, .L2

.L0:
    ldbi.w          t7, (a1)            // pSrc
    ldbi.w          t8, (a1)
    ldbi.w          t9, (a1)
    stbi.w          t7, (t3)            // pStateCurnt
    stbi.w          t8, (t3)
    stbi.w          t9, (t3)

    movi            l0, 0
    movi            l1, 0               // acc0
    movi            l2, 0
    movi            l3, 0               // acc1
    movi            l4, 0
    movi            l5, 0               // acc2

    mov             l8, t0              // px = pState
    mov             l9, t1              // pb = pCoeffs

    ldbi.w          t7, (l8)            // x0
    ldbi.w          t8, (l8)            // x1

    mov             a0, t5              // i = tapCnt
    bez             a0, .L3
.L4:
    ldbi.w          l6, (l9)            // c0
    ldbi.w          t9, (l8)            // x2

    mula.s32        l0, t7, l6
    mula.s32        l2, t8, l6
    mula.s32        l4, t9, l6

    ldbi.w          l6, (l9)            // c0
    ldbi.w          t7, (l8)            // x0

    mula.s32        l0, t8, l6
    mula.s32        l2, t9, l6
    mula.s32        l4, t7, l6

    ldbi.w          l6, (l9)            // c0
    ldbi.w          t8, (l8)            // x1

    mula.s32        l0, t9, l6
    mula.s32        l2, t7, l6
.L5:
    mula.s32        l4, t8, l6
    bloop           a0, .L4, .L5
.L3:

    mov             a0, t6              // i = tapCntN3
    bez             a0, .L6
.L7:
    ldbi.w          l6, (l9)            // c0
    ldbi.w          t9, (l8)            // x2

    mula.s32        l0, t7, l6
    mula.s32        l2, t8, l6
    mula.s32        l4, t9, l6

    mov             t7, t8
.L8:
    mov             t8, t9
    bloop           a0, .L7, .L8
.L6:

    dexti           l0, l0, l1, 31
    dexti           l2, l2, l3, 31
    dexti           l4, l4, l5, 31
    stbi.w          l0, (a2)
    stbi.w          l2, (a2)
    stbi.w          l4, (a2)
.L1:
    addi            t0, t0, 12          // pState = pState + 3
    subi            t4, t4, 1           // blkCnt--
    bnez            t4, .L0
.L2:


    bez             a3, .L10
.L11:
    ldbi.w          t7, (a1)
    stbi.w          t7, (t3)

    movi            l0, 0
    movi            l1, 0

    mov             l8, t0              // px = pState
    mov             l9, t1              // pb = pCoeffs

    mov             a0, t2              // i = numTaps

.L13:
    ldbi.w          t7, (l8)
    ldbi.w          t8, (l9)
.L14:
    mula.s32        l0, t7, t8          // acc0
    bloop           a0, .L13, .L14

    dexti           l0, l0, l1, 31
    stbi.w          l0, (a2)

.L12:
    addi            t0, t0, 4           // pState = pState + 1
    subi            a3, a3, 1
    bnez            a3, .L11
.L10:

    ld.w            t3, (sp, 0x0)       // pStateCurnt
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
    addi            sp, sp, 4
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_fir_q31, .-csky_fir_q31
