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
 * @file     csky_correlate_fast_q15.S
 * @brief    Correlation of Q15 sequences.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_correlate_fast_q15(
 *   q15_t * pSrcA,
 *   uint32_t srcALen,
 *   q15_t * pSrcB,
 *   uint32_t srcBLen,
 *   q15_t * pDst)
 */
    .file           "csky_correlate_fast_q15.S"
    .section        .text.csky_correlate_fast_q15,"ax",@progbits
    .align          2
    .global         csky_correlate_fast_q15
    .type           csky_correlate_fast_q15, @function
csky_correlate_fast_q15:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9

    ld.w            l6, (sp, 0x28)      // pOut
    movi            l9, 2               // inc = 1
    cmphs           a1, a3              // srcALen >= srcBLen
    bf              .L100
    subu            t0, a1, a3          // j
    lsli            t0, t0, 1
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
    lsli            t0, t0, 1
    addu            l6, l6, t0          // pOut
    neg.s32.s       l9, l9              // inc = -1
.L0:

    subi            l0, a3, 1           // blockSize1 = srcBLen - 1


    /* --------------------------
     * Initializations of stage1
     * -------------------------*/
    movi            l1, 1               // count
    mov             l2, a0              // px
    lsli            l3, l0, 1
    addu            t8, l3, a2          // pSrc1
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
    pldbi.d         t2, (l2)
    pldbi.d         t4, (l3)
    mulaca.s16.s    t0, t2, t4
.L4:
    mulaca.s16.s    t0, t3, t5

    bloop           l4, .L3, .L4
.L2:

    andi            l4, l1, 3           // k
    bez             l4, .L5
.L6:
    ldbi.h          t2, (l2)
    ldbi.h          t3, (l3)
.L7:
    mulall.s16.s    t0, t2, t3

    bloop           l4, .L6, .L7
.L5:
    asri            t2, t0, 15
    st.h            t2, (l6, 0x0)       // pOut
    addu            l6, l6, l9

    lsli            l3, l1, 1
    subu            l3, t8, l3          // py = pSrc1 - count
    mov             l2, a0              // px = pIn1

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
    movi            t4, 0               // acc1
    movi            t6, 0               // acc2
    movi            t8, 0               // acc3

    ld.w            t0, (l2, 0x0)       // x0
    addi            l2, l2, 2
    ld.w            t1, (l2, 0x0)       // x1
    addi            l2, l2, 2
    lsri            l8, a3, 2           // k = srcBLen >> 2

.L15:
    ldbi.w          l4, (l3)            // c0
    mulaca.s16.s    t2, t0, l4          // acc0 = x0 * c0
    mulaca.s16.s    t4, t1, l4          // acc1 = x1 * c0

    ld.w            t0, (l2, 0x0)       // x3
    addi            l2, l2, 2
    ld.w            t1, (l2, 0x0)       // x4
    addi            l2, l2, 2
    mulaca.s16.s    t6, t0, l4          // acc2 = x2 * c0
    mulaca.s16.s    t8, t1, l4          // acc3 = x3 * c0

    ldbi.w          l4, (l3)            // c0
    mulaca.s16.s    t2, t0, l4          // acc0 = x2 * c0
    mulaca.s16.s    t4, t1, l4          // acc1 = x3 * c0

    ld.w            t0, (l2, 0x0)       // x0
    addi            l2, l2, 2
    ld.w            t1, (l2, 0x0)       // x1
    addi            l2, l2, 2
    mulaca.s16.s    t6, t0, l4          // acc2 = x3 * c0
.L16:
    mulaca.s16.s    t8, t1, l4          // acc3 = x4 * c0

    bloop           l8, .L15, .L16

    andi            l8, a3, 3           // k

    cmpnei          l8, 1               // if (k == 1)
    bt              .L17
    ldbi.h          l4, (l3)            // c0 = *py
    ld.w            l7, (l2, 0x0)       // x3

    mulaca.s16.s    t2, l4, t0          // acc0
    mulaca.s16.s    t4, l4, t1          // acc1
    mulacax.s16.s   t6, l4, t1          // acc2
    mulacax.s16.s   t8, l4, l7          // acc3
