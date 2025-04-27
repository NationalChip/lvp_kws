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
 * @file     csky_mean_q7.S
 * @brief    Mean value of a Q7 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_mean_q7(
 *   q7_t * pSrc,
 *   uint32_t blockSize,
 *   q7_t * pResult)
 */

    .file           "csky_mean_q7.S"
    .section        .text.csky_mean_q7,"ax",@progbits
    .align          2
    .global         csky_mean_q7
    .type           csky_mean_q7, @function
csky_mean_q7:
    movi            t0, 0               // sum
    movi            t1, 1
    dup.16          t1, t1, 0           // 0x00010001
    lsri            a3, a1, 3           // blkCnt
    bez             a3, .L0
.L1:
    pldbi.d         t2, (a0)
    pext.s8.e       t4, t2
    pext.s8.e       t6, t3
    mulaca.s16.s    t0, t4, t1
    mulaca.s16.s    t0, t5, t1
    mulaca.s16.s    t0, t6, t1
.L2:
    mulaca.s16.s    t0, t7, t1

    bloop           a3, .L1, .L2
.L0:

    andi            a3, a1, 7           // (blockSize - 1u) % 8
    bez             a3, .L3
.L4:
    ldbi.bs         t4, (a0)
.L5:
    mulall.s16.s    t0, t1, t4

    bloop           a3, .L4, .L5

.L3:
    divs            t0, a1
    st.b            t0, (a2, 0x0)
    rts
    .size           csky_mean_q7, .-csky_mean_q7
