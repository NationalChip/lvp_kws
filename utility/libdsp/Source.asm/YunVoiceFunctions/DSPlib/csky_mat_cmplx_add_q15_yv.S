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
 * @file     csky_mat_cmplx_add_q15_yv.S
 * @brief    Q15 matrix complex addition.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/**
 * void csky_dsp_lib_mat_cx_add16(
 * cq15_t * A,
 * cq15_t * B,
 * uint32 N,
 * uint32 M,
 * cq15_t * C)
 *
**/
    .file           "csky_mat_cmplx_add_q15_yv.S"
    .section        .text.csky_dsp_lib_mat_cx_add16,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_mat_cx_add16
    .type           csky_dsp_lib_mat_cx_add16, @function

csky_dsp_lib_mat_cx_add16:
    mult            t0, a2, a3          // numSamples

    ld.w            a2, (sp, 0x0)       // C

    lsri            t1, t0, 2           // blkCnt
    bez             t1, .L0
.L1:
    pldbi.d         t2, (a0)            // inA1, inA2
    pldbi.d         t4, (a1)            // inB1, inB2
    padd.s16.s      t2, t2, t4
    padd.s16.s      t3, t3, t5
    st.w            t2, (a2, 0x0)
    st.w            t3, (a2, 0x4)

    pldbi.d         t2, (a0)            // inA3, inA4
    pldbi.d         t4, (a1)            // inB3, inB4
    padd.s16.s      t2, t2, t4
    padd.s16.s      t3, t3, t5
    st.w            t2, (a2, 0x8)
    st.w            t3, (a2, 0xc)
.L2:
    addi            a2, a2, 16

    bloop           t1, .L1, .L2
.L0:

    andi            t1, t0, 3
    bez             t1, .L3
.L4:
    ldbi.w          t2, (a0)
    ldbi.w          t3, (a1)
    padd.s16.s      t2, t2, t3
.L5:
    stbi.w          t2, (a2)

    bloop           t1, .L4, .L5
.L3:
    rts
    .size           csky_dsp_lib_mat_cx_add16, .-csky_dsp_lib_mat_cx_add16
