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
 * @file     csky_abs_max_q15_yv.S
 * @brief    Compute q15 vector abs max value.
 * @version  V1.0
 * @date     14. Nov 2017
 ******************************************************************************/

/**
 *
 * q15_t csky_dsp_lib_vec_max_abs16(
 * q15_t  * A,
 * uint32_t N)
 *
**/

    .file           "csky_abs_max_q15_yv.S"
    .section        .text.csky_dsp_lib_vec_max_abs16,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_vec_max_abs16
    .type           csky_dsp_lib_vec_max_abs16, @function

csky_dsp_lib_vec_max_abs16:
    lsri            a3, a1, 2           // blkCnt = N >> 2u
    andi            a1, a1, 3           // blkCnt = N % 0x4u
    movi            t5, 0
    bez             a3, .L2


.L4:
    pldbi.d         t2, (a0)            // load 4 q15_t data
    pabs.s16.s      t2, t2              // compute the abs value
    pmax.u16        t5, t5, t2
    pabs.s16.s      t3, t3
.L5:
    pmax.u16        t5, t5, t3

    bloop           a3, .L4, .L5        // while loop

.L2:
    bez             a1, .L0             // the condition to end

.L3:
    ldbi.h          t2, (a0)            // load 1 q15_t data
    pabs.s16.s      t2, t2              // compute the abs value
.L1:
    pmax.u16        t5, t5, t2

    bloop           a1, .L3, .L1        // while loop

.L0:
    dup.16          t2, t5, 0
    dup.16          t3, t5, 1
    pmax.u16        t5, t2, t3
    lsri            a0, t5, 16
    rts                                 // end
    .size           csky_dsp_lib_vec_max_abs16, .-csky_dsp_lib_vec_max_abs16
