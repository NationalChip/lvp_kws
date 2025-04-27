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
 * @file     csky_correlate_opt_q7.S
 * @brief    Correlation of Q7 sequences.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_correlate_opt_q7(
 *   q7_t * pSrcA,
 *   uint32_t srcALen,
 *   q7_t * pSrcB,
 *   uint32_t srcBLen,
 *   q7_t * pDst,
 *   q15_t * pScratch1,
 *   q15_t * pScratch2)
 */
    .file           "csky_correlate_opt_q7.S"
    .section        .text.csky_correlate_opt_q7,"ax",@progbits
    .align          2
    .global         csky_correlate_opt_q7
    .type           csky_correlate_opt_q7, @function
csky_correlate_opt_q7:
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

    ld.w            l7, (sp, 0x2c)      // pScratch1
    ld.w            l8, (sp, 0x30)      // pScratch2
    mov             t0, l7              // pScr1 = pScratch1
    mov             t1, l8              // pScr2 = pScratch2

    lsri            t2, a3, 3           // srcBLen >> 3
    bez             t2, .L1
.L2:
    pldbi.d         l0, (a2)            // pIn2
    pext.s8.e       t4, l0
    stbi.w          t4, (t1)            // pSrc2
    stbi.w          t5, (t1)
    pext.s8.e       t4, l1
    stbi.w          t4, (t1)
.L3:
    stbi.w          t5, (t1)

    bloop           t2, .L2, .L3
.L1:

    andi            t2, a3, 7
    bez             t2, .L4
.L5:
    ldbi.bs         l0, (a2)            // pIn2
.L6:
    stbi.h          t1, (t0)

    bloop           t2, .L5, .L6
.L4:

    /* csky_fill_q15 */
    movi            t3, 0
    subi            t4, a3, 1
    lsri            t2, t4, 2
    bez             t2, .L73
.L64:
    stbi.w          t3, (t0)
.L67:
    stbi.w          t3, (t0)

    bloop           t2, .L64, .L67
.L73:

    andi            t2, t4, 3
    bez             t2, .L76
.L77:
    stbi.h          t3, (t0)
    bloop           t2, .L77, .L77
.L76:
    /* csky_fill_q15 end */


    lsri            t2, a1, 3           // k = srcALen >> 2
    bez             t2, .L7
.L8:
    pldbi.d         l0, (a0)
    pext.s8.e       l2, l0
    stbi.w          l2, (t0)
    stbi.w          l3, (t0)
    pext.s8.e       l2, l1
    stbi.w          l2, (t0)
.L9:
    stbi.w          l3, (t0)

    bloop           t2, .L8, .L9
.L7:

    andi            t1, a1, 7
    bez             t1, .L10
.L11:
    ldbi.bs         t2, (a0)
.L12:
    stbi.h          t2, (t0)

    bloop           t2, .L11, .L12
.L10:

    /* csky_fill_q15 */
    movi            t3, 0
    subi            t4, a3, 1
    lsri            t2, t4, 2
    bez             t2, .L13
.L14:
    stbi.w          t3, (t0)
.L15:
    stbi.w          t3, (t0)

    bloop           t2, .L14, .L15
.L13:

    andi            t2, t4, 3
    bez             t2, .L16
.L17:
    stbi.h          t3, (t0)
    bloop           t2, .L17, .L17
.L16:
    /* csky_fill_q15 end */

    mov             a2, l8              // py = pScratch2
    mov             a0, l8              // pScr2 = pScratch2

    addu            t8, a1, a3
    subi            t8, t8, 1
    lsri            t8, t8, 2           // blkCnt
    bez             t8, .L20
.L21:
    mov             l0, l7              // pScr1 = pScratch1
    movi            t0, 0               // acc0
    movi            t1, 0
    movi            t2, 0               // acc1
    movi            t3, 0

    ldbi.w          l2, (l0)            // x1
    ldbi.w          l3, (l0)            // x2

    lsri            t9, a3, 2           // tapCnt
    bez             t9, .L23
