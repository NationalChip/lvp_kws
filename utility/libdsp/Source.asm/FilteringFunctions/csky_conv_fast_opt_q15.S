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
 * @file     csky_conv_fast_opt_q15.S
 * @brief    Fast Q15 Convolution.
 * @version  V1.0
 * @date     24. Aug 2017
 ******************************************************************************/

/*
 * void csky_conv_fast_opt_q15(
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

    .file           "csky_conv_fast_opt_q15.S"
    .section        .text.csky_conv_fast_opt_q15,"ax",@progbits
    .align          2
    .global         csky_conv_fast_opt_q15
    .type           csky_conv_fast_opt_q15, @function

csky_conv_fast_opt_q15:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, lr
    ld.w            l7, (sp, 0x2c)      // *pOut = pDst
    cmphs           a1, a3              // if(srcAlen >= srcBlen)
    bt              .L0

    mov             l0, a1              // else branch
    mov             a1, a3
    mov             a3, l0

    mov             l0, a0
    mov             a0, a2
    mov             a2, l0

.L0:
    ld.w            l0, (sp, 0x34)      // load *pScratch2
    subi            l1, a3, 1
    lsli            l1, l1, 1
    add             l0, l1, l0          // pSrc2 = pScratch2
    mov             l1, a2              // px    = pIn2
    lsri            l2, a3, 2           // k     = srcBLen >> 2u
    bez             l2, .L3             // while(k > 0u )

.L1:
    pldbi.d         t0, (l1)            // copy second buffer in reserval manner
    subi            l0, 2
    dup.16          t4, t0, 1
    pkgll           t3, t4, t0
    st.w            t3, (l0, 0x0)
    subi            l0, l0, 4
    dup.16          t4, t1, 1
    pkgll           t3, t4, t1
    st.w            t3, (l0, 0x0)

.L2:
    subi            l0, l0, 2
    bloop           l2, .L1, .L2        // while loop

.L3:
    andi            l9, a3, 3           // k = srcBLen % 0x4u
    bez             l9, .L6             // while(k > 0u)

.L4:
    ldbi.hs         t0, (l1)            // *pSrc2-- = *px++
    st.h            t0, (l0, 0x0)

.L5:
    subi            l0, l0, 2
    bloop           l9, .L4, .L5        // while loop

.L6:
    mov             l8, a3              // protect a0, a1, a2 and a3
    mov             l4, a0
    mov             l5, a1
    mov             l6, a2

    ld.w            l0, (sp, 0x30)      // pScr1  = pScratch1
    subi            l9, a3, 1           // srcBLen - 1u
    movi            a0, 0
    mov             a1, l0
    mov             a2, l9
    bsr             csky_fill_q15       // call the function csky_fill_q15
    lsli            l9, l9, 1
    add             l3, l0, l9          // pScr1 += (srcBLen - 1u)

    mov             a0, l4
    mov             a1, l3
    mov             a2, l5
    bsr             csky_copy_q15       // call the function csky_fill_q15
    lsli            l9, l5, 1
    add             l3, l3, l9          // pScr1 += srcALen

    movi            a0, 0
    mov             a1, l3
    subi            a2, l8, 1
    bsr             csky_fill_q15       // call the function csky_fill_q15
    subi            a2, l8, 1
    lsli            l9, a2, 1
    add             l3, l3, l9          // pScr1 += (srcBLen - 1u)

    mov             a0, l4
    mov             a1, l5
    mov             a2, l6
    mov             a3, l8

    ld.w            l2, (sp, 0x34)      // py     = pScratch2
    mov             a2, l2              // pIn2   = py
    subi            l4, a3, 1
    add             l4, a1, l4          // blkCnt
    lsri            t0, l4, 2
    bez             t0, .L15            // while(blkCnt > 0u)

.L7:
    mov             l3, l0              // pScr1  = pScratch1
    movi            t6, 0               // acc0   = 0
    movi            t7, 0               // acc1   = 0
    movi            t8, 0               // acc2   = 0
    movi            t9, 0               // acc3   = 0
    pldbi.d         t2, (l3)            // load four samples from scratch1 buffer
    lsri            t1, a3, 2           // tapCnt = (srcBLen) >> 2u
    bez             t1, .L10            // while(tapCnt > 0u)

.L8:
    pldbi.d         t4, (a2)            // load four samples from smaller buffer
    mulaca.s16.s    t6, t2, t4          // acc0   = __SMLAD(x1, y1, acc0)
    mulaca.s16.s    t8, t3, t4          // acc2   = __SMLAD(x2, y1, acc2)
    asri            t2, t2, 16          // x3     = __PKHBT(x2, x1, 0)
    pkgll           l8, t3, t2
    mulacax.s16.s   t7, l8, t4          // acc1   = __SMLADX(x3, y1, acc1)
    ldbi.w          t2, (l3)            // x1     = _SIM32_OFFSET(pScr1)
    mulaca.s16.s    t6, t3, t5          // acc0   = __SMLAD(x2, y2, acc0)
    mulaca.s16.s    t8, t2, t5          // acc2   = __SMLAD(x1, y2, acc2)
    asri            t3, t3, 16          // x3     = __PKHBT(x1, x2, 0)
    pkgll           l8, t2, t3
    mulacax.s16.s   t9, l8, t4          // acc3   = __SMLAD(x3, y1, acc3)
    mulacax.s16.s   t7, l8, t5          // acc1   = __SMLAD(x3, y2, acc1)
    ldbi.w          t3, (l3)            // x2     = _SIM32_OFFSET(pSrc1 + 2u)
    asri            l9, t2, 16          // x3     = __PKHBT(x2, x1, 0)
    pkgll           l8, t3, l9

.L9:
    mulacax.s16.s   t9, l8, t5          // acc3   = __SMLADX(x3, y2, acc3)
    bloop           t1, .L8, .L9        // while loop

.L10:
    subi            l3, l3, 8           // pScr1 -= 4u
    andi            t1, a3, 3           // tapCnt = (srcBLen & 3u)
    bez             t1, .L13            // while(tapCnt > 0u )

.L11:
    movi            t4, 0    
    ldbi.hs         t4, (a2)            // multiply and accumulate
    pldbi.d         t2, (l3)
    mulaca.s16.s    t6, t2, t4
    mulacax.s16.s   t7, t2, t4
    mulaca.s16.s    t8, t3, t4
    mulacax.s16.s   t9, t3, t4

.L12:
    subi            l3, l3, 6
    bloop           t1, .L11, .L12      // while loop

.L13:
    asri            t6, t6, 15          // process and store the results
    clipi.s32       t6, t6, 16
    asri            t7, t7, 15
    clipi.s32       t7, t7, 16
    pkgll           t6, t6, t7
    stbi.w          t6, (l7)
    asri            t8, t8, 15
    clipi.s32       t8, t8, 16
    asri            t9, t9, 15
    clipi.s32       t9, t9, 16
    pkgll           t8, t8, t9
    stbi.w          t8, (l7)

    mov             a2, l2              // pIn2       = py
    addi            l0, l0, 8           // pScratch1 += 4u

.L14:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L7             // while loop

.L15:
    andi            t1, l4, 3           // blkCnt
    bez             t1, .L24            // while(blkCnt > 0)

.L16:
    mov             l3, l0              // pScr1 = pScratch1
    movi            t6, 0               // acc0  = 0
    lsri            t8, a3, 1           // tapCnt
    andi            t9, a3, 1
    bez             t8, .L19            // while(tapCnt > 0u)

.L17:
    ldbi.w          t2, (l3)
    ldbi.w          t3, (a2)

.L18:
    mulaca.s16.s    t6, t2, t3
    bloop           t8, .L17, .L18      // while loop

.L19:
    bez             t9, .L22            // while (tapCnt > 0)

.L20:
    ldbi.hs         t2, (l3)            // mutiply and accumulate
    ldbi.hs         t3, (a2)

.L21:
    mulall.s16.s    t6, t2, t3
    bloop           t9, .L20, .L21      // while loop

.L22:
    asri            t6, t6, 15          // process and store the result
    clipi.s32       t6, t6, 16
    stbi.h          t6, (l7)
    mov             a2, l2              // pIn2       = py
    addi            l0, l0, 2           // pScratch1 += 1u

.L23:
    subi            t1, t1, 1           // blkCnt--
    bhz             t1, .L16            // while loop

.L24:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, lr
    .size           csky_conv_fast_opt_q15, .-csky_conv_fast_opt_q15
