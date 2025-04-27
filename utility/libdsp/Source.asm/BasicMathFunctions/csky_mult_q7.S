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
 * @file     csky_mult_q7.S
 * @brief    Q7 vector multiplication.
 * @version  V1.0
 * @date     04. Aug 2017
 ******************************************************************************/

/**
 *void csky_mult_q7(
 *q7_t * pSrcA,
 *q7_t * pSrcB,
 *q7_t * pDst,
 *uint32_t blockSize)
**/

    .file           "csky_mult_q7.S"
    .section        .text.csky_mult_q7,"ax",@progbits
    .align          2
    .global         csky_mult_q7
    .type           csky_mult_q7, @function

csky_mult_q7:
    lsri            t0, a3, 2           //blkCnt = blockSize >> 2u
    andi            t1, a3, 3           //blkCnt = blockSize % 0x04
    bez             t0, .L2

.L4:
    ldbi.w          t2, (a0)            //load 4 q7_t data from source a
    ldbi.w          t3, (a1)            //load 4 q7_t data from source b
    pext.s8.e       t4, t2              //prepare for multiply
    pext.s8.e       t6, t3
    pmul.s16        t8, t4, t6          //multiply
    pmul.s16        t2, t5, t7
    pkgll           t8, t8, t9
    pkgll           t2, t2, t3
    pasri.s16       t8, t8, 7           //abandon the low 7 bit
    pasri.s16       t2, t2, 7
    pclipi.s16      t8, t8, 8           //saturate
    pclipi.s16      t2, t2, 8
    narl            t4, t8, t2
.L5:
    stbi.w          t4, (a2)            //store the data

    bloop           t0, .L4, .L5        //while loop

.L2:
    bez             t1, .L0             //condition to end

.L3:
    movi            t2, 0
    movi            t4, 0
    ldbi.bs         t2, (a0)            //load 1 q7_t data from source a
    ldbi.bs         t3, (a1)            //load 1 q7_t data from source b
    pext.s8.e       t4, t2
    pext.s8.e       t6, t3
    mulll.s16       t6, t4, t6
    asri            t6, t6, 7
    clipi.s32       t5, t6, 8
.L1:
    stbi.b          t5, (a2)            //store the data

    bloop           t1, .L3, .L1        //while loop

.L0:
    rts                                 //end
    .size           csky_mult_q7, .-csky_mult_q7
