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
 * @file     csky_conv_q7.S
 * @brief    Convolution of Q7 sequences.
 * @version  V1.0
 * @date     23. Aug 2017
 ******************************************************************************/

/*
 * void csky_conv_q7(
 *   q7_t * pSrcA
 *   uint32_t srcALen,
 *   q7_t * pSrcB,
 *   uint32_t srcBLen
 *   q7_t * pDst)
 */

    .file           "csky_conv_q7.S"
    .section        .text.csky_conv_q7,"ax",@progbits
    .align          2
    .global         csky_conv_q7
    .type           csky_conv_q7, @function

csky_conv_q7:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    ld.w            l0, (sp, 0x28)      // *pDst
    cmphs           a1, a3              // if(srcAlen >= srcBlen)
    bt              .L0

    mov             l1, a1              // else branch
    mov             a1, a3
    mov             a3, l1

    mov             l1, a0
    mov             a0, a2
    mov             a2, l1

.L0:
    subi            l1, a3, 1           // blockSize1 = srcBlen - 1u

    /* --------------------------
     * Initializations of stage1
     * -------------------------*/

    movi            l4, 1               // count = 1u
    mov             l2, a0              // px    = pIn1
    mov             l3, a2              // py    = pIn2
    bez             l1, .L2             // while(blockSize1 > 0u)

.L1:
    movi            t2, 0               // sum = 0
    lsri            t0, l4, 2           // k   = count >> 2u
    andi            t1, l4, 3           // k   = count % 0x4u
    bez             t0, .L5             // while( k > 0u)

.L3:
    ldbi.w          t3, (l2)
    subi            l3, 3
    ld.w            t4, (l3, 0x0)
    subi            l3, 1
    pext.s8.e       t6, t3
    pext.s8.e       t8, t4
    mulacax.s16.s   t2, t6, t9

.L4:
    mulacax.s16.s   t2, t7, t8

    bloop           t0, .L3, .L4        // while loop

.L5:
    bez             t1, .L8             // while(k > 0u)

.L6:
    ldbi.bs         t3, (l2)            // sum += ((q15_t) *px++ * *py--)
    sextb           t3, t3
    ld.bs           t4, (l3, 0x0)
    sextb           t4, t4
    subi            l3, l3, 1

.L7:
    mulall.s16.s    t2, t3, t4

    bloop           t1, .L6, .L7        // while loop

.L8:
    asri            t2, t2, 7           // sum >> 7u
    clipi.s32       t2, t2, 8           // saturate
    stbi.b          t2, (l0)            // store the data
    add             l3, a2, l4          // py = pIn2 + count
    mov             l2, a0              // px = a0
    addi            l4, l4, 1           // count++

.L39:
    subi            l1, l1, 1           // blockSize1--
    bhz             l1, .L1             // while loop

    /* --------------------------
     * Initializations of stage2
     * -------------------------*/

.L2:
    mov             l2, a0              // px = pIn1
    subi            l5, a3, 1           // srcBlen - 1u
    add             l1, a2, l5          // pSrc2
    mov             l3, l1              // py = pSrc2
    movi            l4, 0               // count = 0
    sub             l5, a1, l5

    cmphsi          a3, 4               // if(srcBLen >= 4u)
    bf              .L9
    lsri            l6, l5, 2           // blkCnt = blcokSize2 >> 2u
    bez             l6, .L17            // while(blkCnt > 0u)

.L10:
    movi            t6, 0               // acc0 = 0
    movi            t7, 0               // acc1 = 0
    movi            t8, 0               // acc2 = 0
    movi            t9, 0               // acc3 = 0

    ldbi.bs         t1, (l2)            // x0   = *px++
    sextb           t1, t1
    ldbi.bs         t2, (l2)            // x1   = *px++
    sextb           t2, t2
    ldbi.bs         t3, (l2)            // x2   = *px++
    sextb           t3, t3
    lsri            t0, a3, 2           // k = srcBLen >> 2u
    subi            l3, 1

