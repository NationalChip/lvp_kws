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
 * @file     csky_dot_prod_q7.S
 * @brief    Q7 dot product.
 * @version  V1.0
 * @date     04. Aug 2017
 ******************************************************************************/

/**
 *void csky_dot_prod_q7(
 *q7_t * pSrcA,
 *q7_t * pSrcB,
 *uint32_t blockSize,
 *q31_t * result)
**/

    .file           "csky_dot_prod_q7.S"
    .section        .text.csky_dot_prod_q7,"ax",@progbits
    .align          2
    .global         csky_dot_prod_q7
    .type           csky_dot_prod_q7, @function

csky_dot_prod_q7:
    movi            t0, 0               //sum = 0
    lsri            t1, a2, 3           //blkCnt = blockSize >> 3u
    andi            t2, a2, 7           //blkCnt = blockSize % 0x07u
    bez             t1, .L2

.L4:
    pldbi.d         t4, (a0)            //load 8 q7_t data from source a
    pldbi.d         t6, (a1)            //load 8 q7_t data from source b
    mulaca.s8       t8, t4, t6
    mulaca.s8       t9, t5, t7
    add.s32.s       t0, t0, t8
.L5:
    add.s32.s       t0, t0, t9

    bloop           t1, .L4, .L5        //while loop

.L2:
    bez             t2, .L0             //condition to end

.L3:
    ldbi.bs         t4, (a0)            //load 1 q7_t data from source a
    ldbi.bs         t6, (a1)            //load 1 q7_t data from source b
.L1:
    mulall.s16.s    t0, t4, t6          //mul and add

    bloop           t2, .L3, .L1        //while loop

.L0:
    st.w            t0, (a3)
    rts                                 //end

    .size           csky_dot_prod_q7, .-csky_dot_prod_q7
