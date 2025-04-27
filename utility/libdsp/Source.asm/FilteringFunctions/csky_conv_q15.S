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
 * @file     csky_conv_q15.S
 * @brief    Convolution of Q15 sequences.
 * @version  V1.0
 * @date     18. Aug 2017
 ******************************************************************************/

/*
 * void csky_conv_q15(
 *   q15_t * pSrcA
 *   uint32_t srcALen,
 *   q15_t * pSrcB,
 *   uint32_t srcBLen
 *   q15_t * pDst)
 */

    .file           "csky_conv_q15.S"
    .section        .text.csky_conv_q15,"ax",@progbits
    .align          2
    .global         csky_conv_q15
    .type           csky_conv_q15, @function

csky_conv_q15:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    ld.w            l6, (sp, 0x28)      // *pOut = pDst
    cmphs           a1, a3              // if(srcAlen >= srcBlen)
    bt              .L0

    mov             l0, a1              // else branch
    mov             a1, a3
    mov             a3, l0

    mov             l0, a0
    mov             a0, a2
    mov             a2, l0

.L0:
    subi            l0, a3, 1           // if branch
    
    /* -------------------------
     * Initializations of stage1
     * -------------------------*/
    movi            l1, 1               // count = 1u
    mov             l2, a0              // px    = pIn1
    mov             l3, a2              // py    = pIn2

.L2:
    bez             l0, .L1             // blockSize1 > 0
    movi            t0, 0               // sum = 0
    movi            t1, 0
    mov             t2, l1              // k = count

.L3:
    movi            t3, 0
    movi            t4, 0
    ldbi.hs         t3, (l2)            // sum = __SMLALD(*px++, *py--, sum)
    ld.hs           t4, (l3, 0x0)
    subi            l3, 2

.L4:
    mulaca.s16.e    t0, t3, t4
    bloop           t2, .L3, .L4        // while loop

    dexti           t0, t0, t1, 15      // sum >> 15
    clipi.s32       t0, t0, 16          // saturate
    stbi.h          t0, (l6)            // store the result
    mov             l2, a0              // px = pIn1
    lsli            t0, l1, 1
    add             l3, a2, t0          // py = pIn2 + count

    subi            l0, 1               // blockSize1--
    addi            l1, 1               // count++
    cmphsi          l1, 4               // while loop
    bf              .L2

    subi            l3, 2               // py = py - 1
    bez             l0, .L1
.L5:
    movi            t4, 0               // sum = 0
    movi            t5, 0
    lsri            t0, l1, 2           // k = count >> 2u

.L6:
    pldbi.d         t2, (l2)
    ld.w            t1, (l3, 0x0)
    subi            l3, 4
    mulacax.s16.e   t4, t2, t1          // x[0], x[1] are multiplied with y[srcBLen - 1], y[srcBLen - 2]
    ld.w            t1, (l3, 0x0)       // x[2], x[3] are multiplied with y[srcBLen - 3], y[arcBLen - 4]
    subi            l3, 4

.L7:
    mulacax.s16.e   t4, t3, t1
    bloop           t0, .L6, .L7        // while loop

    addi            l3, 2               // py = py + 1u
    andi            t0, l1, 3           // k = count % 0x4
    bez             t0, .L45

.L8:
    ldbi.hs         t1, (l2)
    ld.hs           t2, (l3, 0x0)
    mulall.s16.e    t4, t2, t1          // sum = __SMLALD(*px++, *py--, sum)

.L9:
    subi            l3, 2               // py--
    bloop           t0, .L8, .L9        // while loop

.L45:
    dexti           t4, t4, t5, 15      // sum >> 15
    clipi.s32       t4, t4, 16          // saturate
    stbi.h          t4, (l6)            // store the result
    subi            t6, l1, 1
    lsli            t6, t6, 1
    add             l3, a2, t6          // py = pIn2 + (count - 1u)
    mov             l2, a0              // px = pIn1
    addi            l1, 1               // count++
    subi            l0, 1               // blockSize1--
    bhz             l0, .L5             // while loop

    /* --------------------------
     * Initializations of stage2
     * -------------------------*/
