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
 * @file     csky_conv_partial_fast_opt_q15.S
 * @brief    Fast partial convolution of Q15 sequences.
 * @version  V1.0
 * @date     01. Sep 2017
 ******************************************************************************/

/*
 * csky_status csky_conv_partial_fast_opt_q15(
 *   q15_t * pSrcA
 *   uint32_t srcALen,
 *   q15_t * pSrcB,
 *   uint32_t srcBLen
 *   q15_t * pDst,
 *   uint32_t firstIndex,
 *   uint32_t numPoints
 *   q15_t * pScratch1
 *   q15_t * pScratch2)
 */

.import             csky_fill_q15
.import             csky_copy_q15

    .file           "csky_conv_partial_fast_opt_q15.S"
    .section        .text.csky_conv_partial_fast_opt_q15,"ax",@progbits
    .align          2
    .global         csky_conv_partial_fast_opt_q15
    .type           csky_conv_partial_fast_opt_q15, @function

csky_conv_partial_fast_opt_q15:
    push            l0, l1, l2, l3, l4 ,l5, l6, l7, l8, l9, lr
    ld.w            l0, (sp, 0x30)      // load firstIndex
    ld.w            l1, (sp, 0x34)      // load numPoints
    addu            t0, l0, l1
    subi            t1, a3, 1
    addu            t3, a1, t1

    cmplt           t3, t0              // if(firstIndex + numPoints) > ((srcAlen + (srcBlen - 1u)
    bf              .L0
    movi            a0, 0
    subi            a0, a0, 1           // if branch
    br              .L26

.L0:                                    // else branch
    cmphs           a1, a3              // if(srcALen >= srcBLen)
    bt              .L1

    mov             l2, a0              // else branch
    mov             a0, a2
    mov             a2, l2

    mov             l2, a1
    mov             a1, a3
    mov             a3, l2

.L1:
    ld.w            l2, (sp, 0x3c)
    mov             l4, l2              // py = pScratch2
    subi            t1, a3, 1
    lsli            t9, t1, 1
    add             l5, l2, t9          // pSrc2
    mov             l3, a2              // px = pIn2

    lsri            t0, a3, 2           // k = srcBLen >> 2u
    bez             t0, .L4             // while(k > 0)

.L2:
    pldbi.d         t2, (l3)            // copy second  buffer in reversal manner
    asri            t4, t2, 16
    pkgll           t2, t4, t2
    subi            l5, l5, 2
    st.w            t2, (l5, 0x0)
    asri            t4, t3, 16
    pkgll           t3, t4, t3
    subi            l5, l5, 4
    st.w            t3, (l5, 0x0)

.L3:
    subi            l5, l5, 2
    bloop           t0, .L2, .L3

.L4:
    andi            t0, a3, 3           // k = srcBLen % 0x4u
    bez             t0, .L7             // while(k > 0u)

.L5:
    ldbi.hs         t2, (l3)            // *pScr2-- = *px++
    st.h            t2, (l5, 0x0)

.L6:
    subi            l5, l5, 2
    bloop           t0, .L5, .L6

.L7:
    ld.w            l2, (sp, 0x38)
    mov             l6, l2              // pScr1 = pScratch1
    mov             l7, a0              // protect the a0, a1 and a2
    mov             l8, a1
    mov             l9, a2
    mov             l5, a3

    movi            a0, 0               // call the function csky_fill_q15
    mov             a1, l6
    subi            a2, l5, 1
    bsr             csky_fill_q15
    subi            a2, l5, 1
    lsli            a2, a2, 1
    add             l6, l6, a2          // pScr1 += srcBLen - 1u

    mov             a0, l7              // call the function csky_copy_q15
    mov             a1, l6
    mov             a2, l8
    bsr             csky_copy_q15
    lsli            a2, l8, 1
    add             l6, l6, a2          // pSrc1 += srcALen

    movi            a0, 0               // call the function csky_fill_q15
    mov             a1, l6
    subi            a2, l5, 1
    bsr             csky_fill_q15
    subi            a2, l5, 1
    lsli            a2, a2, 1
    add             l6, l6, a2          // pScr1 += srcBLen - 1

    mov             a0, l7
    mov             a1, l8
    mov             a2, l9
    mov             a3, l5

    mov             a2, l4              // pIn2       = py
    lsli            t7, l0, 1
    add             l2, l2, t7          // pScratch1 += firstIndex
    ld.w            l0, (sp, 0x2c)
    add             l0, l0, t7          // pOut       = pDst + firstIndex

    lsri            t0, l1, 2           // blkCnt     = numPoints >> 2
    bez             t0, .L16            // while (blkCnt > 0)

