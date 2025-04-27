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
 * @file     csky_conv_q31.S
 * @brief    Convolution of Q31 sequences.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_conv_q31(
 *   q31_t * pSrcA,
 *   uint32_t srcALen,
 *   q31_t * pSrcB,
 *   uint32_t srcBLen,
 *   q31_t * pDst)
 */
    .file           "csky_conv_q31.S"
    .section        .text.csky_conv_q31,"ax",@progbits
    .align          2
    .global         csky_conv_q31
    .type           csky_conv_q31, @function
csky_conv_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9

    ld.w            l6, (sp, 0x28)      // pOut
    cmphs           a1, a3              // srcALen >= srcBLen
    bt              .L0

    mov             l0, a1
    mov             a1, a3              // srcALen
    mov             a3, l0              // srcBLen

    mov             l0, a0
    mov             a0, a2              // pIn1
    mov             a2, l0              // pIn2
.L0:

    subi            l0, a3, 1           // blockSize1 = srcBLen - 1


    /* --------------------------
     * Initializations of stage1
     * -------------------------*/
    movi            l1, 1               // count
    mov             l2, a0              // px
    mov             l3, a2              // py

    movi            l9, 4
    neg.s32.s       l9, l9

    /* ------------------------
     * Stage1 process
     * ----------------------*/

    bez             l0, .L1             // blockSize1 > 0
.L8:
    movi            t0, 0
    movi            t1, 0
    lsri            l4, l1, 2           // k

    bez             l4, .L2
.L3:
    pldbi.d         t2, (l2)
    ldbir.w         t4, (l3), l9
    ldbir.w         t5, (l3), l9
    mula.s32        t0, t2, t4
    mula.s32        t0, t3, t5

    pldbi.d         t2, (l2)
    ldbir.w         t4, (l3), l9
    ldbir.w         t5, (l3), l9
    mula.s32        t0, t2, t4

.L4:
    mula.s32        t0, t3, t5

    bloop           l4, .L3, .L4        // while(k > 0u)

.L2:
    andi            l4, l1, 3           // k
    bez             l4, .L5

.L6:
    ldbi.w          t2, (l2)
    ldbir.w         t3, (l3), l9

.L7:
    mula.s32        t0, t2, t3

    bloop           l4, .L6, .L7

.L5:
    dexti           t2, t0, t1, 31
    stbi.w          t2, (l6)            // pOut

    lsli            l8, l1, 2
    addu            l3, a2, l8          // py = pIn2 + count
    mov             l2, a0              // px = pIn1
.L9:
    addi            l1, 1               // count++
    subi            l0, 1               // blockSize1--
    bhz             l0, .L8
.L1:


    /* --------------------------
     * Initializations of stage2
     * ------------------------*/
    mov             l2, a0              // px = pIn1
    subi            l7, a3, 1           // srcBLen - 1
    subu            l5, a1, l7          // blockSize2
    lsli            l8, l7, 2
    addu            l7, a2, l8          // pSrc2
    mov             l3, l7              // py = pSrc2
    movi            l1, 0               // count = 0
    movi            l4, 3

    /* -------------------
     * Stage2 process
     * ------------------*/

    cmphsi          a3, 4               // srcBLen >= 4
    bf              .L10

    divs            l0, l5, l4          // blkCnt
    bez             l0, .L12

.L13:
    movi            t2, 0               // acc0
    movi            t3, 0
    movi            t4, 0               // acc1
    movi            t5, 0
    movi            t6, 0               // acc2
    movi            t7, 0

    pldbi.d         t0, (l2)            // x0, x1
    divs            l8, a3, l4          // k = srcBLen / 3

.L15:
    ldbi.w          t8, (l2)            // x2
    ldbir.w         t9, (l3), l9        // c0
    mula.s32        t2, t0, t9          // acc0 = x0 * c0
    mula.s32        t4, t1, t9          // acc1 = x1 * c0
    mula.s32        t6, t8, t9          // acc2 = x2 * c0

    ldbi.w          t0, (l2)            // x0
    ldbir.w         t9, (l3), l9        // c0
    mula.s32        t2, t1, t9          // acc0 = x1 * c0
    mula.s32        t4, t8, t9          // acc1 = x2 * c0
    mula.s32        t6, t0, t9          // acc2 = x0 * c0

    ldbi.w          t1, (l2)            // x1
    ldbir.w         t9, (l3), l9        // c0
    mula.s32        t2, t8, t9          // acc0 = x2 * c0
    mula.s32        t4, t0, t9          // acc1 = x0 * c0

.L16:
    mula.s32        t6, t1, t9          // acc2 = x1 * c0

    bloop           l8, .L15, .L16

    divs            l8, a3, l4
    mult            l8, l8, l4
    subu            l8, a3, l8          // k

    bez             l8, .L17