.L1:
    mov             l2, a0              // px = pIn1
    subi            l0, a3, 1           // py = pIn2 + (srcBLen -1u)
    lsli            t1, l0, 1
    add             l5, a2, t1
    mov             l3, l5
    movi            l1, 0               // count = 0u
    sub             l0, a1, l0          // blockSize2
    
    cmphsi          a3, 4               // if(srcBLen >= 4u)
    bf              .L10
    lsri            l7, l0, 2           // if branch, blkCnt = blockSize2 >> 2u
    bez             l7, .L16            // while(blkCnt > 0u)

.L12:
    subi            l3, 2               // py   = py - 1u
    movi            t4, 0               // acc0 = 0
    movi            t5, 0
    movi            t6, 0               // acc1 = 0
    movi            t7, 0
    movi            t8, 0               // acc2 = 0
    movi            t9, 0
    movi            l8, 0               // acc3 = 0
    movi            l9, 0
    
    ldbi.w          t0, (l2)            // x0
    subi            l2, l2, 2
    ldbi.w          t1, (l2)            // x1
    subi            l2, l2, 2
    lsri            t2, a3, 2           // k  = srcBLen >> 2u
    bez             t2, .L44

.L13:
    ld.w            t3, (l3, 0x0)       // c0
    subi            l3, l3, 4           // py  -= 2
    mulacax.s16.e   t4, t0, t3          // acc0 = __SMLALDX(x0, c0, acc0)
    mulacax.s16.e   t6, t1, t3          // acc1 = __SMLALDX(x1, c0, acc1)

    ldbi.w          t0, (l2)            // x2
    subi            l2, l2, 2
    ldbi.w          t1, (l2)            // x3
    subi            l2, l2, 2
    mulacax.s16.e   t8, t0, t3          // acc2 = __SMLALDX(x2, c0, acc2)
    mulacax.s16.e   l8, t1, t3          // acc3 = __SMLALDX(x3, c0, acc3)

    ld.w            t3, (l3, 0x0)       // c0
    subi            l3, l3, 4           // py  -= 2
    mulacax.s16.e   t4, t0, t3          // acc0 = __SMLALDX(x2, c0, acc0)
    mulacax.s16.e   t6, t1, t3          // acc1 = __SMLALDX(x3, c0, acc1)

    ldbi.w          t0, (l2)            // x0
    subi            l2, l2, 2
    ldbi.w          t1, (l2)            // x1
    subi            l2, l2, 2
    mulacax.s16.e   t8, t0, t3          // acc2 = __SMLALDX(x0, c0, acc2)

.L14:
    mulacax.s16.e   l8, t1, t3          // acc3 = __SMLALDX(x1, c0, acc3)

    bloop           t2, .L13, .L14       // while loop

.L44:
    movi            t3, 0
    andi            t2, a3, 3           // k = srcBLen % 0x4u
    bez             t2, .L46
    cmpnei          t2, 1               // if(k == 1u)
    bt              .L15
    ld.hs           t3, (l3, 0x02)      // c0   = *(py + 1)
    ldbi.w          l4, (l2)            // x3
    mulaca.s16.e    t4, t0, t3          // acc0 = __SMLALD(x0, c0, acc0)
    mulaca.s16.e    t6, t1, t3          // acc1 = __SMLALD(x1, c0, acc1)
    mulacax.s16.e   t8, t1, t3          // acc2 = __SMLALDX(x1, c0, acc2)
    mulacax.s16.e   l8, l4, t3          // acc3 = __SMLALDX(x3, c0, acc3)
    br              .L46

.L15:
    cmpnei          t2, 2               // if(k == 2u)
    bt              .L17
    ld.w            t3, (l3, 0x0)       // c0
    mulacax.s16.e   t4, t0, t3          // acc0 = __SMLALDX(x0, c0, acc0)
    mulacax.s16.e   t6, t1, t3          // acc1 = __SMLALDX(x1, c0, acc1)
    ldbi.w          t0, (l2)            // x3
    subi            l2, l2, 2
    ldbi.w          t1, (l2)            // x2
    subi            l2, l2, 2
    mulacax.s16.e   t8, t0, t3          // acc2 = __SMLALDX(x3, c0, acc2)
    mulacax.s16.e   l8, t1, t3          // acc3 = __SMLALDX(x2, c0, acc3)
    br              .L46

