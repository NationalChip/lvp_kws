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
 * @file     csky_cmplx_mag_squared_q31.S
 * @brief    Q31 complex magnitude squared.
 * @version  V1.0
 * @date     14. Aug 2017
 ******************************************************************************/

/*
 * void csky_cmplx_maga_squared_q31(
 *   q31_t * pSrc,
 *   q31_t * pDst,
 *   uint32_t numSamples)
 */

    .file           "csky_cmplx_mag_squared_q31.S"
    .section        .text.csky_cmplx_mag_squared_q31,"ax",@progbits
    .align          2
    .global         csky_cmplx_mag_squared_q31
    .type           csky_cmplx_mag_squared_q31, @function

csky_cmplx_mag_squared_q31:
    lsri            t0, a2, 2           //blkCnt = blockSize >> 2u
    andi            t1, a2, 3           //blkCnt = blockSize % 0x4u
    bez             t0, .L1

.L2:
    pldbi.d         t2, (a0)            //load 1 q31_t data from source
    mul.s32         t4, t2, t2          //multiply
    mul.s32         t6, t3, t3
    asri            t4, t5, 1
    asri            t6, t7, 1
    add.s32.s       t4, t4, t6          //add
    stbi.w          t4, (a1)            //store the result

    pldbi.d         t2, (a0)            //repeat
    mul.s32         t4, t2, t2
    mul.s32         t6, t3, t3
    asri            t4, t5, 1
    asri            t6, t7, 1
    add.s32.s       t4, t4, t6
    stbi.w          t4, (a1)

    pldbi.d         t2, (a0)
    mul.s32         t4, t2, t2
    mul.s32         t6, t3, t3
    asri            t4, t5, 1
    asri            t6, t7, 1
    add.s32.s       t4, t4, t6
    stbi.w          t4, (a1)

    pldbi.d         t2, (a0)
    mul.s32         t4, t2, t2
    mul.s32         t6, t3, t3
    asri            t4, t5, 1
    asri            t6, t7, 1
    add.s32.s       t4, t4, t6
.L3:
    stbi.w          t4, (a1)

    bloop           t0, .L2, .L3        //while loop

.L1:
    bez             t1, .L0             //condition to end

.L4:
    pldbi.d         t2, (a0)            //same as before
    mul.s32         t4, t2, t2
    mul.s32         t6, t3, t3
    asri            t4, t5, 1
    asri            t6, t7, 1
    add.s32.s       t4, t4, t6
.L5:
    stbi.w          t4, (a1)

    bloop           t1, .L4, .L5        //while loop

.L0:
    rts                                 //end
    .size           csky_cmplx_mag_squared_q31, .-csky_cmplx_mag_squared_q31
