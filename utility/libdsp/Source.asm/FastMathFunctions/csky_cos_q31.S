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
 * @file     csky_cos_q31.S
 * @brief    Fast cosine calculation for Q31 values.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#define FAST_MATH_Q31_SHIFT 22
.import sinTable_q31

/*
 * q31_t csky_cos_q31(
 *   q31_t x)
 */

    .file           "csky_cos_q31.S"
    .section        .text.csky_cos_q31,"ax",@progbits
    .align          2
    .global         csky_cos_q31
    .type           csky_cos_q31, @function
csky_cos_q31:
    bgeni           a1, 29              // 0x20000000
    addu            a0, a0, a1
    btsti           a0, 31
    bf              .L0
    bgeni           a1, 31              // 0x80000000
    addu            a0, a0, a1
.L0:

    lsri            a1, a0, FAST_MATH_Q31_SHIFT
    lsli            a2, a1, FAST_MATH_Q31_SHIFT
    sub             a2, a0, a2
    lsli            a2, a2, 9           // fract

    lrw             a3, sinTable_q31
    lsli            a1, a1, 2
    addu            a3, a1
    pldbi.d         t0, (a3)

    /*
     * 0x80000000 is unsigned here
     * hence 0x800000000 = 0x1 + 0x7fffffff (signed)
     */
    bmaski          a3, 31              // 0x7fffffff
    subu            a0, a3, a2
    mul.s32         a0, a0, t0          // cosVal
    movi            a3, 1
    mula.s32        a0, a3, t0
    mula.s32.hs     a1, a2, t1          // cosVal

    lsli            a0, a1, 1
    rts
    .size           csky_cos_q31, .-csky_cos_q31
