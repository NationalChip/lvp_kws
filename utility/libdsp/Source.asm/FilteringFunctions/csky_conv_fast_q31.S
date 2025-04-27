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
 * @file     csky_conv_fast_q31.S
 * @brief    Fast Q31 Convolution.
 * @version  V1.0
 * @date     29. Aug 2017
 ******************************************************************************/

/*
 * void csky_conv_fast_q31(
 *   q31_t * pSrcA
 *   uint32_t srcALen,
 *   q31_t * pSrcB,
 *   uint32_t srcBLen
 *   q31_t * pDst)
 */

    .file           "csky_conv_fast_q31.S"
    .section        .text.csky_conv_fast_q31,"ax",@progbits
    .align          2
    .global         csky_conv_fast_q31
    .type           csky_conv_fast_q31, @function

csky_conv_fast_q31:
    push            l0, l1, l2, l3, l4, l5, l6
    ld.w            l0, (sp, 0x1c)      // *pOut = pDst
    cmphs           a1, a3              // if(aecALen >= srcBLen)
    bt              .L0

    mov             t0, a1              // else branch
    mov             a1, a3
    mov             a3, t0

    mov             t0, a0
    mov             a0, a2
    mov             a2, t0

.L0:
    subi            l1, a3, 1           // blockSize1 = srcBLen - 1u

    /* ------------------------
     * Initialization of stage1
     * -----------------------*/

    movi            l2, 1               // count = 1u
    mov             l3, a0              // px    = pIn1
    mov             l4, a2              // py    = Pin2

    /* ------------------------
     *     Stage1 processing
     * -----------------------*/
    bez             l1, .L1             // while(blockSize1 > 0u)

.L3:
    movi            t6, 0               // sum = 0
    lsri            t0, l2, 2           // k   = count >> 2u
    bez             t0, .L6             // while(k > 0u)

.L4:
    pldbi.d         t2, (l3)            // x[0] * y[srcBLen - 1]
    ld.w            t4, (l4, 0x0)
    subi            l4, 4
    mula.s32.hs     t6, t2, t4

    ld.w            t4, (l4, 0x0)       // x[1] * y[srcBLen - 2]
    subi            l4, 4
    mula.s32.hs     t6, t3, t4

    pldbi.d         t2, (l3)            // x[2] * y[srcBLen - 3]
    ld.w            t4, (l4, 0x0)
    subi            l4, 4
    mula.s32.hs     t6, t2, t4

    ld.w            t4, (l4, 0x0)       // x[3] * y[srcBLen - 4]
    subi            l4, 4

.L5:
    mula.s32.hs     t6, t3, t4
    bloop           t0, .L4, .L5        // while loop

.L6:
    andi            t0, l2, 3           // k = count % 0x4u
    bez             t0, .L9             // while(k > 0u)

.L7:
    ldbi.w          t2, (l3)            // perform the multiply and accumulate
    ld.w            t3, (l4)
    subi            l4, 4

.L8:
    mula.s32.hs     t6, t2, t3
    bloop           t0, .L7, .L8        // while loop

.L9:
    lsli.s32.s      t6, t6, 1           // *pOut++ = sum << 1
    stbi.w          t6, (l0)
    
    lsli            t1, l2, 2           // py      = pIn2 + count
    add             l4, a2, t1
    mov             l3, a0              // px      = pIn1

.L10:
    addi            l2, l2, 1           // count++
    subi            l1, l1, 1           // blockSize--
    bhz             l1, .L3             // while loop

    /* --------------------------
     * Initializations of stage2
     * -------------------------*/
.L1:
    mov             l3, a0              // px = pIn1
    subi            l1, a3, 1
    lsli            t1, l1, 2           // pSrc2
    add             l5, a2, t1
    sub             l1, a1, l1
    mov             l4, l5              // py    = pSrc2
    movi            l2, 0               // count = 0u

    /* --------------------------
     *      Stage2 processing
     * ------------------------*/
    cmphsi          a3, 4               // if(srcBlen >= 4u)
    bf              .L12
    lsri            t0, l1, 2           // blkCnt = blockSize2 >> 2u
    bez             t0, .L19            // while(blkCnt > 0u)

