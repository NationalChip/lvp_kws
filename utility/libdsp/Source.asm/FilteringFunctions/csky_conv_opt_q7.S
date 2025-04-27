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
 * @file     csky_conv_opt_q7.S
 * @brief    Convolution of Q7 sequences.
 * @version  V1.0
 * @date     29. Aug 2017
 ******************************************************************************/

/*
 * void csky_conv_opt_q7(
 *   q7_t * pSrcA
 *   uint32_t srcALen,
 *   q7_t * pSrcB,
 *   uint32_t srcBLen
 *   q7_t * pDst
 *   q7_t * pScratch1,
 *   q7_t * pScratch2)
 */
.import             csky_fill_q15

    .file           "csky_conv_opt_q7.S"
    .section        .text.csky_conv_opt_q7,"ax",@progbits
    .align          2
    .global         csky_conv_opt_q7
    .type           csky_conv_opt_q7, @function

csky_conv_opt_q7:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, lr
    ld.w            l0, (sp, 0x2c)      // *pOut = pDst
    cmphs           a1, a3              // if(asrALen >= srcBLen)
    bt              .L0

    mov             l4, a0              // else branch
    mov             a0, a2
    mov             a2, l4

    mov             l4, a1
    mov             a1, a3
    mov             a3, l4

.L0:
    ld.w            l5, (sp, 0x34)      // pScr2 = pScratch2
    subi            l7, a3, 1
    add             l4, a2, l7          // px    = pIn2 + srcBLen - 1
    lsri            t0, a3, 2           // k     = srcBLen >> 2u
    bez             t0, .L3             // while(k > 0u)

.L1:
    subi            l4, 3
    ld.w            t2, (l4, 0x0)       // copy second buffer in reversal manner(four samples)
    subi            l4, 1
    pext.s8.e       t8, t2
    asri            t7, t9, 16
    pkgll           t9, t7, t9
    asri            t7, t8, 16
    pkgll           t8, t7, t8
    stbi.w          t9, (l5)

.L2:
    stbi.w          t8, (l5)
    bloop           t0, .L1, .L2        // while loop

.L3:
    andi            t0, a3, 3           // k = srcBLen % 0x4u
    bez             t0, .L6             // while(k > 0u)

.L4:
    ld.bs           t8, (l4, 0x0)       // copy second buffer in reversal manner(one sample)
    sextb           t8, t8
    subi            l4, l4, 1

.L5:
    stbi.h          t8, (l5)
    bloop           t0, .L4, .L5        // while loop

.L6:
    mov             l1, a0
    mov             l2, a1
    mov             l3, a2
    mov             l4, a3

    ld.w            l9, (sp, 0x30)      // pScr1  = pScratch1
    movi            a0, 0               // call the function csky_fill_q15
    mov             a1, l9
    subi            a2, a3, 1
    bsr             csky_fill_q15
    lsli            a2, l7, 1
    add             l5, l9, a2          // pSrc1 += (srcBLen - 1u)

    lsri            t0, l2, 2           // k      = srcAlen >> 2u
    bez             t0, .L9             // while (k > 0u)

.L7:
    ldbi.w          t2, (l1)            // copy second buffer in reversal manner (four samples)
    pext.s8.e       t8, t2
    stbi.w          t8, (l5)

.L8:
    stbi.w          t9, (l5)
    bloop           t0, .L7, .L8        // while loop

.L9:
    andi            t0, l2, 3           // k = srcALen % 0x4u
    bez             t0, .L12            // while(k > 0u)

.L10:
    ldbi.bs         t2, (l1)            // copy second buffer in reversal manner(one sample)
    sextb           t2, t2

.L11:
    stbi.h          t2, (l5)
    bloop           t0, .L10, .L11      // while loop

.L12:
    movi            a0, 0               // call the function csky_fill_q15
    mov             a1, l5
    mov             a2, l7
    bsr             csky_fill_q15
    lsli            a2, l7, 1
    add             l5, l5, a2          // pScr1 += (secBLen - 1u)

    mov             a0, l1
    mov             a1, l2
    mov             a2, l3
    mov             a3, l4

    ld.w            l4, (sp, 0x34)      // py    = pScratch2
    mov             a2, l4              // pIn2  = (q7_t*)py
    mov             l6, l4              // pScr2 = py
    add             l1, a1, l7
    lsri            t0, l1, 2           // blkCnt
    bez             t0, .L20            // while(blkCnt > 0)

