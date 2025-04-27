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
 * @file     csky_cfft_radix4_q31_yv.S
 * @brief    This file has function definition of Radix-4 FFT & IFFT function and
 *   		     In-place bit reversal using bit reversal table.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_radix4_butterfly_yv_q31(
 *   q31_t * pSrc,
 *   uint32_t fftLen,
 *   q31_t * pCoef,
 *   uint32_t twidCoefModifier)
 */
    .file           "csky_cfft_radix4_q31_yv.S"
    .section        .text.csky_radix4_butterfly_yv_q31,"ax",@progbits
    .align          2
    .global         csky_radix4_butterfly_yv_q31
    .type           csky_radix4_butterfly_yv_q31, @function

csky_radix4_butterfly_yv_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    subi            sp, sp, 0x14
    /*  Calculation of first stage */
    lsri            t9, a1, 2           // n2
    mov             t8, t9              // j
    movi            t0, 0               // i0
    lsli            t3, t9, 3
    mov             t0, a0
    addu            t1, t0, t3
    addu            t2, t1, t3
    addu            t3, t2, t3
    movi            l8, 0               // ia1
    lsli            a3, a3, 3           // twidCoefModifier

.L0:
    ld.w            l0, (t0, 0x0)       // pSrc[(2u * i0)]
    ld.w            l1, (t0, 0x4)       // pSrc[(2u * i0) + 1]
    ld.w            l2, (t1, 0x0)       // pSrc[(2u * i1)]
    ld.w            l3, (t1, 0x4)       // pSrc[(2u * i1) + 1]
    ld.w            l4, (t2, 0x0)       // pSrc[(2u * i2)]
    ld.w            l5, (t2, 0x4)       // pSrc[(2u * i2) + 1]
    ld.w            l6, (t3, 0x0)       // pSrc[(2u * i3)]
    ld.w            l7, (t3, 0x4)       // pSrc[(2u * i3) + 1]
    add.s32.s       t4, l0, l4          // r1
    sub.s32.s       t5, l0, l4          // r2
    add.s32.s       t6, l2, l6          // t1
    add.s32.s       t7, l3, l7          // t2
    add.s32.s       l0, l1, l5          // s1
    sub.s32.s       l1, l1, l5          // s2
    add.s32.s       l9, t4, t6
    stbi.w          l9, (t0)            // pSrc[2u * i0]
    sub.s32.s       t4, t4, t6          // r1 = r1 - t1
    add.s32.s       l9, l0, t7
    stbi.w          l9, (t0)            // pSrc[2u * i0 + 1]
    sub.s32.s       l0, l0, t7          // s1 = s1 - t2
    sub.s32.s       t6, l3, l7          // t1
    sub.s32.s       t7, l2, l6          // t2

    lsli            l9, l8, 1           // ia2 = 2 * ia1
    addu            l9, l9, a2
    ld.w            l2, (l9, 0x0)       // co2
    ld.w            l3, (l9, 0x4)       // si2

    mul.s32         l4, t4, l2
    mula.s32.s      l4, l0, l3
    dexti           l4, l4, l5, 31
    stbi.w          l4, (t1)            // pSrc[2u * i1]

    mul.s32         l4, l0, l2
    muls.s32.s      l4, t4, l3
    dexti           l4, l4, l5, 31
    stbi.w          l4, (t1)            // pSrc[2u * i1 + 1]

    add.s32.s       t4, t5, t6          // r1 = r2 + t1
    sub.s32.s       t5, t5, t6          // r2 = r2 - t1
    sub.s32.s       l0, l1, t7          // s1 = s2 - t2
    add.s32.s       l1, l1, t7          // s2 = s2 + t2

    addu            l9, l8, a2
    ld.w            l2, (l9, 0x0)       // co1
    ld.w            l3, (l9, 0x4)       // si1

    mul.s32         l4, t4, l2
    mula.s32.s      l4, l0, l3
    dexti           l4, l4, l5, 31
    stbi.w          l4, (t2)

    mul.s32         l4, l0, l2
    muls.s32.s      l4, t4, l3
    dexti           l4, l4, l5, 31
    stbi.w          l4, (t2)

    movi            l9, 3
    mult            l9, l8, l9
    addu            l9, l9, a2
    ld.w            l2, (l9, 0x0)       // co3
    ld.w            l3, (l9, 0x4)       // si3

    mul.s32         l4, t5, l2
    mula.s32.s      l4, l1, l3
    dexti           l4, l4, l5, 31
    stbi.w          l4, (t3)            // pSrc[2u * i1]

    mul.s32         l4, l1, l2
    muls.s32.s      l4, t5, l3
    dexti           l4, l5, l5, 31
    stbi.w          l4, (t3)            // pSrc[2u * i1 + 1]