.L11:
    movi            t6, 0               // acc0 = 0
    movi            t7, 0               // acc1 = 0
    movi            t8, 0               // acc2 = 0
    movi            t9, 0               // acc3 = 0

    pldbi.d         t2, (l3)            // load x[0], x[1] and x[2] samples
    ldbi.w          t4, (l3)
    lsri            t1, a3, 2           // k    = srcBLen >> 2u
    bez             t1, .L41

.L13:
    ld.w            l6, (l4, 0x0)       // c0 = *py--
    subi            l4, l4, 4
    ldbi.w          t5, (l3)            // x3 = *px++
    mula.s32.hs     t6, t2, l6          // acc0
    mula.s32.hs     t7, t3, l6          // acc1
    mula.s32.hs     t8, t4, l6          // acc2
    mula.s32.hs     t9, t5, l6          // acc3

    ld.w            l6, (l4, 0x0)       // repeat four times
    subi            l4, l4, 4
    ldbi.w          t2, (l3)
    mula.s32.hs     t6, t3, l6
    mula.s32.hs     t7, t4, l6
    mula.s32.hs     t8, t5, l6
    mula.s32.hs     t9, t2, l6

    ld.w            l6, (l4, 0x0)
    subi            l4, l4, 4
    ldbi.w          t3, (l3)
    mula.s32.hs     t6, t4, l6
    mula.s32.hs     t7, t5, l6
    mula.s32.hs     t8, t2, l6
    mula.s32.hs     t9, t3, l6

    ld.w            l6, (l4, 0x0)
    subi            l4, l4, 4
    ldbi.w          t4, (l3)
    mula.s32.hs     t6, t5, l6
    mula.s32.hs     t7, t2, l6
    mula.s32.hs     t8, t3, l6

.L14:
    mula.s32.hs     t9, t4, l6
    bloop           t1, .L13, .L14      // while loop

.L41:
    andi            t1, a3, 3           // k = srcBLen % 0x4u
    bez             t1, .L17            // while(k > 0u)

.L15:
    ld.w            l6, (l4, 0x0)       // c0 = *py--
    subi            l4, l4, 4
    ldbi.w          t5, (l3)            // x3 = *px++
    mula.s32.hs     t6, t2, l6          // acc0
    mula.s32.hs     t7, t3, l6          // acc1
    mula.s32.hs     t8, t4, l6          // acc2
    mula.s32.hs     t9, t5, l6          // acc3

    mov             t2, t3              // x0 = x1
    mov             t3, t4              // x1 = x2

.L16:
    mov             t4, t5              // x2 = x3
    bloop           t1, .L15, .L16      // while loop

.L17:
    lsli.s32.s      t6, t6, 1           // process and store the results
    lsli.s32.s      t7, t7, 1
    lsli.s32.s      t8, t8, 1
    lsli.s32.s      t9, t9, 1
    stbi.w          t6, (l0)
    stbi.w          t7, (l0)
    stbi.w          t8, (l0)
    stbi.w          t9, (l0)

    addi            l2, l2, 4           // count += 4u
    lsli            l6, l2, 2           // px     = pIn1 + count
    add             l3, l6, a0
    mov             l4, l5              // py = pSrc2

.L18:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L11            // while loop

.L19:
    andi            t0, l1, 3           // blkCnt = blockSize2 % 0x4u
    bez             t0, .L2             // while(blkCnt > 0u)

.L20:
    movi            t6, 0               // sum = 0
    lsri            t1, a3, 2           // k = srcBLen >> 2u
    andi            t9, a3, 3           // k = srcBLen % 0x4u
    bez             t1, .L23            // while(k > 0u)

.L21:
    pldbi.d         t2, (l3)            // perform the multiply and accumulate
    ld.w            t4, (l4, 0x0)
    subi            l4, l4, 4
    mula.s32.hs     t6, t2, t4

    ld.w            t4, (l4, 0x0)       // repeat four times
    subi            l4, l4, 4
    mula.s32.hs     t6, t3, t4

    pldbi.d         t2, (l3)
    ld.w            t4, (l4, 0x0)
    subi            l4, l4, 4
    mula.s32.hs     t6, t2, t4

    ld.w            t4, (l4, 0x0)
    subi            l4, l4, 4

