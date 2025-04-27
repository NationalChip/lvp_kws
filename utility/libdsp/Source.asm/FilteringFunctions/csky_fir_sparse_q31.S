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
 * @file     csky_fir_sparse_q31.S
 * @brief    Q31 sparse FIR filter processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/*
 * void csky_fir_sparse_q31(
 *   csky_fir_sparse_instance_q31 * S,
 *   q31_t * pSrc,
 *   q31_t * pDst,
 *   q31_t * pScratchIn,
 *   q31_t * pScratchOut,
 *   uint32_t blockSize)
 */
    .file           "csky_fir_sparse_q31.S"
    .section        .text.csky_fir_sparse_q31,"ax",@progbits
    .align          2
    .global         csky_fir_sparse_q31
    .type           csky_fir_sparse_q31, @function
csky_fir_sparse_q31:
    push            l0, l1, l2, l3, l4, l5, l6, l7, l8, l9

    ld.w            t0, (sp, 0x28)      // blockSize
    ld.h            t1, (a0, 0xc)       // S->maxDelay
    addu            t1, t1, t0          // delaySize
    ld.w            t2, (a0, 0x4)       // S->pState

    /* csky_circularWrite_q31 start */
    ld.h            t3, (a0, 0x2)       // wOffset = S->stateIndex
    mov             t4, t0              // i = blockSize
    mov             t6, a1              // pIn
    bez             t4, .L0
.L1:
    ldbi.w          t7, (t6)            // *src, src += srcInc
    str.w           t7, (t2, t3 << 2)   // circBuffer[wOffset]
    addi            t3, t3, 1           // wOffset += buffserInc
    cmplt           t3, t1              // woffset >= L
    subu            l0, t3, t1          // wOffset -= L
    movf            t3, l0
.L2:
    subi            t4, t4, 1           // i--
    bnez            t4, .L1
.L0:

    st.h            t3, (a0, 0x2)       // *writeOffset = (uint16_t)wOffset
    /* csky_circularWrite_q31 end */

    subu            t4, t3, t0
    ld.w            t5, (a0, 0x10)      // pTapDelay
    ldbi.w          t6, (t5)
    subu            t4, t4, t6          // readIndex
    bhsz            t4, .L3             // readIndex < 0
    addu            t4, t4, t1          // readIndex += (int32_t) delaySize
.L3:

    /* csky_circularRead_q31 start */
    mov             t6, a3              // dst_base = pb = pScratchIn
    mov             t3, t0              // i = blockSize
    lsli            t7, t3, 2
    addu            t7, t7, t6          // dst_end
    bez             t3, .L4

.L5:
    ldr.w           t8, (t2, t4 << 2)   // circBuffer[rOffset]
    stbi.w          t8, (t6)            // dst += dstInc
    cmpne           t6, t7
    movf            t6, a3              // dst = dst_base
.L7:
    addi            t4, t4, 1           // rOffset += bufferInc
    cmplt           t4, t1              // rOffset >= L
    subu            l0, t4, t1          // rOffset -= L
    movf            t4, l0
.L6:
    subi            t3, t3, 1           // i--
    bnez            t3, .L5
.L4:

    /* csky_circularRead_q31 end */

    ld.w            l0, (a0, 0x8)       // pCoeffs
    ldbi.w          l1, (l0)            // coeff
    mov             l2, a3              // px = pb
    mov             l8, a2

    lsri            t3, t0, 2           // blkCnt = blockSize >> 2
    bez             t3, .L8
.L9:
    pldbi.d         l4, (l2)
    mul.s32.h       l6, l4, l1
    mul.s32.h       l7, l5, l1
    st.w            l6, (l8, 0x0)
    st.w            l7, (l8, 0x4)

    pldbi.d         l4, (l2)
    mul.s32.h       l6, l4, l1
    mul.s32.h       l7, l5, l1
    st.w            l6, (l8, 0x8)
    st.w            l7, (l8, 0xc)

.L10:
    addi            l8, l8, 16
    bloop           t3, .L9, .L10
.L8:

    andi            t3, t0, 3           // blkCnt
    bez             t3, .L13
.L11:
    ldbi.h          l4, (l2)
    mul.s32.h       l6, l4, l1
.L12:
    stbi.w          l6, (l8)

    bloop           t3, .L11, .L12
.L13:

    ldbi.w          l1, (l0)            // coeff
    ldbi.w          t6, (t5)            // *pTapDelay++
    ld.h            l9, (a0, 0x2)       // S->stateIndex
    subu            l9, l9, t0
    subu            l9, l9, t6          // readIndex

    bhsz            l9, .L14            // readIndex < 0
    addu            l9, l9, t1          // readIndex += (int32_t) delaySize
.L14:

    ld.h            t3, (a0, 0x0)       // numTaps
    subi            t4, t3, 2           // tapCnt
    bez             t4, .L51
.L16:

    /* csky_circularRead_q31 start */
    mov             t6, a3              // dst_base = pb = pScratchIn
    mov             t9, t0              // i = blockSize
    lsli            t7, t9, 2
    addu            t7, t7, t6          // dst_end
    bez             t9, .L18

.L19:
    ldr.w           t8, (t2, l9 << 2)   // circBuffer[rOffset]
    stbi.w          t8, (t6)            // dst += dstInc
    cmpne           t6, t7
    movf            t6, a3              // dst = dst_base
