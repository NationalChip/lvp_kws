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
 * @file     csky_cmplx_conj_q31.S
 * @brief    Q31 complex conjugate.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_cmplx_conj_q31(
 *   q31_t * pSrc,
 *   q31_t * pDst,
 *   uint32_t numSamples)
 */

    .file           "csky_cmplx_conj_q31.S"
    .section        .text.csky_cmplx_conj_q31,"ax",@progbits
    .align          2
    .global         csky_cmplx_conj_q31
    .type           csky_cmplx_conj_q31, @function
csky_cmplx_conj_q31:
    lsri            t0, a2, 2           // blkCnt
    bez             t0, .L0
.L1:
    pldbi.d         t2, (a0)            // inR1, inI1
    neg.s32.s       t3, t3              // inI1 = __QSUB(0, inI1)
    stbi.w          t2, (a1)
    stbi.w          t3, (a1)

    pldbi.d         t2, (a0)            // inR2, inI2
    neg.s32.s       t3, t3              // inI2 = __QSUB(0, inI2)
    stbi.w          t2, (a1)
    stbi.w          t3, (a1)

    pldbi.d         t2, (a0)            // inR3, inI3
    neg.s32.s       t3, t3              // inI3 = __QSUB(0, inI3)
    stbi.w          t2, (a1)
    stbi.w          t3, (a1)

    pldbi.d         t2, (a0)            // inR4, inI4
    neg.s32.s       t3, t3              // inI4 = __QSUB(0, inI4)
    stbi.w          t2, (a1)
.L2:
    stbi.w          t3, (a1)

    bloop           t0, .L1, .L2
.L0:

    andi            t0, a2, 3
    bez             t0, .L3
.L4:
    pldbi.d         t2, (a0)            // inR1, inI1
    neg.s32.s       t3, t3              // inI1 = __QSUB(0, inI1)
    stbi.w          t2, (a1)
.L5:
    stbi.w          t3, (a1)

    bloop           t0, .L4, .L5
.L3:

    rts
    .size           csky_cmplx_conj_q31, .-csky_cmplx_conj_q31