.L17:

    cmpnei          l8, 2               // if k == 2
    bt              .L18
    ldbi.w          l4, (l3)            // c0
    mulaca.s16.s    t2, l4, t0          // acc0
    mulaca.s16.s    t4, l4, t1          // acc1

    ld.w            t0, (l2, 0x0)       // x3
    addi            l2, l2, 2
    ld.w            t1, (l2, 0x0)       // x2

    mulaca.s16.s    t6, l4, t0          // acc2
    mulaca.s16.s    t8, l4, t1          // acc3
.L18:

    cmpnei          l9, 3               // if k == 3
    bt              .L19
    ldbi.w          l4, (l3)            // c0
    mulaca.s16.s    t2, l4, t0          // acc0
    mulaca.s16.s    t4, l4, t1          // acc1
    ld.w            t0, (l2, 0x0)       // x3
    addi            l2, l2, 2
    ld.w            l7, (l2, 0x0)       // x2
    addi            l2, l2, 2
    mulaca.s16.s    t6, l4, t0          // acc2
    mulaca.s16.s    t8, l4, l7          // acc3

    ldbi.h          l4, (l3)            // c0
    ld.w            t0, (l2, 0x0)       // x3

    mulacax.s16.s   t2, l4, t1          // acc0
    mulaca.s16.s    t4, l4, l7          // acc1
    mulacax.s16.s   t6, l4, l7          // acc2
    mulacax.s16.s   t8, l4, t0          // acc3
.L19:

    asri            t2, t2, 15
    asri            t4, t4, 15
    asri            t6, t6, 15
    asri            t8, t8, 15
    st.h            t2, (l6)            // pOut
    addu            l6, l6, l9
    st.h            t4, (l6)            // pOut
    addu            l6, l6, l9
    st.h            t6, (l6)            // pOut
    addu            l6, l6, l9
    st.h            t8, (l6)            // pOut
    addu            l6, l6, l9

    addi            l1, 4               // count += 4
    lsli            l2, l1, 1
    addu            l2, a0, l2          // px = pIn1 + count
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
    pldbi.d         t4, (l2)
    pldbi.d         t6, (l3)
    mulaca.s16.s    t2, t4, t6
.L25:
    mulaca.s16.s    t2, t5, t7

    bloop           l8, .L24, .L25
.L23:

    andi            l8, a3, 3           // k
    bez             l8, .L26

.L27:
    ldbi.h          t4, (l2)
    ldbi.h          t5, (l3)
.L28:
    mulall.s16.s    t2, t4, t5

    bloop           l8, .L27, .L28
.L26:

    asri            t4, t2, 15
    st.h            t4, (l6, 0x0)       // pOut
    addu            l6, l6, l9
    addi            l1, 1               // count++

    lsli            l2, l1, 1
    addu            l2, a0, l2          // px
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
    ldbi.h          t0, (l2)
    ldbi.h          t1, (l3)
.L58:
    mulall.s16.s    t2, t0, t1
    bloop           l8, .L57, .L58
.L56:
    asri            t0, t2, 15
    st.h            t0, (l6, 0x0)       // pOut
    addu            l6, l6, l9
    addi            l1, 1               // count++
    lsli            l2, l1, 1
    addu            l2, a0, l2          // px
    mov             l3, a2              // py

    subi            l0, 1
    bnez            l0, .L54
.L53:

.L11:

    /* --------------------------
     * Initializations of stage3
     * -------------------------*/
    subi            l1, a3, 1           // blockSize3
    subu            l8, a1, l1
    lsli            l8, l8, 1
    addu            l8, a0, l8
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
    pldbi.d         t0, (l2)
    pldbi.d         t4, (l3)
    mulaca.s16.s    t2, t0, t4
.L34:
    mulaca.s16.s    t2, t1, t5

    bloop           l0, .L33, .L34
.L32:

    andi            l0, l1, 3           // k
    bez             l0, .L35
.L36:
    ldbi.h          t0, (l2)
    ldbi.h          t1, (l3)
.L37:
    mulall.s16.s    t2, t0, t1

    bloop           l0, .L36, .L37
.L35:

    asri            t0, t2, 15
    st.h            t0, (l6, 0x0)
    addu            l6, l6, l9

    addi            l8, 2
    mov             l2, l8              // px = ++pSrc1
    mov             l3, a2

    subi            l1, 1
    bnez            l1, .L30
.L29:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_correlate_fast_q15, .-csky_correlate_fast_q15