.L17:
    ld.w            t3, (l3, 0x0)       // c0
    mulacax.s16.e   t4, t0, t3          // acc0 = __SMLALDX(x0, c0, acc0)
    mulacax.s16.e   t6, t1, t3          // acc1 = __SMLALDX(x1, c0, acc1)
    ldbi.w          t0, (l2)            // x3
    subi            l2, l2, 2
    ldbi.w          l4, (l2)            // x2
    subi            l2, l2, 2
    mulacax.s16.e   t8, t0, t3          // acc2 = __SMLALDX(x3, c0, acc2)
    mulacax.s16.e   l8, l4, t3          // acc3 = __SMLALDX(x2. c0, acc3)

    subi            l3, l3, 2
    ld.hs           t3, (l3)            // c0   = *(py - 1)
    ldbi.w          t0, (l2)            // x3
    mulacax.s16.e   t4, t1, t3          // acc0 = __SMLALDX(x1, c0, acc0)
    mulaca.s16.e    t6, l4, t3          // acc1 = __SMLALD(x2, c0, acc1)
    mulacax.s16.e   t8, l4, t3          // acc2 = __SMLALDX(x2, c0, acc2)
    mulacax.s16.e   l8, t0, t3          // acc3 = __SMLALDX(x3, c0, acc3)

.L46:
    dexti           t4, t4, t5, 15      // acc0 >> 15
    clipi.s32       t4, t4, 16          // saturate
    stbi.h          t4, (l6)            // store acc0
    dexti           t6, t6, t7, 15      // acc1
    clipi.s32       t6, t6, 16
    stbi.h          t6, (l6)
    dexti           t8, t8, t9, 15      // acc2
    clipi.s32       t8, t8, 16
    stbi.h          t8, (l6)
    dexti           l8, l8, l9, 15      // acc3
    clipi.s32       l8, l8, 16
    stbi.h          l8, (l6)

    addi            l1, l1, 4           // count += 4
    lsli            l9, l1, 1
    add             l2, a0, l9          // px = pIn1 + count
    mov             l3, l5              // py = pSrc2

.L18:
    subi            l7, 1
    bhz             l7, .L12            // while loop

.L16:
    andi            l7, l0, 3           // blkCnt = blockSize2 % 0x4u
    bez             l7, .L41            // while(blkCnt > 0u)

.L19:
    movi            t4, 0               // sum = 0
    movi            t5, 0

    lsri            t6, a3, 2           // k = srcBLen >> 2u
    andi            t7, a3, 3           // k = srcBLen % 0x4u
    bez             t6, .L22            // while(k > 0u)

.L20:
    ldbi.w          t8, (l2)
    subi            l3, 2
    ld.w            t9, (l3, 0x0)
    subi            l3, 2
    mulacax.s16.e   t4, t8, t9          // sum += (q63_t)((q31_t) *px++ * *py--)

    ldbi.w          t8, (l2)
    subi            l3, 2
    ld.w            t9, (l3, 0x0)
    subi            l3, 2

.L21:
    mulacax.s16.e   t4, t8, t9          // repeat
    bloop           t6, .L20, .L21      // while loop

.L22:
    bez             t7, .L40            // while(k >0)

.L23:
    ldbi.hs         t8, (l2)
    ld.hs           t9, (l3)
    subi            l3, l3, 2

.L24:
    mulall.s16.e    t4, t8, t9          // sum += (q63_t)((q31_t) *px++ * *py--)
    bloop           t7, .L23, .L24      // while loop

.L40:
    dexti           t4, t4, t5, 15      // sum >> 15
    clipi.s32       t4, t4, 16          // saturate
    stbi.h          t4, (l6)            // store the result

    addi            l1, l1, 1           // count++
    lsli            t4, l1, 1
    add             l2, a0, t4          // px = pIn1 + count
    mov             l3, l5              // py = pSrc2

.L25:
    subi            l7, l7, 1           // blkCnt--
    bhz             l7, .L19            // while loop
    br              .L41

.L10:
    mov             l7, l0              // else branch, blkCnt = blockSize2

.L26:
    movi            t4, 0               // sum = 0
    movi            t5, 0

    mov             t6, a3              // k   = SrcBLen

.L27:
    ldbi.hs         t8, (l2)            // sum += (q63_t)((q31_t) *px++ * *py--)
    ld.hs           t9, (l3, 0x0)
    subi            l3, l3, 2