.L22:
    mula.s32.hs     t6, t3, t4
    bloop           t1, .L21, .L22      // while loop

.L23:
    bez             t9, .L26            // while(k > 0u)

.L24:
    ldbi.w          t2, (l3)            // perform the multiply and accumulate
    ld.w            t3, (l4, 0x0)
    subi            l4, l4, 4

.L25:
    mula.s32.hs     t6, t2, t3
    bloop           t9, .L24, .L25      // while loop

.L26:
    lsli.s32.s      t6, t6, 1           // process and store the result
    stbi.w          t6, (l0)
    addi            l2, l2, 1           // count++
    lsli            l6, l2, 2           // px = pIn1 + count
    add             l3, a0, l6
    mov             l4, l5              // py = pSrc2

.L27:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L20            // while loop
    br              .L2

.L12:                                   // else branch
    mov             t0, l1              // blkCnt = blockSize2
    bez             t0, .L2             // while(blkCnt > 0u)

.L28:
    movi            t6, 0               // sum = 0
    mov             t1, a3              // k   = srcBLen

.L29:
    ldbi.w          t2, (l3)            // perform multiply and accumulate
    ld.w            t3, (l4, 0x0)
    subi            l4, l4, 4

.L30:
    mula.s32.hs     t6, t2, t3
    bloop           t1, .L29, .L30      // while loop

    lsli.s32.s      t6, t6, 1           // process and store the result
    addi            l2, l2, 1           // count++
    lsli            l6, l2, 2
    add             l3, a0, l6          // px = pIn1 + count
    mov             l4, l5              // py = pSrc2

.L31:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L28            // while loop

    /* --------------------------
     * Initializations of stage3
     * ------------------------*/
.L2:
    lsli            l6, l1, 2
    add             l5, a0, l6          // pSrc1
    mov             l3, l5              // px = pSrc1

    subi            l1, a3, 1
    lsli            l6, l1, 2
    add             l6, a2, l6          // pSrc2
    mov             l4, l6              // py = pSrc2

    /* ------------------------
     *    Stage3 processing
     * ----------------------*/

    bez             l1, .L40            // while(blockSize3 > 0u)

.L32:
    movi            t6, 0               // sum = 0
    lsri            t0, l1, 2           // k   = blockSize3 >> 2u
    bez             t0, .L35            // while(k > 0u)

.L33:
    pldbi.d         t2, (l3)            // perform the pultiply and accumulate four times
    ld.w            t4, (l4, 0x0)
    subi            l4, l4, 4
    mula.s32.hs     t6, t2, t4

    ld.w            t4, (l4, 0x0)
    subi            l4, l4, 4
    mula.s32.hs     t6, t3, t4

    pldbi.d         t2, (l3)
    ld.w            t4, (l4, 0x0)
    subi            l4, l4, 4
    mula.s32.hs     t6, t2, t4

    ld.w            t4, (l4, 0x0)
    subi            l4, l4, 4

.L34:
    mula.s32.hs     t6, t3, t4
    bloop           t0, .L33, .L34      // while loop

.L35:
    andi            t0, l1, 3           // k = blockSize3 % 0x4u
    bez             t0, .L38            // while(k > 0u)

.L36:
    ldbi.w          t2, (l3)            // perform the multiply and accumulate
    ld.w            t3, (l4, 0x0)
    subi            l4, l4, 4

.L37:
    mula.s32.hs     t6, t2, t3
    bloop           t0, .L36, .L37      // while loop

.L38:
    lsli.s32.s      t6, t6, 1           // process and store the result
    stbi.w          t6, (l0)
    addi            l5, l5, 4
    mov             l3, l5              // px = ++pSrc1
    mov             l4, l6              // py = pSrc2

.L39:
    subi            l1, l1, 1           // blockSize3--
    bhz             l1, .L32            // while loop

.L40:
    pop             l0, l1, l2, l3, l4, l5, l6
    .size           csky_conv_fast_q31, .-csky_conv_fast_q31
