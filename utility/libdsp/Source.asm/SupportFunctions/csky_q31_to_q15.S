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
 * @file     csky_q31_to_q15.S
 * @brief    Converts the elements of the Q31 vector to Q15 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_q31_to_q15(
 *   q31_t * pSrc,
 *   q15_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_q31_to_q15.S"
    .section        .text.csky_q31_to_q15,"ax",@progbits
    .align          2
    .global         csky_q31_to_q15
    .type           csky_q31_to_q15, @function
csky_q31_to_q15:
    lsri            a3, a2, 2           // blkCnt
    bez             a3, .L0
.L1:
    pldbi.d         t0, (a0)
    pldbi.d         t2, (a0)
    pkghh           t4, t0, t1
    pkghh           t5, t2, t3
    stbi.w          t4, (a1)
.L2:
    stbi.w          t5, (a1)

    bloop           a3, .L1, .L2
.L0:

    andi            a3, a2, 3
    bez             a3, .L3
.L4:
    ldbi.w          t0, (a0)
    asri            t0, t0, 16
.L5:
    stbi.h          t0, (a1)

    bloop           a3, .L4, .L5
.L3:

    rts
    .size           csky_q31_to_q15, .-csky_q31_to_q15
