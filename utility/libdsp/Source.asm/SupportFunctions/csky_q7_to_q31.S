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
 * @file     csky_q7_to_q31.S
 * @brief    Converts the elements of the Q31 vector to Q7 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_q7_to_q31(
 *   q7_t * pSrc,
 *   q15_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_q7_to_q31.S"
    .section        .text.csky_q7_to_q31,"ax",@progbits
    .align          2
    .global         csky_q7_to_q31
    .type           csky_q7_to_q31, @function
csky_q7_to_q31:
    lsri            a3, a2, 3           // blkCnt
    bez             a3, .L0
    movi            t6, 0
.L1:
    pldbi.d         t0, (a0)
    pext.s8.e       t2, t0
    pext.s8.e       t4, t1
    plsli.16        t2, t2, 8
    plsli.16        t3, t3, 8
    plsli.16        t4, t4, 8
    plsli.16        t5, t5, 8
    pkgll           t8, t6, t2
    pkghh           t9, t6, t2
    stbi.w          t8, (a1)
    stbi.w          t9, (a1)
    pkgll           t8, t6, t3
    pkghh           t9, t6, t3
    stbi.w          t8, (a1)
    stbi.w          t9, (a1)
    pkgll           t8, t6, t4
    pkghh           t9, t6, t4
    stbi.w          t8, (a1)
    stbi.w          t9, (a1)
    pkgll           t8, t6, t5
    pkghh           t9, t6, t5
    stbi.w          t8, (a1)
.L2:
    stbi.w          t9, (a1)

    bloop           a3, .L1, .L2
.L0:

    andi            a3, a2, 7
    bez             a3, .L3
.L4:
    ldbi.bs         t0, (a0)
    lsli            t0, t0, 24
.L5:
    stbi.w          t0, (a1)

    bloop           a3, .L4, .L5
.L3:

    rts
    .size           csky_q7_to_q31, .-csky_q7_to_q31