.L11:
    ld.hs           t4, (l3, 0x0)       // load y[srcBLen - 1] and y[srcBLen - 2]
    subi            l3, 2
    pext.s8.e       l8, t4              // input2
    pkgll           t5, t1, t2          // input1
    mulacax.s16.s   t6, t5, l8          // acc0 = __SMLAD(input1, input2, acc0)
    pkgll           t5, t2, t3          // input1
    mulacax.s16.s   t7, t5, l8          // acc1 = __SMLAD(input1, input2, acc1)
    ldbi.bs         l7, (l2)            // x3   = *px++
    sextb           l7, l7
    pkgll           t5, t3, l7          // input1
    mulacax.s16.s   t8, t5, l8          // acc2 = __SMLAD(input1, input2, acc2)
    ldbi.bs         t1, (l2)            // x0   = *px++
    sextb           t1, t1
    pkgll           t5, l7, t1          // input1
    mulacax.s16.s   t9, t5, l8          // acc3 = __SMLAD(input1, input2, acc3)

    ld.hs           t4, (l3, 0x0)       // load y[srcBLen - 3] and y[srcBLen - 4]
    subi            l3, 2
    ldbi.bs         t2, (l2)            // x1   = *px++
    sextb           t2, t2
    pext.s8.e       l8, t4              // input2
    pkgll           t5, t3, l7          // input1
    mulacax.s16.s   t6, t5, l8          // acc0 = __SMLAD(input1, input2, accc0)
    pkgll           t5, l7, t1          // input1
    mulacax.s16.s   t7, t5, l8          // acc1 = __SMLAD(input1, input2, acc1)
    pkgll           t5, t1, t2          // input1
    mulacax.s16.s   t8, t5, l8          // acc2 = __SMLAD(input1, input2, acc2)
    ldbi.bs         t3, (l2)            // x2   = *px++
    sextb           t3, t3
    pkgll           t5, t2, t3          // input1

.L12:
    mulacax.s16.s   t9, t5, l8          // acc3 = __SMLAD(input1, input2, acc3)

    bloop           t0, .L11, .L12      // while loop

    andi            t0, a3, 3
    bez             t0, .L15
    addi            l3, 1

.L13:
    ld.bs           t4, (l3, 0x0)       // c0 = *py--
    sextb           t4, t4
    ldbi.bs         l7, (l2)            // x3 = *px++
    sextb           l7, l7
    subi            l3, l3, 1
    mulall.s16.s    t6, t1, t4          // acc0 += ((q15_t) x0 * c0)
    mulall.s16.s    t7, t2, t4          // acc1 += ((q15_t) x1 * c0)
    mulall.s16.s    t8, t3, t4          // acc2 += ((q15_t) x2 * c0)
    mulall.s16.s    t9, l7, t4          // acc3 += ((q15_t) x3 * c0)

    mov             t1, t2              // x0    = x1
    mov             t2, t3              // x1    = x2

.L14:
    mov             t3, l7              // x2    = x3

    bloop           t0, .L13, .L14      // while loop

.L15:
    asri            t6, t6, 7           // *pOut++ = (q7_t) (__SSAT_8(acc0 >> 7u))
    clipi.s32       l9, t6, 8
    stbi.b          l9, (l0)
    asri            t7, t7, 7           // *pOut++ = (q7_t) (__SSAT_8(acc1 >> 7u))
    clipi.s32       l9, t7, 8
    stbi.b          l9, (l0)
    asri            t8, t8, 7           // *pOut++ = (q7_t) (__SSAT_8(acc2 >> 7u))
    clipi.s32       l9, t8, 8
    stbi.b          l9, (l0)
    asri            t9, t9, 7           // *pOut++ = (q7_t) (__SSAT_8(acc3 >> 7u))
    clipi.s32       l9, t9, 8
    stbi.b          l9, (l0)

    addi            l4, l4, 4           // count += 4u
    add             l2, a0, l4          // px = pIn1 + count
    subi            l9, a3, 1
    add             l3, a2, l9          // py = pSrc2

.L16:
    subi            l6, l6, 1           // blkCnt--
    bhz             l6, .L10            // while loop

.L17:
    andi            l6, l5, 3           // blkCnt = blkockSize2 % 0x4u
    bez             l6, .L27            // while(blkCnt > 0u)

.L34:    
    movi            l8, 0               // sum = 0
    lsri            t0, a3, 2           // k   = srcBLen >. 2u
    bez             t0, .L20            // while(k >0)

