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
 * @file     csky_biquad_cascade_df1_fast_q31.S
 * @brief    Fast processing function for the 
 *           Q31 Biquad cascade filter.
 * @version  V1.0
 * @date     16. Aug 2017
 ******************************************************************************/

/*
 * void csky_biquad_cascade_df1_fast_q31(
 *   const csky_biquad_casd_df1_inst_q31 *S,
 *   q31_t * pSrc,
 *   q31_t * pDst,
 *   uint32_t blockSize)
 */

    .file           "csky_biquad_cascade_df1_fast_q31.S"
    .section        .text.csky_biquad_cascade_df1_fast_q31,"ax",@progbits
    .align          2
    .global         csky_biquad_cascade_df1_fast_q31
    .type           csky_biquad_cascade_df1_fast_q31, @function

csky_biquad_cascade_df1_fast_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    ld.w            t0, (a0, 0x0)       //load numstage
    ld.w            t1, (a0, 0x08)      //load *pCoeff
    ld.w            l9, (a0, 0x04)      //load *pState
    ld.bs           l0, (a0, 0x0c)      //load shift
    addi            l0, l0, 1
    mov             l6, a1
    mov             l7, a2

.L2:
    lsri            t9, a3, 2           //sample = blockSize >> 2u
    pldbi.d         t2, (t1)            //load the coefficients
    pldbi.d         t4, (t1)
    ldbi.w          t6, (t1)
    pldbi.d         l2, (l9)            //load state in
    pldbi.d         l4, (l9)            //load state out
    subi            l9, l9, 16
    bez             t9, .L1

.L4:
    pldbi.d         t7, (l6)            //load 2 inputs
    mul.s32.rh      l1, t7, t2          //acc  = b0 * x[n]
    mul.s32.rh      l8, t8, t2
    mula.s32.rhs    l1, l2, t3          //acc += b1 * x[n-1]
    mula.s32.rhs    l8, t7, t3
    mula.s32.rhs    l1, l3, t4          //acc += b2 * x[n-2]
    mula.s32.rhs    l8, l2, t4
    mula.s32.rhs    l1, l4, t5          //acc += a1 * y[n-1]
    mula.s32.rhs    l8, l4, t6
    mula.s32.rhs    l1, l5, t6          //acc += a2 * y[n-2]
    lsl             l1, l1, l0          //acc << shift
    stbi.w          l1, (l7)            //store the result
    mula.s32.rhs    l8, l1, t5
    lsl             l8, l8, l0
    stbi.w          l8, (l7)

    pldbi.d         l2, (l6)            //repeat
    mul.s32.rh      l5, l2, t2
    mul.s32.rh      l4, l3, t2
    mula.s32.rhs    l5, t8, t3
    mula.s32.rhs    l4, l2, t3
    mula.s32.rhs    l5, t7, t4
    mula.s32.rhs    l4, t8, t4
    mula.s32.rhs    l5, l8, t5
    mula.s32.rhs    l4, l8, t6
    mula.s32.rhs    l5, l1, t6
    lsl             l5, l5, l0
    stbi.w          l5, (l7)
    mula.s32.rhs    l4, l5, t5
    lsl             l4, l4, l0
    stbi.w          l4, (l7)

    mov             l1, l2
    mov             l2, l3

.L5:
    mov             l3, l1

    bloop           t9, .L4, .L5        //while loop

.L1:
    andi            t9, a3, 3           //sample = blocksize % 0x4u
    bez             t9, .L3             //condition to end aa stage

.L7:
    ldbi.w          t7, (l6)            //same as before
    mul.s32.rh      l1, t7, t2
    mula.s32.rhs    l1, l2, t3
    mula.s32.rhs    l1, l3, t4
    mula.s32.rhs    l1, l4, t5
    mula.s32.rhs    l1, l5, t6
    lsl             l1, l1, l0
    stbi.w          l1, (l7)

    mov             l3, l2
    mov             l2, t7
    mov             l5, l4

.L8:
    mov             l4, l1

    bloop           t9, .L7, .L8        //while loop

.L3:
    mov             l6, a2              //pIn  = pDst
    mov             l7, a2              //pOut = pDst
    stbi.w          l2, (l9)            //store the state
    stbi.w          l3, (l9)
    stbi.w          l4, (l9)
    stbi.w          l5, (l9)

.L9:
    subi            t0, t0, 1           //numstage--
    bhz             t0, .L2             //while loop
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size   csky_biquad_cascade_df1_fast_q31, .-csky_biquad_cascade_df1_fast_q31
