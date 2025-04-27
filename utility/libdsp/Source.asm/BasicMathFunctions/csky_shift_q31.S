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
 * @file     csky_shift_q31.S
 * @brief    Shifts the elements of a Q31 vector by a specified number of bits.
 * @version  V1.0
 * @date     08. Aug 2017
 ******************************************************************************/

/**
 *void csky_shift_q31(
 *q31_t * pSrc,
 *ina1_t shiftBits,
 *q31_t * pDst,
 *uint32_t blockSize)
**/

    .file           "csky_shift_q31.S"
    .section        .text.csky_shift_q31,"ax",@progbits
    .align          2
    .global         csky_shift_q31
    .type           csky_shift_q31, @function

csky_shift_q31:
    lsri            t0, a3, 2           //blkCnt = blockSize >> 2u
    andi            t1, a3, 3           //blkCnt = blockSize % 0x4u
    blz             a1, .L2

.L1:
    bez             t0, .L5

.L3:
    pldbi.d         t2, (a0)            //load 4 q31_t data from source
    pldbi.d         t4, (a0)

    lsl.s32.s       t6, t2, a1          //shift to left when the shift value bigger than 0
    lsl.s32.s       t7, t3, a1
    lsl.s32.s       t2, t4, a1
    lsl.s32.s       t3, t5, a1

    stbi.w          t6, (a2)
    stbi.w          t7, (a2)
    stbi.w          t2, (a2)
.L4:
    stbi.w          t3, (a2)

    bloop           t0, .L3, .L4

.L5:
    bez             t1, .L0

.L6:
    ldbi.w          t2, (a0)
    lsl.s32.s       t2, t2, a1
.L7:
    stbi.w          t2, (a2)

    bloop           t1, .L6, .L7
    br              .L0

.L2:
    neg.s32.s       a1, a1
    bez             t0, .L10

.L8:
    pldbi.d         t2, (a0)
    pldbi.d         t4, (a0)
    asr             t6, t2, a1          //shift to right when the shift value smaller than 0
    asr             t7, t3, a1
    asr             t2, t4, a1
    asr             t3, t5, a1

    stbi.w          t6, (a2)            //store the results
    stbi.w          t7, (a2)
    stbi.w          t2, (a2)
.L9:
    stbi.w          t3, (a2)

    bloop           t0, .L8, .L9

.L10:
    bez             t1, .L0

.L11:
    ldbi.w          t2, (a0)
    asr             t2, t2, a1
.L12:
    stbi.w          t2, (a2)

    bloop           t1, .L11, .L12      //while loop

.L0:
    rts                                 //end
    .size           csky_shift_q31, .-csky_shift_q31
