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
 * @file     csky_cfft_radix4_q15_yv.S
 * @brief    This file has function definition of Radix-4 FFT & IFFT function and
 *   		     In-place bit reversal using bit reversal table.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_radix4_butterfly_yv_q15(
 *   q15_t * pSrc16,
 *   uint32_t fftLen,
 *   q15_t * pCoef16,
 *   uint32_t twidCoefModifier)
 */
    .file           "csky_cfft_radix4_q15_yv.S"
    .section        .text.csky_radix4_butterfly_yv_q15,"ax",@progbits
    .align          2
    .global         csky_radix4_butterfly_yv_q15
    .type           csky_radix4_butterfly_yv_q15, @function

csky_radix4_butterfly_yv_q15:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    subi            sp, sp, 0x8
    /*  Calculation of first stage */
    lsri            t9, a1, 2           // n2
    mov             t8, t9              // j = n2
    mov             t0, a0              // pSi0
    addu            t1, t0, a1          // pSi1
    addu            t2, t1, a1          // pSi2
    addu            t3, t2, a1          // pSi3
    movi            l8, 0               // ic

.L0:
    ld.w            l0, (t0, 0x0)       // pSi0
    ld.w            l1, (t1, 0x0)       // pSi1
    ld.w            l2, (t2, 0x0)       // pSi2
    ld.w            l3, (t3, 0x0)       // pSi3

    padd.s16.s      l4, l0, l2          // R
    psub.s16.s      l2, l0, l2          // S
    padd.s16.s      l7, l1, l3          // T

    padd.s16.s      l5, l4, l7
    stbi.w          l5, (t0)

    psub.s16.s      l5, l4, l7          // R

    lsli            l9, l8, 3
    addu            l9, a2, l9
    ld.w            l6, (l9, 0x0)       // C2
    mulca.s16.s     t4, l6, l5
    mulcsx.s16      t5, l6, l5
    pkg             t4, t4, 15, t5, 15
    stbi.w          t4, (t1)

    psub.s16.s      l7, l1, l3          // T
    pasx.s16.s      l5, l2, l7          // R
    psax.s16.s      l2, l2, l7          // S

    lsli            l9, l8, 2
    addu            l9, a2, l9
    ld.w            l6, (l9, 0x0)       // C1
    mulca.s16.s     t4, l6, l2
    mulcsx.s16      t5, l6, l2
    pkg             t4, t4, 15, t5, 15
    stbi.w          t4, (t2)

    movi            l9, 12
    mult            l9, l8, l9
    addu            l9, a2, l9
    ld.w            l6, (l9, 0x0)       // C1
    mulca.s16.s     t4, l6, l5
    mulcsx.s16      t5, l6, l5
    pkg             t4, t4, 15, t5, 15
    stbi.w          t4, (t3)

.L1:
    addu            l8, l8, a3          // ic = ic + twidCoefModifier

    bloop           t8, .L0, .L1

    /* Start of middle stages process */
    lsli            a3, a3, 2           // twidCoefModifier <<= 2
    st.w            a0, (sp, 0x0)

    asri            t8, a1, 2           // k = fftLen / 4
.L11:
    cmphsi          t8, 5               // k > 4
    bf              .L10
    mov             t7, t9              // n1 = n2
    asri            t9, t9, 2           // n2 >>= 2
    movi            l8, 0               // ia1 = 0
    st.w            l8, (sp, 0x4)

    movi            t6, 0               // j = 0
.L3:
    cmplt           t6, t9              // j <= n2 - 1
    bf              .L2
    ld.w            l8, (sp, 0x4)
    movi            l9, 4
    mult            l3, l9, l8
    addu            l9, l3, a2
    ld.w            l0, (l9, 0x0)       // C1
    addu            l9, l9, l3
    ld.w            l1, (l9, 0x0)       // C2
    addu            l9, l9, l3
    ld.w            l2, (l9, 0x0)       // C3
    addu            l8, l8, a3          // ia1 = ia1 + twidCoefModifier
    st.w            l8, (sp, 0x4)

    ld.w            a0, (sp, 0x0)       // pSrc16
    lsli            t1, t6, 2           // 2 * j
    addu            t3, t1, a0          // pSi0
    lsli            t2, t9, 2           // 2 * n2
    addu            t4, t3, t2          // pSi1
    addu            t5, t4, t2          // pSi2
    addu            t2, t5, t2          // pSi3

    mov             t0, t6              // i0 = j