.L1:
    addu            l8, l8, a3          // ia1 = ia1 + twidCoefModifier

    bloop           t8, .L0, .L1

    /* Start of Middle stages process */
    lsli            a3, a3, 2           // twidCoefModifier <<= 2
    st.w            a0, (sp, 0x0)
    st.w            a1, (sp, 0x4)
    st.w            a2, (sp, 0x8)
    st.w            a3, (sp, 0xc)

    asri            t8, a1, 2           // k = fftLen / 4
.L11:
    cmphsi          t8, 5               // k > 4
    bf              .L10
    mov             t7, t9              // n1 = n2
    asri            t9, t9, 2           // n2 >>= 2
    movi            l8, 0               // ia1 = 0
    st.w            l8, (sp, 0x10)

    movi            t6, 0               // j = 0
.L3:
    cmplt           t6, t9              // j <= n2 - 1
    bf              .L2
    ld.w            l8, (sp, 0x10)
    ld.w            a2, (sp, 0x8)
    addu            l9, l8, a2
    ld.w            l0, (l9, 0x0)       // co1
    ld.w            l1, (l9, 0x4)       // si1
    addu            l9, l9, l8
    ld.w            l2, (l9, 0x0)       // co2
    ld.w            l3, (l9, 0x4)       // si2
    addu            l9, l9, l8
    ld.w            l4, (l9, 0x0)       // co3
    ld.w            l5, (l9, 0x4)       // si3
    ld.w            a3, (sp, 0xc)
    addu            l8, l8, a3          // ia1 = ia1 + twidCoefModifier
    st.w            l8, (sp, 0x10)

    mov             t0, t6              // i0 = j
.L5:
    ld.w            a1, (sp, 0x4)
    cmplt           t0, a1              // i0 < fftLen
    bf              .L4
    lsli            t1, t0, 3           // i0 * 2
    lsli            t2, t9, 3           // n2
    ld.w            a0, (sp, 0x0)
    add.s32.s       t3, t1, a0          // pSrc[2 * i0]
    add.s32.s       t4, t3, t2          // pSrc[2 * i1]
    add.s32.s       t5, t4, t2          // pSrc[2 * i2]
    add.s32.s       t2, t5, t2          // pSrc[2 * i3]
    ld.w            l6, (t3, 0x0)
    ld.w            l7, (t5, 0x0)
    add.s32.s       l8, l6, l7          // r1
    sub.s32.s       l9, l6, l7          // r2
    ld.w            l6, (t4, 0x0)
    ld.w            l7, (t2, 0x0)
    add.s32.s       t1, l6, l7          // t1
    add.s32.s       a0, l8, t1          // r1 + t1
    st.w            a0, (t3, 0x0)
    sub.s32.s       l8, l8, t1          // r1 = r1 - t1
    sub.s32.s       a2, l6, l7          // t2 = pSrc[2u * i1] - pSrc[2u * i3]
    ld.w            l6, (t3, 0x4)
    ld.w            l7, (t5, 0x4)
    add.s32.s       a0, l6, l7          // s1
    sub.s32.s       a1, l6, l7          // s2
    ld.w            l6, (t4, 0x4)
    ld.w            l7, (t2, 0x4)
    add.s32.s       a3, l6, l7          // t2
    add.s32.s       t1, a3, a0          // s1 + t2
    st.w            t1, (t3, 0x4)
    sub.s32.s       a0, a0, a3          // s1 = s1 - t2
    sub.s32.s       t1, l6, l7          // t1

    mul.s32         l6, l8, l2
    mula.s32.s      l6, a0, l3
    dexti           l6, l6, l7, 31
    st.w            l6, (t4, 0x0)
    mul.s32         l6, a0, l2
    muls.s32.s      l6, l8, l3
    dexti           l6, l6, l7, 31
    st.w            l6, (t4, 0x4)

    add.s32.s       l8, l9, t1          // r1 = r2 + t1
    sub.s32.s       l9, l9, t1          // r2 = r2 - t1
    sub.s32.s       a0, a1, a2          // s1 = s2 - t2
    add.s32.s       a1, a1, a2          // s2 = s2 + t2

    mul.s32         l6, l8, l0
    mula.s32.s      l6, a0, l1
    dexti           l6, l6, l7, 31
    st.w            l6, (t5, 0x0)
    mul.s32         l6, a0, l0
    muls.s32.s      l6, l8, l1
    dexti           l6, l6, l7, 31
    st.w            l6, (t5, 0x4)

    mul.s32         l6, l9, l4
    mula.s32.s      l6, a1, l5
    dexti           l6, l6, l7, 31
    st.w            l6, (t2, 0x0)
    mul.s32         l6, a1, l4
    muls.s32.s      l6, l9, l5
    dexti           l6, l6, l7, 31
    st.w            l6, (t2, 0x4)

    addu            t0, t0, t7          // i0 += n1
    br              .L5
