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
 * @file     csky_conv_fast_q15.S
 * @brief    Fast Q15 Convolution.
 * @version  V1.0
 * @date     28. Aug 2017
 ******************************************************************************/

/*
 * void csky_conv_fast_q15(
 *   q15_t * pSrcA
 *   uint32_t srcALen,
 *   q15_t * pSrcB,
 *   uint32_t srcBLen
 *   q15_t * pDst)
 */

    .file           "csky_conv_fast_q15.S"
    .section        .text.csky_conv_fast_q15,"ax",@progbits
    .align          2
    .global         csky_conv_fast_q15
    .type           csky_conv_fast_q15, @function

csky_conv_fast_q15:
    push            l0, l1, l2, l3, l4, l5, l6, l7
    ld.w            l0, (sp, 0x20)      // *pOut = pDst
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

    /* --------------------------
     * Initializaition of stage1  
     * -------------------------*/

    movi            l2, 1               // count = 1u
    mov             l3, a0              // px    = pIn1
    mov             l4, a2              // py    = pIn2

    /* --------------------------
     *      Stage1 processing
     * -------------------------*/
.L3:
    bez             l1, .L1             // while(blockSize1 > 0u)
    movi            t0, 0               // sum   = 0
    mov             t1, l2              // k     = count

.L4:
    ldbi.hs         t2, (l3)            // sum   = __SMLAD(*px++, *py--, sum)
    ld.h            t3, (l4, 0x0)
    subi            l4, l4, 2

.L5:
    mulall.s16.s    t0, t2, t3
    bloop           t1, .L4, .L5        // while loop

    asri            t0, t0, 15          // *pOut++ = (q15_t) (sum >> 15)
    stbi.h          t0, (l0)
    lsli            l7, l2, 1
    add             l4, a2, l7          // py = pIn2 + count
    mov             l3, a0              // px = pIn1
    addi            l2, 1               // count++
    subi            l1, 1               // blockSize1--
    cmphsi          l2, 4               // while loop
    bf              .L3

    bez             l1, .L1             // while(blockSize1 > 0u)
    subi            l4, l4, 2           // py  = py - 1

.L6:
    movi            t0, 0               // sum = 0
    lsri            t1, l2, 2           // k   = count >> 2u
    andi            t2, l2, 3           // k   = count % 0x4u

.L7:
    pldbi.d         t4, (l3)            // perform the multiply and accumulates
    ld.w            t6, (l4, 0x0)
    subi            l4, l4, 4
    mulacax.s16.s   t0, t4, t6
    ld.w            t6, (l4, 0x0)
    subi            l4, l4, 4

.L8:
    mulacax.s16.s   t0, t5, t6
    bloop           t1, .L7, .L8        // while loop

    addi            l4, l4, 2           // py = py + 1u
    bez             t2, .L11            // while(k > 0u)
.L9:
    ldbi.hs         t4, (l3)            // sum = __SMLAD(*px++, *py--, sum)
    ld.hs           t5, (l4)
    subi            l4, l4, 2

.L10:
    mulall.s16.s    t0, t4, t5
    bloop           t2, .L9, .L10       // while loop

.L11:
    asri            t0, t0, 15          // process and store the results
    stbi.h          t0, (l0)
    subi            l7, l2, 1           // py = pIn2 + (count - 1u)
    lsli            l7, l7, 1
    add             l4, a2, l7
    mov             l3, a0              // px = pIn1
    addi            l2, l2, 1           // count++

.L12:
    subi            l1, l1, 1           // blockSize1--
    bhz             l1, .L6             // while loop

    /* --------------------------
     * Initiallizations of stage2
     * -------------------------*/

.L1:
    mov             l3, a0              // px    = pIn1
    subi            t0, a3, 1           // pSrc2 = pIn2 + (srcBLen - 1u)
    sub             l1, a1, t0          // blockSize2
    lsli            t0, t0, 1
    add             l5, a2, t0
    mov             l4, l5              // py    = pSrc2
    movi            l2, 0               // count = 0u

    /* -------------------------
     *     Stage2 processing
     * ------------------------*/

    cmphsi          a3, 4               // if(srcBLen >= 4u)
    bf              .L14                // else
    lsri            t0, l1, 2           // blkCnt = blockSize2 >> 2u
    bez             t0, .L22            // while(blkCnt >0u)

