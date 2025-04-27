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
 * @file     csky_rfft_q31_yv.S
 * @brief    RFFT & RIFFT Q31 process function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_split_rfft_yv_q31(
 *     q31_t * pSrc,
 *     uint32_t fftLen,
 *     q31_t * pATable,
 *     q31_t * pDst,
 *     uint32_t modifier)
 */
    .file           "csky_rfft_q31_yv.S"
    .section        .text.csky_split_rfft_yv_q31,"ax",@progbits
    .align          2
    .global         csky_split_rfft_yv_q31
    .type           csky_split_rfft_yv_q31, @function

csky_split_rfft_yv_q31:
    push            l0, l1
    addi            t0, a3, 8           // pOut1 = &pDst[2]
    bgeni           t1, 31              // 0x80000000
    addi            t2, a0, 8           // pIn1 = &pSrc[2]
    lsli            t3, a1, 3
    subi            t3, t3, 4
    addu            t3, t3, a0          // pIn2 = &pSrc[(2u * fftLen) - 1u]

    ld.w            t8, (sp, 0x8)       // modifier
    lsli            t8, t8, 3           // modifier * 2
    addu            a2, a2, t8          // pCoefA

    subi            t9, a1, 1           // i
    bez             t9, .L0
    subi            t3, t3, 4
.L1:
    ld.w            t4, (a2, 0x0)       // CoefA1
    ld.w            t5, (a2, 0x4)       // CoefA2
    pldbi.d         t6, (t2)
    mul.s32.rh      l0, t6, t4          // outR
    mul.s32.rh      l1, t6, t5          // outI
    muls.s32.rhs    l0, t7, t5
    mula.s32.rhs    l1, t7, t4

    ld.w            t6, (t3, 0x4)
    ld.w            t7, (t3, 0x0)
    subu            t4, t1, t4
    abs.s32.s       t4, t4               // CoefB1
    muls.s32.rhs    l0, t6, t5
    muls.s32.rhs    l1, t6, t4
    mula.s32.rhs    l0, t7, t4
    lsli            l0, l0, 1
    muls.s32.rhs    l1, t7, t5
    lsli            l1, l1, 1

    stbi.w          l0, (t0)            // *pOut1++ = outR
    stbi.w          l1, (t0)            // *pOut1++ = outI

    addu            a2, a2, t8
.L2:
    subi            t3, t3, 8
    bloop           t9, .L1, .L2
.L0:

    lsli            a1, a1, 3
    addu            a1, a1, a3

    ld.w            t0, (a0, 0x0)
    ld.w            t1, (a0, 0x4)
    subh.s32        t2, t0, t1
    stbi.w          t2, (a1)
    movi            t3, 0
    st.w            t3, (a1, 0x0)

    addh.s32        t2, t0, t1
    stbi.w          t2, (a3)
    st.w            t3, (a3, 0x0)

    pop             l0, l1
    .size           csky_split_rfft_yv_q31, .-csky_split_rfft_yv_q31

/*
 * void csky_split_rifft_yv_q31(
 *     q31_t * pSrc,
 *     uint32_t fftLen,
 *     q31_t * pATable,
 *     q31_t * pDst,
 *     uint32_t modifier)
 */
    .section        .text.csky_split_rifft_yv_q31,"ax",@progbits
    .align          2
    .global         csky_split_rifft_yv_q31
    .type           csky_split_rifft_yv_q31, @function

csky_split_rifft_yv_q31:
    lsli            t1, a1, 3
    addi            t1, t1, 4
    addu            t1, t1, a0          // pIn2 = &pSrc[(2u * fftLen) + 1u]
    bgeni           t0, 31              // 0x80000000

    ld.w            t2, (sp, 0x0)       // modifier
    lsli            t2, t2, 3           // modifier * 2

    bez             a1, .L10            // fftLen
    subi            t1, t1, 4
.L11:
    ld.w            t4, (a2, 0x0)       // CoefA1
    ld.w            t5, (a2, 0x4)       // CoefA2
    neg.s32.s       t3, t5              // tmp = -CoefA2
    pldbi.d         t6, (a0)
    mul.s32.rh      t8, t6, t4          // outR
    mul.s32.rh      t9, t6, t3          // outI
    mula.s32.rhs    t8, t7, t5
    mula.s32.rhs    t9, t7, t4

    ld.w            t6, (t1, 0x4)
    ld.w            t7, (t1, 0x0)
    subu            t4, t0, t4
    abs.s32.s       t4, t4
    mula.s32.rhs    t8, t6, t5
    muls.s32.rhs    t9, t6, t4
    mula.s32.rhs    t8, t7, t4
    lsli            t8, t8, 1
    mula.s32.rhs    t9, t7, t5
    lsli            t9, t9, 1

    stbi.w          t8, (a3)            // *pOut1++ = outR
    stbi.w          t9, (a3)            // *pOut1++ = outI

    addu            a2, a2, t2

.L12:
    subi            t1, t1, 8
    bloop           a1, .L11, .L12
.L10:

    rts
    .size           csky_split_rifft_yv_q31, .-csky_split_rifft_yv_q31
