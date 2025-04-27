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
 * @file     csky_biquad_cascade_df1_32x64_q31.S
 * @brief    High precision Q31 Biquad cascade filter function.
 * @version  V1.0
 * @date     15. Aug 2017
 ******************************************************************************/

/*
 * void csky_biquad_cas_df1_32x64_q31(
 *   csky_biquad_cas_df1_32x64_ins_q31 *S,
 *   q31_t * pSrc,
 *   q31_t * pDst,
 *   uint32_t blockSize)
 */

    .file           "csky_biquad_cas_df1_32x64_q31.S"
    .section        .text.csky_biquad_cas_df1_32x64_q31,"ax",@progbits
    .align          2
    .global         csky_biquad_cas_df1_32x64_q31
    .type           csky_biquad_cas_df1_32x64_q31, @function

csky_biquad_cas_df1_32x64_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    subi            sp, sp, 8
    st.w            a2, (sp, 0x4)
    ld.w            t0, (a0, 0x0)       //load the numStage
    ld.w            l9, (a0, 0x4)       //load *pState
    ld.w            t1, (a0, 0x8)       //load *pCoeff
    ld.w            l1, (a0, 0xc)       //load shift
    addi            l1, 1
    st.w            l9, (sp, 0x0)

.L2:
    lsri            t9, a3, 2
    bez             t9, .L1
    pldbi.d         t2, (t1)            //load the coefficients
    pldbi.d         t4, (t1)
    ldbi.w          t6, (t1)

    ld.w            l9, (sp, 0x0)
    addi            l9, l9, 4
    ldbi.w          l2, (l9)            //load the state
    addi            l9, l9, 4
    ldbi.w          l3, (l9)
    pldbi.d         l4, (l9)
    pldbi.d         l6, (l9)
    subi            l9, l9, 32
    st.w            l9, (sp, 0x0)

.L4:
    ldbi.w          l0, (a1)            //load the input
    mul.s32         l8, l0, t2          //acc  = b0 * x[n]
    mula.s32        l8, l2, t3          //acc += b1 * x[n-1]
    mula.s32        l8, l3, t4          //acc += b2 * x[n-2]
    blz             t5, .L9
    mul.u32         t7, l5, t5          //a1 * y[n-1]
    mov             t7, t8
    br              .L10
.L9:
    abs             t8, t5
    mul.u32         t7, l5, t8
    mov             t7, t8
    neg.s32.s       t7, t7
.L10:
    asri            t8, t7, 31
    mula.s32        t7, l4, t5
    add.s64.s       l8, l8, t7          //acc += a1 * y[n-1]
    blz             t6, .L11
    mul.u32         t7, l7, t6
    mov             t7, t8
    br              .L12
.L11:
    abs             t8, t6
    mul.u32         t7, l7, t8          //a2 * y[n-2]
    mov             t7, t8
    neg.s32.s       t7, t7
.L12:
    asri            t8, t7, 31
    mula.s32        t7, l6, t6
    add.s64.s       l8, l8, t7          //acc += a2 * y[n-2]
    movi            t7, 32
    sub             t8, t7, l1
    dext            l6, l8, l9, t8
    lsl             l7, l8, l1          //acc << shift
    lsr             l8, l8, t8          //acc_l >> lShift
    lsl             l9, l9, l1          //acc_h << uShift
    or              l9, l8, l9          //acc_h = acc_l | acc_h
    stbi.w          l9, (a2)            //store the result

    ldbi.w          l3, (a1)            //another input
    mul.s32         l8, l3, t2
    mula.s32        l8, l0, t3
    mula.s32        l8, l2, t4
    blz             t5, .L13
    mul.u32         t7, l7, t5
    mov             t7, t8
    br              .L14
.L13:
    abs             t8, t5
    mul.u32         t7, l7, t8
    mov             t7, t8
    neg.s32.s       t7, t7
.L14:
    asri            t8, t7, 31
    mula.s32        t7, l6, t5
    add.s64.s       l8, l8, t7
    blz             t6, .L15
    mul.u32         t7, l5, t6
    mov             t7, t8
    br              .L16
.L15:
    abs             t8, t6
    mul.u32         t7, l5, t8
    mov             t7, t8
    neg.s32.s       t7, t7
.L16:
    asri            t8, t7, 31
    mula.s32        t7, l4, t6
    add.s64.s       l8, l8, t7
    movi            t7, 32
    sub             t8, t7, l1
    dext            l4, l8, l9, t8
    lsl             l5, l8, l1
    lsr             l8, l8, t8
    lsl             l9, l9, l1
    or              l9, l8, l9
    stbi.w          l9, (a2)
    
    ldbi.w          l2, (a1)
    mul.s32         l8, l2, t2
    mula.s32        l8, l3, t3
    mula.s32        l8, l0, t4
    blz             t5, .L17
    mul.u32         t7, l5, t5
    mov             t7, t8
    br              .L18
