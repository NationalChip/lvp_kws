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
 * @file     csky_correlate_q7.S
 * @brief    Correlation of Q7 sequences.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_correlate_q7(
 *   q7_t * pSrcA,
 *   uint32_t srcALen,
 *   q7_t * pSrcB,
 *   uint32_t srcBLen,
 *   q7_t * pDst)
 */
    .file           "csky_correlate_q7.S"
    .section        .text.csky_correlate_q7,"ax",@progbits
    .align          2
    .global         csky_correlate_q7
    .type           csky_correlate_q7, @function
csky_correlate_q7:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9

    ld.w            l6, (sp, 0x28)      // pOut
    movi            l9, 1               // inc = 1
    cmphs           a1, a3              // srcALen >= srcBLen
    bf              .L100
    subu            t0, a1, a3          // j
    addu            l6, l6, t0          // pOut
    br              .L0
.L100:
    mov             l0, a1
    mov             a1, a3              // srcALen
    mov             a3, l0              // srcBLen

    mov             l0, a0
    mov             a0, a2              // pIn1
    mov             a2, l0              // pIn2

    addu            t0, a1, a3
    subi            t0, t0, 2
    addu            l6, l6, t0          // pOut
    neg.s32.s       l9, l9              // inc = -1
.L0:

    subi            l0, a3, 1           // blockSize1 = srcBLen - 1


    /* --------------------------
     * Initializations of stage1
     * -------------------------*/
    movi            l1, 1               // count
    mov             l2, a0              // px
    addu            t8, l0, a2          // pSrc1
    mov             l3, t8              // py = pSrc1

    /* ------------------------
     * Stage1 process
     * ----------------------*/

    bez             l0, .L1             // blockSize1 > 0
.L8:
    movi            t0, 0
    lsri            l4, l1, 2           // k

    bez             l4, .L2
.L3:
    ldbi.w          t2, (l2)
    ldbi.w          t3, (l3)
    pext.s8.e       t4, t2
    pext.s8.e       t6, t3
    mulaca.s16.s    t0, t4, t6
.L4:
    mulaca.s16.s    t0, t5, t7

    bloop           l4, .L3, .L4
.L2:

    andi            l4, l1, 3           // k
    bez             l4, .L5
.L6:
    ldbi.bs         t2, (l2)
    ldbi.bs         t3, (l3)
.L7:
    mulall.s16.s    t0, t2, t3

    bloop           l4, .L6, .L7
.L5:
    asri            t2, t0, 7
    clipi.s32       t2, t2, 8
    st.b            t2, (l6, 0x0)       // pOut
    addu            l6, l6, l9

    subu            l3, t8, l1          // py = pSrc1 - count
    mov             l2, a0              // px = pIn1
.L9:
    addi            l1, 1               // count++
    subi            l0, 1               // blockSize1--
    bnez            l0, .L8
.L1:


    /* --------------------------
     * Initializations of stage2
     * ------------------------*/
    mov             l2, a0              // px = pIn1
    mov             l3, a2              // py = pIn2
    movi            l1, 0               // count = 0
    subi            l5, a3, 1
    subu            l5, a1, l5          // blockSize2


    /* -------------------
     * Stage2 process
     * ------------------*/

    cmphsi          a3, 4               // srcBLen >= 4
    bf              .L10

    lsri            l0, l5, 2           // blkCnt
    bez             l0, .L12

.L13:
    movi            t2, 0               // acc0
    movi            t3, 0               // acc1
    movi            t4, 0               // acc2
    movi            t5, 0               // acc3

    ldbi.bs         t0, (l2)            // x0
    ldbi.bs         t1, (l2)            // x1
    ldbi.bs         t6, (l2)            // x2

    lsri            l8, a3, 2           // k = srcBLen >> 2

.L15:
    ldbi.bs         l4, (l3)            // c0
    ldbi.bs         l7, (l3)            // c1
    ldbi.bs         t7, (l2)            // x3
    pkgll           t8, t0, t1          // input1
    pkgll           l4, l4, l7          // input2
    mulaca.s16.s    t2, t8, l4          // acc0
    pkgll           t8, t1, t6
    mulaca.s16.s    t3, t8, l4          // acc1
    pkgll           t8, t6, t7
    mulaca.s16.s    t4, t8, l4          // acc2
    ldbi.bs         t0, (l2)            // x0
    pkgll           t8, t7, t0
    mulaca.s16.s    t5, t8, l4          // acc3

    ldbi.bs         l4, (l3)            // c0
    ldbi.bs         l7, (l3)            // c1
    pkgll           l4, l4, l7          // input2
    ldbi.bs         t1, (l2)            // x1
    pkgll           t8, t6, t7
    mulaca.s16.s    t2, t8, l4          // acc0
    pkgll           t8, t7, t0
    mulaca.s16.s    t3, t8, l4          // acc1
    pkgll           t8, t0, t1
    mulaca.s16.s    t4, t8, l4          // acc2
    ldbi.bs         t6, (l2)            // x2
    pkgll           t8, t1, t6
