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
 * @file     csky_cmplx_dot_prod_q15.S
 * @brief    Q15 complex conjugate.
 * @version  V1.0
 * @date     10. Aug 2017
 ******************************************************************************/

/*
 * void csky_cmplx_dot_prod_q15(
 *   q15_t * pSrcA,
 *   q15_t * pDstB,
 *   uint32_t numSamples
 *   q31_t * realResult,
 *   q31_t * imagResult)
 */

    .file           "csky_cmplx_dot_prod_q15.S"
    .section        .text.csky_cmplx_dot_prod_q15,"ax",@progbits
    .align          2
    .global         csky_cmplx_dot_prod_q15
    .type           csky_cmplx_dot_prod_q15, @function

csky_cmplx_dot_prod_q15:
    ld.w            t0, (sp, 0x0)
    lsri            t1, a2, 2           //blkCnt = blockSize >> 2u   
    andi            a2, a2, 3           //nlkCnt = blockSize % 0x4u
    movi            t2, 0               //real_sum = 0
    movi            t3, 0
    movi            t4, 0               //imag_sum = 0
    movi            t5, 0
    bez             t1, .L1

.L2:
    pldbi.d         t6, (a0)            //load 2 q15_t complex deta from source a
    pldbi.d         t8, (a1)            //load 2 q15_t complex data from source b
    mulacs.s16.e    t2, t6, t8          //real
    mulacs.s16.e    t2, t7, t9
    mulacax.s16.e   t4, t6, t8          //image
    mulacax.s16.e   t4, t7, t9

    pldbi.d         t6, (a0)            //repeat
    pldbi.d         t8, (a1)
    mulacs.s16.e    t2, t6, t8
    mulacs.s16.e    t2, t7, t9
    mulacax.s16.e   t4, t6, t8
.L3:
    mulacax.s16.e   t4, t7, t9

    bloop           t1, .L2, .L3        //while loop

.L1:
    bez             a2, .L0             //condition to end

.L4:
    ldbi.w          t6, (a0)            //load 1 q15_t complex data from source a
    ldbi.w          t8, (a1)            //load 1 q15_t complex data from source b
    mulacs.s16.e    t2, t6, t8          //real
.L5:
    mulacax.s16.e   t4, t6, t8          //image

    bloop           a2, .L4, .L5        //while loop

.L0:
    dexti           t6, t2, t3, 6       //prepare for store
    dexti           t7, t4, t5, 6
    st.w            t6, (a3, 0x0)       //store the result
    st.w            t7, (t0, 0x0)
    rts                                 //end
    .size           csky_cmplx_dot_prod_q15, .-csky_cmplx_dot_prod_q15
