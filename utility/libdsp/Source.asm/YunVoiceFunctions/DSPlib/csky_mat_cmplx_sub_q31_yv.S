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
 * @file     csky_mat_cmplx_sub_q31_yv.S
 * @brief    Q31 matrix complex subtraction.
 * @version  V1.0
 * @date     14. Nov 2017
 ******************************************************************************/

/**
 * void csky_dsp_lib_mat_cx_sub32(
 * cq31_t * A,
 * cq31_t * B,
 * uint32 N,
 * uint32 M,
 * cq31_t * C)
 *
**/
    .file           "csky_mat_cmplx_add_q31_yv.S"
    .section        .text.csky_dsp_lib_mat_cx_sub32,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_mat_cx_sub32
    .type           csky_dsp_lib_mat_cx_sub32, @function

csky_dsp_lib_mat_cx_sub32:
    mult            t0, a2, a3          // numSamples

    ld.w            a2, (sp, 0x0)       // C

    lsri            t1, t0, 2           // blkCnt
    bez             t1, .L0
.L1:
    pldbi.d         t2, (a0)            // inA1
    pldbi.d         t4, (a1)            // inB1
    sub.s32.s       t2, t2, t4
    sub.s32.s       t3, t3, t5
    st.w            t2, (a2, 0x0)
    st.w            t3, (a2, 0x4)

    pldbi.d         t2, (a0)            // inA2
    pldbi.d         t4, (a1)            // inB2
    sub.s32.s       t2, t2, t4
    sub.s32.s       t3, t3, t5
    st.w            t2, (a2, 0x8)
    st.w            t3, (a2, 0xc)

    pldbi.d         t2, (a0)            // inA3
    pldbi.d         t4, (a1)            // inB3
    sub.s32.s       t2, t2, t4
    sub.s32.s       t3, t3, t5
    st.w            t2, (a2, 0x10)
    st.w            t3, (a2, 0x14)

    pldbi.d         t2, (a0)            // inA4
    pldbi.d         t4, (a1)            // inB4
    sub.s32.s       t2, t2, t4
    sub.s32.s       t3, t3, t5
    st.w            t2, (a2, 0x18)
    st.w            t3, (a2, 0x1c)
.L2:
    addi            a2, a2, 32

    bloop           t1, .L1, .L2
.L0:

    andi            t1, t0, 3
    bez             t1, .L3
.L4:
    pldbi.d         t2, (a0)
    pldbi.d         t4, (a1)
    sub.s32.s       t2, t2, t4
    sub.s32.s       t3, t3, t5
    st.w            t2, (a2, 0x0)
    st.w            t3, (a2, 0x4)
.L5:
    addi            a2, a2, 8

    bloop           t1, .L4, .L5
.L3:
    rts
    .size           csky_dsp_lib_mat_cx_sub32, .-csky_dsp_lib_mat_cx_sub32
