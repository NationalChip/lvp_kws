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
 * @file     csky_correlate_opt_q15.S
 * @brief    Correlation of Q15 sequences.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_correlate_opt_q15(
 *   q15_t * pSrcA,
 *   uint32_t srcALen,
 *   q15_t * pSrcB,
 *   uint32_t srcBLen,
 *   q15_t * pDst,
 *   q15_t * pScratch)
 */
    .file           "csky_correlate_opt_q15.S"
    .section        .text.csky_correlate_opt_q15,"ax",@progbits
    .align          2
    .global         csky_correlate_opt_q15
    .type           csky_correlate_opt_q15, @function
csky_correlate_opt_q15:
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

    ld.w            l7, (sp, 0x2c)      // pScratch
    mov             t0, l7              // pScr = pScratch

    /* csky_fill_q15 */
    movi            t1, 0
    subi            t4, a3, 1
    lsri            t2, t4, 2
    bez             t2, .L1
.L2:
    stbi.w          t1, (t0)
.L3:
    stbi.w          t1, (t0)

    bloop           t2, .L2, .L3
.L1:

    andi            t2, t4, 3
    bez             t2, .L4
.L5:
    stbi.h          t1, (t0)
    bloop           t2, .L5, .L5
.L4:
    /* csky_fill_q15 end */

    /* csky_copy_q15 */
    lsri            t1, a1, 2
    bez             t1, .L7
.L8:
    pldbi.d         t2, (a0)
    stbi.w          t2, (t0)
.L9:
    stbi.w          t3, (t0)

    bloop           t1, .L8, .L9
.L7:

    andi            t1, a1, 3
    bez             t1, .L10
.L11:
    ldbi.h          t2, (a0)
.L12:
    stbi.h          t2, (t0)

    bloop           t1, .L11, .L12
.L10:
    /* csky_copy_q15 end */

    /* csky_fill_q15 */
    movi            t1, 0
    subi            t4, a3, 1
    lsri            t2, t4, 2
    bez             t2, .L13
.L14:
    stbi.w          t1, (t0)
.L15:
    stbi.w          t1, (t0)

    bloop           t2, .L14, .L15
.L13:

    andi            t2, t4, 3
    bez             t2, .L16
.L17:
    stbi.h          t1, (t0)
    bloop           t2, .L17, .L17
.L16:
    /* csky_fill_q15 end */

    mov             l8, a2              // py = pIn2

    addu            t8, a1, a3
    subi            t8, t8, 1
    lsri            t8, t8, 2           // blkCnt
    bez             t8, .L20
.L21:
    mov             l0, l7              // pScr = pScratch
    movi            t0, 0               // acc0
    movi            t1, 0
    movi            t2, 0               // acc1
    movi            t3, 0
    movi            t4, 0               // acc2
    movi            t5, 0
    movi            t6, 0               // acc3
    movi            t7, 0

    ldbi.w          l2, (l0)            // x1
    ldbi.w          l3, (l0)            // x2

    lsri            t9, a3, 2           // tapCnt
    bez             t9, .L23
.L24:
    ldbi.w          l4, (a2)            // y1
    ldbi.w          l5, (a2)            // y2

    mulaca.s16.e    t0, l4, l2          // acc0
    mulaca.s16.e    t4, l4, l3          // acc2
    pkg             l1, l3, 0, l2, 16   // x3
    mulacax.s16.e   t2, l1, l4          // acc1

    ldbi.w          l2, (l0)            // x1
    mulaca.s16.e    t0, l3, l5          // acc0
    mulaca.s16.e    t4, l2, l5          // acc2

    pkg             l1, l2, 0, l3, 16   // x3
    mulacax.s16.e   t6, l1, l4          // acc3
    mulacax.s16.e   t2, l1, l5          // acc1

    ldbi.w          l3, (l0)            // x2
    pkg             l1, l3, 0, l2, 16   // x3
.L25:
    mulacax.s16.e   t6, l1, l5          // acc3

    bloop           t9, .L24, .L25
.L23:

    subi            l0, l0, 8           // pScr -= 4

    andi            t9, t9, 3           // tapCnt--
    bez             t9, .L26
.L27:
    pldbi.d         l2, (l0)            // pScr
    ldbi.hs         l4, (a2)            // pIn2
    mulall.s16.e    t0, l2, l4          // acc0
    mulahl.s16.e    t2, l2, l4          // acc1
    mulall.s16.e    t4, l3, l4          // acc2
    mulahl.s16.e    t6, l3, l4          // acc3
.L28:
    subi            l0, l0, 6           // pScr -= 3
    bloop           t9, .L27, .L28
.L26:
    dexti           t0, t0, t1, 15
    dexti           t2, t2, t3, 15
    dexti           t4, t4, t5, 15
    dexti           t6, t6, t7, 15
    clipi.s32       t0, t0, 16
    clipi.s32       t2, t2, 16
    clipi.s32       t4, t4, 16
    clipi.s32       t6, t6, 16
    st.h            t0, (l6, 0x0)
    addu            l6, l6, l9
    st.h            t2, (l6, 0x0)
    addu            l6, l6, l9
    st.h            t4, (l6, 0x0)
    addu            l6, l6, l9
    st.h            t6, (l6, 0x0)       // *pOut
    addu            l6, l6, l9          // pOut += inc
    mov             a2, l8              // pIn2 = py

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
    movi            t1, 0               // acc0

    lsri            t9, a3, 2           // tapCnt = srcBLen >> 2
    bez             t9, .L33
.L34:
    pldbi.d         l2, (l0)
    pldbi.d         l4, (a2)
    mulaca.s16.e    t0, l2, l4          // acc0
.L35:
    mulaca.s16.e    t0, l3, l5          // acc0

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

    dexti           t0, t0, t1, 15
    clipi.s32       t0, t0, 16
    st.h            t0, (l6, 0x0)
    addu            l6, l6, l9
    mov             a2, l8              // pIn2 = py

    addi            l7, l7, 2           // pScratch += 1
    subi            t8, t8, 1
    bnez            t8, .L31
.L30:

    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_correlate_opt_q15, .-csky_correlate_opt_q15
