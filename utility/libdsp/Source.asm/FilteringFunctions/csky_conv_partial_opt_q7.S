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
 * @file     csky_conv_partial_opt_q7.S
 * @brief    Partial convolution of Q7 sequences.
 * @version  V1.0
 * @date     31. Aug 2017
 ******************************************************************************/

/*
 * csky_status csky_conv_partial_opt_q7(
 *   q7_t * pSrcA
 *   uint32_t srcALen,
 *   q7_t * pSrcB,
 *   uint32_t srcBLen
 *   q7_t * pDst,
 *   uint32_t firstIndex,
 *   uint32_t numPoints
 *   q7_t * pScratch1,
 *   q7_t * pScratch2)
 */

.import             csky_fill_q15

    .file           "csky_conv_partial_opt_q7.S"
    .section        .text.csky_conv_partial_opt_q7,"ax",@progbits
    .align          2
    .global         csky_conv_partial_opt_q7
    .type           csky_conv_partial_opt_q7, @function

csky_conv_partial_opt_q7:
    push            l0, l1, l2, l3, l4 ,l5, l6, l7, l8, l9, lr
    ld.w            l0, (sp, 0x30)      // load firstIndex
    ld.w            l1, (sp, 0x34)      // load numPoints
    add             t0, l0, l1
    subi            t1, a3, 1
    add             t3, a1, t1

    cmplt           t3, t0              // if(firstIndex + numPoints) > ((srcAlen + (srcBlen - 1u)
    bf              .L0
    movi            a0, 0               // if branch
    subi            a0, a0, 1
    br              .L31

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
    ld.w            l3, (sp, 0x3c)
    mov             l2, l3              // pScr2 = pScratch2
    subi            t1, a3, 1
    add             l4, a2, t1          // px    = pIn2 + srcBLen - 1u
    lsri            t0, a3, 2           // k     = srcBLen >> 2u
    bez             t0, .L4             // while(k > 0)

.L2:
    subi            l4, l4, 3           // load four samples from the second buffer
    ld.w            t2, (l4, 0x0)
    subi            l4, l4, 1
    pext.s8.e       t4, t2              // process and store in reversal manner
    asri            t6, t4, 16
    pkgll           t4, t6, t4
    asri            t6, t5, 16
    pkgll           t5, t6, t5
    stbi.w          t5, (l2)

.L3:
    stbi.w          t4, (l2)
    bloop           t0, .L2, .L3

.L4:
    andi            t0, a3, 3           // k = srcBLen % 0x4u
    bez             t0, .L7             // while(k > 0u)

.L5:
    ld.bs           t2, (l4, 0x0)       // x4       = *px--
    sextb           t2, t2
    subi            l4, l4, 1

.L6:
    stbi.h          t2, (l2)            // *pScr2++ = x4
    bloop           t0, .L5, .L6

.L7:
    ld.w            l6, (sp, 0x38)
    mov             l5, l6              // pSrc1 = pScratch1
    mov             l7, a0              // protect a0, a1 and a2, a3
    mov             l8, a1
    mov             l9, a2
    mov             l2, a3

    movi            a0, 0               // call the function csky_fill_q15
    mov             a1, l5
    subi            a2, l2, 1
    bsr             csky_fill_q15
    subi            a2, l2, 1
    lsli            a2, a2, 1
    add             l5, l5, a2          // pSrc1 += srcBLen - 1

    lsri            t0, l8, 2           // k      = srcALen >> 2u
    bez             t0, .L10            // while(k > 0u)

.L8:
    ldbi.w          t2, (l7)            // copy four samples per time
    pext.s8.e       t4, t2
    stbi.w          t4, (l5)

.L9:
    stbi.w          t5, (l5)
    bloop           t0, .L8, .L9

.L10:
    andi            t0, l8, 3           // k = srcALen % 0x4u
    bez             t0, .L13            // while(k > 0)

.L11:
    ldbi.bs         t2, (l7)
    sextb           t2, t2

.L12:
    stbi.h          t2, (l5)
    bloop           t0, .L11, .L12

.L13:
    movi            a0, 0               // call the function csky_fill_q15
    mov             a1, l5
    subi            a2, l2, 1
    bsr             csky_fill_q15
    subi            a2, l2, 1
    lsli            a2, a2, 1
    add             l5, l5, a2

    mov             a0, l7
    mov             a1, l8
    mov             a2, l9
    mov             a3, l2

    mov             l7, l3              // py         = pScratch2
    mov             a2, l7              // pIn2       = py
    mov             l2, l7              // pScr2      = py
    ld.w            l9, (sp, 0x2c)
    add             l9, l9, l0          // pOut       = pDst + firstIndex
    lsli            l0, l0, 1 
    add             l6, l6, l0          // pScratch1 += firstIndex

    lsri            t0, l1, 2           // blkCnt     = numPoints >> 2
    bez             t0, .L22            // while(blkCnt > 0)

