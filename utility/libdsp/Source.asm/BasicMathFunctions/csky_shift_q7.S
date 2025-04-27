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
 * @file     csky_shift_q7.S
 * @brief    Shifts the elements of a Q7 vector by a specified number of bits.
 * @version  V1.0
 * @date     04. Aug 2017
 ******************************************************************************/

/**
 * void csky_shift_q7(
 * q7_t * pSrc,
 * int8_t shiftBits,
 * q7_t * pDst,
 * uint32_t blockSize)
**/

    .file	"csky_shift_q7.S"
    .section	.text.csky_shift_q7,"ax",@progbits
    .align	2
	.global	csky_shift_q7
    .type	csky_shift_q7, @function

csky_shift_q7:
    lsri            t0, a3, 3           // blkCnt = blockSize >> 3u
    andi            t1, a3, 7           // blkCnt = blockSize % 0x7u
    bhz             a1, .L7             // if(sign == 0u)

.L1:
    neg.s32.s       a1, a1              // shiftBits = -shiftBits
    bez             t0, .L4             // while(blkCnt > 0u)

.L2:                                    // else branch
    pldbi.d         t4, (a0)            // load 8 q7_t data from source
    pext.s8.e       t6, t4              // prepare the data for shift
    pext.s8.e       t8, t5
    pasr.s16        t4, t6, a1          // shift to right
    pasr.s16        t5, t7, a1
    pasr.s16        t6, t8, a1
    pasr.s16        t7, t9, a1
    narl            t8, t4, t5          // prepare the data for store
    narl            t9, t6, t7
    stbi.w          t8, (a2)            // store the results
.L3:
    stbi.w          t9, (a2)

    bloop           t0, .L2, .L3

.L4:
    bez             t1, .L0

.L5:
    movi            t2, 0               // *pDst++ = (*pSrc++ >> shiftBits)
    ldbi.bs         t2, (a0)
    pext.s8.e       t2, t2
    pasr.s16        t2, t2, a1
.L6:
    stbi.b          t2, (a2)

    bloop           t1, .L5, .L6
    br              .L0

.L7:
    addi            a1, a1, 8           // shift to left
    bez             t0, .L10

.L8:
    pldbi.d         t2, (a0)            // load four data from source
    pext.s8.e       t6, t2
    pext.s8.e       t8, t3
    plsl.s16.s      t4, t6, a1
    plsl.s16.s      t5, t7, a1
    plsl.s16.s      t6, t8, a1
    plsl.s16.s      t7, t9, a1
    narh            t8, t4, t5          //prepare the data for store
    narh            t9, t6, t7
    stbi.w          t8, (a2)
.L9:
    stbi.w          t9, (a2)

    bloop           t0, .L8, .L9

.L10:
    bez             t1, .L0

.L11:
    movi            t2, 0               // *pDst++ = (*pSrc++ >> shiftBits)
    ldbi.bs         t2, (a0)
    plsl.s16.s      t2, t2, a1
    asri            t2, t2, 8
.L12:
    stbi.b          t2, (a2)

    bloop           t1, .L11, .L12      // while loop

.L0:
    rts                                 // end
    .size           csky_shift_q7, .-csky_shift_q7