.L5:
    cmplt           t0, a1              // i0 < fftLen
    bf              .L4

    ld.w            l3, (t3, 0x0)       // T
    ld.w            l4, (t5, 0x0)       // S

    padd.s16.s      l5, l3, l4          // R
    psub.s16.s      l6, l3, l4          // S

    ld.w            a0, (t4, 0x0)       // T
    ld.w            l7, (t2, 0x0)       // U
    padd.s16.s      l3, a0, l7          // T

    padd.s16.s      t1, l5, l3
    st.w            t1, (t3, 0x0)

    lsli            l9, t7, 2
    addu            t3, t3, l9          // pSi0 += 2 * n1

    psub.s16.s      l5, l5, l3          // R

    mulca.s16.s     t1, l1, l5          // out1
    mulcsx.s16      l8, l1, l5          // out2
    pkg             t1, t1, 15, l8, 15
    st.w            t1, (t4, 0x0)
    addu            t4, t4, l9

    psub.s16.s      l3, a0, l7          // T
    pasx.s16.s      l5, l6, l3          // R
    psax.s16.s      l6, l6, l3          // S

    mulca.s16.s     t1, l0, l6          // out1
    mulcsx.s16      l8, l0, l6          // out2
    pkg             t1, t1, 15, l8, 15
    st.w            t1, (t5, 0x0)
    addu            t5, t5, l9

    mulca.s16.s     t1, l2, l5          // out1
    mulcsx.s16      l8, l2, l5          // out2
    pkg             t1, t1, 15, l8, 15
    st.w            t1, (t2, 0x0)
    addu            t2, t2, l9

    addu            t0, t0, t7          // i0 += n1
    br              .L5
.L4:

    addi            t6, t6, 1           // j++
    br              .L3
.L2:
    lsli            a3, a3, 2           // twidCoefModifier <<= 2
    asri            t8, t8, 2           // k >>= 2
    br              .L11
.L10:

    /* Start of last stage process */
    ld.w            a0, (sp, 0x0)       // ptr1 = pSrc[0]
    lsri            a1, a1, 2           // j
    mov             a2, a0

.L12:
    pldbi.d         l0, (a0)            // xaya, xbyb
    pldbi.d         l2, (a0)            // xcyc, xdyd

    padd.s16.s      l4, l0, l2          // R
    padd.s16.s      l5, l1, l3          // T

    padd.s16.s      t0, l4, l5
    psub.s16.s      t1, l4, l5
    stbi.w          t0, (a2)
    stbi.w          t1, (a2)

    psub.s16.s      l4, l0, l2          // S
    psub.s16.s      l5, l1, l3          // U
    psax.s16.s      t0, l4, l5
    pasx.s16.s      t1, l4, l5
    stbi.w          t0, (a2)
.L13:
    stbi.w          t1, (a2)

    bloop           a1, .L12, .L13

    addi            sp, sp, 0x8
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9

    .size           csky_radix4_butterfly_yv_q15, .-csky_radix4_butterfly_yv_q15


/*
 * void csky_radix4_butterfly_inverse_yv_q15(
 *   q15_t * pSrc16,
 *   uint32_t fftLen,
 *   q15_t * pCoef16,
 *   uint32_t twidCoefModifier)
 */
    .section        .text.csky_radix4_butterfly_inverse_yv_q15,"ax",@progbits
    .align          2
    .global         csky_radix4_butterfly_inverse_yv_q15
    .type           csky_radix4_butterfly_inverse_yv_q15, @function

csky_radix4_butterfly_inverse_yv_q15:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    subi            sp, sp, 0x8
    /*  Calculation of first stage */
    lsri            t9, a1, 2           // n2
    mov             t8, t9              // j = n2
    mov             t0, a0              // pSi0
    addu            t1, t0, a1          // pSi1
    addu            t2, t1, a1          // pSi2
    addu            t3, t2, a1          // pSi3
    movi            l8, 0               // ic

.L20:
    ld.w            l0, (t0, 0x0)       // pSi0
    ld.w            l1, (t1, 0x0)       // pSi1
    ld.w            l2, (t2, 0x0)       // pSi2
    ld.w            l3, (t3, 0x0)       // pSi3

    paddh.s16       l4, l0, l2          // R
    psubh.s16       l2, l0, l2          // S
    paddh.s16       l7, l1, l3          // T

    paddh.s16       l5, l4, l7
    stbi.w          l5, (t0)

    psubh.s16       l5, l4, l7          // R

    lsli            l9, l8, 3
    addu            l9, a2, l9
    ld.w            l6, (l9, 0x0)       // C2
    mulcs.s16       t4, l6, l5
    mulcax.s16.s    t5, l6, l5
    pkg             t4, t4, 15, t5, 15
    stbi.w          t4, (t1)

    psubh.s16       l7, l1, l3          // T
    psaxh.s16       l5, l2, l7          // R
    pasxh.s16       l2, l2, l7          // S

    lsli            l9, l8, 2
    addu            l9, a2, l9
    ld.w            l6, (l9, 0x0)       // C1
    mulcs.s16       t4, l6, l2
    mulcax.s16.s    t5, l6, l2
    pkg             t4, t4, 15, t5, 15
    stbi.w          t4, (t2)

    movi            l9, 12
    mult            l9, l8, l9
    addu            l9, a2, l9
    ld.w            l6, (l9, 0x0)       // C3
    mulcs.s16       t4, l6, l5
    mulcax.s16.s    t5, l6, l5
    pkg             t4, t4, 15, t5, 15
    stbi.w          t4, (t3)