.L16:
    mulaca.s16.s    t5, t8, l4          // acc3

    bloop           l8, .L15, .L16

    andi            l8, a3, 3           // k
    bez             l8, .L17
.L18:
    ldbi.bs         l4, (l3)
    ldbi.bs         t7, (l2)
    mulall.s16.s    t2, t0, l4
    mulall.s16.s    t3, t1, l4
    mulall.s16.s    t4, t6, l4
    mulall.s16.s    t5, t7, l4

    mov             t0, t1
    mov             t1, t6
.L19:
    mov             t6, t7

    bloop           l8, .L18, .L19
.L17:


    asri            t2, t2, 7
    asri            t3, t3, 7
    asri            t4, t4, 7
    asri            t5, t5, 7
    clipi.s32       t2, t2, 8
    clipi.s32       t3, t3, 8
    clipi.s32       t4, t4, 8
    clipi.s32       t5, t5, 8
    st.b            t2, (l6)            // pOut
    addu            l6, l6, l9
    st.b            t3, (l6)            // pOut
    addu            l6, l6, l9
    st.b            t4, (l6)            // pOut
    addu            l6, l6, l9
    st.b            t5, (l6)            // pOut
    addu            l6, l6, l9

    addi            l1, 4               // count += 4
    addu            l2, a0, l1          // px = pIn1 + count
    mov             l3, a2              // py = pIn2

    subi            l0, 1               // blCnt--
    bnez            l0, .L13
.L12:

    andi            l0, l5, 3           // blkCnt

    bez             l0, .L20
.L21:
    movi            t2, 0               // sum
    lsri            l8, a3, 2           // k

    bez             l8, .L23

.L24:
    ldbi.w          t0, (l2)
    ldbi.w          t1, (l3)
    pext.s8.e       t4, t0
    pext.s8.e       t6, t1
    mulaca.s16.s    t2, t4, t6
.L25:
    mulaca.s16.s    t2, t5, t7

    bloop           l8, .L24, .L25
.L23:

    andi            l8, a3, 3           // k
    bez             l8, .L26

.L27:
    ldbi.bs         t4, (l2)
    ldbi.bs         t5, (l3)
.L28:
    mulall.s16.s    t2, t4, t5
    bloop           l8, .L27, .L28
.L26:

    asri            t4, t2, 7
    clipi.s32       t4, t4, 8
    st.b            t4, (l6, 0x0)       // pOut
    addu            l6, l6, l9
    addi            l1, 1               // count++

    addu            l2, a0, l1          // px
    mov             l3, a2

    subi            l0, l0, 1
    bnez            l0, .L21
.L20:
    br              .L11                // else
.L10:

    mov             l0, l5              // blkCnt = blockSize2
    bez             l0, .L53
.L54:
    movi            t2, 0               // sum = 0
    mov             l8, a3              // k = srcBLen
    bez             l8, .L56
.L57:
    ldbi.bs         t0, (l2)
    ldbi.bs         t1, (l3)
.L58:
    mulall.s16.s    t2, t0, t1

    bloop           l8, .L57, .L58
.L56:
    asri            t0, t2, 7
    clipi.s32       t0, t0, 8
    st.b            t0, (l6, 0x0)       // pOut
    addu            l6, l6, l9
    addi            l1, 1               // count++
    addu            l2, a0, l1          // px
    mov             l3, a2              // py
.L55:
    subi            l0, 1
    bnez            l0, .L54
.L53:

.L11:

    /* --------------------------
     * Initializations of stage3
     * -------------------------*/
    subi            l1, a3, 1           // blockSize3
    addu            l8, a0, a1
    subu            l8, l1              // pSrc1
    mov             l2, l8              // px
    mov             l3, a2              // py = pIn2

    /* -------------------
     * Stage3 process
     * ------------------*/
    bez             l1, .L29
.L30:
    movi            t2, 0

    lsri            l0, l1, 2           // k
    bez             l0, .L32
.L33:
    ldbi.w          t0, (l2)
    ldbi.w          t1, (l3)
    pext.s8.e       t4, t0
    pext.s8.e       t6, t1
    mulaca.s16.s    t2, t6, t4
.L34:
    mulaca.s16.s    t2, t7, t5

    bloop           l0, .L33, .L34
.L32:

    andi            l0, l1, 3           // k
    bez             l0, .L35
.L36:
    ldbi.bs         t0, (l2)
    ldbi.bs         t1, (l3)
.L37:
    mulall.s16.s    t2, t0, t1

    bloop           l0, .L36, .L37
.L35:

    asri            t0, t2, 7
    clipi.s32       t0, t0, 8
    st.b            t0, (l6, 0x0)
    addu            l6, l6, l9

    addi            l8, 1
    mov             l2, l8              // px = ++pSrc1
    mov             l3, a2

    subi            l1, 1
    bnez            l1, .L30
.L29:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_correlate_q7, .-csky_correlate_q7