.L21:
    addi            l9, l9, 1           // rOffset += bufferInc
    cmplt           l9, t1              // rOffset >= L
    subu            l2, l9, t1          // rOffset -= L
    movf            l9, l2
.L20:
    subi            t9, t9, 1           // i--
    bnez            t9, .L19
.L18:

    /* csky_circularRead_q31 end */

    mov             l2, a3              // px = pb
    mov             l3, a2              // pScratchOut
    mov             l8, l3

    lsri            t6, t0, 2           // blkCnt = blockSize >> 2
    bez             t6, .L28
.L29:
    pldbi.d         l4, (l2)
    pldbi.d         l6, (l3)
    mula.s32.hs     l6, l4, l1
    mula.s32.hs     l7, l5, l1
    st.w            l6, (l8, 0x0)
    st.w            l7, (l8, 0x4)

    pldbi.d         l4, (l2)
    pldbi.d         l6, (l3)
    mula.s32.hs     l6, l4, l1
    mula.s32.hs     l7, l5, l1
    st.w            l6, (l8, 0x8)
    st.w            l7, (l8, 0xc)

.L30:
    addi            l8, l8, 16
    bloop           t6, .L29, .L30
.L28:

    andi            t6, t0, 3           // blkCnt
    bez             t6, .L33
.L31:
    ldbi.w          l4, (l2)
    ldbi.w          l6, (l3)
    mula.s32.hs     l6, l4, l1
.L32:
    stbi.w          l6, (l8)

    bloop           t6, .L31, .L32
.L33:

    ldbi.w          l1, (l0)            // coeff
    ldbi.w          t6, (t5)            // *pTapDelay++
    ld.h            l9, (a0, 0x2)       // S->stateIndex
    subu            l9, l9, t0
    subu            l9, l9, t6          // readIndex

    bhsz            l9, .L17            // readIndex < 0
    addu            l9, l9, t1          // readIndex += (int32_t) delaySize
.L17:
    subi            t4, t4, 1           // tapCnt--
    bnez            t4, .L16
.L51:

    /* csky_circularRead_q31 start */
    mov             t6, a3              // dst_base = pb = pScratchIn
    mov             t9, t0              // i = blockSize
    lsli            t7, t9, 2
    addu            t7, t7, t6          // dst_end
    bez             t9, .L38

.L39:
    ldr.w           t8, (t2, l9 << 2)   // circBuffer[rOffset]
    stbi.w          t8, (t6)            // dst += dstInc
    cmpne           t6, t7
    movf            t6, a3              // dst = dst_base
.L41:
    addi            l9, l9, 1           // rOffset += bufferInc
    cmplt           l9, t1              // rOffset >= L
    subu            l2, l9, t1          // rOffset -= L
    movf            l9, l2
.L40:
    subi            t9, t9, 1           // i--
    bnez            t9, .L39
.L38:
    /* csky_circularRead_q31 end */

    mov             l2, a3              // px = pb
    mov             l3, a2              // pScratchOut
    mov             l8, l3

    lsri            t6, t0, 2           // blkCnt = blockSize >> 2
    bez             t6, .L58
.L59:
    pldbi.d         l4, (l2)
    pldbi.d         l6, (l3)
    mula.s32.hs     l6, l4, l1
    mula.s32.hs     l7, l5, l1
    st.w            l6, (l8, 0x0)
    st.w            l7, (l8, 0x4)

    pldbi.d         l4, (l2)
    pldbi.d         l6, (l3)
    mula.s32.hs     l6, l4, l1
    mula.s32.hs     l7, l5, l1
    st.w            l6, (l8, 0x8)
    st.w            l7, (l8, 0xc)

.L60:
    addi            l8, l8, 16
    bloop           t6, .L59, .L60
.L58:

    andi            t6, t0, 3           // blkCnt
    bez             t6, .L63
.L61:
    ldbi.w          l4, (l2)
    ldbi.w          l6, (l3)
    mula.s32.hs     l6, l4, l1
.L62:
    stbi.w          l6, (l8)

    bloop           t6, .L61, .L62
.L63:

    mov             l3, a2              // pSrc2
    lsri            t6, t0, 2           // blkCnt = blockSize >> 2
    bez             t6, .L64
.L65:
    pldbi.d         l4, (l3)
    lsli            l4, l4, 1
    lsli            l5, l5, 1
    stbi.w          l4, (a2)            // pOut
    stbi.w          l5, (a2)

    pldbi.d         l4, (l3)
    lsli            l4, l4, 1
    lsli            l5, l5, 1
    stbi.w          l4, (a2)            // pOut
.L66:
    stbi.w          l5, (a2)

    bloop           t6, .L65, .L66
.L64:

    andi            t6, t0, 3           // blkCnt = blockSize % 0x4
    bez             t6, .L67
.L68:
    ldbi.w          l4, (l3)
    lsli            l4, l4, 1
.L69:
    stbi.w          l4, (a2)            // pOut

    bloop           t6, .L68, .L69
.L67:

    pop             l0, l1, l2, l3, l4, l5, l6, l7, l8, l9
    .size           csky_fir_sparse_q31, .-csky_fir_sparse_q31