.L13:
    subi            l4, 2               // py   = py - 1u
    movi            t6, 0               // acc0 = 0
    movi            t7, 0               // acc1 = 0
    movi            t8, 0               // acc2 = 0
    movi            t9, 0               // acc3 = 0

    ldbi.w          t2, (l3)            // load x[0], x[1] and x[1], x[2] samples
    subi            l3, l3, 2
    ldbi.w          t3, (l3)
    subi            l3, l3, 2
    lsri            t1, a3, 2           // k    = srcBLen >> 2u
    bez             t1, .L49

.L15:
    ld.w            l6, (l4, 0x0)       // c0   = *__SIM32(py)--
    subi            l4, l4, 4
    mulacax.s16.s   t6, t2, l6          // acc0 = __SMLADX(x0, c0, acc0)
    mulacax.s16.s   t7, t3, l6          // acc1 = __SMLADX(x1, c0, acc1)
    ldbi.w          t4, (l3)            // x2   = *__SIMD32(px)
    subi            l3, l3, 2
    ldbi.w          t5, (l3)            // x3   = _SIMD32_OFFSET(px+1)
    subi            l3, l3, 2
    mulacax.s16.s   t8, t4, l6          // acc2 = __SMLADX(x2, c0, acc2)
    mulacax.s16.s   t9, t5, l6          // acc3 = __SMLADX(x3, c0, acc3)

    ld.w            l6, (l4, 0x0)       // c0   = *__SIM32(py)--
    subi            l4, l4, 4
    mulacax.s16.s   t6, t4, l6          // acc0 = __SMLADX(x2, c0, acc0)
    mulacax.s16.s   t7, t5, l6          // acc1 = __SMLADX(x3, c0, acc1)
    ldbi.w          t2, (l3)            // x0   = _SIMD32_OFFSET(px+2)
    subi            l3, l3, 2
    ldbi.w          t3, (l3)            // x1   = _SIMD32_OFFSET(px+3)
    subi            l3, l3, 2
    mulacax.s16.s   t8, t2, l6          // acc2 = __SMLADX(x0, c0, acc2)

.L16:
    mulacax.s16.s   t9, t3, l6          // acc3 = __SMLADX(x1, c0, acc3)
    bloop           t1, .L15, .L16      // while loop

.L49:
    andi            t1, a3, 3           // k = srcBLen % 0x4u
    bez             t1, .L20            // if(k == 0), end

.L17:
    cmpnei          t1, 1               // if(k == 1u)
    bt              .L18                // else
    movi            l6, 0
    ld.hs           l6, (l4, 0x02)      // load c0
    lsli            l6, l6, 16
    ldbi.w          t5, (l3)            // x3   = *__SIMD32(px)
    mulaca.s16.s    t6, t2, l6          // acc0 = __SMLAD(x0, c0, acc0)
    mulaca.s16.s    t7, t3, l6          // acc1 = __SMLAD(x1, c0, acc1)
    mulacax.s16.s   t8, t2, l6          // acc2 = __SMLADX(x1, c0, acc2)
    mulacax.s16.s   t9, t5, l6          // acc3 = __SMLADX(x3, co, acc3)

.L18:
    cmpnei          t1, 2               // if(k == 2u)
    bt              .L19                // else
    ld.w            l6, (l4, 0x0)       // c0   = _SIMD32_OFFSET(py)
    ldbi.w          t5, (l3)            // x3   = *__SIMD32(px)
    movi            t4, 0
    ldbi.w          t4, (l3)            // x2   = _SIMD32_OFFSET(px+1)
    subi            l3, l3, 2
    mulacax.s16.s   t6, t2, l6          // acc0 = __SMLADX(x0, c0, acc0)
    mulacax.s16.s   t7, t3, l6          // acc1 = __SMLADX(x1, co, acc1)
    mulacax.s16.s   t8, t5, l6          // acc2 = __SMLADX(x3, c0, acc2)
    mulacax.s16.s   t9, t4, l6          // acc3 = __SMLADX(x2, c0, acc3)

