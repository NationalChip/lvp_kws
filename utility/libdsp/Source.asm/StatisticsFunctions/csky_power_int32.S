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
 * @file     csky_power_q31.S
 * @brief    Sum of the squares of the elements of a Q31 vector.
 * @version  V1.0
 * @date     01. Dec 2017
 ******************************************************************************/
/**
 * @ingroup groupStats
 */

/**
 * @addtogroup power
 * @{
 */

/**
 * @brief Sum of the squares of the elements of a int32 vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult sum of the squares value returned here
 * @return none.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * The function is implemented using a 64-bit internal accumulator.
 * The input is represented in 32-bit integer format. Intermediate
 * multiplication yields 64-bit integer format, and this result is
 * added with saturation to a 64-bit accumulator. And the result
 * is between[0x0, 0x7fffffffffffffff] in 64-bit integer format.
 * So be careful to use it to avoid overflow.
 *
 */

/*
 * void csky_power_int32(
 *   int32 * pSrc,
 *   uint32_t blockSize,
 *   q63_t * pResult)
 */

    .file           "csky_power_int32.S"
    .section        .text.csky_power_int32,"ax",@progbits
    .align          2
    .global         csky_power_int32
    .type           csky_power_int32, @function

csky_power_int32:
    movi            t0, 0
    movi            t1, 0               // sum
    lsri            a3, a1, 2           // blkCnt
    bez             a3, .L0
.L1:
    pldbi.d         t4, (a0)
    pldbi.d         t6, (a0)
    mula.s32.s      t0, t4, t4
    mula.s32.s      t0, t5, t5
    mula.s32.s      t0, t6, t6
.L2:
    mula.s32.s      t0, t7, t7

    bloop           a3, .L1, .L2
.L0:

    andi            a1, a1, 3
    bez             a1, .L3
.L4:
    ldbi.w          t4, (a0)
.L5:
    mula.s32.s      t0, t4, t4

    bloop           a1, .L4, .L5

.L3:
    st.w            t0, (a2, 0x0)
    st.w            t1, (a2, 0x4)
    rts
    .size           csky_power_int32, .-csky_power_int32
