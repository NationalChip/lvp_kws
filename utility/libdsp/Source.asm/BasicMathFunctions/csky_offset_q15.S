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
 * @file     csky_offset_q15.S
 * @brief    Q15 vector offset.
 * @version  V1.0
 * @date     07. Aug 2017
 ******************************************************************************/

/**
 *void csky_offset_q15(
 *q15_t * pSrc,
 *q15_t offset,
 *q15_t * pDst,
 *uint32_t blockSize)
**/

    .file           "csky_offset_q15.S"
    .section        .text.csky_offset_q15,"ax",@progbits
    .align          2
    .global         csky_offset_q15
    .type           csky_offset_q15, @function

csky_offset_q15:
    lsri            t0, a3, 2           //blkCnt = blockSize >> 2u
    andi            t1, a3, 3           //blkCnt = blockSize % 0x04
    bez             t0, .L2

.L4:
    pldbi.d         t2, (a0)            //load 4 q15_t data from source
    dup.16          a1, a1, 0
    padd.s16.s      t6, t2, a1          //addition
    padd.s16.s      t7, t3, a1
    stbi.w          t6, (a2)            //store the results
.L5:
    stbi.w          t7, (a2)

    bloop           t0, .L4, .L5        //while loop

.L2:
    bez             t1, .L0             //condition to end

.L3:
    ldbi.h          t2, (a0)            //load 1 q15_t data from source
    padd.s16.s      t6, t2, a1          //addition
.L1:
    stbi.h          t6, (a2)            //store the result

    bloop           t1, .L3, .L1        //while loop

.L0:
    rts                                 //end

    .size           csky_offset_q15, .-csky_offset_q15
