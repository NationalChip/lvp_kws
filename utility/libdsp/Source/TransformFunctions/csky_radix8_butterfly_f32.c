/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_cfft_radix8_f32.c
* Description:  Radix-8 Decimation in Frequency & CIFFT Floating point processing function.
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
* This file comes from arm_cfft_radix8_f32.c. Only the radix8 butterfly function
* is placed here. And the name of this function is started
* with csky_vdsp2.

* This file comes from arm_cfft_radix-8_f32.c. 
*
*/

/******************************************************************************
 * @file     csky_cfft_radix8_f32.c
 * @brief    Radix-8 Decimation in Frequency CFFT & CIFFT Floating
 *           point processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
* @ingroup ComplexFFT
*/

/**
* @defgroup Radix8_CFFT_CIFFT Radix-8 Complex FFT Functions
*
* \par
* Complex Fast Fourier Transform(CFFT) and Complex Inverse Fast Fourier Transform(CIFFT) is an efficient algorithm to compute Discrete Fourier Transform(DFT) and Inverse Discrete Fourier Transform(IDFT).
* Computational complexity of CFFT reduces drastically when compared to DFT.
* \par
* This set of functions implements CFFT/CIFFT
* for floating-point data types.  The functions operates on in-place buffer which uses same buffer for input and output.
* Complex input is stored in input buffer in an interleaved fashion.
*
* \par
* The functions operate on blocks of input and output data and each call to the function processes
* <code>2*fftLen</code> samples through the transform.  <code>pSrc</code>  points to In-place arrays containing <code>2*fftLen</code> values.
* \par
* The <code>pSrc</code> points to the array of in-place buffer of size <code>2*fftLen</code> and inputs and outputs are stored in an interleaved fashion as shown below.
* <pre> {real[0], imag[0], real[1], imag[1],..} </pre>
*
* \par Lengths supported by the transform:
* \par
* Internally, the function utilize a Radix-8 decimation in frequency(DIF) algorithm
* and the size of the FFT supported are of the lengths [ 64, 512, 4096].
*
*
* \par Algorithm:
*
* <b>Complex Fast Fourier Transform:</b>
* \par
* Input real and imaginary data:
* <pre>
* x(n) = xa + j * ya
* x(n+N/4 ) = xb + j * yb
* x(n+N/2 ) = xc + j * yc
* x(n+3N/4) = xd + j * yd
* </pre>
* where N is length of FFT
* \par
* Output real and imaginary data:
* <pre>
* X(4r) = xa'+ j * ya'
* X(4r+1) = xb'+ j * yb'
* X(4r+2) = xc'+ j * yc'
* X(4r+3) = xd'+ j * yd'
* </pre>
* \par
* Twiddle factors for Radix-8 FFT:
* <pre>
* Wn = co1 + j * (- si1)
* W2n = co2 + j * (- si2)
* W3n = co3 + j * (- si3)
* </pre>
*
* \par
* \image html CFFT.gif "Radix-8 Decimation-in Frequency Complex Fast Fourier Transform"
*
* \par
* Output from Radix-8 CFFT Results in Digit reversal order. Interchange middle two branches of every butterfly results in Bit reversed output.
* \par
* <b> Butterfly CFFT equations:</b>
* <pre>
* xa' = xa + xb + xc + xd
* ya' = ya + yb + yc + yd
* xc' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1)
* yc' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1)
* xb' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2)
* yb' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2)
* xd' = (xa-yb-xc+yd)* co3 + (ya+xb-yc-xd)* (si3)
* yd' = (ya+xb-yc-xd)* co3 - (xa-yb-xc+yd)* (si3)
* </pre>
*
* \par
* where <code>fftLen</code> length of CFFT/CIFFT; <code>ifftFlag</code> Flag for selection of CFFT or CIFFT(Set ifftFlag to calculate CIFFT otherwise calculates CFFT);
* <code>bitReverseFlag</code> Flag for selection of output order(Set bitReverseFlag to output in normal order otherwise output in bit reversed order);
* <code>pTwiddle</code>points to array of twiddle coefficients; <code>pBitRevTable</code> points to the array of bit reversal table.
* <code>twidCoefModifier</code> modifier for twiddle factor table which supports all FFT lengths with same table;
* <code>pBitRevTable</code> modifier for bit reversal table which supports all FFT lengths with same table.
* <code>onebyfftLen</code> value of 1/fftLen to calculate CIFFT;
*
* \par Fixed-Point Behavior
* Care must be taken when using the fixed-point versions of the CFFT/CIFFT function.
* Refer to the function specific documentation below for usage guidelines.
*/


/*
* @brief  Core function for the floating-point CFFT butterfly process.
* @param[in, out] *pSrc            points to the in-place buffer of floating-point data type.
* @param[in]      fftLen           length of the FFT.
* @param[in]      *pCoef           points to the twiddle coefficient buffer.
* @param[in]      twidCoefModifier twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
* @return none.
*/

