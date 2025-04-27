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
 * @file     csky_conv_opt_q15.S
 * @brief    Convolution of Q15 sequences.
 * @version  V1.0
 * @date     29. Aug 2017
 ******************************************************************************/

/*
 * void csky_conv_opt_q15(
 *   q15_t * pSrcA
 *   uint32_t srcALen,
 *   q15_t * pSrcB,
 *   uint32_t srcBLen
 *   q15_t * pDst
 *   q15_t * pScratch1,
 *   q15_t * pScratch2)
 */
.import             csky_fill_q15
.import             csky_copy_q15

    .file           "csky_conv_opt_q15.S"
    .section        .text.csky_conv_opt_q15,"ax",@progbits
    .align          2
    .global         csky_conv_opt_q15
    .type           csky_conv_opt_q15, @function

csky_conv_opt_q15:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, lr
    ld.w            l0, (sp, 0x2c)      // *pOut = pDst
    cmphs           a1, a3              // if(srcALen >= srcBLen)
    bt              .L0

    mov             l4, a0              // else branch
    mov             a0, a2
    mov             a2, l4

    mov             l4, a1
    mov             a1, a3
    mov             a3, l4

.L0:
    ld.w            l6, (sp, 0x34)      // pSrc2
    subi            l7, a3, 1
    lsli            l8, l7, 1
    add             l6, l6, l8
    mov             l4, a2              // px = pIn2
    lsri            t0, a3, 2           // k  = srcBLen >> 2u
    bez             t0, .L3             // while(k > 0u)

.L1:
    pldbi.d         t2, (l4)            // copy second buffer in reversal manner
    subi            l6, 2
    dup.16          t4, t2, 1
    pkgll           t2, t4, t2
    st.w            t2, (l6, 0x0)
    subi            l6, l6, 4
    dup.16          t4, t3, 1
    pkgll           t3, t4, t3
    st.w            t3, (l6, 0x0)

.L2:
    subi            l6, l6, 2
    bloop           t0, .L1, .L2        // while loop

.L3:
    andi            t0, a3, 3           // k = srcBLen % 0x4u
    bez             t0, .L6             // while(k > 0u)

.L4:
    ldbi.hs         t2, (l4)            // *pScr2-- = *px++
    st.h            t2, (l6, 0x0)

.L5:
    subi            l6, l6, 2
    bloop           t0, .L4, .L5        // while loop

.L6:
    mov             l1, a0
    mov             l2, a1
    mov             l3, a2
    mov             l4, a3

    ld.w            l6, (sp, 0x30)      // pScr1 = pScratch1
    movi            a0, 0               // call the function csky_fill_q15
    mov             a1, l6
    subi            a2, l4, 1
    bsr             csky_fill_q15
    lsli            a2, l7, 1
    add             l6, l6, a2          // pScr1 += (srcBLen - 1u)

    mov             a0, l1              // call the function csky_copy_q15
    mov             a1, l6
    mov             a2, l2
    bsr             csky_copy_q15
    lsli            a2, l2, 1
    add             l6, l6, a2          // pScr1 += srcALen

    movi            a0, 0               // call the function csky_fill_q15
    mov             a1, l6
    mov             a2, l7
    bsr             csky_fill_q15
    lsli            a2, l7, 1
    add             l6, l6, a2          // pScr1 += (srcBLen - 1u)

    mov             a0, l1
    mov             a1, l2
    mov             a2, l3
    mov             a3, l4

    ld.w            l5, (sp, 0x34)      // py     = pScratch2
    mov             a2, l5              // pIn2   = py
    add             l7, l7, a1
    lsri            l1, l7, 2           // blkCnt = (srcALen + srcBLen - 1u) >> 2
    ld.w            l7, (sp, 0x30)
    bez             l1, .L15            // while(blkCnt > 0u)

.L7:
    mov             l6, l7
    movi            t2, 0               // acc0   = 0
    movi            t3, 0
    movi            t4, 0               // acc1   = 0
    movi            t5, 0
    movi            t6, 0               // acc2   = 0
    movi            t7, 0
    movi            t8, 0               // acc3   = 0
    movi            t9, 0

    pldbi.d         l8, (l6)            // read four samples from pScratch1 buffer
    lsri            l2, a3, 2           // tapCnt = (srcBLen) >> 2u
    bez             l2, .L10            // while(tapCnt > 0u)

