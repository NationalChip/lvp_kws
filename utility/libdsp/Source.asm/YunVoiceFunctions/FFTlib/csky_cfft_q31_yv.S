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
 * @file     csky_cfft_q31_yv.S
 * @brief    Combined Radix Decimation in Frequency CFFT fixed point processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

.import csky_radix4_butterfly_yv_q31
.import csky_radix4_butterfly_inverse_yv_q31

/*
 * void csky_cfft_radix4by2_yv_q31(
 *     q31_t * pSrc,
 *     uint32_t fftLen,
 *     const q31_t * pCoef)
 */
    .file           "csky_cfft_q31_yv.S"
    .section        .text.csky_cfft_radix4by2_yv_q31,"ax",@progbits
    .align          2
    .global         csky_cfft_radix4by2_yv_q31
    .type           csky_cfft_radix4by2_yv_q31, @function

csky_cfft_radix4by2_yv_q31:
    push            l0, l1, l2, l3, l4,lr
    mov             l0, a0              // pSrc
    mov             l1, a1              // fftLen
    mov             l2, a2              // pCoef

    lsri            a3, a1, 1           // n2 = fftLen >> 1

    movi            t1, 0               // i
    addu            t0, t1, a3          // l = i + n2
    lsli            t0, t0, 3
    addu            t0, a0, t0          // pSrc[2 * l]
.L1:
    cmplt           t1, a3              // i < n2
    bf              .L0

    pldbi.d         t2, (a2)            // cosVal, sinVal
    ld.w            t4, (a0, 0x0)
    ld.w            t5, (t0, 0x0)
    sub.s32.s       t6, t4, t5          // xt
    add.s32.s       t7, t4, t5
    stbi.w          t7, (a0)

    ld.w            t4, (a0, 0x0)
    ld.w            t5, (t0, 0x4)
    add.s32.s       t7, t4, t5
    stbi.w          t7, (a0)
    sub.s32.s       t7, t4, t5          // yt

    mul.s32         t4, t6, t2          // p0
    mul.s32         l3, t7, t2          // p1
    mula.s32.s      t4, t7, t3          // p0
    muls.s32.s      l3, t6, t3          // p1

    dexti           t4, t4, t5, 31
    stbi.w          t4, (t0)
    dexti           l3, l3, l4, 31
    stbi.w          l3, (t0)

    addi            t1, t1, 1           // i++

    br              .L1
.L0:
    mov             a0, l0              // pSrc
    mov             a2, l2              // pCoef
    mov             a1, a3              // n2
    movi            a3, 2
    jbsr            csky_radix4_butterfly_yv_q31

    lsli            a0, l1, 2
    addu            a0, l0, a0          // pSrc + fftLen
    lsri            a1, l1, 1           // n2
    mov             a2, l2
    movi            a3, 2
    jbsr            csky_radix4_butterfly_yv_q31

    lsri            l1, l1, 1           // fftLen >> 1
    movi            t0, 0               // i = 0
    mov             l2, l0
.L3:
    cmplt           t0, l1              // i < fftLen >> 1
    bf              .L2
    pldbi.d         a0, (l0)            // p0, p1
    pldbi.d         a2, (l0)            // xt, yt
    st.w            a0, (l2, 0x0)
    st.w            a1, (l2, 0x4)
    st.w            a2, (l2, 0x8)
    st.w            a3, (l2, 0xc)

    addi            l2, l2, 16
    addi            t0, t0, 1           // i++

    br              .L3
.L2:

    pop             l0, l1, l2, l3, l4, lr
    .size   csky_cfft_radix4by2_yv_q31, .-csky_cfft_radix4by2_yv_q31

    .section        .text.csky_cfft_radix4by2_inverse_yv_q31,"ax",@progbits
    .align          2
    .global         csky_cfft_radix4by2_inverse_yv_q31
    .type           csky_cfft_radix4by2_inverse_yv_q31, @function

csky_cfft_radix4by2_inverse_yv_q31:
    push            l0, l1, l2, lr
    mov             l0, a0              // pSrc
    mov             l1, a1              // fftLen
    mov             l2, a2              // pCoef
    lsri            a3, a1, 1           // n2 = fftLen >> 1

    movi            t1, 0               // i
    addu            t0, t1, a3          // l = i + n2
    lsli            t0, t0, 3
    addu            t0, a0, t0          // pSrc[2 * l]
.L11:
    cmplt           t1, a3              // i < n2
    bf              .L10

    pldbi.d         t2, (a2)            // cosVal, sinVal
    ld.w            t4, (a0, 0x0)
    ld.w            t5, (t0, 0x0)
    subh.s32        t6, t4, t5          // xt
    addh.s32        t7, t4, t5
    stbi.w          t7, (a0)

    ld.w            t4, (a0, 0x0)
    ld.w            t5, (t0, 0x4)
    addh.s32        t7, t4, t5
    stbi.w          t7, (a0)
    subh.s32        t7, t4, t5          // yt

    mul.s32.rh      t4, t6, t2          // p0
    mul.s32.rh      t5, t7, t2          // p1
    muls.s32.rhs    t4, t7, t3          // p0
    mula.s32.rhs    t5, t6, t3          // p1

    lsli            t4, t4, 1
    stbi.w          t4, (t0)
    lsli            t5, t5, 1
    stbi.w          t5, (t0)

    addi            t1, t1, 1           // i++

    br              .L11
.L10:
    mov             a0, l0              // pSrc
    mov             a1, a3              // n2
    mov             a2, l2              // pCoef
    movi            a3, 2
    jbsr            csky_radix4_butterfly_inverse_yv_q31

    lsli            a0, l1, 2
    addu            a0, l0, a0          // pSrc + fftLen
    lsri            a1, l1, 1           // n2
    mov             a2, l2
    movi            a3, 2
    jbsr            csky_radix4_butterfly_inverse_yv_q31

    lsri            l1, l1, 1           // fftLen >> 1
    movi            t0, 0               // i = 0
    mov             l2, l0
.L13:
    cmplt           t0, l1
    bf              .L12
    pldbi.d         a0, (l0)            // p0, p1
    pldbi.d         a2, (l0)            // xt, yt
    st.w            a0, (l2, 0x0)
    st.w            a1, (l2, 0x4)
    st.w            a2, (l2, 0x8)
    st.w            a3, (l2, 0xc)
    addi            l2, l2, 16
    addi            t0, t0, 1           // i++
    br              .L13
.L12:

    pop             l0, l1, l2, lr
    .size           csky_cfft_radix4by2_inverse_yv_q31, .-csky_cfft_radix4by2_inverse_yv_q31

