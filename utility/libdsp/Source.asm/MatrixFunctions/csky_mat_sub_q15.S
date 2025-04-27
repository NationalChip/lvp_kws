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
 * @file     csky_mat_sub_q15.S
 * @brief    Q15 Matrix subtraction.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * csky_status csky_mat_sub_q15(
 *   const csky_matrix_instance_q31 * pSrcA,
 *   const csky_matrix_instance_q31 * pSrcB,
 *   csky_matrix_instance_q31 * pDst)
 */
    .file           "csky_mat_sub_q15.S"
    .section        .text.csky_mat_sub_q15,"ax",@progbits
    .align          2
    .global         csky_mat_sub_q15
    .type           csky_mat_sub_q15, @function
csky_mat_sub_q15:
    /*
     * unsupport CSKY_MATH_MATRIX_CHECK yet
     */
    ld.hs           t0, (a0, 0x0)
    ld.hs           t1, (a0, 0x2)
    mult            t0, t0, t1          // numSamples

    ld.w            a0, (a0, 0x4)       // pInA
    ld.w            a1, (a1, 0x4)       // pInB
    ld.w            a2, (a2, 0x4)       // pOut

    lsri            t1, t0, 2           // blkCnt
    bez             t1, .L0
.L1:
    pldbi.d         t2, (a0)            // inA1, .. inA4
    pldbi.d         t4, (a1)            // inB1, .. inB4
    psub.s16.s      t2, t2, t4
    psub.s16.s      t3, t3, t5
    stbi.w          t2, (a2)
.L2:
    stbi.w          t3, (a2)            // pOut

    bloop           t1, .L1, .L2
.L0:

    andi            t1, t0, 3
    bez             t1, .L3
.L4:
    ldbi.h          t2, (a0)
    ldbi.h          t3, (a1)
    psub.s16.s      t2, t2, t3
.L5:
    stbi.h          t2, (a2)
    bloop           t1, .L4, .L5
.L3:
    rts
    .size           csky_mat_sub_q15, .-csky_mat_sub_q15
