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
 * @file     csky_mat_cmplx_mult_q31.S
 * @brief    Floating-point matrix multiplication.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * csky_status csky_mat_cmplx_mult_q31(
 *   const csky_matrix_instance_q31 * pSrcA,
 *   const csky_matrix_instance_q31 * pSrcB,
 *   csky_matrix_instance_q31 * pDst)
 */
    .file           "csky_mat_cmplx_mult_q31.S"
    .section        .text.csky_mat_cmplx_mult_q31,"ax",@progbits
    .align          2
    .global         csky_mat_cmplx_mult_q31
    .type           csky_mat_cmplx_mult_q31, @function
csky_mat_cmplx_mult_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    /*
     * unsupport CSKY_MATH_MATRIX_CHECK yet
     */
    ld.hs           t0, (a0, 0x0)       // pSrcA->numRows == pDst->numRows
    ld.hs           t1, (a0, 0x2)       // pSrcA->numCols == pSrcB->numRows
    ld.hs           t2, (a1, 0x2)       // pSrcB->numCols == pDst->numCols
    lsli            t3, t2, 3           // numColsB * 2
    subi            t3, t3, 4

    ld.w            a0, (a0, 0x4)       // pIn1, pInA
    ld.w            a1, (a1, 0x4)       // pIn2
    ld.w            a2, (a2, 0x4)       // pOut
    mov             a3, t0              // row
    movi            l6, 0               // i

.L1:
    lsli            l5, l6, 3
    addu            l5, a2, l5          // px = pOut + 2 * i
    mov             l7, t2              // col = numColsB
    mov             l8, a1              // pIn2 = pSrcB->pData
    movi            l4, 0               // j = 0

.L3:
    movi            l0, 0
    movi            l1, 0               // sumReal1
    movi            l2, 0
    movi            l3, 0               // sumImag1
    mov             l9, a0              // pIn1 = pInA

    lsri            t4, t1, 2           // colCnt = numColsA >> 2
    bez             t4, .L5
.L6:
    pldbi.d         t6, (l9)            // a0, b0
    ldbi.w          t8, (l8)            // c0
    ldbir.w         t9, (l8), t3        // d0
    mula.s32        l0, t6, t8
    mula.s32        l2, t7, t8
    neg.s32.s       t7, t7
    mula.s32        l0, t7, t9
    mula.s32        l2, t6, t9

    pldbi.d         t6, (l9)            // a0, b0
    ldbi.w          t8, (l8)            // c0
    ldbir.w         t9, (l8), t3        // d0
    mula.s32        l0, t6, t8
    mula.s32        l2, t7, t8
    neg.s32.s       t7, t7
    mula.s32        l0, t7, t9
    mula.s32        l2, t6, t9

    pldbi.d         t6, (l9)            // a0, b0
    ldbi.w          t8, (l8)            // c0
    ldbir.w         t9, (l8), t3        // d0
    mula.s32        l0, t6, t8
    mula.s32        l2, t7, t8
    neg.s32.s       t7, t7
    mula.s32        l0, t7, t9
    mula.s32        l2, t6, t9

    pldbi.d         t6, (l9)            // a0, b0
    ldbi.w          t8, (l8)            // c0
    ldbir.w         t9, (l8), t3        // d0
    mula.s32        l0, t6, t8
    mula.s32        l2, t7, t8
    neg.s32.s       t7, t7
    mula.s32        l0, t7, t9
.L7:
    mula.s32        l2, t6, t9
    bloop           t4, .L6, .L7
.L5:

    andi            t4, t1, 3           // colCnt = numCols % 4
    bez             t4, .L8
.L9:
    pldbi.d         t6, (l9)            // a0, b0
    ldbi.w          t8, (l8)            // c0
    ldbir.w         t9, (l8), t3        // d0
    mula.s32        l0, t6, t8
    mula.s32        l2, t7, t8
    neg.s32.s       t7, t7
    mula.s32        l0, t7, t9
.L10:
    mula.s32        l2, t6, t9
    bloop           t4, .L9, .L10
.L8:

    bgeni           t6, 31              // 0x80000000
    bmaski          t7, 31              // 0x7fffffff
    dexti           l0, l0, l1, 31
    asri            l1, l1, 31
    dexti           t4, l0, l1, 31
    cmpne           t4, l1
    bf              .L11
    btsti           l1, 31
    movt            l0, t6
    movf            l0, t7
.L11:
    stbi.w          l0, (l5)            // *px++

    dexti           l2, l2, l3, 31
    asri            l3, l3, 31
    dexti           t4, l2, l3, 31
    cmpne           t4, l3
    bf              .L12
    btsti           l3, 31
    movt            l2, t6
    movf            l2, t7
.L12:
    stbi.w          l2, (l5)            // *px++

    addi            l4, 8               // j++
    addu            l8, a1, l4          // pIn2 = pSrcB->pData + 2u * j

    subi            l7, l7, 1           // col--
    bnez            l7, .L3

    addu            l6, l6, t2          // i = i + numColsB
    lsli            t4, t1, 3
    addu            a0, a0, t4          // pInA = pInA + 2 * numColsA

    subi            a3, a3, 1           // row--
    bnez            a3, .L1

    movi            a0, 0

    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_mat_cmplx_mult_q31, .-csky_mat_cmplx_mult_q31
