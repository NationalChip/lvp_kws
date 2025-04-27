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
 * @file     csky_power_q15.S
 * @brief    Sum of the squares of the elements of a Q15 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_power_q15(
 *   q15_t * pSrc,
 *   uint32_t blockSize,
 *   q63_t * pResult)
 */

    .file           "csky_power_q15.S"
    .section        .text.csky_power_q15,"ax",@progbits
    .align          2
    .global         csky_power_q15
    .type           csky_power_q15, @function
csky_power_q15:
    movi            t0, 0
    movi            t1, 0               // sum
    lsri            a3, a1, 2           // blkCnt
    bez             a3, .L0
.L1:
    pldbi.d         t2, (a0)
    mulaca.s16.e    t0, t2, t2
.L2:
    mulaca.s16.e    t0, t3, t3

    bloop           a3, .L1, .L2
.L0:

    andi            a1, a1, 3
    bez             a1, .L3
.L4:
    ldbi.h          t2, (a0)
.L5:
    mulaca.s16.e    t0, t2, t2

    bloop           a1, .L4, .L5

.L3:
    st.w            t0, (a2, 0x0)
    st.w            t1, (a2, 0x4)
    rts
    .size           csky_power_q15, .-csky_power_q15
