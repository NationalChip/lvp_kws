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
 * @file     csky_conv_partial_q15.S
 * @brief    Partial convolution of Q15 sequences.
 * @version  V1.0
 * @date     30. Aug 2017
 ******************************************************************************/

/*
 * csky_status csky_conv_partial_q15(
 *   q15_t * pSrcA
 *   uint32_t srcALen,
 *   q15_t * pSrcB,
 *   uint32_t srcBLen
 *   q15_t * pDst,
 *   uint32_t firstIndex,
 *   uint32_t numPoints)
 */

    .file           "csky_conv_partial_q15.S"
    .section        .text.csky_conv_partial_q15,"ax",@progbits
    .align          2
    .global         csky_conv_partial_q15
    .type           csky_conv_partial_q15, @function

csky_conv_partial_q15:
    push            l0, l1, l2, l3, l4 ,l5, l6, l7, l8, l9
    ld.w            l0, (sp, 0x2c)      // load firstIndex
    ld.w            l1, (sp, 0x30)      // load numPoints
    ld.w            l9, (sp, 0x28)      // load *pDst
    add             t0, l0, l1
    subi            t1, a3, 1
    add             t3, a1, t1

    cmplt           t3, t0              // if(firstIndex + numPoints) > ((srcAlen + (srcBlen - 1u)
    bf              .L0
    movi            a0, 0               // if branch
    subi            a0, a0, 1
    br              .L57

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
    subi            t1, a3, 1
    sub.s32.s       l2, t1, l0          // blockSize1
    blsz            l2, .L2
    cmplt           t1, t0
    bt              .L3
    mov             l2, l1
    br              .L3

.L2:
    movi            l2, 0
    /* -------------------------
     * Initializations of stage1
     * ------------------------*/

.L3:
    lsli            t0, l0, 1
    add             l3, l9, t0          // pOut = pDst + firstIndex
    addi            l4, l0, 1           // count = 1u + firstIndex
    mov             l5, a0              // px    = pIn1
    add             l8, a2, t0          // pSrc2
    mov             l6, l8              // py    = pSrc2

    /* -------------------------
     *     Stage1 processing
     * ------------------------*/

    bez             l2, .L16            // while(blockSize1 > 0)
    mov             t1, l2
.L4:
    movi            t8, 0               // sum = 0
    movi            t9, 0
    mov             t0, l4              // k   = count

.L5:
    ldbi.hs         t2, (l5)            // perform the multiply-accumulates
    ld.hs           t3, (l6, 0x0)
    subi            l6, l6, 2

.L6:
    mulall.s16.e    t8, t2, t3
    bloop           t0, .L5, .L6        // while loop

    dexti           t8, t8, t9, 15      // sum >> 15
    clipi.s32       t8, t8, 16          // saturate
    stbi.h          t8, (l3)            // store the data

    addi            l8, l8, 2           // ++pSrc2
    mov             l6, l8              // py = pSrc2
    mov             l5, a0              // px = pIn1

.L7:
    addi            l4, l4, 1           // count++
    subi            t1, t1, 1           // blockSize1--
    cmplti          l4, 4
    bf              .L8
    bhz             t1, .L4             // while loop

.L8:
    subi            l6, l6, 2           // py = py - 1
    bez             t1, .L16            // while(blockSize1 > 0)

.L9:
    movi            t8, 0               // sum = 0
    movi            t9, 0
    lsri            t0, l4, 2           // k = count >> 2

.L10:
    pldbi.d         t2, (l5)            // perform the multiply-accumulate
    ld.w            t4, (l6)
    subi            l6, l6, 4
    ld.w            t5, (l6)
    subi            l6, l6, 4
    mulacax.s16.e   t8, t2, t4

.L11:
    mulacax.s16.e   t8, t3, t5
    bloop           t0, .L10, .L11      // while loop

    addi            l6, l6, 2           // py  = py + 1u
    andi            t0, l4, 3           // k   = count% 0x4u
    bez             t0, .L14            // while(k > 0)

.L12:
    ldbi.hs         t2, (l5)            // sum = __SMLAD(*px++, *py--, sum)
    ld.hs           t3, (l6, 0x0)
    subi            l6, l6, 2

