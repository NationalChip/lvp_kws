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
 * @file     csky_mult_rnd_q15.S
 * @brief    Q15 vector multiplication with round.
 * @version  V1.0
 * @date     01. Dec 2017
 ******************************************************************************/
/**
 * @ingroup groupMath
 */

/**
 * @addtogroup BasicMult
 * @{
 */

/**
 * @brief           Q15 vector multiplication with rounding
 * @param[in]       *pSrcA points to the first input vector
 * @param[in]       *pSrcB points to the second input vector
 * @param[out]      *pDst points to the output vector
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q15 range [0x8000 0x7FFF] will be
 * saturated. And before saturation, the result will be rounding towards +inf.
 */


/**
 *void csky_mult_rnd_q15(
 *q15_t * pSrcA,
 *q15_t * pSrcB,
 *q15_t * pDst,
 *uint32_t blockSize)
**/

    .file           "csky_mult_rnd_q15.S"
    .section        .text.csky_mult_rnd_q15,"ax",@progbits
    .align          2
    .global         csky_mult_rnd_q15
    .type           csky_mult_rnd_q15, @function

csky_mult_rnd_q15:
    lsri            t0, a3, 2           // blkCnt = blockSize >> 2u
    andi            t1, a3, 3           // blkCnt = blockSize % 0x04
    bez             t0, .L2

.L4:
    pldbi.d         t2, (a0)            // load 4 q15_t data from source a
    pldbi.d         t4, (a1)            // load 4 q15_t data from source b
    prmul.s16.rh    t6, t2, t4          // multiply
    prmul.s16.rh    t7, t3, t5
    st.w            t6, (a2, 0x0)       // store the results
    st.w            t7, (a2, 0x4)
.L5:
    addi            a2, a2, 8

    bloop           t0, .L4, .L5        // while loop

.L2:
    bez             t1, .L0             // condition to end

.L3:
    ldbi.h          t2, (a0)            // load 1 q15_t data from source a
    ldbi.h          t4, (a1)            // load 1 q15_t data from source b
    prmul.s16.rh    t6, t2, t4          // multiply
.L1:
    stbi.h          t6, (a2)            // store the result

    bloop           t1, .L3, .L1        // while loop

.L0:
    rts                                 // end

    .size           csky_mult_rnd_q15, .-csky_mult_rnd_q15
