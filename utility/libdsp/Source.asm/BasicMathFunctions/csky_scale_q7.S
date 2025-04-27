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
 * @file     csky_scale_q7.S
 * @brief    Multiplies a Q7 vector by a scalar.
 * @version  V1.0
 * @date     04. Aug 2017
 ******************************************************************************/

/**
 *void csky_scale_q7(
 *q7_t * pSrc,
 *q7_t scaleFract,
 *int8_t shift,
 *q7_t * pDst,
 *uint32_t blockSize)
**/

    .file           "csky_scale_q7.S"
    .section        .text.csky_scale_q7,"ax",@progbits
    .align          2
    .global         csky_scale_q7
    .type           csky_scale_q7, @function

csky_scale_q7:
    push            l0
    ld.w            t0, (sp, 0x04)       //load the blockSize
    dup.16          a1, a1, 0
    movi            l0, 7               //prepare for shift
    sub             l0, l0, a2
    lsri            t1, t0, 3           //blkCnt = blockSize >> 3u
    andi            t0, t0, 7           //blkCnt = blockSize % 0x07
    bez             t1, .L2

.L4:
    pldbi.d         t4, (a0)            //load 8 q7_t data from source
    pext.s8.e       t6, t4              //prepare the data for multiply
    pmul.s16        t2, t6, a1          //multiply
    pkgll           t4, t2, t3
    pasr.s16        t4, t4, l0          //shift
    pclipi.s16      t2, t4, 8           //saturation

    pmul.s16        t8, t7, a1          //amother 2 data
    pkgll           t4, t8, t9
    pasr.s16        t7, t4, l0
    pclipi.s16      t4, t7, 8
    narl            t6, t2, t4          //prepare for store
    stbi.w          t6, (a3)            //store

    pext.s8.e       t6, t5              //prepare the data for multiply
    pmul.s16        t2, t6, a1          //multiply
    pkgll           t4, t2, t3
    pasr.s16        t4, t4, l0          //shift
    pclipi.s16      t2, t4, 8           //saturation

    pmul.s16        t8, t7, a1          //amother 2 data
    pkgll           t4, t8, t9
    pasr.s16        t7, t4, l0
    pclipi.s16      t4, t7, 8
    narl            t6, t2, t4          //prepare for store
.L5:
    stbi.w          t6, (a3)            //store

    bloop           t1, .L4, .L5        //while loop

.L2:
    bez             t0, .L0             //condition to end

.L3:
    movi            t4, 0
    ldbi.bs         t4, (a0)            //load 1 q7_t data
    pext.s8.e       t5, t4
    pmul.s16        t6, t5, a1          //multiply
    asr             t4, t6, l0          //shift
    clipi.s32       t2, t4, 8           //saturation
.L1:
    stbi.b          t2, (a3)            //store the result

    bloop           t0, .L3, .L1        //while loop

.L0:
    pop             l0                  //end
    rts
    .size           csky_scale_q7, .-csky_scale_q7