.L13:
    mulall.s16.e    t8, t2, t3
    bloop           t0, .L12, .L13      // while loop

.L14:
    dexti           t8, t8, t9, 15
    clipi.s32       t8, t8, 16
    stbi.h          t8, (l3)            // store the result
    mov             l6, l8              // py
    mov             l5, a0              // px
    addi            l8, l8, 2

.L15:
    addi            l4, l4, 1           // count++
    subi            t1, t1, 1           // blockSize1--
    bhz             t1, .L9             // while loop

    /* ---------------------------
     * Initializations of stage2
     * --------------------------*/

.L16:
    subi            t1, a3, 1
    sub.s32.s       t0, l0, t1
    blsz            t0, .L17            // if(firstIndex - srcBLen + 1 > 0)
    lsli            t9, t0, 1
    add             l7, a0, t9          // if branch
    br              .L59

.L17:
    mov             l7, a0              // else branch

.L59:
    mov             l5, l7              // px    = pSrc1
    lsli            t9, t1, 1
    add             l8, a2, t9          // pSrc2
    mov             l6, l8              // py    = pSrc2
    movi            l4, 0               // count = 0u

    add             l9, l0, l1          // blockSize3
    cmplt           a1, l9
    bf              .L18
    sub.s32.s       t2, l9, a1
    br              .L19

.L18:
    movi            t2, 0

.L19:
    subi            t1, a1, 1
    cmplt           t1, l0
    bf              .L20
    sub.s32.s       t2, t2, l0
    add.s32.s       t2, t2, a1

.L20:
    add             t3, t2, l2          // blockSize2
    sub.s32.s       t3, l9, t3
    sub.s32.s       t3, t3, l0
    bhz             t3, .L21
    movi            t3, 0

    /* --------------------------
     *     Stage2 processing
     * -------------------------*/

.L21:
    mov             l0, t3
    subi            sp, sp, 4
    st.w            t2, (sp, 0x0)       // store blockSize3
    
    cmphsi          a3, 4               // if(srcBLen >= 4u)
    bf              .L58

.L22:
    lsri            t0, l0, 2           // blkCnt = blockSize2 >> 2u
    bez             t0, .L30            // while(blkCnt > 0u)

.L23:
    subi            l6, l6, 2           // py  = py -1u
    movi            t2, 0               // acc0 = 0
    movi            t3, 0
    movi            t4, 0               // acc1 = 0
    movi            t5, 0
    movi            t6, 0               // acc2 = 0
    movi            t7, 0
    movi            t8, 0               // acc3 = 0
    movi            t9, 0

    ldbi.w          l1, (l5)            // load x[0], x[1] samples
    subi            l5, l5, 2
    ldbi.w          l2, (l5)            // load x[1], x[2] samples
    subi            l5, l5, 2

    lsri            t1, a3, 2           // k = srcBLen >> 2u

.L24:
    ld.w            l9, (l6, 0x0)       // c0 = *__SIMD32(py)--
    subi            l6, l6, 4
    mulacax.s16.e   t2, l1, l9          // acc0
    mulacax.s16.e   t4, l2, l9          // acc1
    ldbi.w          l1, (l5)            // x2
    subi            l5, l5, 2
    ldbi.w          l2, (l5)            // x3
    subi            l5, l5, 2
    mulacax.s16.e   t6, l1, l9          // acc2
    mulacax.s16.e   t8, l2, l9          // acc3
    ld.w            l9, (l6, 0x0)       // c0
    subi            l6, l6, 4
    mulacax.s16.e   t2, l1, l9          // acc0
    mulacax.s16.e   t4, l2, l9          // acc1
    ldbi.w          l1, (l5)            // x0
    subi            l5, l5, 2
    ldbi.w          l2, (l5)            // x1
    subi            l5, l5, 2
    mulacax.s16.e   t6, l1, l9          // acc2