.L24:
    ldbi.w          l4, (a0)            // y1
    ldbi.w          l5, (a0)            // y2

    mulaca.s16.s    t0, l4, l2          // acc0
    mulaca.s16.s    t2, l4, l3          // acc2
    pkg             l1, l3, 0, l2, 16   // x3
    mulacax.s16.s   t1, l1, l4          // acc1

    ldbi.w          l2, (l0)            // x1
    mulaca.s16.s    t0, l3, l5          // acc0
    mulaca.s16.s    t2, l2, l5          // acc2

    pkg             l1, l2, 0, l3, 16   // x3
    mulacax.s16.s   t3, l1, l4          // acc3
    mulacax.s16.s   t1, l1, l5          // acc1

    ldbi.w          l3, (l0)            // x2
    pkg             l1, l3, 0, l2, 16   // x3
.L25:
    mulacax.s16.s   t3, l1, l5          // acc3

    bloop           t9, .L24, .L25
.L23:

    subi            l0, l0, 8           // pScr -= 4

    andi            t9, t9, 3           // tapCnt--
    bez             t9, .L26
.L27:
    pldbi.d         l2, (l0)            // pScr
    ldbi.hs         l4, (a0)            // pIn2
    mulall.s16.s    t0, l2, l4          // acc0
    mulahl.s16.s    t1, l2, l4          // acc1
    mulall.s16.s    t2, l3, l4          // acc2
    mulahl.s16.s    t3, l3, l4          // acc3
.L28:
    subi            l0, l0, 6           // pScr -= 3
    bloop           t9, .L27, .L28
.L26:
    asri            t0, t0, 7
    asri            t1, t1, 7
    asri            t2, t2, 7
    asri            t3, t3, 7
    clipi.s32       t0, t0, 8
    clipi.s32       t1, t1, 8
    clipi.s32       t2, t2, 8
    clipi.s32       t3, t3, 8
    st.b            t0, (l6, 0x0)
    addu            l6, l6, l9
    st.b            t1, (l6, 0x0)
    addu            l6, l6, l9
    st.b            t2, (l6, 0x0)
    addu            l6, l6, l9
    st.b            t3, (l6, 0x0)       // *pOut
    addu            l6, l6, l9          // pOut += inc
    mov             a0, l8              // pIn2 = py

.L22:
    addi            l7, l7, 8           // pScratch += 4
    subi            t8, t8, 1
    bnez            t8, .L21
.L20:

    addu            t8, a1, a3
    subi            t8, t8, 1
    andi            t8, t8, 3           // blkCnt
    bez             t8, .L30
.L31:
    mov             l0, l7              // pSrc = pScratch
    movi            t0, 0

    lsri            t9, a3, 2           // tapCnt = srcBLen >> 2
    bez             t9, .L33
.L34:
    pldbi.d         l2, (l0)
    pldbi.d         l4, (a2)
    mulaca.s16.s    t0, l2, l4          // acc0
.L35:
    mulaca.s16.s    t0, l3, l5          // acc0

    bloop           t9, .L34, .L35
.L33:

    andi            t9, a3, 3           // tapCnt = srcBLen % 4
    bez             t9, .L36
.L37:
    ldbi.hs         l2, (l0)
    ldbi.hs         l4, (a2)
.L38:
    mulall.s16.e    t0, l2, l4          // acc0

    bloop           t9, .L37, .L38
.L36:

    asri            t0, t0, 7
    clipi.s32       t0, t0, 8
    st.b            t0, (l6, 0x0)
    addu            l6, l6, l9
    mov             a2, l8              // pIn2 = py

    addi            l7, l7, 2           // pScratch += 1
    subi            t8, t8, 1
    bnez            t8, .L31
.L30:

    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_correlate_opt_q7, .-csky_correlate_opt_q7
