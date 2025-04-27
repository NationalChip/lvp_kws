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
 * @file     csky_abs_q15.S
 * @brief    Q15 vector abs.
 * @version  V1.0
 * @date     03. Augest 2017
 ******************************************************************************/

/**
 *void csky_abs_q15(
 *q15_t * pSrc,
 *q15_t * pDst,
 *uint32_t blockSize)
**/

    .file           "csky_abs_q15.S"
    .section        .text.csky_abs_q15,"ax",@progbits
    .align          2
    .global         csky_abs_q15
    .type           csky_abs_q15, @function

csky_abs_q15:
    lsri            a3, a2, 2           //blkCnt = blockSize >> 2u
    andi            a2, a2, 3           //blkCnt = blockSize % 0x4u
    bez             a3, .L2


.L4:
    pldbi.d         t2, (a0)            //load 4 q15_t data
    pabs.s16.s      t2, t2              //compute the abs value
    pabs.s16.s      t3, t3
    stbi.w          t2, (a1)            //store the results
.L5:
    stbi.w          t3, (a1)

    bloop           a3, .L4, .L5        //while loop

.L2:
    bez             a2, .L0             //the condition to end

.L3:
    ldbi.hs         t2, (a0)            //load 1 q15_t data
    abs.s32.s       t2, t2              //compute the abs value
.L1:
    stbi.h          t2, (a1)            //store the results

    bloop           a2, .L3, .L1        //while loop

.L0:
    rts                                 //end
    .size           csky_abs_q15, .-csky_abs_q15