.L25:
    mulacax.s16.e   t8, l2, l9          // acc3
    bloop           t1, .L24, .L25      // while loop

    andi            t1, a3, 3           // k = srcBlen % 0x4u
    bez             t1, .L28            // if(k == 0)
    cmpnei          t1, 1               // if(k == 1) 
    bt              .L26

    ld.hs           l9, (l6, 0x2)       // c0
    lsli            l9, l9, 16
    mulaca.s16.e    t2, l1, l9          // acc0
    mulaca.s16.e    t4, l2, l9          // acc1
    mulacax.s16.e   t6, l2, l9          // acc2
    ldbi.hs         l1, (l5)            // x3
    mulacax.s16.e   t8, l1, l9          // acc3

.L26:
    cmpnei          t1, 2               // if(k == 2)
    bt              .L27
    ld.w            l9, (l6, 0x0)       // c0
    mulacax.s16.e   t2, l1, l9          // acc0
    mulacax.s16.e   t4, l2, l9          // acc1
    ldbi.w          l1, (l5)            // x3
    subi            l5, l5, 2
    ldbi.w          l2, (l5)            // x2
    subi            l5, l5, 2
    mulacax.s16.e   t6, l1, l9          // acc2
    mulacax.s16.e   t8, l2, l9          // acc3

.L27:                                   // k == 3u
    ld.w            l9, (l6, 0x0)       // c0
    mulacax.s16.e   t2, l1, l9          // acc0
    mulacax.s16.e   t4, l2, l9          // acc1
    ldbi.w          l1, (l5)            // x3
    mulacax.s16.e   t6, l1, l9          // acc2
    ldbi.hs         l1, (l5)            // x2
    mulacax.s16.e   t8, l1, l9          // acc3
    subi            l6, l6, 2
    ld.w            l9, (l6, 0x0)       // c0
    lsli            l9, l9, 16
    mulacax.s16.e   t2, l2, l9          // acc0
    mulaca.s16.e    t4, l1, l9          // acc1
    mulacax.s16.e   t6, l1, l9          // acc2
    ldbi.hs         l2, (l5)            // x3
    mulacax.s16.e   t8, l2, l9          // acc3

.L28:
    dexti           t2, t2, t3, 15      // process and store the results
    dexti           t4, t4, t5, 15
    dexti           t6, t6, t7, 15
    dexti           t8, t8, t9, 15
    clipi.s32       t2, t2, 16
    clipi.s32       t4, t4, 16
    clipi.s32       t6, t6, 16
    clipi.s32       t8, t8, 16
    pkgll           t2, t2, t4
    stbi.w          t2, (l3)
    pkgll           t6, t6, t8
    stbi.w          t6, (l3)

    addi            l4, l4, 4           // count += 4u
    lsli            l9, l4, 1
    add             l5, l7, l9          // px     = pSrc1 + count
    mov             l6, l8              // py     = pSrc2

.L29:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L23

.L30:
    andi            t0, l0, 3           // blkCnt = blockSize2 % 0x4u
    bez             t0, .L43            // while(blkCnt > 0)

.L31:
    movi            t8, 0               // sum = 0
    movi            t9, 0
    lsri            t1, a3, 2           // k = srcBLen >> 2u
    bez             t1, .L34            // while(k > 0u)

.L32:
    subi            l6, l6, 2
    pldbi.d         t2, (l5)            // perform the multiply-accumulates
    ld.w            t4, (l6, 0x0)
    subi            l6, l6, 4
    ld.w            t5, (l6, 0x0)
    subi            l6, l6, 2
    mulacax.s16.e   t8, t2, t4

.L33:
    mulacax.s16.e   t8, t3, t5
    bloop           t1, .L32, .L33

.L34:
    andi            t1, a3, 3           // k = srcBLen % 0x4u
    bez             t1, .L37            // while(k > 0)

.L35:
    ldbi.hs         t2, (l5)            // sum += (q63_t)((q31_t) * px++ * *py--)
    ld.hs           t3, (l6, 0x0)
    subi            l6, l6, 2

.L36:
    mulall.s16.e    t8, t2, t3
    bloop           t1, .L35, .L36

.L37:
    dexti           t8, t8, t9, 15
    clipi.s32       t8, t8, 16
    stbi.h          t8, (l3)

    addi            l4, l4, 1           // count++
    lsli            l9, l4, 1
    add             l5, l7, l9          // px
    mov             l6, l8              // py