.L14:
    mov             l5, l6              // pScr1      = pScratch1
    movi            t6, 0               // acc0       = 0
    movi            t7, 0               // acc1       = 0
    movi            t8, 0               // acc2       = 0
    movi            t9, 0               // acc3       = 0

    pldbi.d         t2, (l5)            // load four samples from scratch buffer
    lsri            t1, a3, 2           // tapCnt = srcBLen >> 2u
    bez             t1, .L17            // while(tapCnt > 0u)

.L15:
    pldbi.d         t4, (l2)            // load four samples from smaller buffer
    mulaca.s16.s    t6, t2, t4          // acc0
    mulaca.s16.s    t8, t3, t4          // acc2
    asri            l8, t2, 16          // x3
    pkgll           l8, t3, l8
    mulacax.s16.s   t7, l8, t4          // acc1
    ldbi.w          t2, (l5)            // x1
    asri            l8, t3, 16          // x3
    pkgll           l8, t2, l8
    mulacax.s16.s   t9, l8, t4          // acc3
    mulaca.s16.s    t6, t3, t5          // acc0
    mulaca.s16.s    t8, t2, t5          // acc2
    mulacax.s16.s   t7, l8, t5          // acc1
    ldbi.w          t3, (l5)            // x2
    asri            l8, t2, 16          // x3
    pkgll           l8, t3, l8

.L16:
    mulacax.s16.s   t9, l8, t5          // acc3
    bloop           t1, .L15, .L16

.L17:
    subi            l5, l5, 8           // pSrc1 -= 4u
    andi            t1, a3, 3           // tapCnt = srcBLen & 3u
    bez             t1, .L20            // while(tapCnt > 0u)

.L18:
    movi            t4, 0
    pldbi.d         t2, (l5)            // load four samples
    ldbi.hs         t4, (l2)
    mulaca.s16.s    t6, t2, t4          // acc0
    mulacax.s16.s   t7, t2, t4          // acc1
    mulaca.s16.s    t8, t3, t4          // acc2
    mulacax.s16.s   t9, t3, t4          // acc3

.L19:
    subi            l5, l5, 6           // pSrc1 -= 3u
    bloop           t1, .L18, .L19

.L20:
    asri            t6, t6, 7           // process and store the results
    asri            t7, t7, 7
    asri            t8, t8, 7
    asri            t9, t9, 7
    clipi.s32       t6, t6, 8
    clipi.s32       t7, t7, 8
    clipi.s32       t8, t8, 8
    clipi.s32       t9, t9, 8
    pkgll           t6, t6, t7
    pkgll           t8, t8, t9
    narl            t6, t6, t8
    stbi.w          t6, (l9)
    mov             l2, l7              // pSrc2      = py
    addi            l6, l6, 8           // pScratch1 += 4u

.L21:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L14

.L22:
    andi            t0, l1, 3           // blkCnt = numPoints & 3u
    bez             t0, .L30            // while(blkCnt > 0)

.L23:
    mov             l5, l6              // pScr1  = pScratch1
    movi            t8, 0               // acc0   = 0
    lsri            t1, a3, 1           // tapCnt = srcBLen >> 1u
    bez             t1, .L26            // while(tapCnt > 0)

.L24:
    ldbi.w          t2, (l5)            // x1
    ldbi.w          t3, (l2)            // y1

.L25:
    mulaca.s16.s    t8, t2, t3          // acc0
    bloop           t1, .L24, .L25

.L26:
    andi            t1, a3, 1           // tapCnt = srcBLen & 1u
    bez             t1, .L32            // while(tapCnt > 0u)

.L27:
    ldbi.hs         t2, (l5)
    ldbi.hs         t3, (l2)

.L28:
    mulall.s16.s    t8, t2, t3          // acc0
    bloop           t1, .L27, .L28

.L32:
    asri            t8, t8, 7
    clipi.s32       t8, t8, 8
    stbi.b          t8, (l9)
    mov             l2, l7              // pSrc2     = py
    addi            l6, l6, 2           // pSrcatch += 1u

.L29:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L23

.L30:
    movi            a0, 0

.L31:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, lr
    .size           csky_conv_partial_opt_q7, .-csky_conv_partial_opt_q7
