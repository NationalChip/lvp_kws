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
 * @file     csky_conv_partial_q7.S
 * @brief    Partial convolution of Q7 sequences.
 * @version  V1.0
 * @date     31. Aug 2017
 ******************************************************************************/

/*
 * csky_status csky_conv_partial_q7(
 *   q7_t * pSrcA
 *   uint32_t srcALen,
 *   q7_t * pSrcB,
 *   uint32_t srcBLen
 *   q7_t * pDst,
 *   uint32_t firstIndex,
 *   uindt32_t numPoints)
 */

    .file           "csky_conv_partial_q7.S"
    .section        .text.csky_conv_partial_q7,"ax",@progbits
    .align          2
    .global         csky_conv_partial_q7
    .type           csky_conv_partial_q7, @function

csky_conv_partial_q7:
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
    br              .L51

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

    /* --------------------------
     * Initializations of stage1
     * -------------------------*/
.L3:
    add             l3, l9, l0          // pOut  = pDst + firstIndex
    addi            l4, l0, 1           // count = 1u + firstIndex
    mov             l5, a0              // px    = pIn1
    add             l8, a2, l0
    mov             l6, l8              // py    = pIn2 + firstIndex

    /* -------------------------
     *    Stage1 processing
     * -----------------------*/

    bez             l2, .L12            // while(blockSize1 > 0)
    mov             t1, l2

.L4:
    movi            t8, 0               // sum = 0
    lsri            t0, l4, 2           // k   = count >> 2u
    bez             t0, .L7             // while(k > 0u)

.L5:
    subi            l6, 3
    ldbi.w          t2, (l5)
    ld.w            t3, (l6, 0x0)
    subi            l6, 1
    pext.s8.e       t4, t2
    pext.s8.e       t6, t3
    mulacax.s16.s   t8, t4, t7          // x[0] * y [srcBLen - 1] + x[1] * y[srcBlen - 2]

.L6:
    mulacax.s16.s   t8, t5, t6          // x[2] * y [srcBLen - 2] + x[3] * y[srcBlen - 4]
    bloop           t0, .L5, .L6

.L7:
    andi            t0, l4, 3           // k = count % 0x4u
    bez             t0, .L10            // while(k > 0)

.L8:
    ldbi.bs         t2, (l5)            // sum += ((q31_t) *px++ * *py--)
    sextb           t2, t2
    ld.bs           t3, (l6, 0x0)
    sextb           t3, t3
    subi            l6, l6, 1

.L9:
    mulall.s16.s    t8, t2, t3
    bloop           t0, .L8, .L9

.L10:
    asri            t8, t8, 7           // sum >> 7
    clipi.s32       t8, t8, 8           // saturate
    stbi.b          t8, (l3)            // store

    addi            l8, l8, 1
    mov             l6, l8              // py = ++ pSrc2
    mov             l5, a0              // px = pIn1
    addi            l4, l4, 1           // count++

.L11:
    subi            t1, t1, 1           // blockSize1--
    bhz             t1, .L4

    /* -------------------------
     * Initializations of stage2
     * -----------------------*/
.L12:
    subi            t1, a3, 1
    sub.s32.s       t0, l0, t1
    blsz            t0, .L13            // if(firstIndex - srcBLen +1 > 0)

    add             l7, a0, t0
    br              .L14

.L13:
    mov             l7, a0

.L14:
    mov             l5, l7              // px    = pSrc1
    add             l8, a2, t1
    mov             l6, l8              // py    = pIn2 + (srcBLen - 1u)
    movi            l4, 0               // count = 0u

    add             l9, l0, l1
    cmplt           a1, l9
    bf              .L15
    sub.s32.s       t2, l9, a1
    br              .L16

.L15:
    movi            t2, 0

.L16:
    subi            t1, a1, 1
    cmplt           t1, l0              // blockSize3
    bf              .L17
    sub.s32.s       t2, t2, l0
    sub.s32.s       t2, t2, a1

.L17:
    add             t3, t2, l2          // blockSize2
    sub.s32.s       t3, l9, t3
    sub.s32.s       t3, t3, l0
    bhz             t3, .L18
    movi            t3, 0
 
   /* -----------------------
     *    Stage2 processing
     * ----------------------*/

.L18:
    mov             l0, t3
    mov             l9, t2

    cmphsi          a3, 4               // if(srcBLen >= 4u)
    bf              .L36

.L19:
    lsri            t0, l0, 2           // blkCnt = blockSize2 >> 2u
    bez             t0, .L28            // while(blkCnt > 0u)

.L20:
    movi            t6, 0               // acc0 = 0
    movi            t7, 0               // acc1 = 0
    movi            t8, 0               // acc2 = 0
    movi            t9, 0               // acc3 = 0

    ldbi.w          t2, (l5)            // load x[0], x[1], x[2] and x[3] samples
    pext.s8.e       t4, t2
    subi            l5, l5, 1
    lsri            t1, a3, 2           // k    = srcBLen >> 2u

.L21:
    subi            l6, 3
    ld.w            t3, (l6, 0x0)
    subi            l6, 1
    pext.s8.e       t2, t3
    mulacax.s16.s   t6, t4, t3          // acc0
    lsri            t4, t4, 16
    pkgll           t4, t4, t5
    mulacax.s16.s   t7, t4, t3          // acc1
    mulacax.s16.s   t8, t5, t3          // acc2

    ldbi.w          t4, (l5)
    pext.s8.e       l1, t4
    mulacax.s16.s   t9, l1, t3          // acc3
    mulacax.s16.s   t6, t5, t2          // acc0
    mulacax.s16.s   t7, l1, t2          // acc1
    lsri            t5, l1, 16
    pkgll           t5, t5, l2
    mulacax.s16.s   t8, t5, t2          // acc2
    mulacax.s16.s   t9, l2, t2          // acc3
    subi            l5, l5, 3
    ldbi.w          l2, (l5)
    pext.s8.e       t4, l2