.L4:

    addi            t6, t6, 1           // j++
    br              .L3
.L2:
    ld.w            a3, (sp, 0xc)
    lsli            a3, a3, 2           // twidCoefModifier <<= 2
    st.w            a3, (sp, 0xc)
    asri            t8, t8, 2           // k >>= 2
    br              .L11
.L10:

    /* Start of last stage process */
    ld.w            a1, (sp, 0x4)       // fftLen
    ld.w            a0, (sp, 0x0)       // ptr1 = pSrc[0]
    lsri            a1, a1, 2           // j
    mov             a2, a0

.L12:
    pldbi.d         l0, (a0)            // xaya
    pldbi.d         l2, (a0)            // xbyb
    pldbi.d         l4, (a0)            // xcyc
    pldbi.d         l6, (a0)            // xdyd

    add.s32.s       t0, l0, l4          // xa + xc
    add.s32.s       t1, l2, l6          // xb + xd
    add.s32.s       t2, l1, l5          // ya + yc
    add.s32.s       t3, l3, l7          // yb + yd
    add.s32.s       t4, t0, t1          // xa_out = xa + xc + xb + xd
    sub.s32.s       t5, t0, t1          // xc_out = xa + xc - (xb + xd)
    add.s32.s       t6, t2, t3          // ya_out = ya + yc + yb + yd
    sub.s32.s       t7, t2, t3          // yc_out = ya + yc - (yb + yd)
    st.w            t4, (a2, 0x0)
    st.w            t6, (a2, 0x4)
    st.w            t5, (a2, 0x8)
    st.w            t7, (a2, 0xc)

    sub.s32.s       t0, l0, l4          // xa - xc
    sub.s32.s       t1, l2, l6          // xb - xd
    sub.s32.s       t2, l1, l5          // ya - yc
    sub.s32.s       t3, l3, l7          // yb - yd
    add.s32.s       t4, t0, t3          // xb_out = xa - xc + yb - yd
    sub.s32.s       t5, t0, t3          // xd_out = xa - xc - (yb - yd)
    sub.s32.s       t6, t2, t1          // yb_out = ya - yc - (xb - xd)
    add.s32.s       t7, t2, t1          // yd_out = ya - yc + xb - xd
    st.w            t4, (a2, 0x10)
    st.w            t6, (a2, 0x14)
    st.w            t5, (a2, 0x18)
    st.w            t7, (a2, 0x1c)
.L13:
    addi            a2, a2, 32

    bloop           a1, .L12, .L13

    addi            sp, sp, 0x14
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_radix4_butterfly_yv_q31, .-csky_radix4_butterfly_yv_q31

