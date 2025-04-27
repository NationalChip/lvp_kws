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
 * @file     csky_abs_max_q31.S
 * @brief    Compute q31 vector abs max value.
 * @version  V1.0
 * @date     01. Dec 2017
 ******************************************************************************/
/**
 * @ingroup groupMath
 */

/**
 * @addtogroup BasicAbsMax
 * @{
 */

/**
 * @brief Q31 vector absolute max value.
 * @param[in]       *pSrc points to the input buffer
 * @param[out]      *pDst points to the output buffer
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * The Q31 value -1 (0x80000000) will be saturated to the maximum allowable
 * positive value 0x7FFFFFFF.
 */

/**
 *
 * void csky_abs_max_q31(
 * q31_t  * pSrc,
 * q31_t  * pDst,
 * uint32_t blockSize)
 *
**/

    .file           "csky_abs_max_q31.S"
    .section        .text.csky_abs_max_q31,"ax",@progbits
    .align          2
    .global         csky_abs_max_q31
    .type           csky_abs_max_q31, @function

csky_abs_max_q31:
    lsri            a3, a2, 2           // blkCnt = blockSize >> 2u
    andi            a2, a2, 3           // blkCnt = blockSize % 0x4u
    movi            t8, 0
    bez             a3, .L2


.L4:
    pldbi.d         t2, (a0)            // load 4 q31_t data
    pldbi.d         t4, (a0)
    abs.s32.s       t2, t2
    abs.s32.s       t3, t3
    abs.s32.s       t4, t4
    abs.s32.s       t5, t5
    max.u32         t8, t8, t2
    max.u32         t8, t8, t3
    max.u32         t8, t8, t4
.L5:
    max.u32         t8, t8, t5

    bloop           a3, .L4, .L5        // while loop

.L2:
    bez             a2, .L0             // the condition to end

.L3:
    ldbi.w          t2, (a0)            // load 1 q31_t data
    abs.s32.s       t2, t2              // compute the abs value
.L1:
    max.u32         t8, t8, t2

    bloop           a2, .L3, .L1        // while loop

.L0:
    stbi.w          t8, (a1)
    rts                                 // end
    .size           csky_abs_max_q31, .-csky_abs_max_q31
