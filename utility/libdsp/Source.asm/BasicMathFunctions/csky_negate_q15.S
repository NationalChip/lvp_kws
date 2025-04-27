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
 * @file     csky_negate_q15.S
 * @brief    Negates Q15 vectors.
 * @version  V1.0
 * @date     04. Aug 2017
 ******************************************************************************/

/**
 *void csky_negate_q15(
 *q15_t * pSrc,
 *q15_t * pDst,
 *uint32_t blockSize)
**/

    .file           "csky_negate_q15.S"
    .section        .text.csky_negate_q15,"ax",@progbits
    .align          2
    .global         csky_negate_q15
    .type           csky_negate_q15, @function

csky_negate_q15:
    lsri            t0, a2, 2           //blkCnt = blockSize >> 2u
    andi            t1, a2, 3           //blkCnt = blockSize % 0x04
    bez             t0, .L2

.L4:
    pldbi.d         t2, (a0)            //load 4 q15_t data
    pneg.s16.s      t4, t2              //compute the negate value
    pneg.s16.s      t5, t3
    stbi.w          t4, (a1)            //store the results
.L5:
    stbi.w          t5, (a1)

    bloop           t0, .L4, .L5        //while loop

.L2:
    bez             t1, .L0             //condition to end

.L3:
    ldbi.h          t2, (a0)            //load 1 q15_t data
    pneg.s16.s      t4, t2              //compute the q_15t value
.L1:
    stbi.h          t4, (a1)            //store the data

    bloop           t1, .L3, .L1        //while loop

.L0:
    rts                                 //end

    .size           csky_negate_q15, .-csky_negate_q15