/*
 * void csky_radix4_butterfly_inverse_yv_q31(
 *   q31_t * pSrc,
 *   uint32_t fftLen,
 *   q31_t * pCoef,
 *   uint32_t twidCoefModifier)
 */
    .section        .text.csky_radix4_butterfly_inverse_yv_q31,"ax",@progbits
    .align          2
    .global         csky_radix4_butterfly_inverse_yv_q31
    .type           csky_radix4_butterfly_inverse_yv_q31, @function

csky_radix4_butterfly_inverse_yv_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    subi            sp, sp, 0x14
    /*  Calculation of first stage */
    lsri            t9, a1, 2           // n2
    mov             t8, t9              // j
    lsli            t3, t9, 3
    mov             t0, a0
    addu            t1, t0, t3
    addu            t2, t1, t3
    addu            t3, t2, t3
    movi            l8, 0               // ia1
    lsli            a3, a3, 3           // twidCoefModifier

.L20:
    ld.w            l0, (t0, 0x0)       // pSrc[(2u * i0)]
    ld.w            l1, (t0, 0x4)       // pSrc[(2u * i0) + 1]
    ld.w            l2, (t1, 0x0)       // pSrc[(2u * i1)]
    ld.w            l3, (t1, 0x4)       // pSrc[(2u * i1) + 1]
    ld.w            l4, (t2, 0x0)       // pSrc[(2u * i2)]
    ld.w            l5, (t2, 0x4)       // pSrc[(2u * i2) + 1]
    ld.w            l6, (t3, 0x0)       // pSrc[(2u * i3)]
    ld.w            l7, (t3, 0x4)       // pSrc[(2u * i3) + 1]
    addh.s32        t4, l0, l4          // r1
    subh.s32        t5, l0, l4          // r2
    addh.s32        t6, l2, l6          // t1
    addh.s32        t7, l3, l7          // t2
    addh.s32        l0, l1, l5          // s1
    subh.s32        l1, l1, l5          // s2
    addh.s32        l9, t4, t6
    stbi.w          l9, (t0)            // pSrc[2u * i0]
    subh.s32        t4, t4, t6          // r1 = r1 - t1
    addh.s32        l9, l0, t7
    stbi.w          l9, (t0)            // pSrc[2u * i0 + 1]
    subh.s32        l0, l0, t7          // s1 = s1 - t2
    subh.s32        t6, l3, l7          // t1
    subh.s32        t7, l2, l6          // t2

    lsli            l9, l8, 1           // ia2 = 2 * ia1
    addu            l9, l9, a2
    ld.w            l2, (l9, 0x0)       // co2
    ld.w            l3, (l9, 0x4)       // si2

    mul.s32.h       l4, t4, l2
    mul.s32.h       l5, l0, l3
    sub.s32.s       l4, l4, l5
    lsli            l4, l4, 1
    stbi.w          l4, (t1)            // pSrc[2u * i1]

    mul.s32.h       l4, l0, l2
    mul.s32.h       l5, t4, l3
    add.s32.s       l4, l4, l5
    lsli            l4, l4, 1
    stbi.w          l4, (t1)            // pSrc[2u * i1 + 1]

    subh.s32        t4, t5, t6          // r1 = r2 - t1
    addh.s32        t5, t5, t6          // r2 = r2 + t1
    addh.s32        l0, l1, t7          // s1 = s2 + t2
    subh.s32        l1, l1, t7          // s2 = s2 - t2


    addu            l9, l8, a2
    ld.w            l2, (l9, 0x0)       // co1
    ld.w            l3, (l9, 0x4)       // si1

    mul.s32.h       l4, t4, l2
    mul.s32.h       l5, l0, l3
    sub.s32.s       l4, l4, l5
    lsli            l4, l4, 1
    stbi.w          l4, (t2)

    mul.s32.h       l4, l0, l2
    mul.s32.h       l5, t4, l3
    add.s32.s       l4, l4, l5
    lsli            l4, l4, 1
    stbi.w          l4, (t2)

    movi            l9, 3
    mult            l9, l8, l9
    addu            l9, l9, a2
    ld.w            l2, (l9, 0x0)       // co3
    ld.w            l3, (l9, 0x4)       // si3

    mul.s32.h       l4, t5, l2
    mul.s32.h       l5, l1, l3
    sub.s32.s       l4, l4, l5
    lsli            l4, l4, 1
    stbi.w          l4, (t3)            // pSrc[2u * i1]

    mul.s32.h       l4, l1, l2
    mul.s32.h       l5, t5, l3
    add.s32.s       l4, l4, l5
    lsli            l4, l4, 1
    stbi.w          l4, (t3)            // pSrc[2u * i1 + 1]

