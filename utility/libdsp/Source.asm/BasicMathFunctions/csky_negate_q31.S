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
 * @file     csky_negate_q31.S
 * @brief    Negates Q31 vectors.
 * @version  V1.0
 * @date     07. Aug 2017
 ******************************************************************************/

/**
  *void csky_negate_q31(
  *q31_t * pSrc,
  *q31_t * pDst,
  *uint32_t blockSize)
**/

    .file           "csky_negate_q31.S"
    .section        .text.csky_negate_q31,"ax",@progbits
    .align          2
    .global         csky_negate_q31
    .type           csky_negate_q31, @function

csky_negate_q31:
    lsri            t0, a2, 2           //blkCnt = blockSize >> 2u
    andi            t1, a2, 3           //blkCnt = blockSize % 0x04
    bez             t0, .L2

.L4:
    pldbi.d         t2, (a0)            //load 2 q31_t data
    neg.s32.s       t4, t2              //compute the negate value
    neg.s32.s       t5, t3
    stbi.w          t4, (a1)            //store the results
    stbi.w          t5, (a1)

    pldbi.d         t2, (a0)            //repeat
    neg.s32.s       t4, t2
    neg.s32.s       t5, t3
    stbi.w          t4, (a1)
.L5:
    stbi.w          t5, (a1)

    bloop           t0, .L4, .L5        //while loop

.L2:
    bez             t1, .L0             //condition to end

.L3:
    ldbi.w          t2, (a0)            //load 1 q31_t data
    neg.s32.s       t4, t2              //compute the negate value
.L1:
    stbi.w          t4, (a1)            //store the result

    bloop           t1, .L3, .L1        //while loop

.L0:
    rts                                 //end

    .size           csky_negate_q31, .-csky_negate_q31