.L19:
    ld.w            l6, (l4, 0x0)       // c0   = _SIMD32_OFFset(py)
    subi            l4, l4, 2
    ldbi.w          t5, (l3)            // x3   = *__SIMD32(px)
    subi            l3, l3, 2
    ldbi.w          t4, (l3)            // x2   = _SIMD32_OFFSET(px+1)
    subi            l3, l3, 2
    mulacax.s16.s   t6, t2, l6          // acc0 = __SMLADX(x0, c0, acc0)
    mulacax.s16.s   t7, t3, l6          // acc1 = __SMLADX(x1, c0, acc1)
    mulacax.s16.s   t8, t5, l6          // acc2 = __SMLADX(x3, c0, acc2)
    mulacax.s16.s   t9, t4, l6          // acc3 = __SMLADX(x2, c0, acc3)
    ld.hs           l6, (l4, 0x0)       // load c0
    lsli            l6, l6, 16
    ldbi.w          t4, (l3)            // x3   = _SIMD32_OFFSET(px+2)
    subi            l3, l3, 2
    mulacax.s16.s   t6, t2, l6          // acc0 = __SMLADX(x1, c0, acc0)
    mulaca.s16.s    t7, t4, l6          // acc1 = __SMLAD(x2, c0, acc1)
    mulacax.s16.s   t8, t4, l6          // acc2 = __SMLADX(x2, c0, acc2)
    mulacax.s16.s   t9, t5, l6          // acc3 = __SMLADX(x3, c0, acc3)

.L20:
    asri            t6, t6, 15          // process and store the results
    asri            t7, t7, 15
    asri            t8, t8, 15
    asri            t9, t9, 15
    pkgll           t6, t6, t7
    stbi.w          t6, (l0)
    pkgll           t8, t8, t9
    stbi.w          t8, (l0)

    addi            l2, l2, 4           // count += 4u
    mov             l4, l5              // py  = pSrc2
    lsli            l7, l2, 1
    add             l3, a0, l7          // px  = pIn1 + count

.L21:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L13            // while loop

.L22:
    andi            t0, l1, 3           // blkCnt = blockSize2 % 0x4u
    bez             t0, .L2             // while(blkCnt > 0u)

.L30:
    movi            t9, 0               // sum = 0
    lsri            t1, a3, 2           // k   = srcBLen >> 2u
    bez             t1, .L25            // while(k > 0u)

.L23:
    subi            l4, 2
    pldbi.d         t2, (l3)            // perform the multiply and accumulates
    ld.w            t4, (l4, 0x0)
    subi            l4, l4, 4
    ld.w            t5, (l4, 0x0)
    subi            l4, l4, 2
    mulacax.s16.s   t9, t2, t4

.L24:
    mulacax.s16.s   t9, t3, t5
    bloop           t1, .L23, .L24      // while loop

.L25:
    andi            t1, a3, 3           // k = srcBLen % 0x4u
    bez             t1, .L28            // while(k >0u)

.L26:
    ldbi.hs         t2, (l3)            // sum += ((q31_t) *px++ * *py--)
    ld.h            t3, (l4, 0x0)
    subi            l4, l4, 2

.L27:
    mulall.s16.s    t9, t2, t3
    bloop           t1, .L26, .L27      // while loop

.L28:
    asri            t9, t9, 15          // process and store the result
    stbi.h          t9, (l0)
    addi            l2, l2, 1           // count++
    lsli            l7, l2, 1           // px = pIn1 + count
    add             l3, a0, l7
    mov             l4, l5              // py = pSrc2

.L29:
    subi            t0, t0, 1           // blkCnt--
    bhz             t0, .L30            // while loop
    br              .L2

.L14:                                   // else branch
    mov             t0, l1              // blkCnt = blockSize2