.L28:
    mulall.s16.e    t4, t8, t9

    bloop           t6, .L27, .L28      // while loop

    dexti           t4, t4, t5, 15      // sum >> 15
    clipi.s32       t4, t4, 16          // saturate
    stbi.h          t4, (l6)            // store the result
    addi            l1, l1, 1           // count++
    lsli            t8, l1, 1
    add             l2, l2, t8          // px = pIn1 + count
    mov             l3, l5              // py

.L29:
    subi            l7, l7, 1           // blkCnt--
    bhz             l7, .L26            // while loop

    /* -------------------------
     * Initialization of stage3
     * ------------------------*/
.L41:
    subi            l4, a3, 1           // blockSize3 = srcBLen - 1u
    bez             l4, .L42
    sub             l5, a1, l4          // srcALen - (srcBLen - 1u)
    lsli            t0, l5, 1
    add             l2, a0, t0          // px = pSrc1
    mov             l8, l2

    lsli            t0, l4, 1
    add             l9, a2, t0          // pSrc2 = pIn2 + (srcBLen - 1u)
    subi            a2, l9, 2           // pIn2  = pSrc2 - 1u
    mov             l3, a2              // py    = pSrc1

    lsri            t2, l4, 2           // j     = blockSize3 >> 2u
    bez             t2, .L47

.L30:
    bez             l4, .L42            // while(blockSize3 > 0u)
    lsri            t3, l4, 2           // k   = blockSize3 >> 2u
    movi            t4, 0               // sum = 0
    movi            t5, 0
    bez             t3, .L48            // while(k > 0u)

.L31:
    ldbi.w          t0, (l2)            // x[srcALen - srcBLen + 1], x[srcALen - srcBlen + 2] are multiplied 
    ld.w            t1, (l3, 0x0)       // with y[srcBlen - 1], y[secBlen - 2] respectively.
    subi            l3, l3, 4
    mulacax.s16.e   t4, t0, t1

    ldbi.w          t0, (l2)            // x[srcALen - srcBLen + 3], x[srcALen - srcBlen + 4] are multiplied 
    ld.w            t1, (l3, 0x0)       // with y[srcBlen - 3], y[secBlen - 4] respectively.
    subi            l3, l3, 4

.L32:
    mulacax.s16.e   t4, t0, t1

    bloop           t3, .L31, .L32      // while loop

.L48:
    addi            l3, 2
    andi            t9, l4, 3           // k  = blockSize3 % 0x04u
    bez             t9, .L43

.L33:
    ldbi.hs         t0, (l2)            // sum = __SMLALD(*px++, *py--, sum)
    ld.hs           t1, (l3, 0x0)
    subi            l3, 2

.L34:
    mulall.s16.e    t4, t0, t1

    bloop           t9, .L33, .L34      // while loop

.L43:
    dexti           t4, t4, t5, 15      // sum >> 15
    clipi.s32       t4, t4, 16          // saturate
    stbi.h          t4, (l6)            // store the data
    addi            l8, l8, 2           // px = ++pSrc1
    mov             l2, l8
    mov             l3, a2              // py = pIn2

.L35:
    subi            t2, 1               // j--
    subi            l4, 1               // blockSize3--
    bhz             t2, .L30            // while loop

.L47:
    bez             l4, .L42
    addi            l3, 2
.L36:
    mov             t3, l4              // k = blockSize3

    movi            t4, 0               // sum = 0
    movi            t5, 0

.L37:
    ldbi.hs         t0, (l2)            // sum = __SMLAD(*px++, *py--, sum)
    ld.hs           t1, (l3, 0x0)
    subi            l3,  2

.L38:
    mulall.s16.e    t4, t0, t1

    bloop           t3, .L37, .L38      // while loop

    dexti           t4, t4, t5, 15      // sum >> 15
    clipi.s32       t4, t4, 16          // saturate
    stbi.h          t4, (l6)
    addi            l8, 2               // px = ++pSrc1
    mov             l2, l8
    mov             l3, l9              // py = pSrc2

.L39:
    subi            l4, 1               // blockSize3--
    bhz             l4, .L36            // while loop

.L42:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_conv_q15, .-csky_conv_q15