.L21:
    addu            l8, l8, a3          // ia1 = ia1 + twidCoefModifier

    bloop           t8, .L20, .L21

    /* Start of Middle stages process */
    lsli            a3, a3, 2           // twidCoefModifier <<= 2
    st.w            a0, (sp, 0x0)
    st.w            a1, (sp, 0x4)
    st.w            a2, (sp, 0x8)
    st.w            a3, (sp, 0xc)

    asri            t8, a1, 2           // k = fftLen / 4
.L211:
    cmphsi          t8, 5               // k > 4
    bf              .L210
    mov             t7, t9              // n1 = n2
    asri            t9, t9, 2           // n2 >>= 2
    movi            l8, 0               // ia1 = 0
    st.w            l8, (sp, 0x10)

    movi            t6, 0               // j = 0
.L23:
    cmplt           t6, t9              // j <= n2 - 1
    bf              .L22
    ld.w            l8, (sp, 0x10)
    ld.w            a2, (sp, 0x8)
    addu            l9, l8, a2
    ld.w            l0, (l9, 0x0)       // co1
    ld.w            l1, (l9, 0x4)       // si1
    addu            l9, l9, l8
    ld.w            l2, (l9, 0x0)       // co2
    ld.w            l3, (l9, 0x4)       // si2
    addu            l9, l9, l8
    ld.w            l4, (l9, 0x0)       // co3
    ld.w            l5, (l9, 0x4)       // si3
    ld.w            a3, (sp, 0xc)
    addu            l8, l8, a3          // ia1 = ia1 + twidCoefModifier
    st.w            l8, (sp, 0x10)

    mov             t0, t6              // i0 = j
.L25:
    ld.w            a1, (sp, 0x4)
    cmplt           t0, a1              // i0 < fftLen
    bf              .L24
    lsli            t1, t0, 3           // i0 * 2
    lsli            t2, t9, 3           // n2
    ld.w            a0, (sp, 0x0)
    addu            t3, t1, a0          // pSrc[2 * i0]
    addu            t4, t3, t2          // pSrc[2 * i1]
    addu            t5, t4, t2          // pSrc[2 * i2]
    addu            t2, t5, t2          // pSrc[2 * i3]
    ld.w            l6, (t3, 0x0)
    ld.w            l7, (t5, 0x0)
    addh.s32        l8, l6, l7          // r1
    subh.s32        l9, l6, l7          // r2
    ld.w            l6, (t4, 0x0)
    ld.w            l7, (t2, 0x0)
    addh.s32        t1, l6, l7          // t1
    addh.s32        a0, l8, t1          // r1 + t1
    st.w            a0, (t3, 0x0)
    subh.s32        l8, l8, t1          // r1 = r1 - t1
    subh.s32        a2, l6, l7          // t2 = pSrc[2u * i1] - pSrc[2u * i3]
    ld.w            l6, (t3, 0x4)
    ld.w            l7, (t5, 0x4)
    addh.s32        a0, l6, l7          // s1
    subh.s32        a1, l6, l7          // s2
    ld.w            l6, (t4, 0x4)
    ld.w            l7, (t2, 0x4)
    addh.s32        a3, l6, l7          // t2
    addh.s32        t1, a3, a0          // s1 + t2
    st.w            t1, (t3, 0x4)
    subh.s32        a0, a0, a3          // s1 = s1 - t2
    subh.s32        t1, l6, l7          // t1

    mul.s32.h       l6, l8, l2
    mul.s32.h       l7, a0, l3
    sub.s32.s       l6, l6, l7
    lsli            l6, l6, 1
    st.w            l6, (t4, 0x0)
    mul.s32.h       l6, a0, l2
    mul.s32.h       l7, l8, l3
    add.s32.s       l6, l6, l7
    lsli            l6, l6, 1
    st.w            l6, (t4, 0x4)

    subh.s32        l8, l9, t1          // r1 = r2 - t1
    addh.s32        l9, l9, t1          // r2 = r2 + t1
    addh.s32        a0, a1, a2          // s1 = s2 + t2
    subh.s32        a1, a1, a2          // s2 = s2 - t2

    mul.s32.h       l6, l8, l0
    mul.s32.h       l7, a0, l1
    sub.s32.s       l6, l6, l7
    lsli            l6, l6, 1
    st.w            l6, (t5, 0x0)
    mul.s32.h       l6, a0, l0
    mul.s32.h       l7, l8, l1
    add.s32.s       l6, l6, l7
    lsli            l6, l6, 1
    st.w            l6, (t5, 0x4)

    mul.s32.h       l6, l9, l4
    mul.s32.h       l7, a1, l5
    sub.s32.s       l6, l6, l7
    lsli            l6, l6, 1
    st.w            l6, (t2, 0x0)
    mul.s32.h       l6, a1, l4
    mul.s32.h       l7, l9, l5
    add.s32.s       l6, l6, l7
    lsli            l6, l6, 1
    st.w            l6, (t2, 0x4)

    addu            t0, t0, t7          // i0 += n1
    br              .L25
