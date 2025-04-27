/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_bitreversal2.c
* Description:  arm_bitreversal_32 function done in assembly for maximum speed.
*               Called after doing an fft to reorder the output.
*               The function is loop unrolled by 2. arm_bitreversal_16 as well.
*
* $Date:        27. January 2017
* $Revision:    V.1.5.1
*
* Target Processor: Cortex-M cores
*-------------------------------------------------------------------- */
/*
* Copyright (C) 2010-2019 ARM Limited or its affiliates. All rights reserved.
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
* -------------------------------------------------------------------- */
/*
* Copyright (C) 2016-2020 T-HEAD Limited. All rights reserved.
*
* This file comes from arm_bitreversal2.S. It is renamed by replacing arm with csky.
* Depending on the origin one, the functions in the file are realized by csky ISA. 
*
*/

/******************************************************************************
 * @file     csky_bitreversal2.S
 * @brief    This is the csky_bitreversal_32 function done in
 *           assembly for maximum speed.  This function is called
 *           after doing an fft to reorder the output.  The function
 *           is loop unrolled by 2. csky_bitreversal_16 as well.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

.section .text

/*
 * @brief  In-place bit reversal function.
 * @param[in, out] *pSrc        points to the in-place buffer of unknown 32-bit data type.
 * @param[in]      bitRevLen    bit reversal table length
 * @param[in]      *pBitRevTab  points to bit reversal table.
 * @return none.
 */
    .file           "csky_bitreversal2.S"
    .section        .text.csky_bitreversal_32,"ax",@progbits
    .align          2
    .global         csky_bitreversal_32
    .type           csky_bitreversal_32, @function

csky_bitreversal_32:
    addi     a3, a1, 1
    push     r4-r6
    mov      a1, a2
    lsri     a3, a3, 1
csky_bitreversal_32_0:
    ldh      a2, (a1, 0x2)
    ldh      r6, (a1, 0x0)
    addu     a2, a0, a2
    addu     r6, a0, r6
    ldw      r5, (a2, 0x0)
    ldw      r4, (r6, 0x0)
    stw      r5, (r6, 0x0)
    stw      r4, (a2, 0x0)
    ldw      r5, (a2, 0x4)
    ldw      r4, (r6, 0x4)
    addi     a1, a1, 4
    stw      r5, (r6, 0x4)
.L2:
    stw      r4, (a2, 0x4)
    bloop    a3, csky_bitreversal_32_0, .L2
    pop      r4-r6
    jmp      lr
    .size    csky_bitreversal_32, .-csky_bitreversal_32

    .section        .text.csky_bitreversal_16,"ax",@progbits
    .align          2
    .global         csky_bitreversal_16
    .type           csky_bitreversal_16, @function

csky_bitreversal_16:
    addi     a3, a1, 1
    push     r4-r6
    mov      a1, a2
    lsri     a3, a3, 1
csky_bitreversal_16_0:
    ldh      a2, (a1, 0x2)
    ldh      r6, (a1, 0x0)
    lsri     a2, a2, 1
    lsri     r6, r6, 1
    addu     a2, a0, a2
    addu     r6, a0, r6
    ldw      r5, (a2, 0x0)
    ldw      r4, (r6, 0x0)
    addi     a1, a1, 4
    stw      r5, (r6, 0x0)
.L3:
    stw      r4, (a2, 0x0)
    bloop    a3, csky_bitreversal_16_0, .L3
    pop      r4-r6
    jmp      lr
    .size    csky_bitreversal_16, .-csky_bitreversal_16
