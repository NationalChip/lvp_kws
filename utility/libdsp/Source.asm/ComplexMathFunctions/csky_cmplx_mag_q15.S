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
 * @file     csky_cmplx_mag_q15.S
 * @brief    Q15 complex magnitude.
 * @version  V1.0
 * @date     11. Aug 2017
 ******************************************************************************/

/*
 * void csky_cmplx_mag_q15(
 *   q15_t * pSrc,
 *   q15_t * pDst,
 *   uint32_t numSamples)
 */
.import  csky_sqrt_q15

    .file           "csky_cmplx_mag_q15.S"
    .section        .text.csky_cmplx_mag_q15,"ax",@progbits
    .align          2
    .global         csky_cmplx_mag_q15
    .type           csky_cmplx_mag_q15, @function

csky_cmplx_mag_q15:
    push            l0, l1, l2, l3, lr
    mov             l0, a0
    mov             l1, a1
    lsri            l2, a2, 2           //blkCnt = blockSize >> 2u
    andi            l3, a2, 3           //blkCnt = blockSize % 0x4u
    bez             l2, .L1

.L2:
    pldbi.d         t4, (l0)            //load 2 q15_t complex data from source
    mulca.s16.s     a0, t4, t4          //multiply and add
    asri            a0, a0, 17          //prepare for sqrt
    mov             a1, l1
    bsr             csky_sqrt_q15       //call the sqrt function
    addi            l1, l1, 2
    mulca.s16.s     a0, t5, t5          //another data
    asri            a0, a0, 17
    mov             a1, l1
    bsr             csky_sqrt_q15
    addi            l1, l1, 2

    pldbi.d         t4, (l0)            //repeat
    mulca.s16.s     a0, t4, t4
    asri            a0, a0, 17
    mov             a1, l1
    bsr             csky_sqrt_q15
    addi            l1, l1, 2
    mulca.s16.s     a0, t5, t5
    asri            a0, a0, 17
    mov             a1, l1
    bsr             csky_sqrt_q15
.L3:
    addi            l1, l1, 2

    bloop           l2, .L2, .L3        //while loop

.L1:
    bez             l3, .L0             //condition to end

.L4:
    ldbi.w          t4, (l0)            //load a q15_t complex data from source
    mulca.s16.s     a0, t4, t4          //multiply and add
    asri            a0, a0, 17          //prepare for sqrt
    mov             a1, l1
    bsr             csky_sqrt_q15       //call the sqrt function
.L5:
    addi            l1, l1, 2

    bloop           l3, .L4, .L5        //while loop

.L0:
    pop             l0, l1, l2, l3, lr
    .size           csky_cmplx_mag_q15, .-csky_cmplx_mag_q15