.L21:
    addu            l8, l8, a3          // ic = ic + twidCoefModifier

    bloop           t8, .L20, .L21

    /* Start of middle stages process */
    lsli            a3, a3, 2           // twidCoefModifier <<= 2
    st.w            a0, (sp, 0x0)

    asri            t8, a1, 2           // k = fftLen / 4
.L211:
    cmphsi          t8, 5               // k > 4
    bf              .L210
    mov             t7, t9              // n1 = n2
    asri            t9, t9, 2           // n2 >>= 2
    movi            l8, 0               // ia1 = 0
    st.w            l8, (sp, 0x4)

    movi            t6, 0               // j = 0
.L23:
    cmplt           t6, t9              // j <= n2 - 1
    bf              .L22
    ld.w            l8, (sp, 0x4)
    movi            l9, 4
    mult            l3, l9, l8
    addu            l9, l3, a2
    ld.w            l0, (l9, 0x0)       // C1
    addu            l9, l9, l3
    ld.w            l1, (l9, 0x0)       // C2
    addu            l9, l9, l3
    ld.w            l2, (l9, 0x0)       // C3
    addu            l8, l8, a3          // ia1 = ia1 + twidCoefModifier
    st.w            l8, (sp, 0x4)

    ld.w            a0, (sp, 0x0)       // pSrc16
    lsli            t1, t6, 2           // 2 * j
    addu            t3, t1, a0          // pSi0
    lsli            t2, t9, 2           // 2 * n2
    addu            t4, t3, t2          // pSi1
    addu            t5, t4, t2          // pSi2
    addu            t2, t5, t2          // pSi3

    mov             t0, t6              // i0 = j
.L25:
    cmplt           t0, a1              // i0 < fftLen
    bf              .L24

    ld.w            l3, (t3, 0x0)       // T
    ld.w            l4, (t5, 0x0)       // S

    paddh.s16       l5, l3, l4          // R
    psubh.s16       l6, l3, l4          // S

    ld.w            a0, (t4, 0x0)       // T
    ld.w            l7, (t2, 0x0)       // U
    paddh.s16       l3, a0, l7          // T

    paddh.s16       t1, l5, l3
    st.w            t1, (t3, 0x0)

    lsli            l9, t7, 2
    addu            t3, t3, l9          // pSi0 += 2 * n1

    psubh.s16       l5, l5, l3          // R

    mulcs.s16       t1, l1, l5          // out1
    mulcax.s16.s    l8, l1, l5          // out2
    pkg             t1, t1, 15, l8, 15
    st.w            t1, (t4, 0x0)
    addu            t4, t4, l9

    psubh.s16       l3, a0, l7          // T
    psaxh.s16       l5, l6, l3          // R
    pasxh.s16       l6, l6, l3          // S

    mulcs.s16       t1, l0, l6          // out1
    mulcax.s16.s    l8, l0, l6          // out2
    pkg             t1, t1, 15, l8, 15
    st.w            t1, (t5, 0x0)
    addu            t5, t5, l9

    mulcs.s16       t1, l2, l5          // out1
    mulcax.s16.s    l8, l2, l5          // out2
    pkg             t1, t1, 15, l8, 15
    st.w            t1, (t2, 0x0)
    addu            t2, t2, l9

    addu            t0, t0, t7          // i0 += n1
    br              .L25
.L24:

    addi            t6, t6, 1           // j++
    br              .L23
.L22:
    lsli            a3, a3, 2           // twidCoefModifier <<= 2
    asri            t8, t8, 2           // k >>= 2
    br              .L211
.L210:

    /* Start of last stage process */
    ld.w            a0, (sp, 0x0)       // ptr1 = pSrc[0]
    lsri            a1, a1, 2           // j
    mov             a2, a0

.L212:
    pldbi.d         l0, (a0)            // xaya, xbyb
    pldbi.d         l2, (a0)            // xcyc, xdyd

    paddh.s16       l4, l0, l2          // R
    paddh.s16       l5, l1, l3          // T

    paddh.s16       t0, l4, l5
    psubh.s16       t1, l4, l5
    st.w            t0, (a2, 0x0)
    st.w            t1, (a2, 0x4)

    psubh.s16       l4, l0, l2          // S
    psubh.s16       l5, l1, l3          // U
    pasxh.s16       t0, l4, l5
    psaxh.s16       t1, l4, l5
    st.w            t0, (a2, 0x8)
    st.w            t1, (a2, 0xc)
.L213:
    addi            a2, a2, 16

    bloop           a1, .L212, .L213

    addi            sp, sp, 0x8
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_radix4_butterfly_inverse_yv_q15, .-csky_radix4_butterfly_inverse_yv_q15