.L18:
    ldbi.w          t8, (l2)            // px
    ld.w            t9, (l3, 0x0)       // py

    mula.s32        t2, t0, t9          // acc0 = x0 * c0
    mula.s32        t4, t1, t9          // acc1 = x1 * c0
    mula.s32        t6, t8, t9          // acc2 = x2 * c0

    mov             t0, t1              // x0 = x1
    mov             t1, t8              // x1 = x2

.L19:
    subi            l3, 4               // py--
    bloop           l8, .L18, .L19

.L17:

    dexti           t0, t2, t3, 31
    stbi.w          t0, (l6)            // pOut
    dexti           t0, t4, t5, 31
    stbi.w          t0, (l6)            // pOut
    dexti           t0, t6, t7, 31
    stbi.w          t0, (l6)            // pOut

    addi            l1, 3               // count += 3
    lsli            l8, l1, 2
    addu            l2, a0, l8          // px = pIn1 + count

.L14:
    mov             l3, l7              // py = pSrc2
    subi            l0, 1
    bhz             l0, .L13
.L12:

    divs            l0, l5, l4
    mult            l0, l0, l4
    subu            l0, l5, l0          // blkCnt

    bez             l0, .L20
.L21:
    movi            t2, 0               // sum
    movi            t3, 0
    lsri            l8, a3, 2           // k

    bez             l8, .L23

.L24:
    pldbi.d         t4, (l2)
    ldbir.w         t7, (l3), l9
    mula.s32        t2, t4, t7
    ldbir.w         t7, (l3), l9
    mula.s32        t2, t5, t7

    pldbi.d         t4, (l2)
    ldbir.w         t7, (l3), l9
    mula.s32        t2, t4, t7
    ldbir.w         t7, (l3), l9

.L25:
    mula.s32        t2, t5, t7

    bloop           l8, .L24, .L25
.L23:

    andi            l8, a3, 3           // k
    bez             l8, .L26

.L27:
    ldbi.w          t4, (l2)
    ldbir.w         t5, (l3), l9

.L28:
    mula.s32        t2, t4, t5
    bloop           l8, .L27, .L28
.L26:

    dexti           t4, t2, t3, 31
    stbi.w          t4, (l6)            // pOut
    addi            l1, 1               // count++

    lsli            l8, l1, 2
    addu            l2, a0, l8          // px
    mov             l3, l7

.L22:
    subi            l0, l0, 1
    bhz             l0, .L21
.L20:
    br              .L11                // else

.L10:

    movi            t6, 4
    neg.s32.s       t6, t6

    mov             l0, l5              // blkCnt = blockSize2
    bez             l0, .L11
.L54:
    movi            t2, 0
    movi            t3, 0               // sum = 0
    mov             l8, a3              // k = srcBLen
    bez             l8, .L56
.L57:
    ldbi.w          t0, (l2)
    ldbir.w         t1, (l3), l9
.L58:
    mula.s32        t2, t0, t1

    bloop           l8, .L57, .L58
.L56:
    dexti           t0, t2, t3, 31
    stbi.w          t0, (l6)            // pOut
    addi            l1, 1               // count++

    lsli            l8, l1, 2
    addu            l2, a0, l1          // px
    mov             l3, l7              // py
.L55:
    subi            l0, 1
    bhz             l0, .L54

.L11:

    /* --------------------------
     * Initializations of stage3
     * -------------------------*/
    subi            l1, a3, 1           // blockSize3
    subu            t0, a1, l1
    lsli            t0, t0, 2
    addu            l8, a0, t0          // pSrc1
    mov             l2, l8              // px
    lsli            t0, l1, 2
    addu            l7, a2, t0
    mov             l3, l7              // py

    /* -------------------
     * Stage3 process
     * ------------------*/
    bez             l1, .L29
.L30:
    movi            t2, 0
    movi            t3, 0

    lsri            l0, l1, 2           // k
    bez             l0, .L32
.L33:
    pldbi.d         t0, (l2)
    ldbir.w         t4, (l3), l9
    mula.s32        t2, t0, t4
    ldbir.w         t4, (l3), l9
    mula.s32        t2, t1, t4

    pldbi.d         t0, (l2)
    ldbir.w         t4, (l3), l9
    mula.s32        t2, t0, t4
    ldbir.w         t4, (l3), l9

.L34:
    mula.s32        t2, t1, t4
    bloop           l0, .L33, .L34
.L32:

    andi            l0, l1, 3           // k
    bez             l0, .L35
.L36:
    ldbi.w          t0, (l2)
    ldbir.w         t1, (l3), l9

.L37:
    mula.s32        t2, t0, t1

    bloop           l0, .L36, .L37
.L35:

    dexti           t0, t2, t3, 31
    stbi.w          t0, (l6)

    addi            l8, 4
    mov             l2, l8
    mov             l3, l7

.L31:
    subi            l1, 1
    bhz             l1, .L30
.L29:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_conv_q31, .-csky_conv_q31
