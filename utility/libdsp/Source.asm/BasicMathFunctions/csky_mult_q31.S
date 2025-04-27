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
 * @file     csky_mult_q31.S
 * @brief    Q31 vector multiplication.
 * @version  V1.0
 * @date     04. Aug 2017
 ******************************************************************************/

/**
 *void csky_mult_q31(
 *q31_t * pSrcA,
 *q31_t * pSrcB,
 *q31_t * pDst,
 *uint32_t blockSize)
**/

    .file           "csky_mult_q31.S"
    .section        .text.csky_mult_q31,"ax",@progbits
    .align          2
    .global         csky_mult_q31
    .type           csky_mult_q31, @function

csky_mult_q31:
    lsri            t0, a3, 2           //blkCnt = blockSize >> 2u
    andi            t1, a3, 3           //blkCnt = blockSize % 0x04
    bez             t0, .L2

.L4:
    pldbi.d         t2, (a0)            //load 2 q31_t data from source a
    pldbi.d         t4, (a1)            //load 2 q31_t data from source b
    rmul.s32.h      t6, t2, t4          //multiply
    rmul.s32.h      t7, t3, t5
    stbi.w          t6, (a2)            //store the results
    stbi.w          t7, (a2)

    pldbi.d         t2, (a0)            //repeat
    pldbi.d         t4, (a1)
    rmul.s32.h      t6, t2, t4
    rmul.s32.h      t7, t3, t5
    stbi.w          t6, (a2)
.L5:
    stbi.w          t7, (a2)

    bloop           t0, .L4, .L5        //while loop

.L2:
    bez             t1, .L0             //condition to end

.L3:
    ldbi.w          t2, (a0)            //load 1 q31_t data from source a
    ldbi.w          t4, (a1)            //load 1 q31_t data from source b
    rmul.s32.h      t6, t2, t4          //multiply
.L1:
    stbi.w          t6, (a2)            //store the result

    bloop           t1, .L3, .L1        //while loop

.L0:
    rts                                 //end

    .size           csky_mult_q31, .-csky_mult_q31
