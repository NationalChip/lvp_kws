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
 * @file     csky_add_q7.S
 * @brief    Q7 vector addition.
 * @version  V1.0
 * @date     04. Aug 2017
 ******************************************************************************/
/**
 *void csky_add_q7(
 *q7_t * pSrcA,
 *q7_t * pSrcB,
 *q7_t * pDst,
 *uint32_t blockSize)
**/

    .file           "csky_add_q7.S"
    .section        .text.csky_add_q7,"ax",@progbits
    .align          2
    .global         csky_add_q7
    .type           csky_add_q7, @function

csky_add_q7:
    lsri            t0, a3, 2           //blkCnt = blockSize >> 2u
    andi            t1, a3, 3           //blkCnt = blockSize % 0x4u
    bez             t0, .L2

.L4:
    ldbi.w          t2, (a0)            //load 4 q7_t data from source a
    ldbi.w          t4, (a1)            //load 4 q7_t data from source b
    padd.s8.s       t6, t2, t4          //addition
.L5:
    stbi.w          t6, (a2)            //store the results

    bloop           t0, .L4, .L5        //while loop

.L2:
    bez             t1, .L0             //condition to end

.L3:
    ldbi.bs         t2, (a0)            //load 1 q7_t data from source a
    ldbi.bs         t4, (a1)            //load 1 q7_t data from source b
    padd.s8.s       t6, t2, t4          //addition
.L1:
    stbi.b          t6, (a2)            //store the result

    bloop           t1, .L3, .L1        //while loop

.L0:
    rts                                 //end

    .size           csky_add_q7, .-csky_add_q7
