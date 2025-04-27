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
 * @file     csky_mat_cmplx_mult_q15.S
 * @brief    Floating-point matrix multiplication.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * csky_status csky_mat_cmplx_mult_q15(
 *   const csky_matrix_instance_q15 * pSrcA,
 *   const csky_matrix_instance_q15 * pSrcB,
 *   csky_matrix_instance_q15 * pDst)
 */
    .file           "csky_mat_cmplx_mult_q15.S"
    .section        .text.csky_mat_cmplx_mult_q15,"ax",@progbits
    .align          2
    .global         csky_mat_cmplx_mult_q15
    .type           csky_mat_cmplx_mult_q15, @function
csky_mat_cmplx_mult_q15:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    /*
     * unsupport CSKY_MATH_MATRIX_CHECK yet
     */
    ld.hs           t0, (a0, 0x0)       // pSrcA->numRows == pDst->numRows
    ld.hs           t1, (a0, 0x2)       // pSrcA->numCols == pSrcB->numRows
    ld.hs           t2, (a1, 0x2)       // pSrcB->numCols == pDst->numCols
    lsli            t3, t2, 2           // numColsB * 2

    ld.w            a0, (a0, 0x4)       // pIn1, pInA
    ld.w            a1, (a1, 0x4)       // pIn2
    ld.w            a2, (a2, 0x4)       // pOut
    mov             a3, t0              // row
    movi            l6, 0               // i

    /*
     * similar with csky_mat_cmplx_mult_q31,
     * faster than Marix transpose preprocess
     */
.L1:
    lsli            l5, l6, 2
    addu            l5, a2, l5          // px = pOut + 2 * i
    mov             l7, t2              // col = numColsB
    mov             l8, a1              // pIn2 = pSrcB->pData
    movi            l4, 0               // j = 0

.L3:
    movi            l0, 0
    movi            l1, 0               // sumImag
    movi            l2, 0
    movi            l3, 0               // sumReal
    mov             l9, a0              // pIn1 = pInA

    lsri            t4, t1, 2           // colCnt = numColsA >> 2
    bez             t4, .L5
.L6:
    pldbi.d         t6, (l9)            // a0, b0, a1, b1
    pldbir.d        t8, (l8), t3        // c0, d0, c1, d1
    mulacax.s16.e   l0, t6, t8          // sumImag + a0 * d0 + b0 * c0
    mulacs.s16.e    l2, t6, t8          // sumReal + a0 * c0 - b0 * d0
    mulacax.s16.e   l0, t7, t9          // sumImag + a1 * d1 + b1 * c1
    mulacs.s16.e    l2, t7, t9          // sumReal + a1 * c1 - b1 * d1

    pldbi.d         t6, (l9)            // a0, b0, a1, b1
    pldbir.d        t8, (l8), t3        // c0, d0, c1, d1
    mulacax.s16.e   l0, t6, t8          // sumImag + a0 * d0 + b0 * c0
    mulacs.s16.e    l2, t6, t8          // sumReal + a0 * c0 - b0 * d0
    mulacax.s16.e   l0, t7, t9          // sumImag + a1 * d1 + b1 * c1
.L7:
    mulacs.s16.e    l2, t7, t9          // sumReal + a1 * c1 - b1 * d1

    bloop           t4, .L6, .L7
.L5:

    andi            t4, t1, 3           // colCnt = numCols % 4
    bez             t4, .L8
.L9:
    ldbi.w          t6, (l9)            // a0, b0
    ldbir.w         t8, (l8), t3        // c0, d0
    mulacax.s16.e   l0, t6, t8          // sumImag + a0 * d0 + b0 * c0
.L10:
    mulacs.s16.e    l2, t6, t8          // sumReal + a0 * c0 - b0 * d0
    bloop           t4, .L9, .L10
.L8:

    dexti           l0, l0, l1, 15
    dexti           l2, l2, l3, 15
    clipi.s32       l0, l0, 16
    clipi.s32       l2, l2, 16
    stbi.h          l2, (l5)            // *px++
    stbi.h          l0, (l5)            // *px++

    addi            l4, 4               // j++
    addu            l8, a1, l4          // pIn2 = pSrcB->pData + 2u * j

    subi            l7, l7, 1           // col--
    bnez            l7, .L3

    addu            l6, l6, t2          // i = i + numColsB
    lsli            t4, t1, 2
    addu            a0, a0, t4          // pInA = pInA + 2 * numColsA

    subi            a3, a3, 1           // row--
    bnez            a3, .L1

    movi            a0, 0

    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_mat_cmplx_mult_q15, .-csky_mat_cmplx_mult_q15
