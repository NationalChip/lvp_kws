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
 * @file     csky_cmplx_mult_cmplx_re_q31.S
 * @brief    Q31 complex-by-complex multiplication.
 * @version  V1.0
 * @date     14. Aug 2017
 ******************************************************************************/

/*
 * void csky_cmplx_mult_cmplx_re_q31(
 *   q31_t * pSrcA,
 *   q31_t * pSrcB,
 *   q31_t * pDst
 *   uint32_t numSamples)
 */

    .file           "csky_cmplx_mult_cmplx_re_q31.S"
    .section        .text.csky_cmplx_mult_cmplx_re_q31,"ax",@progbits
    .align          2
    .global         csky_cmplx_mult_cmplx_re_q31
    .type           csky_cmplx_mult_cmplx_re_q31, @function

csky_cmplx_mult_cmplx_re_q31:
    lsri            t0, a3, 2           //blkCnt = blockSize >> 2u
    bez             t0, .L1
    movi            t1, 0
    subi            t1, t1, 4

.L2:
    pldbi.d         t2, (a0)            //load 1 q31_t data from source a
    pldbir.d        t4, (a1), t1        //load 1 q31_t data from source b
    neg.s32.s       t4, t4
    neg.s32.s       t5, t5
    mul.s32.h       t6, t2, t4          //the real part
    mul.s32.h       t7, t3, t5
    subh.s32        t6, t6, t7
    mul.s32.h       t8, t2, t5          //the image part
    mul.s32.h       t9, t3, t4
    addh.s32        t8, t8, t9
    stbi.w          t6, (a2)
    stbi.w          t8, (a2)

    pldbi.d         t2, (a0)            //load 1 q31_t data from source a
    pldbir.d        t4, (a1), t1        //load 1 q31_t data from source b
    neg.s32.s       t4, t4
    neg.s32.s       t5, t5
    mul.s32.h       t6, t2, t4          //the real part
    mul.s32.h       t7, t3, t5
    subh.s32        t6, t6, t7
    mul.s32.h       t8, t2, t5          //the image part
    mul.s32.h       t9, t3, t4
    addh.s32        t8, t8, t9
    stbi.w          t6, (a2)
    stbi.w          t8, (a2)

    pldbi.d         t2, (a0)            //load 1 q31_t data from source a
    pldbir.d        t4, (a1), t1        //load 1 q31_t data from source b
    neg.s32.s       t4, t4
    neg.s32.s       t5, t5
    mul.s32.h       t6, t2, t4          //the real part
    mul.s32.h       t7, t3, t5
    subh.s32        t6, t6, t7
    mul.s32.h       t8, t2, t5          //the image part
    mul.s32.h       t9, t3, t4
    addh.s32        t8, t8, t9
    stbi.w          t6, (a2)
    stbi.w          t8, (a2)

    pldbi.d         t2, (a0)            //load 1 q31_t data from source a
    pldbir.d        t4, (a1), t1        //load 1 q31_t data from source b
    neg.s32.s       t4, t4
    neg.s32.s       t5, t5
    mul.s32.h       t6, t2, t4          //the real part
    mul.s32.h       t7, t3, t5
    subh.s32        t6, t6, t7
    mul.s32.h       t8, t2, t5          //the image part
    mul.s32.h       t9, t3, t4
    addh.s32        t8, t8, t9
    stbi.w          t6, (a2)
.L3:
    stbi.w          t8, (a2)

    bloop           t0, .L2, .L3        //while loop

.L1:
    andi            t0, a3, 3           //blkCnt = blockSize % 0x4u
    bez             t0, .L0             //condition to end

.L4:
    pldbi.d         t2, (a0)            //same as before
    pldbir.d        t4, (a1), t1
    neg.s32.s       t4, t4
    neg.s32.s       t5, t5
    mul.s32.h       t6, t2, t4
    mul.s32.h       t7, t3, t5
    subh.s32        t6, t6, t7
    mul.s32.h       t8, t2, t5
    mul.s32.h       t9, t3, t4
    addh.s32        t8, t8, t9
    stbi.w          t6, (a2)
.L5:
    stbi.w          t8, (a2)

    bloop           t0, .L4, .L5        //while loop

.L0:
    rts                                 //end
    .size           csky_cmplx_mult_cmplx_re_q31, .-csky_cmplx_mult_cmplx_re_q31