.L8:
    mov             l5, l2              // pScr1 = pScratch1
    movi            t6, 0               // acc0  = 0
    movi            t7, 0               // acc1  = 0
    movi            t8, 0               // acc2  = 0
    movi            t9, 0               // acc3  = 0

    pldbi.d         t2, (l5)            // load four samples from scratch1 buffer
    lsri            t1, a3, 2           // tapCnt = srcBLen >> 2u
    bez             t1, .L11            // while(tapCnt > 0u)

.L9:
    pldbi.d         l6, (a2)            // load four samples form smaller buffer
    mulaca.s16.s    t6, t2, l6          // acc0
    mulaca.s16.s    t8, t3, l6          // acc2
    asri            t4, t2, 16
    pkgll           t4, t3, t4          // x3
    mulacax.s16.s   t7, t4, l6          // acc1
    ldbi.w          t2, (l5)            // x1
    mulaca.s16.s    t6, t3, l7          // acc0
    mulaca.s16.s    t8, t2, l7          // acc2
    asri            t4, t3, 16          // x3
    pkgll           t4, t2, t4
    mulacax.s16.s   t9, t4, l6          // acc3
    mulacax.s16.s   t7, t4, l7          // acc1
    ldbi.w          t3, (l5)            // x2
    asri            t4, t2, 16          // x3
    pkgll           t4, t3, t4

.L10:
    mulacax.s16.s   t9, t4, l7          // acc3
    bloop           t1, .L9, .L10

.L11:
    subi            l5, l5, 8           // pSrc1 -= 4u
    andi            t1, a3, 3           // tapCnt = srcBLen & 3u
    bez             t1, .L14            // while (tapCnt > 0u)

.L12:
    movi            l6, 0
    pldbi.d         t2, (l5)            // load four samples
    ldbi.hs         l6, (a2)
    mulaca.s16.s    t6, t2, l6          // acc0
    mulacax.s16.s   t7, t2, l6          // acc1
    mulaca.s16.s    t8, t3, l6          // acc2
    mulacax.s16.s   t9, t3, l6          // acc3

.L13:
    subi            l5, l5, 6           // pScr1 -= 3u
    bloop           t1, .L12, .L13

.L14:
    asri            t6, t6, 15          // process and store the results
    clipi.s32       t6, t6, 16
    asri            t7, t7, 15
    clipi.s32       t7, t7, 16
    pkgll           t6, t6, t7
    stbi.w          t6, (l0)
    asri            t8, t8, 15
    clipi.s32       t8, t8, 16
    asri            t9, t9, 15
    clipi.s32       t9, t9, 16
    pkgll           t8, t8, t9
    stbi.w          t8, (l0)
    mov             a2, l4              // pIn2       = py
    addi            l2, l2, 8           // pScratch1 += 4u

.L15:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L8

.L16:
    andi            t0, l1, 3           // blkCnt = numPoints & 0x3
    bez             t0, .L23            // while(blkCnt > 0)

.L17:
    mov             l5, l2              // pScr1 = pScratch1
    movi            t8, 0               // acc0  = 0

    lsri            t1, a3, 1           // tapCnt = srcBLen >> 1u
    bez             t1, .L20            // while(tapCnt > 0u)

.L18:
    ldbi.w          t2, (l5)
    ldbi.w          t3, (a2)

.L19:
    mulaca.s16.s    t8, t2, t3          // acc0 = __SMLAD(x1, y1, acc0)
    bloop           t1, .L18, .L19

.L20:
    andi            t1, a3, 1           // tapCnt = srcBLen & 1u
    bez             t1, .L23            // while(tapCnt > 0u)

.L21:
    ldbi.hs         t2, (l5)            // acc0 += (*pSrc1++ * *pIn2++)
    ldbi.hs         t3, (a2)

.L22:
    mulall.s16.s    t8, t2, t3
    bloop           t1, .L21, .L22

.L23:
    asri            t8, t8, 15
    clipi.s32       t8, t8, 16
    stbi.h          t8, (l0)

    mov             a2, l4              // pIn2       = py
    addi            l2, l2, 2           // pScratch1 += 1u

.L24:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L17

.L25:
    movi            a0, 0

.L26:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, lr
    .size           csky_conv_partial_fast_opt_q15, .-csky_conv_partial_fast_opt_q15
