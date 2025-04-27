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
 * @file     csky_lms_q31.S
 * @brief    Processing function for the Q31 LMS filter.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_lms_q31(
 *   const csky_lms_instance_q31 * S,
 *   q31_t * pSrc,
 *   q31_t * pRef,
 *   q31_t * pOut,
 *   q31_t * pErr,
 *   uint32_t blockSize)
 */
    .file           "csky_lms_q31.S"
    .section        .text.csky_lms_q31,"ax",@progbits
    .align          2
    .global         csky_lms_q31
    .type           csky_lms_q31, @function
csky_lms_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    ld.h            t0, (a0, 0x0)       // numTaps
    ld.w            t1, (a0, 0x10)      // S->postShift
    movi            t2, 31
    subu            t1, t2, t1          // lShift
    ld.w            t9, (sp, 0x28)      // pErr
    ld.w            l6, (a0, 0xc)       // mu

    ld.w            t2, (a0, 0x4)       // pState
    subi            t3, t0, 1
    lsli            t3, t3, 2
    addu            t3, t3, t2          // pStateCurnt

    ld.w            t6, (a0, 0x8)       // pCoeffs

    ld.w            t4, (sp, 0x2c)      // blkCnt
    bez             t4, .L0
.L1:
    ldbi.w          t5, (a1)
    stbi.w          t5, (t3)            // *pStateCurnt++ = *pSrc++

    mov             t5, t2              // px = pState
    mov             t7, t6              // pb = pCoeffs
    movi            l0, 0
    movi            l1, 0               // sum
    lsri            t8, t0, 2           // tapCnt = numTaps >> 2

    bez             t8, .L3

.L4:
    pldbi.d         l2, (t5)
    pldbi.d         l4, (t7)
    mula.s32        l0, l2, l4
    mula.s32        l0, l3, l5
    pldbi.d         l2, (t5)
    pldbi.d         l4, (t7)
    mula.s32        l0, l2, l4
.L5:
    mula.s32        l0, l3, l5

    bloop           t8, .L4, .L5
.L3:

    andi            t8, t0, 3           // tapCnt = numTaps % 4
    bez             t8, .L8
.L6:
    ldbi.w          l2, (t5)
    ldbi.w          l3, (t7)
.L7:
    mula.s32        l0, l2, l3

    bloop           t8, .L6, .L7
.L8:

    dext            l2, l0, l1, t1      // acc
    stbi.w          l2, (a3)            // *pOut++ = acc

    ldbi.w          l3, (a2)            // *pRef++
    subu            l3, l3, l2          // e
    stbi.w          l3, (t9)            // *pErr++ = e

    mul.s32         l0, l3, l6
    dexti           l2, l0, l1, 31      // alpha

    mov             l3, t2              // px = pState
    addi            t2, t2, 4           // pState++
    mov             t7, t6              // pb = pCoeffs
    mov             t5, t7

    lsri            t8, t0, 2           // tapCnt = numTaps >> 2
    bez             t8, .L9
.L10:
    pldbi.d         l4, (t7)            // pb
    pldbi.d         l8, (l3)            // px
    mul.s32.h       l0, l2, l8          // coef
    mul.s32.h       l1, l2, l9
    lsli            l0, l0, 1
    lsli            l1, l1, 1
    add.s32.s       l0, l0, l4
    add.s32.s       l1, l1, l5
    stbi.w          l0, (t5)
    stbi.w          l1, (t5)

    pldbi.d         l4, (t7)            // pb
    pldbi.d         l8, (l3)            // px
    mul.s32.h       l0, l2, l8          // coef
    mul.s32.h       l1, l2, l9
    lsli            l0, l0, 1
    lsli            l1, l1, 1
    add.s32.s       l0, l0, l4
    add.s32.s       l1, l1, l5
    stbi.w          l0, (t5)
.L11:
    stbi.w          l1, (t5)

    bloop           t8, .L10, .L11
.L9:

    andi            t8, t0, 3
    bez             t8, .L12
.L13:
    ldbi.w          l4, (t7)
    ldbi.w          l8, (l3)
    mul.s32.h       l0, l2, l8
    lsli            l0, l0, 1
    add.s32.s       l0, l0, l4
.L14:
    stbi.w          l0, (t5)

    bloop           t8, .L13, .L14
.L12:

    subi            t4, t4, 1           // blkCnt--
    bnez            t4, .L1
.L0:

    ld.w            t3, (a0, 0x4)       // pStateCurnt = S->pState
    subi            t0, t0, 1
    lsri            t8, t8, 2           // tapCnt = (numTaps - 1u) >> 2
    bez             t8, .L17
.L15:
    pldbi.d         l2, (t2)
    stbi.w          l2, (t3)
    stbi.w          l3, (t3)
    pldbi.d         l2, (t2)
    stbi.w          l2, (t3)
.L16:
    stbi.w          l3, (t3)

    bloop           t8, .L15, .L16
.L17:

    andi            t8, t8, 3
    bez             t8, .L20
.L18:
    ldbi.w          l2, (t2)
.L19:
    stbi.w          l2, (t3)

    bloop           t8, .L18, .L19
.L20:

    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_lms_q31, .-csky_lms_q31