.L13:
    mov             l5, l9              // pScr1 = pScratch1
    movi            t6, 0               // acc0  = 0
    movi            t7, 0               // acc1  = 0
    movi            t8, 0               // acc2  = 0
    movi            t9, 0               // acc3  = 0

    pldbi.d         t2, (l5)            // load four samples from scratch1 buffer
    lsri            t1, a3, 2           // tapCnt = srcBLen >> 2u
    bez             t1, .L16            // while(tapCnt > 0u)

.L14:
    pldbi.d         t4, (l6)            // load four samples from smaller buffer
    mulaca.s16.s    t6, t2, t4          // acc0 = __SMLAD(x1, y1, acc0)
    mulaca.s16.s    t8, t3, t4          // acc2 = __SMLAD(x2, y1, acc2)
    asri            l2, t2, 16          // x3   = __PKHBT(x2, x1, 0)
    pkgll           l3, t3, l2
    mulacax.s16.s   t7, l3, t4          // acc1 = __SMLADX(x3, y1, acc1)
    ldbi.w          t2, (l5)            // x1   = *__SIMD32(pScr1)++
    asri            l2, t3, 16          // x3   = __PKHBT(x1, x2, 0)
    pkgll           l3, t2, l2
    mulacax.s16.s   t9, l3, t4          // acc3 = __SMLADX(x3, y1, acc3)
    mulaca.s16.s    t6, t3, t5          // acc0 = __SMLAD(x2, y2, acc0)
    mulaca.s16.s    t8, t2, t5          // acc2 = __SMLAD(x1, y2, acc2)
    mulacax.s16.s   t7, l3, t5          // acc1 = __SMLADX(x3, y2, acc1)
    ldbi.w          t3, (l5)            // x2   = *__SIMD21(pSrc1)++
    asri            l2, t2, 16          // x3   = __PKHBT(x2, x1, 0)
    pkgll           l3, t3, l2

.L15:
    mulacax.s16.s   t9, l3, t5          // acc3 = __SMLADX(x3, y2, acc3)
    bloop           t1, .L14, .L15      // while loop

.L16:
    subi            l5, l5, 8           // pScr1 -= 4u
    andi            t1, a3, 3           // tapCnt = srcBLen & 3u
    bez             t1, .L19            // while (tapCnt > 0u)

.L17:
    movi            t4, 0
    pldbi.d         t2, (l5)            // perform the multiply and accumulate(four samples)
    ldbi.hs         t4, (l6)
    mulaca.s16.s    t6, t2, t4
    mulacax.s16.s   t7, t2, t4
    mulaca.s16.s    t8, t3, t4
    mulacax.s16.s   t9, t3, t4

.L18:
    subi            l5, l5, 6           // pScr1 -= 3u
    bloop           t1, .L17, .L18      // while loop

.L19:
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
    stbi.w          t6, (l0)

    mov             l6, l4              // pScr2      = py
    addi            l9, l9, 8           // pScratch1 += 4u

.L30:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L13            // while loop

.L20:
    andi            t0, l1, 3           // blkCnt
    bez             t0, .L29            // while(blkCnt > 0u)

.L21:
    mov             l5, l9              // pScr1 = pScratch1
    movi            t6, 0               // acc0  = 0
    lsri            t1, a3, 1           // tapCnt = srcBLen >> 1u
    bez             t1, .L24            // while(tapCnt > 0u)

.L22:
    ldbi.w          t2, (l5)            // perform multiply and accumulate(two samples)
    ldbi.w          t3, (l6)

.L23:
    mulaca.s16.s    t6, t2, t3
    bloop           t1, .L22, .L23      // while(tapCnt > 0u)

.L24:
    andi            t1, a3, 1           // tapCnt = srcBLen & 1u
    bez             t1, .L27            // while(tapCnt > 0u)

.L25:
    ldbi.hs         t2, (l5)            // perform ,ultiply and accumulate(one sample)
    ldbi.hs         t3, (l6)

.L26:
    mulall.s16.s    t6, t2, t3
    bloop           t1, .L25, .L26      // while loop

.L27:
    asri            t6, t6, 7           // process and store the result
    clipi.s32       t6, t6, 8
    stbi.b          t6, (l0)

    mov             l6, l4              // pScr2      = py
    addi            l9, l9, 2           // pScratch1 += 1u

.L28:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L21            // while loop

.L29:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, lr
    .size           csky_conv_opt_q7, .-csky_conv_opt_q7