.L18:
    subi            l3, 3
    ldbi.w          t1, (l2)
    ld.w            t2, (l3)
    subi            l3, 1
    pext.s8.e       t6, t1
    pext.s8.e       t8, t2
    mulacax.s16.s   l8, t6, t9

.L19:
    mulacax.s16.s   l8, t7, t8

    bloop           t0, .L18, .L19      // while loop

.L20:
    andi            t0, a3, 3           // k = srcBLen % 0x4u
    bez             t0, .L35            // while(k >0)

.L21:
    ldbi.bs         t1, (l2)            // sum += ((q15_t) * px++ * *py--)
    sextb           t1, t1
    ld.bs           t2, (l3)
    sextb           t2, t2
    subi            l3, l3, 1

.L22:
    mulall.s16.s    l8, t1, t2

    bloop           t0, .L21, .L22      // while loop

.L35:
    asri            l8, l8, 7           // sum >> 7
    clipi.s32       l9, l8, 8           // saturate
    stbi.b          l9, (l0)            // store the data
    addi            l4, l4, 1           // count++
    add             l2, a0, l4          // px = pIn1 + count
    subi            l9, a3, 1           // py = pSrc2
    add             l3, a2, l9

.L23:
    subi            l6, l6, 1           // blkCnt--
    bhz             l6, .L34            // while loop
    br              .L27

.L9:
    bez             l5, .L27            // condition to end
    mov             l6, l5

.L36:
    movi            t6, 0               // sum = 0
    mov             t0, a3              // k   = srcBLen

.L24:
    ldbi.bs         t1, (l2)            //sum += ((q15_t) *px++ * *Py--)
    sextb           t1, t1
    ld.bs           t2, (l3, 0x0)
    sextb           t2, t2
    subi            l3, l3, 1

.L25:
    mulall.s16.s    t6, t1, t2
    bloop           t0, .L24, .L25      // while loop

    asri            t6, t6, 7           // sum >> 7u
    clipi.s32       l9, t6, 8           // saturate
    stbi.b          l9, (l0)            // store the result
    addi            l4, l4, 1           // count++
    add             l2, a0, l4          // px = pIn1 + count
    subi            l9, a3, 1           // py = pSrc2
    add             l3, a2, l9
   
.L26:
    subi            l6, 1               // blkCnt--
    bhz             l6, .L36            // while loop

    /* -------------------------
     * initializations of stage3
     * ------------------------*/
.L27:
    add             l6, a0, l5          // pSrc1
    mov             l2, l6              // px = pSrc1

    subi            l5, a3, 1           // blockSize3
    add             l7, a2, l5          // pSrc2
    mov             l3, l7              // py = pSrc2
    bez             l5, .L37

.L28:
    movi            t4, 0               // sum = 0
    lsri            t0, l5, 2           // k = blockSize3 >> 2u
    bez             t0, .L38

.L29:
    subi            l3, 3
    ldbi.w          t2, (l2)
    ld.w            t3, (l3, 0x0)
    subi            l3, 1
    pext.s8.e       t6, t2
    pext.s8.e       t8, t3
    mulacax.s16.s   t4, t6, t9

.L30:
    mulacax.s16.s   t4, t7, t8

    bloop           t0, .L29, .L30      // while loop

.L38:
    andi            t0, l5, 3           // k = blockSize3 % 0x4u
    bez             t0, .L40            // while(k >0)

.L31:
    ldbi.bs         t2, (l2)            // sum += ((q15_t) *px++ * *py--)
    sextb           t2, t2
    ld.bs           t3, (l3, 0x0)
    sextb           t3, t3
    subi            l3, l3, 1

.L32:
    mulall.s16.s    t4, t2, t3

    bloop           t0, .L31, .L32      // while loop

.L40:
    asri            t4, t4, 7           // sum >> 7u
    clipi.s32       l9, t4, 8           // saturate
    stbi.b          l9, (l0)            // store the result

    addi            l6, l6, 1           // ++pSrc1
    mov             l2, l6              // px = ++ pSrc1
    mov             l3, l7              // py = pSrc2

.L33:
    subi            l5, l5, 1           // blockSize3--
    bhz             l5, .L28            // while loop

.L37:
    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_conv_q7, .-csky_conv_q7
