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
 * @file     csky_biquad_cascade_df1_fast_q15.S
 * @brief    Fast processing function for the 
 *           Q15 Biquad cascade filter.
 * @version  V1.0
 * @date     15. Aug 2017
 ******************************************************************************/

/*
 * void csky_biquad_cascade_df1_fast_q15(
 *   const csky_biquad_casd_df1_inst_q15 *S,
 *   q15_t * pSrc,
 *   q15_t * pDst,
 *   uint32_t blockSize)
 */

    .file           "csky_biquad_cascade_df1_fast_q15.S"
    .section        .text.csky_biquad_cascade_df1_fast_q15,"ax",@progbits
    .align          2
    .global         csky_biquad_cascade_df1_fast_q15
    .type           csky_biquad_cascade_df1_fast_q15, @function

csky_biquad_cascade_df1_fast_q15:
    push            l0, l1, l2, l3, l4, l5
    ld.bs           t0, (a0, 0x0)       //load numstage
    ld.w            t1, (a0, 0x8)       //load *pCoeff
    ld.w            l5, (a0, 0x4)       //load *pState
    ld.bs           l0, (a0, 0xc)       //load shift
    movi            l1, 15
    sub.s32.s       l0, l1, l0
    mov             l3, a1              // *pIn  = pSrc
    mov             l4, a2              // *pOut = pDst

.L2:
    lsri            t2, a3, 2           //sample = blockSize >> 2u
    pldbi.d         t3, (t1)            //load the coefficients
    ldbi.w          t5, (t1)

    pldbi.d         t6, (l5)            //load state in and state out
    subi            l5, l5, 8
    bez             t2, .L1

.L4:
    ldbi.w          t8, (l3)            //load 2 Xn form source
    mulca.s16.s     t9, t3, t8          //acc  = b0 * x[n] + 0 * 0
    mulaca.s16.s    t9, t4, t6          //acc += b1 * x[n-1] + b2 * x[n-2]
    mulaca.s16.s    t9, t5, t7          //acc += a1 * y[n-1] + a2 * y[n-2]
    asr             t9, t9, l0          //acc >> shift
    clipi.s32       l1, t9, 16          //saturation
    pkgll           t6, t8, t6          //update the state
    pkgll           t7, l1, t7
    mulcax.s16.s    t9, t3, t8          //another input
    mulaca.s16.s    t9, t4, t6
    mulaca.s16.s    t9, t5, t7
    asr             t9, t9, l0
    clipi.s32       l1, t9, 16
    asri            l2, t8, 16
    pkgll           t6, l2, t6
    mov             l2, t7
    pkgll           t7, l1, l2
    pkgll           t9, l2, l1
    stbi.w          t9, (l4)            //store the result

    ldbi.w          t8, (l3)            //load 2 Xn form source
    mulca.s16.s     t9, t3, t8          //acc  = b0 * x[n] + 0 * 0
    mulaca.s16.s    t9, t4, t6          //acc += b1 * x[n-1] + b2 * x[n-2]
    mulaca.s16.s    t9, t5, t7          //acc += a1 * y[n-1] + a2 * y[n-2]
    asr             t9, t9, l0          //acc >> shift
    clipi.s32       l1, t9, 16          //saturation
    pkgll           t6, t8, t6          //update the state
    pkgll           t7, l1, t7
    mulcax.s16.s    t9, t3, t8          //another input
    mulaca.s16.s    t9, t4, t6
    mulaca.s16.s    t9, t5, t7
    asr             t9, t9, l0
    clipi.s32       l1, t9, 16
    asri            l2, t8, 16
    pkgll           t6, l2, t6
    mov             l2, t7
    pkgll           t7, l1, l2
    pkgll           t9, l2, l1

.L5:
    stbi.w          t9, (l4)            //store the result

    bloop           t2, .L4, .L5        //while loop

.L1:
    andi            t2, a3, 3           //sample = blockSize % 0x04u
    bez             t2, .L3             //condition to end one stage

.L7:
    ldbi.hs         t8, (l3)            //same as before
    mulca.s16.s     t9, t3, t8
    mulaca.s16.s    t9, t4, t6
    mulaca.s16.s    t9, t5, t7
    asr             t9, t9, l0
    clipi.s32       l1, t9, 16
    pkgll           t6, t8, t6
    pkgll           t7, l1, t7

.L8:
    stbi.h          l1, (l4)

    bloop           t4, .L7, .L8        //while loop

.L3:
    mov             l3, a2              //pIn  = pDst
    mov             l4, a2              //pOut = pDst
    stbi.w          t6, (l5)
    stbi.w          t7, (l5)

.L9:
    subi            t0, t0, 1           //numstage--
    bhz             t0, .L2             //while loop

    pop             l0, l1, l2, l3, l4, l5
    .size           csky_biquad_cascade_df1_fast_q15, .-csky_biquad_cascade_df1_fast_q15
