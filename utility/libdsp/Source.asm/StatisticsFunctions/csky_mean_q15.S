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
 * @file     csky_mean_q15.S
 * @brief    Mean value of a Q15 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_mean_q15(
 *   q15_t * pSrc,
 *   uint32_t blockSize,
 *   q15_t * pResult)
 */

    .file           "csky_mean_q15.S"
    .section        .text.csky_mean_q15,"ax",@progbits
    .align          2
    .global         csky_mean_q15
    .type           csky_mean_q15, @function
csky_mean_q15:
    movi            t0, 0               // sum
    movi            t2, 1
    dup.16          t2, t2, 0           // 0x00010001
    lsri            t3, a1, 2           // blkCnt
    bez             t3, .L0
.L1:
    pldbi.d         t4, (a0)
    mulaca.s16.s    t0, t2, t4
.L2:
    mulaca.s16.s    t0, t2, t5

    bloop           t3, .L1, .L2
.L0:

    andi            t3, a1, 3
    bez             t3, .L3
.L4:
    ldbi.h          t4, (a0)
.L5:
    mulaca.s16.s    t0, t2, t4

    bloop           t3, .L4, .L5

.L3:
    divs            t0, a1
    st.h            t0, (a2, 0x0)
    rts
    .size           csky_mean_q15, .-csky_mean_q15
