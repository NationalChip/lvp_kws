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
 * @file     csky_dot_prod_q31.S
 * @brief    Q31 dot product.
 * @version  V1.0
 * @date     04. Aug 2017
 ******************************************************************************/

/**
 *void csky_dot_prod_q31(
 *q31_t * pSrcA,
 *q31_t * pSrcB,
 *uint32_t blockSize,
 *q63_t * result)
**/


    .file           "csky_dot_prod_q31.S"
    .section        .text.csky_dot_prod_q31,"ax",@progbits
    .align          2
    .global         csky_dot_prod_q31
    .type           csky_dot_prod_q31, @function

csky_dot_prod_q31:
    movi            t0, 0               //sum = 0
    movi            t1, 0
    lsri            t2, a2, 2           //blkCnt = blockSize >> 2u
    andi            t3, a2, 3           //blkCnt = blockSize % 0x04u
    bez             t2, .L2

.L4:
    pldbi.d         t4, (a0)            //load 2 q31_t data from source a
    pldbi.d         t8, (a1)            //load 2 q31_t data from source b
    mul.s32         t6, t4, t8          //multiply
    dexti           t6, t6, t7, 14      //abandon the low 14 bits
    asri            t7, t7, 14
    add.64          t0, t0, t6          //addition

    mul.s32         t6, t5, t9          //another data
    dexti           t6, t6, t7, 14
    asri            t7, t7, 14
    add.64          t0, t0, t6

    pldbi.d         t4, (a0)            //load 2 q31_t data from source a
    pldbi.d         t8, (a1)            //load 2 q31_t data from source b
    mul.s32         t6, t4, t8          //multiply
    dexti           t6, t6, t7, 14      //abandon the low 14 bits
    asri            t7, t7, 14
    add.64          t0, t0, t6          //addition

    mul.s32         t6, t5, t9          //another data
    dexti           t6, t6, t7, 14
    asri            t7, t7, 14
.L5:
    add.64          t0, t0, t6

    bloop           t2, .L4, .L5        //while loop

.L2:
    bez             t3, .L0             //the condition to end

.L3:
    ldbi.w          t4, (a0)            //same as .L4 loop
    ldbi.w          t5, (a1)
    mul.s32         t6, t4, t5
    dexti           t6, t6, t7, 14
    asri            t7, t7, 14
.L1:
    add.64          t0, t0, t6

    bloop           t3, .L3, .L1        //while loop

.L0:
    stbi.w          t0, (a3)            //result = sum
    stbi.w          t1, (a3)
    rts                                 //end
    .size           csky_dot_prod_q31, .-csky_dot_prod_q31
