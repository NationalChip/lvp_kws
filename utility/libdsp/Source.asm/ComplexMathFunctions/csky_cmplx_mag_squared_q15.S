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
 * @file     csky_cmplx_mag_squared_q15.S
 * @brief    Q15 complex magnitude squared.
 * @version  V1.0
 * @date     14. Aug 2017
 ******************************************************************************/

/*
 * void csky_cmplx_mag_squared_q15(
 *   q15_t * pSrc,
 *   q15_t * pDst,
 *   uint32_t numSamples)
 */

    .file           "csky_cmplx_mag_squared_q15.S"
    .section        .text.csky_cmplx_mag_squared_q15,"ax",@progbits
    .align          2
    .global         csky_cmplx_mag_squared_q15
    .type           csky_cmplx_mag_squared_q15, @function

csky_cmplx_mag_squared_q15:
    lsri            t0, a2, 2           //blkCnt = blockSize >> 2u
    andi            t1, a2, 3           //blkCnt = blockSize % 0x4u
    bez             t0, .L1

.L2:
    pldbi.d         t2, (a0)            //load 2 q15_t complex data form source
    mulca.s16.s     t4, t2, t2          //multiply and add
    asri            t4, t4, 17          //prepare for store
    stbi.h          t4, (a1)            //store
    mulca.s16.s     t4, t3, t3          //another data
    asri            t4, t4, 17
    stbi.h          t4, (a1)
  
    pldbi.d         t2, (a0)            //repeat
    mulca.s16.s     t4, t2, t2
    asri            t4, t4, 17
    stbi.h          t4, (a1)
    mulca.s16.s     t4, t3, t3
    asri            t4, t4, 17
.L3:
    stbi.h          t4, (a1)

    bloop           t0, .L2, .L3        //while loop

.L1:
    bez             t1, .L0             //condotion to end

.L4:
    ldbi.w          t2, (a0)            //load 1 q15_t complex data form source
    mulca.s16.s     t4, t2, t2          //multiply and add
    asri            t4, t4, 17          //prepare for store
.L5:
    stbi.h          t4, (a1)            //store the result

    bloop           t1, .L1, .L5        //while loop

.L0:
    rts                                 //end
    .size           csky_cmplx_mag_squared_q15, .-csky_cmplx_mag_squared_q15