.L17:
    abs             t8, t5
    mul.u32         t7, l5, t8
    mov             t7, t8
    neg.s32.s       t7, t7
.L18:
    asri            t8, t7, 31
    mula.s32        t7, l4, t5
    add.s64.s       l8, l8, t7
    blz             t6, .L19
    mul.u32         t7, l7, t6
    mov             t7, t8
    br              .L20
.L19:
    abs             t8, t6
    mul.u32         t7, l7, t8
    mov             t7, t8
    neg.s32.s       t7, t7
.L20:
    asri            t8, t7, 31
    mula.s32        t7, l6, t6
    add.s64.s       l8, l8, t7
    movi            t7, 32
    sub             t8, t7, l1
    dext            l6, l8, l9, t8
    lsl             l7, l8, l1
    lsr             l8, l8, t8
    lsl             l9, l9, l1
    or              l9, l8, l9
    stbi.w          l9, (a2)

    ldbi.w          l0, (a1)
    mul.s32         l8, l0, t2
    mula.s32        l8, l2, t3
    mula.s32        l8, l3, t4
    blz             t5, .L21
    mul.u32         t7, l7, t5
    mov             t7, t8
    br              .L22
.L21:
    abs             t8, t5
    mul.u32         t7, l7, t8
    mov             t7, t8
    neg.s32.s       t7, t7
.L22:
    asri            t8, t7, 31
    mula.s32        t7, l6, t5
    add.s64.s       l8, l8, t7
    blz             t6, .L23
    mul.u32         t7, l5, t6
    mov             t7, t8
    br              .L24
.L23:
    abs             t8, t6
    mul.u32         t7, l5, t8
    mov             t7, t8
    neg.s32.s       t7, t7
.L24:
    asri            t8, t7, 31
    mula.s32        t7, l4, t6
    add.s64.s       l8, l8, t7
    movi            t7, 32
    sub             t8, t7, l1
    dext            l4, l8, l9, t8
    lsl             l5, l8, l1
    lsr             l8, l8, t8
    lsl             l9, l9, l1
    or              l9, l8, l9
    stbi.w          l9, (a2)

    mov             l3, l2

.L5:
    mov             l2, l0
    bloop           t9, .L4, .L5        //while loop

.L1:
    andi            t9, a3, 3
    bez             t9, .L3             //one stage end

.L6:
    ldbi.w          l0, (a1)            //same as before
    mul.s32         l8, l0, t2
    mula.s32        l8, l2, t3
    mula.s32        l8, l3, t4
    blz             t5, .L25
    mul.u32         t7, l5, t5
    mov             t7, t8
    br              .L26
.L25:
    abs             t8, t5
    mul.u32         t7, l5, t8
    mov             t7, t8
    neg.s32.s       t7, t7
.L26:
    asri            t8, t7, 31
    mula.s32        t7, l4, t5
    add.s64.s       l8, l8, t7
    blz             t6, .L27
    mul.u32         t7, l7, t6
    mov             t7, t8
    br              .L28
.L27:
    abs             t8, t6
    mul.u32         t7, l7, t8
    mov             t7, t8
    neg.s32.s       t7, t7
.L28:
    asri            t8, t7, 31
    mula.s32        t7, l6, t6
    add.s64.s       l8, l8, t7
    movi            t7, 32
    sub             t8, t7, l1
    dext            l6, l8, l9, t8
    lsl             l7, l8, l1
    lsr             l8, l8, t8
    lsl             l9, l9, l1
    or              l9, l8, l9
    stbi.w          l9, (a2)
    
    mov             t7, l4
    mov             t8, l5
    mov             l4, l6
    mov             l5, l7
    mov             l6, t7
    mov             l7, t8
    mov             l2, l3

.L7:
    mov             l3, l0
    bloop           t9, .L6, .L7        //while loop

.L3:
    ld.w            a1, (sp, 0x4)       //pIn  = pDst
    ld.w            a2, (sp, 0x4)       //Pout = pDst
    movi            t7, 0               //update the state
    ld.w            l9, (sp, 0x0)
    stbi.w          t7, (l9)
    stbi.w          l2, (l9)
    stbi.w          t7, (l9)
    stbi.w          l3, (l9)
    stbi.w          l4, (l9)
    stbi.w          l5, (l9)
    stbi.w          l6, (l9)
    stbi.w          l7, (l9)
    st.w            l9, (sp, 0x0)

.L8:
    subi            t0, t0, 1           //stage--
    bhz             t0, .L2             //while loop
    addi            sp, sp, 8
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_biquad_cas_df1_32x64_q31, .-csky_biquad_cas_df1_32x64_q31 
