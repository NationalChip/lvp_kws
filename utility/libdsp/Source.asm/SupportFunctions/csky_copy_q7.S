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
 * @file     csky_copy_q7.S
 * @brief    Copies the elements of a Q7 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_copy_q7(
 *   q7_t * pSrc,
 *   q7_t * pDst,
 *   uint32_t blockSize)
 */
    .file           "csky_copy_q7.S"
    .section        .text.csky_copy_q7,"ax",@progbits
    .align          2
    .global         csky_copy_q7
    .type           csky_copy_q7, @function
csky_copy_q7:
    lsri            a3, a2, 3           // blkCnt
    bez             a3, .L0
.L1:
    pldbi.d         t0, (a0)
    stbi.w          t0, (a1)
.L2:
    stbi.w          t1, (a1)

    bloop           a3, .L1, .L2
.L0:

    andi            a3, a2, 7
    bez             a3, .L3
.L4:
    ldbi.b          t0, (a0)
.L5:
    stbi.b          t0, (a1)

    bloop           a3, .L4, .L5
.L3:

    rts
    .size           csky_copy_q7, .-csky_copy_q7