.L8:
    pldbi.d         t0, (a2)            // read four samples from smaller buffer
    mulaca.s16.e    t2, l8, t0          // acc0 = __SMLAD(x1, y1, acc0)
    mulaca.s16.e    t6, l9, t0          // acc2 = __SMLAD(x2, y1, acc2)
    asri            l3, l8, 16          // x3   = __PKHBT(x1, x2, 0)
    pkgll           l4, l9, l3
    mulacax.s16.e   t4, l4, t0          // acc1 = __SMLADX(x3, y1, acc1)
    ldbi.w          l8, (l6)            // x1   = _SIMD32_OFFSET(pScr1)
    mulaca.s16.e    t2, l9, t1          // acc0 = __SMLAD(x2, y2, acc0)
    mulaca.s16.e    t6, l8, t1          // acc2 = __SMLAD(x1, y2, acc2)
    asri            l3, l9, 16          // x3   = __PKHBT(x1, x2, 0)
    pkgll           l4, l8, l3
    mulacax.s16.e   t8, l4, t0          // acc3 = __SMLADX(x3, y1, acc3)
    mulacax.s16.e   t4, l4, t1          // acc1 = __SMLADX(x3, y2, acc1)
    ldbi.w          l9, (l6)            // x2   = _SIMD32_OFFSET(pSrc1 + 2u)
    asri            l3, l8, 16          // x3   = __PKHBT(x2, x1, 0)
    pkgll           l4, l9, l3

.L9:
    mulacax.s16.e   t8, l4, t1          // acc3 = __SMLADX(x3, y2, acc3)
    bloop           l2, .L8, .L9        // while loop

.L10:
    subi            l6, l6, 8           // pScr1 -= 4u
    andi            l2, a3, 3           // tapCnt = (srcBLen) & 3u
    bez             l2, .L13            // while(tapCnt > 0u)

.L11:
    movi            t0, 0
    ldbi.hs         t0, (a2)            // perform the multiply and accumulate
    pldbi.d         l8, (l6)
    mulaca.s16.e    t2, l8, t0
    mulacax.s16.e   t4, l8, t0
    mulaca.s16.e    t6, l9, t0
    mulacax.s16.e   t8, l9, t0

.L12:
    subi            l6, l6, 6           // pSrc1 -= 3u
    bloop           l2, .L11, .L12      // while loop

.L13:
    dexti           t2, t2, t3, 15      // process and store the results
    clipi.s32       t2, t2, 16
    dexti           t4, t4, t5, 15
    clipi.s32       t4, t4, 16
    pkgll           t2, t2, t4
    stbi.w          t2, (l0)
    dexti           t6, t6, t7, 15
    clipi.s32       t6, t6, 16
    dexti           t8, t8, t9, 15
    clipi.s32       t8, t8, 16
    pkgll           t6, t6, t8
    stbi.w          t6, (l0)

    mov             a2, l5              // pIn2       = py
    addi            l7, l7, 8           // pScratch1 += 4u

.L14:
    subi            l1, l1, 1           // blkCnt--
    bhz             l1, .L7             // while loop

.L15:
    subi            l8, a3, 1
    add             l8, a1, l8
    andi            l1, l8, 3           // blkCnt
    bez             l1, .L24            // while(blkCnt > 0u)

.L16:
    mov             l6, l7              // pSrc1 = pScratch1
    movi            t8, 0               // acc0  = 0
    movi            t9, 0
    lsri            l2, a3, 1           // tapCnt = srcBLen >> 1u
    bez             l2, .L19            // while(tapCnt > 0u)

.L17:
    ldbi.w          t2, (l6)            // two samples multiply and accumulate
    ldbi.w          t3, (a2)

.L18:
    mulaca.s16.e    t8, t2, t3
    bloop           l2, .L17, .L18      // while loop

.L19:
    andi            l2, a3, 1           // tapCnt = srcBLen & 1u
    bez             l2, .L22            // while(tapCnt > 0u)

.L20:
    ldbi.hs         t2, (l6)            // acc0 += (*pSrc1 ++ * *pIn2++)
    ldbi.hs         t3, (a2)

.L21:
    mulall.s16.e    t8, t2, t3
    bloop           l2, .L20, .L21      // while loop

.L22:
    dexti           t8, t8, t9, 15      // process and store the result
    clipi.s32       t8, t8, 16
    stbi.h          t8, (l0)
    mov             a2, l5              // pIn2       = py
    addi            l7, l7, 2           // pScratch1 += 1u

.L23:
    subi            l1, l1, 1           // blkCnt--
    bhz             l1, .L16            // while loop

.L24:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, lr
    .size           csky_conv_opt_q15, .-csky_conv_opt_q15