.L33:
    movi            t1, 0               // sum = 0
    mov             t2, a3              // k   = srcBLen


.L48:
    ldbi.hs         t3, (l3)            // sum += ((q31_t) *px++ * *py--)
    ld.hs           t4, (l4, 0x0)
    subi            l4, l4, 2

.L31:
    mulall.s16.s    t1, t3, t4
    bloop           t2, .L48, .L31      // while loop

    asri            t1, t1, 16          // process and store the result
    stbi.h          t1, (l0)
    addi            l2, l2, 1           // count++
    lsli            l7, l2, 1           // px = pIn1 + count
    add             l3, a0, l7
    mov             l4, l5              // py = pSrc2

.L32:
    subi            t0, t0, 1           // block--
    bhz             t0, .L33            // while loop

    /* ------------------------
     * Inializations of stage3
     * -----------------------*/

.L2:
    subi            l1, a3, 1           // blockSize3
    sub             l7, a1, l1
    lsli            l7, l7, 1
    add             l2, a0, l7          // pSrc1
    mov             l3, l2              // px = pSrc1
    lsli            l7, l1, 1
    add             l5, a2, l7          // pSrc2
    subi            a2, l5, 2           // pIn2
    mov             l4, a2              // py = pIn2

    /* ------------------------
     *   Stage3 processing
     * ----------------------*/

    lsri            t0, l1, 2           // j = blockSize3 >> 2u
    bez             t0, .L42            // while(j > 0)

.L34:
    bez             l1, .L47
    movi            t9, 0               // sum = 0
    lsri            t1, l1, 2           // k   = blockSize3 >> 2u
    //addi            l4, 2
    bez             t1, .L37            // while(k > 0u)

.L35:
    pldbi.d         t2, (l3)            // sum = __SMLADX(*__SIMD32(px)++, *__SIMD32(py)--, sum)
    ld.w            t4, (l4, 0x0)
    subi            l4, l4, 4
    ld.w            t5, (l4, 0x0)
    subi            l4, l4, 4
    mulacax.s16.s   t9, t2, t4

.L36:
    mulacax.s16.s   t9, t3, t5
    bloop           t1, .L35, .L36      // while loop

.L37:
    andi            t1, l1, 3           // k  = blockSize3 % 0xu
    bez             t1, .L40            // while(k > 0u)
    addi            l4, 2

.L38:
    ldbi.hs         t2, (l3)            // sum = __SMLAD(*px++, *py--, sum)
    ld.hs           t3, (l4, 0x0)
    subi            l4, l4, 2

.L39:
    mulall.s16.s    t9, t2, t3
    bloop           t1, .L38, .L39      // while loop

.L40:
    asri            t9, t9, 15          // process and store the result
    stbi.h          t9, (l0)
    addi            l2, l2, 2           // px = ++pSrc1
    mov             l3, l2
    mov             l4, a2              // py = pIn2

.L41:
    subi            t0, t0, 1           // j--
    subi            l1, l1, 1           // blockSize3--
    bhz             t0, .L34

.L42:
    addi            l4, l4, 2           // py = py + 1u
    bez             l1, .L47            // while(blockSize3 > 0u)

.L43:
    movi            t9, 0               // sum = 0
    mov             t0, l1              // k   = blockSize3

.L44:
    ldbi.hs         t2, (l3)            // sum = __SMLAD(*px++, *py--, sum)
    ld.hs           t3, (l4, 0x0)
    subi            l4, 2

.L45:
    mulall.s16.s    t9, t2, t3
    bloop           t0, .L44, .L45      // while loop

    asri            t9, t9, 15          // process and store the result
    stbi.h          t9, (l0)
    addi            l2, l2, 2           // px = ++pSrc1
    mov             l3, l2
    mov             l4, l5              // py = pSrc2

.L46:
    subi            l1, l1, 1           // blockSize3--
    bhz             l1, .L43            // while loop

.L47:
    pop             l0, l1, l2, l3, l4, l5, l6, l7
    .size           csky_conv_fast_q15, .-csky_conv_fast_q15
