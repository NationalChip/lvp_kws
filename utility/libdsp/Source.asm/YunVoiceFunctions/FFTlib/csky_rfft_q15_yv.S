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
 * @file     csky_rfft_q15_yv.S
 * @brief    RFFT & RIFFT Q15 process function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_split_rfft_yv_q15(
 *     q15_t * pSrc,
 *     uint32_t fftLen,
 *     q15_t * pATable,
 *     q15_t * pDst,
 *     uint32_t modifier)
 */
    .file           "csky_rfft_q15_yv.S"
    .section        .text.csky_split_rfft_yv_q15,"ax",@progbits
    .align          2
    .global         csky_split_rfft_yv_q15
    .type           csky_split_rfft_yv_q15, @function

csky_split_rfft_yv_q15:
    push            l0, l1, l2, l3
    ld.w            t9, (sp, 0x10)
    lsli            t9, t9, 2           // modifier * 2
    addu            a2, a2, t9          // pCoefA
    bgeni           l3, 15

    addi            t0, a0, 4           // pSrc1 = &pSrc[2]
    lsli            t1, a1, 2
    subi            t1, t1, 4
    addu            t1, t1, a0          // pSrc2 = &pSrc[(2u * fftLen) - 2u]

    addi            t2, a3, 4           // pD1

    subi            t7, a1, 1           // i
    bez             t7, .L0
.L1:
    ldbi.w          t4, (t0)            // *pSrc1   ++
    ld.w            t5, (a2, 0x0)       // *pCoefA
    ld.w            t6, (t1, 0x0)       // *pSrc2
    psub.16         l2, l3, t5          // CoefB
    pabs.s16.s      t8, l2
    pkg             l2, t8, 0, l2, 16
    mulcs.s16       l0, t4, t5          // outR
    mulaca.s16.s    l0, t6, l2
    mulcsx.s16      l1, t6, l2
    mulacax.s16.s   l1, t4, t5          // outI
    asri            l0, l0, 15
    asri            l1, l1, 15
    stbi.h          l0, (t2)
    stbi.h          l1, (t2)

    addu            a2, a2, t9

.L2:
    subi            t1, t1, 4           // pSrc2--
    bloop           t7, .L1, .L2

.L0:
    ld.hs           t0, (a0, 0x0)
    ld.hs           t1, (a0, 0x2)
    movi            t2, 0
    subh.s32        t3, t0, t1
    lsli            t4, a1, 2
    addu            t4, t4, a3
    st.h            t3, (t4, 0x0)
    st.h            t2, (t4, 0x2)
    addh.s32        t3, t0, t1
    st.h            t3, (a3, 0x0)
    st.h            t2, (a3, 0x2)

    pop             l0, l1, l2, l3
    .size           csky_split_rfft_yv_q15, .-csky_split_rfft_yv_q15

/*
 * void csky_split_rifft_yv_q15(
 *     q15_t * pSrc,
 *     uint32_t fftLen,
 *     q15_t * pATable,
 *     q15_t * pDst,
 *     uint32_t modifier)
 */
    .section        .text.csky_split_rifft_yv_q15,"ax",@progbits
    .align          2
    .global         csky_split_rifft_yv_q15
    .type           csky_split_rifft_yv_q15, @function

csky_split_rifft_yv_q15:
    push            l0
    ld.w            t9, (sp, 0x4)       // modifier
    lsli            t9, t9, 2           // modifier * 2
    lsli            t0, a1, 2
    addu            t0, t0, a0          // pSrc2 = &pSrc[2u * fftLen]
    bgeni           l0, 15

    bez             a1, .L10
.L11:
    ldbi.w          t1, (a0)            // pSrc1
    ld.w            t2, (t0)            // pSrc2
    ld.w            t4, (a2)            // pCoefA
    psub.16         t8, l0, t4
    pabs.s16.s      t3, t8
    pkg             t3, t3, 0, t8, 16

    mulcs.s16       t6, t2, t3
    mulaca.s16.s    t6, t1, t4          // outR
    mulcax.s16.s    t7, t2, t3
    pneg.s16.s      t7, t7
    mulacsx.s16.s   t7, t4, t1          // outI

    pkg             t6, t6, 15, t7, 15
    stbi.w          t6, (a3)

    addu            a2, a2, t9          // pCoefA

.L12:
    subi            t0, t0, 4           // pSrc2
    bloop           a1, .L11, .L12
.L10:
    pop             l0
    .size           csky_split_rifft_yv_q15, .-csky_split_rifft_yv_q15
