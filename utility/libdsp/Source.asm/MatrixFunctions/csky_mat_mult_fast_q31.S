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
 * @file     csky_mat_mult_fast_q31.S
 * @brief    Q31 matrix multiplication (fast variant).
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * csky_status csky_mat_mult_fast_q31(
 *   const csky_matrix_instance_q31 * pSrcA,
 *   const csky_matrix_instance_q31 * pSrcB,
 *   csky_matrix_instance_q31 * pDst)
 */
    .file           "csky_mat_mult_fast_q31.S"
    .section        .text.csky_mat_mult_fast_q31,"ax",@progbits
    .align          2
    .global         csky_mat_mult_fast_q31
    .type           csky_mat_mult_fast_q31, @function
csky_mat_mult_fast_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    /*
     * unsupport CSKY_MATH_MATRIX_CHECK yet
     */
    ld.hs           t0, (a0, 0x0)       // pSrcA->numRows == pDst->numRows
    ld.hs           t1, (a0, 0x2)       // pSrcA->numCols == pSrcB->numRows
    ld.hs           t2, (a1, 0x2)       // pSrcB->numCols == pDst->numCols
    lsli            t3, t2, 2

    ld.w            a0, (a0, 0x4)       // pIn1, pInA
    ld.w            a1, (a1, 0x4)       // pIn2
    ld.w            a2, (a2, 0x4)       // pOut
    mov             a3, t0              // row
    movi            l6, 0               // i

.L1:
    lsli            l5, l6, 2
    addu            l5, a2, l5          // px = pOut + i
    mov             l7, t2              // col = numColsB
    mov             l8, a1              // pIn2 = pSrcB->pData
    movi            l4, 0               // j = 0

.L3:
    movi            l0, 0               // sum
    mov             l9, a0              // pIn1 = pInA

    lsri            t4, t1, 2           // colCnt = numColsA >> 2
    bez             t4, .L5
.L6:
    pldbi.d         t6, (l9)            // a0, a1
    pldbir.d        t8, (l8), t3        // b0, b1
    mula.s32.hs     l0, t6, t8
    mula.s32.hs     l0, t7, t9

    pldbi.d         t6, (l9)            // a2, a3
    pldbir.d        t8, (l8), t3        // b2, b3
    mula.s32.hs     l0, t6, t8
.L7:
    mula.s32.hs     l0, t7, t9

    bloop           t4, .L6, .L7
.L5:

    andi            t4, t1, 3           // colCnt = numCols % 4
    bez             t4, .L8
.L9:
    ldbi.w          t6, (l9)            // a0
    ldbir.w         t8, (l8), t3        // b0
.L10:
    mula.s32.hs     l0, t6, t8
    bloop           t4, .L9, .L10
.L8:

    lsli.s32.s      l0, l0, 1
    stbi.w          l0, (l5)            // *px++

    addi            l4, 4               // j++
    addu            l8, a1, l4          // pIn2 = pSrcB->pData + j

    subi            l7, l7, 1           // col--
    bnez            l7, .L3

    addu            l6, l6, t2          // i = i + numColsB
    lsli            t4, t1, 2
    addu            a0, a0, t4          // pInA = pInA + numColsA

    subi            a3, a3, 1           // row--
    bnez            a3, .L1

    movi            a0, 0

    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_mat_mult_fast_q31, .-csky_mat_mult_fast_q31