.L24:

    addi            t6, t6, 1           // j++
    br              .L23
.L22:
    ld.w            a3, (sp, 0xc)
    lsli            a3, a3, 2           // twidCoefModifier <<= 2
    st.w            a3, (sp, 0xc)
    asri            t8, t8, 2           // k >>= 2
    br              .L211
.L210:

    /* Start of last stage process */
    ld.w            a1, (sp, 0x4)       // fftLen
    ld.w            a0, (sp, 0x0)       // ptr1 = pSrc[0]
    lsri            a1, a1, 2           // j
    mov             a2, a0

.L212:
    pldbi.d         l0, (a0)            // xaya
    pldbi.d         l2, (a0)            // xbyb
    pldbi.d         l4, (a0)            // xcyc
    pldbi.d         l6, (a0)            // xdyd

    addh.s32        t0, l0, l4          // xa + xc
    addh.s32        t1, l2, l6          // xb + xd
    addh.s32        t2, l1, l5          // ya + yc
    addh.s32        t3, l3, l7          // yb + yd
    addh.s32        t4, t0, t1          // xa_out = xa + xc + xb + xd
    subh.s32        t5, t0, t1          // xc_out = xa + xc - (xb + xd)
    addh.s32        t6, t2, t3          // ya_out = ya + yc + yb + yd
    subh.s32        t7, t2, t3          // yc_out = ya + yc - (yb + yd)
    st.w            t4, (a2, 0x0)
    st.w            t6, (a2, 0x4)
    st.w            t5, (a2, 0x8)
    st.w            t7, (a2, 0xc)

    subh.s32        t0, l0, l4          // xa - xc
    subh.s32        t1, l2, l6          // xb - xd
    subh.s32        t2, l1, l5          // ya - yc
    subh.s32        t3, l3, l7          // yb - yd
    subh.s32        t4, t0, t3          // xb_out = xa - xc - (yb - yd)
    addh.s32        t5, t0, t3          // xd_out = xa - xc + yb - yd
    addh.s32        t6, t1, t2          // yb_out = ya - yc + xb - xd
    subh.s32        t7, t2, t1          // yd_out = ya - yc - (xb - xd)
    st.w            t4, (a2, 0x10)
    st.w            t6, (a2, 0x14)
    st.w            t5, (a2, 0x18)
    st.w            t7, (a2, 0x1c)
.L213:
    addi            a2, a2, 32

    bloop           a1, .L212, .L213

    addi            sp, sp, 0x14
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_radix4_butterfly_inverse_yv_q31, .-csky_radix4_butterfly_inverse_yv_q31
