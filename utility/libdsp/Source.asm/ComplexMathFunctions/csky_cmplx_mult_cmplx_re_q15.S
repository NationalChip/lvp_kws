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
 * @file     csky_cmplx_mult_cmplx_re_q15.S
 * @brief    Q15 complex-by-complex multiplication.
 * @version  V1.0
 * @date     14. Aug 2017
 ******************************************************************************/

/*
 * void csky_cmplx_mult_cmplx_re_q15(
 *   q15_t * pSrcA,
 *   q15_t * pSrcB,
 *   q15_t * pDst
 *   uint32_t numSamples)
 */

    .file           "csky_cmplx_mult_cmplx_re_q15.S"
    .section        .text.csky_cmplx_mult_cmplx_re_q15,"ax",@progbits
    .align          2
    .global         csky_cmplx_mult_cmplx_re_q15
    .type           csky_cmplx_mult_cmplx_re_q15, @function

csky_cmplx_mult_cmplx_re_q15:
    lsri            t0, a3, 2           //blkCnt = blockSize >> 2u
    movi            t1, 0
    subi            t1, 4
    bez             t0, .L1

.L2:
    pldbi.d         t2, (a0)            //load 2 q15_t data from source a
    pldbir.d        t4, (a1), t1        //load 2 q15_t data from source b
    pneg.s16.s      t4, t4
    pneg.s16.s      t5, t5
    pmulx.s16       t6, t2, t4          //the real part
    asri            t6, t6, 17
    asri            t7, t7, 17
    sub.s32.s       t6, t6, t7
    pmul.s16        t8, t2, t4
    asri            t8, t8, 17          //prepare for store
    asri            t9, t9, 17
    addu            t8, t8, t9
    pkgll           t6, t6, t8
    stbi.w          t6, (a2)            //store the result
    pmulx.s16       t6, t3, t5          //the real part
    asri            t6, t6, 17
    asri            t7, t7, 17
    sub.s32.s       t6, t6, t7
    pmul.s16        t8, t3, t5
    asri            t8, t8, 17          //prepare for store
    asri            t9, t9, 17
    addu            t8, t8, t9
    pkgll           t6, t6, t8
    stbi.w          t6, (a2)            //store the result

    pldbi.d         t2, (a0)            //load 2 q15_t data from source a
    pldbir.d        t4, (a1), t1        //load 2 q15_t data from source b
    pneg.s16.s      t4, t4
    pneg.s16.s      t5, t5
    pmulx.s16       t6, t2, t4          //the real part
    asri            t6, t6, 17
    asri            t7, t7, 17
    sub.s32.s       t6, t6, t7
    pmul.s16        t8, t2, t4
    asri            t8, t8, 17          //prepare for store
    asri            t9, t9, 17
    addu            t8, t8, t9
    pkgll           t6, t6, t8
    stbi.w          t6, (a2)            //store the result
    pmulx.s16       t6, t3, t5          //the real part
    asri            t6, t6, 17
    asri            t7, t7, 17
    sub.s32.s       t6, t6, t7
    pmul.s16        t8, t3, t5
    asri            t8, t8, 17          //prepare for store
    asri            t9, t9, 17
    addu            t8, t8, t9
    pkgll           t6, t6, t8
.L3:
    stbi.w          t6, (a2)            //store the result

    bloop           t0, .L2, .L3        //while loop

.L1:
    andi            t0, a3, 3           //blkCnt = blockSize % 0x4u
    bez             t0, .L0             //condition to end

.L4:
    ldbi.w          t2, (a0)            //load 1 q15_t data from source a
    ldbir.w         t3, (a1), t1        //load 1 q15_t data from source b
    pneg.s16.s      t3, t3
    pmulx.s16       t6, t2, t3          //the real part
    asri            t6, t6, 17
    asri            t7, t7, 17
    sub.s32.s       t6, t6, t7
    pmul.s16        t8, t2, t3
    asri            t8, t8, 17          //prepare for store
    asri            t9, t9, 17
    addu            t8, t8, t9
    pkgll           t6, t6, t8
.L5:
    stbi.w          t6, (a2)            //store the result

    bloop           t0, .L4, .L5
.L0:
    rts                                 //end
    .size           csky_cmplx_mult_cmplx_re_q15, .-csky_cmplx_mult_cmplx_re_q15
