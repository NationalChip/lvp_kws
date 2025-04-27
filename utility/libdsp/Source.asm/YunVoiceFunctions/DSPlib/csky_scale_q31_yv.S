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
 * @file     csky_scale_q31_yv.S
 * @brief    Multiplies a Q31 vector by a scalar.
 * @version  V1.0
 * @date     13. Nov 2017
 ******************************************************************************/

/**
 * void csky_dsp_lib_vec_scale_q31(
 * q31_t * A,
 * q31_t scaleFract,
 * int8_t shift,
 * q31_t * B,
 * uint32_t N)
**/


    .file           "csky_scale_q31_yv.S"
    .section        .text.csky_dsp_lib_vec_scale_q31,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_vec_scale_q31
    .type           csky_dsp_lib_vec_scale_q31, @function

csky_dsp_lib_vec_scale_q31:
    ld.w            t0, (sp, 0x0)       // load N
    lsri            t1, t0, 2           // blkCnt = N >> 2u
    andi            t0, t0, 3           // blkCnt = N % 0x04
    movi            t2, 1
    add.s32.s       a2, a2, t2          // kShift = shift + 1
    blz             a2, .L2             // if (sign == 0u)

.L1:
    bez             t1, .L5

.L3:                                    // if branch
    pldbi.d         t4, (a0)            // load 2 q31_t data from source
    mul.s32.h       t6, t4, a1          // multiply the scale
    lsl.s32.s       t6, t6, a2          // shift to left
    stbi.w          t6, (a3)            // store the result

    mul.s32.h       t6, t5, a1          // another data
    lsl.s32.s       t6, t6, a2
    stbi.w          t6, (a3)

    pldbi.d         t4, (a0)            // load 2 q31_t data from source
    mul.s32.h       t6, t4, a1          // multiply the scale
    lsl.s32.s       t6, t6, a2          // shift to right
    stbi.w          t6, (a3)            // store the result

    mul.s32.h       t6, t5, a1          // another data
    lsl.s32.s       t6, t6, a2
.L4:
    stbi.w          t6, (a3)

    bloop           t1, .L3, .L4        // while loop

.L5:
    bez             t0, .L0

.L6:
    ldbi.w          t4, (a0)            // load one data
    mul.s32.h       t6, t4, a1
    lsl.s32.s       t6, t6, a2
.L7:
    stbi.w          t6, (a3)

    bloop           t0, .L6, .L7
    br              .L0                 // end

.L2:
    neg.s32.s       a2, a2
    bez             t1, .L10

.L8:
    pldbi.d         t4, (a0)            // else branch
    mul.s32.h       t6, t4, a1
    asr             t6, t6, a2          // shift to right
    stbi.w          t6, (a3)

    mul.s32.h       t6, t5, a1
    asr             t6, t6, a2
    stbi.w          t6, (a3)

    pldbi.d         t4, (a0)
    mul.s32.h       t6, t4, a1
    asr             t6, t6, a2
    stbi.w          t6, (a3)

    mul.s32.h       t6, t5, a1
    asr             t6, t6, a2
.L9:
    stbi.w          t6, (a3)

    bloop           t1, .L8, .L9        // while loop

.L10:
    ldbi.w          t4, (a0)            // load one data
    mul.s32.h       t6, t4, a1
    asr             t6, t6, a2
.L11:
    stbi.w          t6, (a3)

    bloop           t0, .L10, .L11

.L0:
    rts                                 // end
    .size           csky_dsp_lib_vec_scale_q31, .-csky_dsp_lib_vec_scale_q31
