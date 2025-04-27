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
 * @file     csky_mean_q31.S
 * @brief    Mean value of a Q31 vector.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_mean_q31(
 *   q31_t * pSrc,
 *   uint32_t blockSize,
 *   q31_t * pResult)
 */

    .file           "csky_mean_q31.S"
    .section        .text.csky_mean_q31,"ax",@progbits
    .align          2
    .global         csky_mean_q31
    .type           csky_mean_q31, @function
csky_mean_q31:
    movi            t0, 0
    movi            t1, 0               // sum
    movi            t2, 1
    lsri            t3, a1, 2           // blkCnt
    bez             t3, .L0
.L1:
    pldbi.d         t4, (a0)
    pldbi.d         t6, (a0)
    mula.s32        t0, t2, t4
    mula.s32        t0, t2, t5
    mula.s32        t0, t2, t6
.L2:
    mula.s32        t0, t2, t7

    bloop           t3, .L1, .L2
.L0:

    andi            t3, a1, 3
    bez             t3, .L3
.L4:
    ldbi.w          t4, (a0)
.L5:
    mula.s32        t0, t2, t4

    bloop           t3, .L4, .L5

.L3:
    divsl           t0, t0, a1
    st.w            t0, (a2, 0x0)
    rts
    .size           csky_mean_q31, .-csky_mean_q31
