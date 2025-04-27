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
 * @file     csky_scale_q15.S
 * @brief    Multiplies a Q15 vector by a scalar.
 * @version  V1.0
 * @date     07. Aug 2017
 ******************************************************************************/

/**
 *void csky_scale_q15(
 *q15_t * pSrc,
 *q15_t scaleFract,
 *int8_t shift,
 *q15_t * pDst,
 *uint32_t blockSize)
**/

    .file           "csky_scale_q15.S"
    .section        .text.csky_scale_q15,"ax",@progbits
    .align          2
    .global         csky_scale_q15
    .type           csky_scale_q15, @function

csky_scale_q15:
    ld.w            t0, (sp, 0x0)       //load the blockSize
    dup.16          a1, a1, 0           //prepare for 16 multiply
    movi            t8, 15              //prepare for shift
    sub             t8, t8, a2
    lsri            t1, t0, 2           //blkCnt = blockSize >> 2u
    andi            t0, t0, 3           //blkCnt = blockSize % 0x04
    bez             t1, .L2

.L4:
    pldbi.d         t4, (a0)            //load 4 q15_t data from source
    pmul.s16        t6, t4, a1          //multiply the scale
    asr             t6, t6, t8          //shift
    asr             t7, t7, t8
    clipi.s32       t2, t6, 16          //saturation
    clipi.s32       t4, t7, 16
    pkgll           t6, t2, t4          //prepare for store
    stbi.w          t6, (a3)            //store the results

    pmul.s16        t6, t5, a1          //repeat
    asr             t6, t6, t8
    asr             t7, t7, t8
    clipi.s32       t2, t6, 16
    clipi.s32       t4, t7, 16
    pkgll           t6, t2, t4
.L5:
    stbi.w          t6, (a3)

    bloop           t1, .L4, .L5        //while loop

.L2:
    bez             t0, .L0             //condition to end

.L3:
    ldbi.hs         t4, (a0)            //load 1 q15_t data from source
    mulll.s16       t6, t4, a1          //multiply the scale
    asr             t6, t6, t8          //shift
    clipi.s32       t2, t6, 16          //saturation
.L1:
    stbi.h          t2, (a3)            //store the data

    bloop           t0, .L3, .L1        //while loop

.L0:
    rts                                 //end
    .size           csky_scale_q15, .-csky_scale_q15
