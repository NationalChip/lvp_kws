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
 * @file     csky_mat_trans_q31.S
 * @brief    Q31 matrix transpose.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * csky_status csky_mat_trans_q31(
 *   const csky_matrix_instance_q31 * pSrc,
 *   csky_matrix_instance_q31 * pDst)
 */
    .file           "csky_mat_trans_q31.S"
    .section        .text.csky_mat_trans_q31,"ax",@progbits
    .align          2
    .global         csky_mat_trans_q31
    .type           csky_mat_trans_q31, @function
csky_mat_trans_q31:
    /*
     * unsupport CSKY_MATH_MATRIX_CHECK yet
     */
    ld.hs           t0, (a0, 0x0)       // nRows
    ld.hs           t1, (a0, 0x2)       // nColumns

    ld.w            a0, (a0, 0x4)       // pIn
    ld.w            a1, (a1, 0x4)       // pOut

    movi            a3, 0               // i
    lsli            t3, t0, 2

.L0:

    lsri            t2, t1, 2           // blkCnt
    addu            a2, a1, a3          // px = pOut + i

    bez             t2, .L2
.L3:
    pldbi.d         t4, (a0)            // in1, in2
    pldbi.d         t6, (a0)            // in1, in2
    stbir.w         t4, (a2), t3
    stbir.w         t5, (a2), t3
    stbir.w         t6, (a2), t3
.L4:
    stbir.w         t7, (a2), t3
    bloop           t2, .L3, .L4
.L2:

    andi            t2, t1, 3           // nColumns % 4
    bez             t2, .L5
.L6:
    ldbi.w          t4, (a0)
.L7:
    stbi.w          t4, (a2)
    bloop           t2, .L6, .L7
.L5:

    addi            a3, 4               // i++

    subi            t0, t0, 1
    bnez            t0, .L0

    movi            a0, 0
    rts
    .size           csky_mat_trans_q31, .-csky_mat_trans_q31