void csky_radix8_butterfly_f32(
float32_t * pSrc,
uint16_t fftLen,
const float32_t * pCoef,
uint16_t twidCoefModifier)
{
   uint32_t ia1, ia2, ia3, ia4, ia5, ia6, ia7;
   uint32_t i1, i2, i3, i4, i5, i6, i7, i8;
   uint32_t id;
   uint32_t n1, n2, j;

   float32_t r1, r2, r3, r4, r5, r6, r7, r8;
   float32_t t1, t2;
   float32_t s1, s2, s3, s4, s5, s6, s7, s8;
   float32_t p1, p2, p3, p4;
   float32_t co2, co3, co4, co5, co6, co7, co8;
   float32_t si2, si3, si4, si5, si6, si7, si8;
   const float32_t C81 = 0.70710678118f;

   n2 = fftLen;

   do
   {
      n1 = n2;
      n2 = n2 >> 3;
      i1 = 0;

      do
      {
         i2 = i1 + n2;
         i3 = i2 + n2;
         i4 = i3 + n2;
         i5 = i4 + n2;
         i6 = i5 + n2;
         i7 = i6 + n2;
         i8 = i7 + n2;
         r1 = pSrc[2 * i1] + pSrc[2 * i5];
         r5 = pSrc[2 * i1] - pSrc[2 * i5];
         r2 = pSrc[2 * i2] + pSrc[2 * i6];
         r6 = pSrc[2 * i2] - pSrc[2 * i6];
         r3 = pSrc[2 * i3] + pSrc[2 * i7];
         r7 = pSrc[2 * i3] - pSrc[2 * i7];
         r4 = pSrc[2 * i4] + pSrc[2 * i8];
         r8 = pSrc[2 * i4] - pSrc[2 * i8];
         t1 = r1 - r3;
         r1 = r1 + r3;
         r3 = r2 - r4;
         r2 = r2 + r4;
         pSrc[2 * i1] = r1 + r2;
         pSrc[2 * i5] = r1 - r2;
         r1 = pSrc[2 * i1 + 1] + pSrc[2 * i5 + 1];
         s5 = pSrc[2 * i1 + 1] - pSrc[2 * i5 + 1];
         r2 = pSrc[2 * i2 + 1] + pSrc[2 * i6 + 1];
         s6 = pSrc[2 * i2 + 1] - pSrc[2 * i6 + 1];
         s3 = pSrc[2 * i3 + 1] + pSrc[2 * i7 + 1];
         s7 = pSrc[2 * i3 + 1] - pSrc[2 * i7 + 1];
         r4 = pSrc[2 * i4 + 1] + pSrc[2 * i8 + 1];
         s8 = pSrc[2 * i4 + 1] - pSrc[2 * i8 + 1];
         t2 = r1 - s3;
         r1 = r1 + s3;
         s3 = r2 - r4;
         r2 = r2 + r4;
         pSrc[2 * i1 + 1] = r1 + r2;
         pSrc[2 * i5 + 1] = r1 - r2;
         pSrc[2 * i3]     = t1 + s3;
         pSrc[2 * i7]     = t1 - s3;
         pSrc[2 * i3 + 1] = t2 - r3;
         pSrc[2 * i7 + 1] = t2 + r3;
         r1 = (r6 - r8) * C81;
         r6 = (r6 + r8) * C81;
         r2 = (s6 - s8) * C81;
         s6 = (s6 + s8) * C81;
         t1 = r5 - r1;
         r5 = r5 + r1;
         r8 = r7 - r6;
         r7 = r7 + r6;
         t2 = s5 - r2;
         s5 = s5 + r2;
         s8 = s7 - s6;
         s7 = s7 + s6;
         pSrc[2 * i2]     = r5 + s7;
         pSrc[2 * i8]     = r5 - s7;
         pSrc[2 * i6]     = t1 + s8;
         pSrc[2 * i4]     = t1 - s8;
         pSrc[2 * i2 + 1] = s5 - r7;
         pSrc[2 * i8 + 1] = s5 + r7;
         pSrc[2 * i6 + 1] = t2 - r8;
         pSrc[2 * i4 + 1] = t2 + r8;

         i1 += n1;
      } while(i1 < fftLen);

      if(n2 < 8)
         break;

      ia1 = 0;
      j = 1;

      do
      {
         /*  index calculation for the coefficients */
         id  = ia1 + twidCoefModifier;
         ia1 = id;
         ia2 = ia1 + id;
         ia3 = ia2 + id;
         ia4 = ia3 + id;
         ia5 = ia4 + id;
         ia6 = ia5 + id;
         ia7 = ia6 + id;

         co2 = pCoef[2 * ia1];
         co3 = pCoef[2 * ia2];
         co4 = pCoef[2 * ia3];
         co5 = pCoef[2 * ia4];
         co6 = pCoef[2 * ia5];
         co7 = pCoef[2 * ia6];
         co8 = pCoef[2 * ia7];
         si2 = pCoef[2 * ia1 + 1];
         si3 = pCoef[2 * ia2 + 1];
         si4 = pCoef[2 * ia3 + 1];
         si5 = pCoef[2 * ia4 + 1];
         si6 = pCoef[2 * ia5 + 1];
         si7 = pCoef[2 * ia6 + 1];
         si8 = pCoef[2 * ia7 + 1];

         i1 = j;

         do
         {
            /*  index calculation for the input */
            i2 = i1 + n2;
            i3 = i2 + n2;
            i4 = i3 + n2;
            i5 = i4 + n2;
            i6 = i5 + n2;
            i7 = i6 + n2;
            i8 = i7 + n2;
            r1 = pSrc[2 * i1] + pSrc[2 * i5];
            r5 = pSrc[2 * i1] - pSrc[2 * i5];
            r2 = pSrc[2 * i2] + pSrc[2 * i6];
            r6 = pSrc[2 * i2] - pSrc[2 * i6];
            r3 = pSrc[2 * i3] + pSrc[2 * i7];
            r7 = pSrc[2 * i3] - pSrc[2 * i7];
            r4 = pSrc[2 * i4] + pSrc[2 * i8];
            r8 = pSrc[2 * i4] - pSrc[2 * i8];
            t1 = r1 - r3;
            r1 = r1 + r3;
            r3 = r2 - r4;
            r2 = r2 + r4;
            pSrc[2 * i1] = r1 + r2;
            r2 = r1 - r2;
            s1 = pSrc[2 * i1 + 1] + pSrc[2 * i5 + 1];
            s5 = pSrc[2 * i1 + 1] - pSrc[2 * i5 + 1];
            s2 = pSrc[2 * i2 + 1] + pSrc[2 * i6 + 1];
            s6 = pSrc[2 * i2 + 1] - pSrc[2 * i6 + 1];
            s3 = pSrc[2 * i3 + 1] + pSrc[2 * i7 + 1];
            s7 = pSrc[2 * i3 + 1] - pSrc[2 * i7 + 1];
            s4 = pSrc[2 * i4 + 1] + pSrc[2 * i8 + 1];
            s8 = pSrc[2 * i4 + 1] - pSrc[2 * i8 + 1];
            t2 = s1 - s3;
            s1 = s1 + s3;
            s3 = s2 - s4;
            s2 = s2 + s4;
            r1 = t1 + s3;
            t1 = t1 - s3;
            pSrc[2 * i1 + 1] = s1 + s2;
            s2 = s1 - s2;
            s1 = t2 - r3;
            t2 = t2 + r3;
            p1 = co5 * r2;
            p2 = si5 * s2;
            p3 = co5 * s2;
            p4 = si5 * r2;
            pSrc[2 * i5]     = p1 + p2;
            pSrc[2 * i5 + 1] = p3 - p4;
            p1 = co3 * r1;
            p2 = si3 * s1;
            p3 = co3 * s1;
            p4 = si3 * r1;
            pSrc[2 * i3]     = p1 + p2;
            pSrc[2 * i3 + 1] = p3 - p4;
            p1 = co7 * t1;
            p2 = si7 * t2;
            p3 = co7 * t2;
            p4 = si7 * t1;
            pSrc[2 * i7]     = p1 + p2;
            pSrc[2 * i7 + 1] = p3 - p4;
            r1 = (r6 - r8) * C81;
            r6 = (r6 + r8) * C81;
            s1 = (s6 - s8) * C81;
            s6 = (s6 + s8) * C81;
            t1 = r5 - r1;
            r5 = r5 + r1;
            r8 = r7 - r6;
            r7 = r7 + r6;
            t2 = s5 - s1;
            s5 = s5 + s1;
            s8 = s7 - s6;
            s7 = s7 + s6;
            r1 = r5 + s7;
            r5 = r5 - s7;
            r6 = t1 + s8;
            t1 = t1 - s8;
            s1 = s5 - r7;
            s5 = s5 + r7;
            s6 = t2 - r8;
            t2 = t2 + r8;
            p1 = co2 * r1;
            p2 = si2 * s1;
            p3 = co2 * s1;
            p4 = si2 * r1;
            pSrc[2 * i2]     = p1 + p2;
            pSrc[2 * i2 + 1] = p3 - p4;
            p1 = co8 * r5;
            p2 = si8 * s5;
            p3 = co8 * s5;
            p4 = si8 * r5;
            pSrc[2 * i8]     = p1 + p2;
            pSrc[2 * i8 + 1] = p3 - p4;
            p1 = co6 * r6;
            p2 = si6 * s6;
            p3 = co6 * s6;
            p4 = si6 * r6;
            pSrc[2 * i6]     = p1 + p2;
            pSrc[2 * i6 + 1] = p3 - p4;
            p1 = co4 * t1;
            p2 = si4 * t2;
            p3 = co4 * t2;
            p4 = si4 * t1;
            pSrc[2 * i4]     = p1 + p2;
            pSrc[2 * i4 + 1] = p3 - p4;

            i1 += n1;
         } while(i1 < fftLen);

         j++;
      } while(j < n2);

      twidCoefModifier <<= 3;
   } while(n2 > 7);
}

/**
* @} end of Radix8_CFFT_CIFFT group
*/