.L22:
    subi            l5, 1
    bloop           t1, .L21, .L22

.L23:
    andi            t1, a3, 3           // k = srcBLen % 0x4u
    bez             t1, .L26            // while(k > 0)

    subi            l5, 2
.L24:
    ld.bs           t3, (l6, 0x0)
    subi            l6, l6, 1
    mulaca.s16.s    t6, t4, t3          // acc0
    mulacax.s16.s   t7, t4, t3          // acc1
    mulaca.s16.s    t8, t5, t3          // acc2
    mulacax.s16.s   t9, t5, t3          // acc3

    ldbi.w          l2, (l5)
    pext.s8.e       t4, l2

.L25:
    subi            l5, 3
    bloop           t1, .L24, .L25

.L26:
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
    stbi.w          t6, (l3)

    addi            l4, l4, 4           // count += 4u
    add             l5, l7, l4          // px     = pSrc1 + count
    mov             l6, l8              // py     = pSrc2

.L27:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L20

.L28:
    andi            t0, l0, 3           // blkCnt = blockSize2 % 0x4u
    bez             t0, .L34

.L29:
    movi            t9, 0               // sum = 0
    lsri            t1, a3, 2           // k = srcBLen >> 2u
    bez             t1, .L32            // while(k > 0)

.L30:
    subi            l6, 3
    ldbi.w          t2, (l5)
    ld.w            t3, (l6, 0x0)
    subi            l6, 1
    pext.s8.e       t4, t2
    pext.s8.e       t6, t3
    mulacax.s16.s   t9, t4, t7

.L31:
    mulacax.s16.s   t9, t5, t6
    bloop           t1, .L30, .L31

.L32:
    andi            t1, a3, 3           // k = srcBLen % 0x4u
    bez             t1, .L34            // while(k > 0u)

.L52:
    ldbi.bs         t2, (l5)
    sextb           t2, t2
    ld.bs           t3, (l6, 0x0)
    sextb           t3, t3
    subi            l6, l6, 1

.L33:
    mulall.s16.s    t9, t2, t3
    bloop           t1, .L52, .L33

.L34:
    asri            t9, t9, 7           // process and store the result
    clipi.s32       t9, t9, 8
    stbi.b          t9, (l3)
    addi            l4, l4, 1           // count++
    add             l5, l7, l4          // px
    mov             l6, l8              // py

.L35:
    subi            t0, t0, 1           // blkCnt--           
    bhz             t0, .L29
    br              .L41

.L36:                                   // else branch
    mov             t0, l0              // blkCnt = blkSize2
    bez             t0, .L41            // while(blkCnt > 0)

.L37:
    movi            t9, 0               // sum = 0
    mov             t1, a3              // k   = srcBLen

.L38:
    ldbi.bs         t2, (l5)
    sextb           t2, t2
    ld.bs           t3, (l6, 0x0)
    sextb           t3, t3
    subi            l6, l6, 1

.L39:
    mulall.s16.s    t9, t2, t3
    bloop           t1, .L38, .L39

    asri            t9, t9, 7
    clipi.s32       t9, t9, 8
    stbi.b          t9, (l3)
    addi            l4, l4, 1
    add             l5, l7, l4
    mov             l6, l8   

.L40:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L37

    /* --------------------------
     * Initializations of stage3
     * ------------------------*/

.L41:
    subi            l4, a3, 1           // count = srcBLen - 1u
    sub             t0, a1, l4
    add             l1, a0, t0
    mov             l5, l1              // px    = pSrc1
    add             l2, a2, l4
    mov             l6, l2              // py    = pSrc2

    /* -------------------------
     *     Stage3 processing
     * -----------------------*/

    mov             t0, l9
    bez             l9, .L50

.L42:
    movi            t9, 0               // sum = 0
    lsri            t1, l4, 2           // k   = count >> 2u
    bez             t1, .L45            // while(k > 0u)

.L43:
    subi            l6, 3
    ldbi.w          t2, (l5)
    ld.w            t3, (l6, 0x0)
    subi            l6, 1
    pext.s8.e       t4, t2
    pext.s8.e       t6, t3
    mulacax.s16.s   t9, t4, t7

.L44:
    mulacax.s16.s   t9, t5, t6
    bloop           t1, .L43, .L44

.L45:
    andi            t1, l4, 3           // k = count % 0x4u
    bez             t1, .L48

.L46:
    ldbi.bs         t2, (l5)
    sextb           t2, t2
    ld.bs           t3, (l6, 0x0)
    sextb           t3, t3
    subi            l6, l6, 1

.L47:
    mulall.s16.s    t9, t2, t3
    bloop           t1, .L46, .L47

.L48:
    asri            t9, t9, 7
    clipi.s32       t9, t9, 8
    stbi.b          t9, (l3)
    subi            l4, l4, 1           // count--
    addi            l1, l1, 1
    mov             l5, l1              // px
    mov             l6, l2              // py
   
.L49:
    subi            t0, t0, 1           // blockSize3--
    bhz             t0, .L42

.L50:
    movi            a0, 0

.L51:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_conv_partial_q7, .-csky_conv_partial_q7
