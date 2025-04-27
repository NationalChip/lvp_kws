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
 * @file     csky_cfft_q15.S
 * @brief    Combined Radix Decimation in Frequency CFFT fixed point processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

.import csky_radix4_butterfly_q15
.import csky_radix4_butterfly_inverse_q15

/*
 * void csky_cfft_radix4by2_q15(
 *     q15_t * pSrc,
 *     uint32_t fftLen,
 *     const q15_t * pCoef)
 */
    .file           "csky_cfft_q15.S"
    .section        .text.csky_cfft_radix4by2_q15,"ax",@progbits
    .align          2
    .global         csky_cfft_radix4by2_q15
    .type           csky_cfft_radix4by2_q15, @function
csky_cfft_radix4by2_q15:
    push            l0, l1, l2, lr
    mov             l0, a0              // pSrc
    mov             l1, a1              // fftLen
    mov             l2, a2              // pCoef
    lsri            a3, a1, 1           // n2 = fftLen >> 1

    movi            t1, 0               // i
    addu            t0, t1, a3          // l = i + n2
    lsli            t0, t0, 2
    addu            t0, a0, t0          // pSrc[2 * l]
.L1:
    cmplt           t1, a3              // i < n2
    bf              .L0

    ldbi.w          t2, (a2)            // cosVal, sinVal
    ld.w            t4, (a0, 0x0)
    ld.w            t5, (t0, 0x0)
    pasri.s16       t4, t4, 1
    pasri.s16       t5, t5, 1
    psub.16         t6, t4, t5          // xt, yt
    paddh.s16       t7, t4, t5
    stbi.w          t7, (a0)            // pSrc[2 * i]

    pmul.s16        t8, t6, t2
    addu            t8, t8, t9
    asri            t8, t8, 16
    stbi.h          t8, (t0)            // pSrc[2u * l]

    pmulx.s16       t8, t6, t2          // xt * sinVal, yt * cosVal
    subu            t8, t9, t8
    asri            t8, t8, 16
    stbi.h          t8, (t0)            // pSrc[2u * l + 1u]

    addi            t1, t1, 1
    br              .L1
.L0:
    mov             a0, l0
    mov             a2, l2
    mov             a1, a3              // n2
    movi            a3, 2
    jbsr            csky_radix4_butterfly_q15

    lsli            a0, l1, 1
    addu            a0, l0, a0          // pSrc + fftLen
    lsri            a1, l1, 1           // n2
    mov             a2, l2
    movi            a3, 2
    jbsr            csky_radix4_butterfly_q15

    lsri            l1, l1, 1           // fftLen >> 1
    movi            t0, 0               // i = 0
    mov             l2, l0
.L3:
    cmplt           t0, l1
    bf              .L2
    pldbi.d         a0, (l0)            // p0, p1
    plsli.16        a0, a0, 1
    plsli.16        a1, a1, 1
    stbi.w          a0, (l2)
    stbi.w          a1, (l2)
    addi            t0, t0, 1
    br              .L3
.L2:

    pop             l0, l1, l2, lr
    .size   csky_cfft_radix4by2_q15, .-csky_cfft_radix4by2_q15

    .section        .text.csky_cfft_radix4by2_inverse_q15,"ax",@progbits
    .align          2
    .global         csky_cfft_radix4by2_inverse_q15
    .type           csky_cfft_radix4by2_inverse_q15, @function
csky_cfft_radix4by2_inverse_q15:
    push            l0, l1, l2, lr
    mov             l0, a0              // pSrc
    mov             l1, a1              // fftLen
    mov             l2, a2              // pCoef
    lsri            a3, a1, 1           // n2 = fftLen >> 1

    movi            t1, 0               // i
    addu            t0, t1, a3          // l = i + n2
    lsli            t0, t0, 2
    addu            t0, a0, t0          // pSrc[2 * l]
.L11:
    cmplt           t1, a3              // i < n2
    bf              .L10

    ldbi.w          t2, (a2)            // cosVal, sinVal
    ld.w            t4, (a0, 0x0)
    ld.w            t5, (t0, 0x0)
    pasri.s16       t4, t4, 1
    pasri.s16       t5, t5, 1
    psub.16         t6, t4, t5          // xt, yt
    paddh.s16       t7, t4, t5
    stbi.w          t7, (a0)            // pSrc[2 * i]

    pmul.s16        t8, t6, t2          // xt * cosVal, yt * sinVal
    subu            t8, t8, t9
    asri            t8, t8, 16
    stbi.h          t8, (t0)            // pSrc[2u * l]

    pmulx.s16       t8, t6, t2          // xt * sinVal, yt * cosVal
    addu            t8, t9, t8
    asri            t8, t8, 16
    stbi.h          t8, (t0)            // pSrc[2u * l + 1u]
    addi            t1, t1, 1
    br              .L11
.L10:
    mov             a0, l0
    mov             a2, l2
    mov             a1, a3              // n2
    movi            a3, 2
    jbsr            csky_radix4_butterfly_inverse_q15

    lsli            a0, l1, 1
    addu            a0, l0, a0          // pSrc + fftLen
    lsri            a1, l1, 1           // n2
    mov             a2, l2
    movi            a3, 2
    jbsr            csky_radix4_butterfly_inverse_q15

    lsri            l1, l1, 1           // fftLen >> 1
    movi            t0, 0               // i = 0
    mov             l2, l0
.L13:
    cmplt           t0, l1
    bf              .L12
    pldbi.d         a0, (l0)            // p0, p1
    plsli.16        a0, a0, 1
    plsli.16        a1, a1, 1
    stbi.w          a0, (l2)
    stbi.w          a1, (l2)
    addi            t0, t0, 1
    br              .L13
.L12:
    pop             l0, l1, l2, lr
    .size           csky_cfft_radix4by2_inverse_q15, .-csky_cfft_radix4by2_inverse_q15

