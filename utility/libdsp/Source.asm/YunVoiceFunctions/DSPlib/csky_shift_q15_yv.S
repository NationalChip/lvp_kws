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
 * @file     csky_shift_q15_yv.S
 * @brief    Shifts the elements of a Q15 vector by a specified number of bits.
 * @version  V1.0
 * @date     13. Nov 2017
 ******************************************************************************/

/**
 * void csky_dsp_lib_vec_shf16(
 * q15_t * A,
 * int8_t shift_val,
 * uint32_t N,
 * q15_t * C)
 *
**/

    .file           "csky_shift_q15_yv.S"
    .section        .text.csky_dsp_lib_vec_shr16,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_vec_shf16
    .type           csky_dsp_lib_vec_shf16, @function

csky_dsp_lib_vec_shf16:
    lsri            t0, a2, 2           // blkCnt   = N >> 2u
    andi            t1, a2, 3           // blkCnt   = N % 0x4u
    blz             a1, .L2             // if(sign == 0u)

.L1:                                    // if branch
    bez             t0, .L5             // while(blkCnt > 0u)

.L3:
    pldbi.d         t4, (a0)            // load 4 q15_t data from source
    plsl.s16.s      t6, t4, a1          // shift to left
    plsl.s16.s      t7, t5, a1
    st.w            t6, (a3, 0x0)
    st.w            t7, (a3, 0x4)
.L4:
    addi            a3, a3, 8

    bloop           t0, .L3, .L4

.L5:
    bez             t1, .L0             // while(blkCnt > 0)

.L6:
    ldbi.hs         t4, (a0)            // load one q15_t data
    plsl.s16.s      t6, t4, a1
.L7:
    stbi.h          t6, (a3)

    bloop           t1, .L6, .L7
    br              .L0                 // end

.L2:
    neg.s32.s       a1, a1              // shiftBits = -shiftBits
    bez             t0, .L10

.L8:                                    // else branch
    pldbi.d         t4, (a0)            // load four data
    pasr.s16        t6, t4, a1          // shift to right
    pasr.s16        t7, t5, a1
    st.w            t6, (a3, 0x0)
    st.w            t7, (a3, 0x4)
.L9:
    addi            a3, a3, 8

    bloop           t0, .L8, .L9

.L10:
    bez             t1, .L0             // while(blkCnt--)

.L11:
    ldbi.hs         t4, (a0)            // load one data
    pasr.s16        t6, t4, a1
.L12:
    stbi.h          t6, (a3)

    bloop           t1, .L11, .L12

.L0:
    rts                                 //end
    .size           csky_dsp_lib_vec_shf16, .-csky_dsp_lib_vec_shf16
