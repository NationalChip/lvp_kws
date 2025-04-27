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
 * @file     csky_sin_cos_q31.S
 * @brief    Cosine & Sine calculation for Q31 values.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/
.import sinTable_q31

/*
 * void csky_sin_cos_q31(
 *   q31_t theta,
 *   q31_t * pSinVal,
 *   q31_t * pCosVal)
 */
    .file           "csky_sin_cos_q31.S"
    .section        .text.csky_sin_cos_q31,"ax",@progbits
    .align          2
    .global         csky_sin_cos_q31
    .type           csky_sin_cos_q31, @function
csky_sin_cos_q31:
    push            l0, l1
    lsri            a3, a0, 23          // indexS
    addi            t0, a3, 128
    andi            t0, t0, 0x1ff       // indexC
    lsli            t1, a3, 23
    subu            a0, a0, t1
    lsli            a0, a0, 8           // fract

    /* difference with c code, pSinVal first */
    lrw             t2, sinTable_q31
    lsli            t0, t0, 2
    addu            t3, t2, t0
    ldw             t0, (t3, 0x0)       // d1
    ldw             t1, (t3, 0x4)       // d2
    lsli            a3, a3, 2
    addu            t3, t2, a3
    ldw             t2, (t3, 0x0)       // f1
    ldw             t3, (t3, 0x4)       // f2

    subu            t7, t3, t2          // Df
    lrw             a3, 0x1921FB5       // Dn

    mul.s32         t4, t0, a3
    mula.s32        t4, t1, a3          // Dn * ((q63_t)d1 + d2)
    movi            t6, 0
    sub.64          t4, t4, t6          // temp - ((q63_t)Df << 32)

    dexti           t4, t4, t5, 31
    asri            t5, t5, 31          // temp >> 31
    mul.u32         t8, t4, a0          // fract * temp[31:0]
    movi            t4, 0
    mult            t5, t5, a0          // fract * temp[63:32]
    add.64          t4, t4, t8          // (q63_t)fract * (temp >> 31)

    movi            l0, 3
    mul.s32         t8, l0, t7          // 3 * df
    lsli            l0, t8, 31
    lsri            l1, t8, 1
    lsli            t9, t9, 31
    or              l1, l1, t9          // (3 * Df) << 31
    add.64          t4, t4, l0

    lsli            l0, t0, 1
    asri            l1, t0, 31          // d1 << 1
    mul.u32         t8, l0, a3          // d1[31:0] * Dn
    mult            l1, l1, a3          // d1[63:32] * Dn
    movi            l0, 0
    add.64          t8, t8, l0          // ((q63_t)d1 << 1) * Dn
    mula.s32        t8, t1, a3
    /* (temp + (3*(q63_t)Df << 31)) - (d2 + ((q63_t)d1 << 1))*Dn */
    sub.64          t4, t4, t8

    dexti           t4, t4, t5, 31
    asri            t5, t5, 31          // temp >> 31
    mul.u32         t8, t4, a0          // fract * temp[31:0]
    movi            t4, 0
    mult            t5, t5, a0          // fract * temp[63:32]
    add.64          t4, t4, t8          // (q63_t)fract * (temp >> 31)

    mul.s32         t8, t0, a3          // d1 * Dn
    add.64          t4, t4, t8

    dexti           t4, t4, t5, 31
    asri            t5, t5, 31          // temp >> 31
    mul.u32         t8, t4, a0          // fract * temp[31:0]
    movi            t4, 0
    mult            t5, t5, a0          // fract * temp[63:32]
    add.64          t4, t4, t8          // (q63_t)fract * (temp >> 31)

    dexti           t4, t4, t5, 31
    asri            t5, t5, 31          // temp >> 31
    mov             t8, t2
    asri            t9, t2, 31
    add.64          t4, t4, t8          // (temp >> 31) + (q63_t)f1

    dexti           t8, t4, t5, 31
    cmpne           t8, t5
    bf              .L0
    bgeni           t8, 31
    bmaski          t9, 31
    btsti           t5, 31
    movt            t4, t8              // 0x80000000
    movf            t4, t9              // 0x7fffffff
.L0:
    stw             t4, (a1, 0x0)       // pSinVal

    /* pCosVal */
    neg.s32.s       t2, t2              // d1
    neg.s32.s       t3, t3              // d2

    subu            t7, t1, t0          // Df
    lrw             a3, 0x1921FB5       // Dn

    mul.s32         t4, t2, a3
    mula.s32        t4, t3, a3          // Dn * ((q63_t)d1 + d2)
    movi            t6, 0
    sub.64          t4, t4, t6          // temp - ((q63_t)Df << 32)

    dexti           t4, t4, t5, 31
    asri            t5, t5, 31          // temp >> 31
    mul.u32         t8, t4, a0          // fract * temp[31:0]
    movi            t4, 0
    mult            t5, t5, a0          // fract * temp[63:32]
    add.64          t4, t4, t8          // (q63_t)fract * (temp >> 31)

    movi            l0, 3
    mul.s32         t8, l0, t7          // 3 * df
    lsli            l0, t8, 31
    lsri            l1, t8, 1
    lsli            t9, t9, 31
    or              l1, l1, t9          // (3 * Df) << 31
    add.64          t4, t4, l0

    lsli            l0, t2, 1
    asri            l1, t2, 31          // d1 << 1
    mul.u32         t8, l0, a3          // d1[31:0] * Dn
    movi            l0, 0
    mult            l1, l1, a3          // d1[63:32] * Dn
    add.64          t8, t8, l0          // ((q63_t)d1 << 1) * Dn
    mula.s32        t8, t3, a3
    /* (temp + (3*(q63_t)Df << 31)) - (d2 + ((q63_t)d1 << 1))*Dn */
    sub.64          t4, t4, t8

    dexti           t4, t4, t5, 31
    asri            t5, t5, 31          // temp >> 31
    mul.u32         t8, t4, a0          // fract * temp[31:0]
    movi            t4, 0
    mult            t5, t5, a0          // fract * temp[63:32]
    add.64          t4, t4, t8          // (q63_t)fract * (temp >> 31)

    mul.s32         t8, t2, a3          // d1 * Dn
    add.64          t4, t4, t8

    dexti           t4, t4, t5, 31
    asri            t5, t5, 31          // temp >> 31
    mul.u32         t8, t4, a0          // fract * temp[31:0]
    movi            t4, 0
    mult            t5, t5, a0          // fract * temp[63:32]
    add.64          t4, t4, t8          // (q63_t)fract * (temp >> 31)

    dexti           t4, t4, t5, 31
    asri            t5, t5, 31          // temp >> 31
    asri            t1, t0, 31
    add.64          t4, t4, t0          // (temp >> 31) + (q63_t)f1

    dexti           t8, t4, t5, 31
    cmpne           t8, t5
    bf              .L1
    bgeni           t8, 31
    bmaski          t9, 31
    btsti           t5, 31
    movt            t4, t8              // 0x80000000
    movf            t4, t9              // 0x7fffffff
.L1:
    stw             t4, (a2, 0x0)       // pCosVal

    pop             l0, l1
    .size           csky_sin_cos_q31, .-csky_sin_cos_q31

