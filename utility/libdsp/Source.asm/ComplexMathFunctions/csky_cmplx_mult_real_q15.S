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
 * @file     csky_cmplx_mult_real_q15.S
 * @brief    Q15 complex-by-real multiplication.
 * @version  V1.0
 * @date     14. Aug 2017
 ******************************************************************************/

/*
 * void csky_cmplx_mult_real_q15(
 *   q15_t * pSrcCmplx,
 *   q15_t * pSrcReal,
 *   q15_t * pCmplxDst
 *   uint32_t numSamples)
 */

    .file           "csky_cmplx_mult_real_q15.S"
    .section        .text.csky_cmplx_mult_real_q15,"ax",@progbits
    .align          2
    .global         csky_cmplx_mult_real_q15
    .type           csky_cmplx_mult_real_q15, @function

csky_cmplx_mult_real_q15:
    lsri            t0, a3, 2           //blkCnt = blockSize >> 2u
    andi            t1, a3, 3           //blkCnt = blockSize % 0x4u
    bez             t0, .L1

.L2:
    pldbi.d         t2, (a0)            //load 2 q15_t data from complex source
    pldbi.d         t4, (a1)            //load 4 q15_t data from real source
    dup.16          t6, t4, 0           //prepare for multiply
    prmul.s16.h     t2, t2, t6          //multiply
    stbi.w          t2, (a2)            //store the result
    dup.16          t6, t4, 1           //another data
    prmul.s16.h     t3, t3, t6
    stbi.w          t3, (a2)

    pldbi.d         t2, (a0)            //load 2 q15_t data from complex source
    dup.16          t6, t5, 0
    prmul.s16.h     t2, t2, t6
    stbi.w          t2, (a2)
    dup.16          t6, t5, 1
    prmul.s16.h     t3, t3, t6
.L3:
    stbi.w          t3, (a2)

    bloop           t0, .L2, .L3        //while loop

.L1:
    bez             t1, .L0             //condition to end

.L4:
    ldbi.w          t2, (a0)            //real multply for one data
    ldbi.hs         t3, (a1)
    dup.16          t4, t3, 0
    prmul.s16.h     t3, t2, t4
.L5:
    stbi.w          t3, (a2)

    bloop           t1, .L4, .L5        //while loop

.L0:
    rts                                 //end
    .size           csky_cmplx_mult_real_q15, .-csky_cmplx_mult_real_q15
