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
 * @file     csky_cmplx_mag_q31.S
 * @brief    Q31 complex magnitude.
 * @version  V1.0
 * @date     14. Aug 2017
 ******************************************************************************/

/*
 * void csky_cmplx_mag_q31(
 *   q31_t * pSrc,
 *   q31_t * pDst,
 *   uint32_t numSamples)
 */

.import  csky_sqrt_q31

    .file           "csky_cmplx_mag_q31.S"
    .section        .text.csky_cmplx_mag_q31,"ax",@progbits
    .align          2
    .global         csky_cmplx_mag_q31
    .type           csky_cmplx_mag_q31, @function

csky_cmplx_mag_q31:
    push            l0, l1, l2, l3,lr
    mov             l0, a0
    mov             l1, a1
    lsri            l2, a2, 2           //blkCnt = blockSize >> 2u
    andi            l3, a2, 3           //blkCnt = blockSize % 0x4u
    bez             l2, .L1

.L2:
    pldbi.d         t2, (l0)            //load 1 q31_t data from source
    mul.s32         t4, t2, t2          //multiply
    mul.s32         t6, t3, t3          //multiply
    asri            t5, t5, 1           //prepare for add and sqrt
    asri            t7, t7, 1
    add.s32.s       a0, t5, t7
    mov             a1, l1
    bsr             csky_sqrt_q31       //call the sqrt function
    addi            l1, l1, 4

    pldbi.d         t2, (l0)            //repeat
    mul.s32         t4, t2, t2
    mul.s32         t6, t3, t3
    asri            t5, t5, 1
    asri            t7, t7, 1
    add.s32.s       a0, t7, t5
    mov             a1, l1
    bsr             csky_sqrt_q31
    addi            l1, l1, 4

    pldbi.d         t2, (l0)
    mul.s32         t4, t2, t2
    mul.s32         t6, t3, t3
    asri            t5, t5, 1
    asri            t7, t7, 1
    add.s32.s       a0, t7, t5
    mov             a1, l1
    bsr             csky_sqrt_q31
    addi            l1, l1, 4

    pldbi.d         t2, (l0)
    mul.s32         t4, t2, t2
    mul.s32         t6, t3, t3
    asri            t5, t5, 1
    asri            t7, t7, 1
    add.s32.s       a0, t7, t5
    mov             a1, l1
    bsr             csky_sqrt_q31
.L3:
    addi            l1, l1, 4

    bloop           l2, .L2, .L3        //while loop

.L1:
    bez             l3, .L0             //condition to end

.L4:
    pldbi.d         t2, (l0)            //same as before
    mul.s32         t4, t2, t2
    mul.s32         t6, t3, t3
    asri            t5, t5, 1
    asri            t7, t7, 1
    add.s32.s       a0, t7, t5
    mov             a1, l1
    bsr             csky_sqrt_q31
.L5:
    addi            l1, l1, 4

    bloop           l3, .L4, .L5        //while loop

.L0:
    pop             l0, l1, l2, l3, lr
    .size           csky_cmplx_mag_q31, .-csky_cmplx_mag_q31
