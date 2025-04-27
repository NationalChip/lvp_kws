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
 * @file     csky_rms_q15.S
 * @brief    Root Mean Square of the elements of a Q15 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

.import csky_sqrt_q15

/*
 * void csky_rms_q15(
 *   q15_t * pSrc,
 *   uint32_t blockSize,
 *   q15_t * pResult)
 */

    .file           "csky_rms_q15.S"
    .section        .text.csky_rms_q15,"ax",@progbits
    .align          2
    .global         csky_rms_q15
    .type           csky_rms_q15, @function
csky_rms_q15:
    push            lr
    movi            t0, 0
    movi            t1, 0               // sum
    lsri            a3, a1, 2           // blkCnt
    bez             a3, .L0
.L1:
    pldbi.d         t4, (a0)
    mulaca.s16.e    t0, t4, t4
.L2:
    mulaca.s16.e    t0, t5, t5

    bloop           a3, .L1, .L2
.L0:

    andi            a3, a1, 3
    bez             a3, .L3
.L4:
    ldbi.h          t4, (a0)
.L5:
    mulaca.s16.e    t0, t4, t4

    bloop           a3, .L4, .L5

.L3:
    mov             a0, a1
    movi            a1, 0
    divsl           a0, t0, a0
    dexti           a0, a0, a1, 15
    clipi.s32       a0, a0, 16
    mov             a1, a2
    jbsr            csky_sqrt_q15
    pop             lr
    .size           csky_rms_q15, .-csky_rms_q15
