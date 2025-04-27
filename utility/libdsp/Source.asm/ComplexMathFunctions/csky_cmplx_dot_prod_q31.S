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
 * @file     csky_cmplx_dot_prod_q31.S
 * @brief    Q31 complex conjugate.
 * @version  V1.0
 * @date     10. Aug 2017
 ******************************************************************************/

/*
 * void csky_cmplx_dot_prod_q15(
 *   q31_t * pSrcA,
 *   q31_t * pDstB,
 *   uint32_t numSamples
 *   q63_t * realResult,
 *   q63_t * imagResult)
 */

    .file           "csky_cmplx_dot_prod_q31.S"
    .section        .text.csky_cmplx_dot_prod_q31,"ax",@progbits
    .align          2
    .global         csky_cmplx_dot_prod_q31
    .type           csky_cmplx_dot_prod_q31, @function

csky_cmplx_dot_prod_q31:
    push            l0, l1, l2, l3, l4, l5
    ld.w            t0, (sp, 0x18)
    lsri            t1, a2, 2           //blkCnt = blockSize >> 2u
    andi            a2, a2, 3           //blkCnt = blockSize % 4u
    movi            t2, 0               //real_sum = 0
    movi            t3, 0
    movi            t4, 0               //imag_sum = 0
    movi            t5, 0
    bez             t1, .L1

.L2:
    pldbi.d         t6, (a0)            //load 1 q31_t complex data from source a
    pldbi.d         t8, (a1)            //load 1 q31_t complex data from source b
    mul.s32         l0, t6, t8          //compute the real part
    mul.s32         l2, t7, t9
    dexti           l4, l0, l1, 14
    asri            l5, l1, 14
    dexti           l0, l2, l3, 14
    asri            l1, l3, 14
    sub.64          l4, l4, l0
    add.64          t2, t2, l4
    mul.s32         l0, t6, t9          //compute the image oart
    mul.s32         l2, t7, t8
    dexti           l4, l0, l1, 14
    asri            l5, l1, 14
    dexti           l0, l2, l3, 14
    asri            l1, l3, 14
    add.64          l4, l4, l0
    add.64          t4, t4, l4

    pldbi.d         t6, (a0)            //load 1 q31_t complex data from source a
    pldbi.d         t8, (a1)            //load 1 q31_t complex data from source b
    mul.s32         l0, t6, t8          //compute the real part
    mul.s32         l2, t7, t9
    dexti           l4, l0, l1, 14
    asri            l5, l1, 14
    dexti           l0, l2, l3, 14
    asri            l1, l3, 14
    sub.64          l4, l4, l0
    add.64          t2, t2, l4
    mul.s32         l0, t6, t9          //compute the image oart
    mul.s32         l2, t7, t8
    dexti           l4, l0, l1, 14
    asri            l5, l1, 14
    dexti           l0, l2, l3, 14
    asri            l1, l3, 14
    add.64          l4, l4, l0
    add.64          t4, t4, l4

    pldbi.d         t6, (a0)            //load 1 q31_t complex data from source a
    pldbi.d         t8, (a1)            //load 1 q31_t complex data from source b
    mul.s32         l0, t6, t8          //compute the real part
    mul.s32         l2, t7, t9
    dexti           l4, l0, l1, 14
    asri            l5, l1, 14
    dexti           l0, l2, l3, 14
    asri            l1, l3, 14
    sub.64          l4, l4, l0
    add.64          t2, t2, l4
    mul.s32         l0, t6, t9          //compute the image oart
    mul.s32         l2, t7, t8
    dexti           l4, l0, l1, 14
    asri            l5, l1, 14
    dexti           l0, l2, l3, 14
    asri            l1, l3, 14
    add.64          l4, l4, l0
    add.64          t4, t4, l4

    pldbi.d         t6, (a0)            //load 1 q31_t complex data from source a
    pldbi.d         t8, (a1)            //load 1 q31_t complex data from source b
    mul.s32         l0, t6, t8          //compute the real part
    mul.s32         l2, t7, t9
    dexti           l4, l0, l1, 14
    asri            l5, l1, 14
    dexti           l0, l2, l3, 14
    asri            l1, l3, 14
    sub.64          l4, l4, l0
    add.64          t2, t2, l4
    mul.s32         l0, t6, t9          //compute the image oart
    mul.s32         l2, t7, t8
    dexti           l4, l0, l1, 14
    asri            l5, l1, 14
    dexti           l0, l2, l3, 14
    asri            l1, l3, 14
    add.64          l4, l4, l0
.L3:
    add.64          t4, t4, l4

    bloop           t1, .L2, .L3        //while loop

.L1:
    bez             a2, .L0             //condition to end

.L4:
    pldbi.d         t6, (a0)
    pldbi.d         t8, (a1)
    mul.s32         l0, t6, t8
    mul.s32         l2, t7, t9
    dexti           l4, l0, l1, 14
    asri            l5, l1, 14
    dexti           l0, l2, l3, 14
    asri            l1, l3, 14
    sub.64          l4, l4, l0
    add.64          t2, t2, l4
    mul.s32         l0, t6, t9
    mul.s32         l2, t7, t8
    dexti           l4, l0, l1, 14
    asri            l5, l1, 14
    dexti           l0, l2, l3, 14
    asri            l1, l3, 14
    add.64          l4, l4, l0
.L5:
    add.64          t4, t4, l4

    bloop           a2, .L4, .L5        //while loop

.L0:
    stbi.w          t2, (a3)            //store the data
    stbi.w          t3, (a3)
    stbi.w          t4, (t0)
    stbi.w          t5, (t0)
    pop             l0, l1, l2, l3, l4, l5
    .size           csky_cmplx_dot_prod_q31, .-csky_cmplx_dot_prod_q31