.L38:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L31
    br              .L43

.L58:                                   // else branch
    mov             t0, l0              // blkCnt = blockSize2
    bez             t0, .L43            // while(blkCnt > 0u)

.L42:
    movi            t8, 0               // sum = 0
    movi            t9, 0
    mov             t1, a3              // k = srcBLen

.L39:
    ldbi.hs         t2, (l5)            // sum
    ld.hs           t3, (l6, 0x0)
    subi            l6, l6, 2

.L40:
    mulall.s16.e    t8, t2, t3
    bloop           t1, .L39, .L40

    dexti           t8, t8, t9, 15
    clipi.s32       t8, t8, 16
    stbi.h          t8, (l3)

    addi            l4, l4, 1
    lsli            t9, l4, 1
    add             l5, l7, t9
    mov             l6, l8

.L41:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L42

    /* --------------------------
     * Initializations of stage3
     * ------------------------*/

.L43:
    ld.w            l0, (sp, 0x0)       // load blockSize3
    bez             l0, .L57            // while (blockSize3 > 0u)
    subi            l4, a3, 1           // count = srcBLen - 1u
    sub             l1, a1, l4
    lsli            l9, l1, 1
    add             l1, a0, l9
    mov             l5, l1              // px    = pSrc1

    lsli            l9, l4, 1
    add             l2, a2, l9          // pSrc2
    subi            a2, l2, 2
    mov             l6, a2              // py    = pIn2

    /* -------------------------
     *     Stage3 processing
     * -----------------------*/

    lsri            t0, l4, 2           // j = count >> 2u
    bez             t0, .L52            // while (j > 0u)

.L44:
    bez             l0, .L57            // while (blockSize3 > 0)
    movi            t8, 0               // sum = 0
    movi            t9, 0
    lsri            t1, l4, 2           // k   = count >> 2u
    bez             t1, .L47            // while( k > 0)

.L45:
    ldbi.w          t2, (l5)            // perform multiply and accumulate
    ld.w            t3, (l6, 0x0)
    subi            l6, l6, 4
    mulacax.s16.e   t8, t2, t3
    ldbi.w          t2, (l5)
    ld.w            t3, (l6, 0x0)
    subi            l6, l6, 4

.L46:
    mulacax.s16.e   t8, t2, t3
    bloop           t1, .L45, .L46

.L47:
    addi            l6, l6, 2           // py = py + 1u
    andi            t1, l4, 3           // k  = count % 0x4u
    bez             t1, .L50            // while(k > 0)

.L48:
    ldbi.hs         t2, (l5)            // sum
    ld.hs           t3, (l6, 0x0)
    subi            l6, l6, 2

.L49:
    mulall.s16.e    t8, t2, t3
    bloop           t1, .L48, .L49

.L50:
    dexti           t8, t8, t9, 15      // process and store the result
    clipi.s32       t8, t8, 16
    stbi.h          t8, (l3)
    addi            l1, l1, 2
    mov             l5, l1              // px
    mov             l6, a2              // py
    subi            l4, l4, 1           // count--

.L51:
    subi            t0, t0, 1           // j--
    subi            l0, l0, 1           // blockSize3--
    bhz             t0, .L44

.L52:
    addi            l6, l6, 2           // py = py + 1u
    bez             l0, .L57            // while(blockSize3 > 0)

.L53:
    movi            t8, 0               // sum = 0
    movi            t9, 0
    mov             t1, l4              // k   = count

.L54:
    ldbi.hs         t2, (l5)            // sum
    ld.hs           t3, (l6, 0x0)
    subi            l6, l6, 2

.L55:
    mulall.s16.e    t8, t2, t3
    bloop           t1, .L54, .L55

    dexti           t8, t8, t9, 15
    clipi.s32       t8, t8, 16
    stbi.h          t8, (l3)
    addi            l1, l1, 2
    mov             l5, l1              // px
    mov             l6, l2              // py
    subi            l4, l4, 1           // count--

.L56:
    subi            l0, l0, 1           // blockSize3--
    bhz             l0, .L53

.L57:
    movi            a0, 0

    addi            sp, sp, 4
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_conv_partial_q15, .-csky_conv_partial_q15
