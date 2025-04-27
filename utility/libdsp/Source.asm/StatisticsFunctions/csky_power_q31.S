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
 * @file     csky_power_q31.S
 * @brief    Sum of the squares of the elements of a Q31 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_power_q31(
 *   q31_t * pSrc,
 *   uint32_t blockSize,
 *   q63_t * pResult)
 */

    .file           "csky_power_q31.S"
    .section        .text.csky_power_q31,"ax",@progbits
    .align          2
    .global         csky_power_q31
    .type           csky_power_q31, @function
csky_power_q31:
    movi            t0, 0
    movi            t1, 0               // sum
    lsri            a3, a1, 2           // blkCnt
    bez             a3, .L0
.L1:
    pldbi.d         t4, (a0)
    pldbi.d         t6, (a0)
    mul.s32         t2, t4, t4
    dexti           t2, t2, t3, 14
    asri            t3, t3, 14
    add.64          t0, t0, t2          // sum += ((q63_t) in * in) >> 14u
    mul.s32         t2, t5, t5
    dexti           t2, t2, t3, 14
    asri            t3, t3, 14
    add.64          t0, t0, t2
    mul.s32         t2, t6, t6
    dexti           t2, t2, t3, 14
    asri            t3, t3, 14
    add.64          t0, t0, t2
    mul.s32         t2, t7, t7
    dexti           t2, t2, t3, 14
    asri            t3, t3, 14
.L2:
    add.64          t0, t0, t2

    bloop           a3, .L1, .L2
.L0:

    andi            a1, a1, 3
    bez             a1, .L3
.L4:
    ldbi.w          t4, (a0)
    mul.s32         t2, t4, t4
    dexti           t2, t2, t3, 14
    asri            t3, t3, 14
.L5:
    add.64          t0, t0, t2

    bloop           a1, .L4, .L5

.L3:
    st.w            t0, (a2, 0x0)
    st.w            t1, (a2, 0x4)
    rts
    .size           csky_power_q31, .-csky_power_q31
