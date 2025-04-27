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
 * @file     csky_cmplx_conj_q31_yv.S
 * @brief    Q31 complex conjugate.
 * @version  V1.0
 * @date     13. Nov 2017
 ******************************************************************************/

/**
 * void csky_dsp_lib_vec_cx_conj_q31(
 * q31_t * A,
 * uint32_t N,
 * q31_t * C)
 *
* */

    .file           "csky_cmplx_conj_q31_yv.S"
    .section        .text.csky_dsp_lib_vec_cx_conj_q31,"ax",@progbits
    .align          2
    .global         csky_dsp_lib_vec_cx_conj_q31
    .type           csky_dsp_lib_vec_cx_conj_q31, @function

csky_dsp_lib_vec_cx_conj_q31:
    lsri            t0, a1, 2           // blkCnt
    bez             t0, .L0
.L1:
    pldbi.d         t2, (a0)            // inR1, inI1
    neg.s32.s       t3, t3              // inI1 = __QSUB(0, inI1)
    st.w            t2, (a2, 0x0)
    st.w            t3, (a2, 0x4)

    pldbi.d         t2, (a0)            // inR2, inI2
    neg.s32.s       t3, t3              // inI2 = __QSUB(0, inI2)
    st.w            t2, (a2, 0x8)
    st.w            t3, (a2, 0xc)

    pldbi.d         t2, (a0)            // inR3, inI3
    neg.s32.s       t3, t3              // inI3 = __QSUB(0, inI3)
    st.w            t2, (a2, 0x10)
    st.w            t3, (a2, 0x14)

    pldbi.d         t2, (a0)            // inR4, inI4
    neg.s32.s       t3, t3              // inI4 = __QSUB(0, inI4)
    st.w            t2, (a2, 0x18)
    st.w            t3, (a2, 0x1c)
.L2:
    addi            a2, a2, 32

    bloop           t0, .L1, .L2
.L0:

    andi            t0, a1, 3
    bez             t0, .L3
.L4:
    pldbi.d         t2, (a0)            // inR1, inI1
    neg.s32.s       t3, t3              // inI1 = __QSUB(0, inI1)
    st.w            t2, (a2, 0x0)
    st.w            t3, (a2, 0x4)
.L5:
    addi            a2, a2, 8

    bloop           t0, .L4, .L5
.L3:

    rts
    .size           csky_dsp_lib_vec_cx_conj_q31, .-csky_dsp_lib_vec_cx_conj_q31
