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
 * @file     csky_mat_scale_q15.S
 * @brief    Multiplies a Q15 matrix by a scalar.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * csky_status csky_mat_scale_q15(
 *   const csky_matrix_instance_q15 * pSrc,
 *   q15_t scaleFract,
 *   int32_t shift,
 *   csky_matrix_instance_q15 * pDst)
 */
    .file           "csky_mat_scale_q15.S"
    .section        .text.csky_mat_scale_q15,"ax",@progbits
    .align          2
    .global         csky_mat_scale_q15
    .type           csky_mat_scale_q15, @function
csky_mat_scale_q15:
    /*
     * unsupport CSKY_MATH_MATRIX_CHECK yet
     */
    ld.hs           t0, (a0, 0x0)
    ld.hs           t1, (a0, 0x2)
    mult            t0, t0, t1          // numSamples
    dup.16          a1, a1, 0

    ld.w            a0, (a0, 0x4)       // pIn
    ld.w            a3, (a3, 0x4)       // pOut
    movi            t1, 15
    subu            a2, t1, a2          // totShift

    lsri            t1, t0, 2           // blkCnt
    bez             t1, .L0
.L1:
    pldbi.d         t2, (a0)            // in1, in2
    pmul.s16        t4, t2, a1
    pmul.s16        t6, t3, a1
    asr             t4, t4, a2
    asr             t5, t5, a2
    asr             t6, t6, a2
    asr             t7, t7, a2
    clipi.s32       t4, t4, 16
    clipi.s32       t5, t5, 16
    clipi.s32       t6, t6, 16
    clipi.s32       t7, t7, 16
    pkgll           t2, t4, t5
    pkgll           t3, t6, t7
    stbi.w          t2, (a3)
.L2:
    stbi.w          t3, (a3)            // pOut

    bloop           t1, .L1, .L2
.L0:

    andi            t1, t0, 3
    bez             t1, .L3
.L4:
    ldbi.hs         t2, (a0)
    mult            t2, t2, a1
    asr             t2, t2, a2
.L5:
    stbi.h          t2, (a2)

    bloop           t1, .L4, .L5
.L3:
    movi            a0, 0
    rts
    .size           csky_mat_scale_q15, .-csky_mat_scale_q15
