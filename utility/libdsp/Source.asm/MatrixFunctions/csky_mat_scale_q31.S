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
 * @file     csky_mat_scale_q31.S
 * @brief    Multiplies a Q31 matrix by a scalar.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * csky_status csky_mat_scale_q31(
 *   const csky_matrix_instance_q31 * pSrc,
 *   q31_t scaleFract,
 *   int32_t shift,
 *   csky_matrix_instance_q31 * pDst)
 */
    .file           "csky_mat_scale_q31.S"
    .section        .text.csky_mat_scale_q31,"ax",@progbits
    .align          2
    .global         csky_mat_scale_q31
    .type           csky_mat_scale_q31, @function
csky_mat_scale_q31:
    /*
     * unsupport CSKY_MATH_MATRIX_CHECK yet
     */
    ld.hs           t0, (a0, 0x0)
    ld.hs           t1, (a0, 0x2)
    mult            t0, t0, t1          // numSamples

    ld.w            a0, (a0, 0x4)       // pIn
    ld.w            a3, (a3, 0x4)       // pOut
    addi            a2, a2, 1           // totShift

    lsri            t1, t0, 2           // blkCnt
    bez             t1, .L0
.L1:
    pldbi.d         t2, (a0)            // in1, in2
    mul.s32.h       t2, t2, a1
    mul.s32.h       t3, t3, a1
    lsl.s32.s       t2, t2, a2
    lsl.s32.s       t3, t3, a2
    stbi.w          t2, (a3)
    stbi.w          t3, (a3)            // pOut

    pldbi.d         t2, (a0)            // in3, in4
    mul.s32.h       t2, t2, a1
    mul.s32.h       t3, t3, a1
    lsl.s32.s       t2, t2, a2
    lsl.s32.s       t3, t3, a2
    stbi.w          t2, (a3)
.L2:
    stbi.w          t3, (a3)            // pOut

    bloop           t1, .L1, .L2
.L0:

    andi            t1, t0, 3
    bez             t1, .L3
.L4:
    ldbi.w          t2, (a0)
    mul.s32.h       t2, t2, a1
    lsl.s32.s       t2, t2, a2
.L5:
    stbi.w          t2, (a2)
    bloop           t1, .L4, .L5
.L3:
    movi            a0, 0
    rts
    .size           csky_mat_scale_q31, .-csky_mat_scale_q31
