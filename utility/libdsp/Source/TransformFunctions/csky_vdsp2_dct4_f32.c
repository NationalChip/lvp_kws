/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_dct4_f32.c
* Description:  Processing function of DCT4 & IDCT4 F32
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
* This file comes from arm_dct4_f32.c. It is renamed by replacing arm with csky_vdsp2.
*
*/

/******************************************************************************
 * @file     csky_vdsp2_dct4_f32.c
 * @brief    Processing function of DCT4 & IDCT4 F32.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_vdsp2_math.h"

/*Copy the conjugate value of the RFFT ouput to the following memory*/
inline void rfft_result_copy_f32(float32_t *pSrc, uint16_t length)
{
  uint32_t i;
  float32_t *pS1, *pS2;

  /* Add the conjugate part for DCT usage.*/
  i = (((uint32_t)length >> 1u) -1u) >> 2u;
  pS1 = &pSrc[length+2];
  pS2 = &pSrc[length-2];

  do
  {
    *pS1++  = *pS2++;
    *pS1++  = -*pS2;
    pS2    -= 3;

    *pS1++  = *pS2++;
    *pS1++  = -*pS2;
    pS2    -= 3;

    *pS1++  = *pS2++;
    *pS1++  = -*pS2;
    pS2    -= 3;

    *pS1++  = *pS2++;
    *pS1++  = -*pS2;
    pS2    -= 3;

    i--;

  }while(i > 0u);

  i = (((uint32_t)length >> 1u) -1u) % 4u;
  do
  {
    *pS1++  = *pS2++;
    *pS1++  = -*pS2;
    pS2    -= 3;

    i--;

  }while(i > 0u);

}
/**
 * @ingroup groupTransforms
 */

/**
 * @defgroup DCT4_IDCT4 DCT Type IV Functions
 * Representation of signals by minimum number of values is important for storage and transmission.
 * The possibility of large discontinuity between the beginning and end of a period of a signal
 * in DFT can be avoided by extending the signal so that it is even-symmetric.
 * Discrete Cosine Transform (DCT) is constructed such that its energy is heavily concentrated in the lower part of the
 * spectrum and is very widely used in signal and image coding applications.
 * The family of DCTs (DCT type- 1,2,3,4) is the outcome of different combinations of homogeneous boundary conditions.
 * DCT has an excellent energy-packing capability, hence has many applications and in data compression in particular.
 *
 * DCT is essentially the Discrete Fourier Transform(DFT) of an even-extended real signal.
 * Reordering of the input data makes the computation of DCT just a problem of
 * computing the DFT of a real signal with a few additional operations.
 * This approach provides regular, simple, and very efficient DCT algorithms for practical hardware and software implementations.
 *
 * DCT type-II can be implemented using Fast fourier transform (FFT) internally, as the transform is applied on real values, Real FFT can be used.
 * DCT4 is implemented using DCT2 as their implementations are similar except with some added pre-processing and post-processing.
 * DCT2 implementation can be described in the following steps:
 * - Re-ordering input
 * - Calculating Real FFT
 * - Multiplication of weights and Real FFT output and getting real part from the product.
 *
 * This process is explained by the block diagram below:
 * \image html DCT4.gif "Discrete Cosine Transform - type-IV"
 *
 * \par Algorithm:
 * The N-point type-IV DCT is defined as a real, linear transformation by the formula:
 * \image html DCT4Equation.gif
 * where <code>k = 0,1,2,.....N-1</code>
 *\par
 * Its inverse is defined as follows:
 * \image html IDCT4Equation.gif
 * where <code>n = 0,1,2,.....N-1</code>
 *\par
 * The DCT4 matrices become involutory (i.e. they are self-inverse) by multiplying with an overall scale factor of sqrt(2/N).
 * The symmetry of the transform matrix indicates that the fast algorithms for the forward
 * and inverse transform computation are identical.
 * Note that the implementation of Inverse DCT4 and DCT4 is same, hence same process function can be used for both.
 *
 * \par Lengths supported by the transform:
 *  As DCT4 internally uses Real FFT, it supports all the lengths supported by csky_vdsp2_rfft_fast_f32().
 * The library provides separate functions for Q15, Q31, and floating-point data types.
 * \par Instance Structure
 * The instances for Real FFT and FFT, cosine values table and twiddle factor table are stored in an instance data structure.
 * A separate instance structure must be defined for each transform.
 * There are separate instance structure declarations for each of the 3 supported data types.
 *
 * \par How to use the function
 * Same as the RFFT functions, the DCT4 functions also have two ways to use them. The recommended
 * one is to call them straightforwardly without initialization, which has a adventage of less
 * memory usage. And it is more convenient to use RFFT functions, when the
 * initialization function is not needed.
 * \par
 * The other way needs an associated initialization function for each data type.
 * The initialization function performs the following operations:
 * - Sets the values of the internal structure fields.
 * - Initializes Real FFT as its process function is used internally in DCT4, by calling csky_vdsp2_rfft_fast_init_f32().
 * \par
 * For these two ways, they both need to initialize the related instance structure.
 * The difference between them is that, the recommended way initializes the structures
 * manually and the second one initializes the structures by the associated
 * initialization function. Alougth, these two ways are both available, but the one without
 * initialization is perfered.
 * The initialized instance structures are as follows:
 * <pre>
 *csky_vdsp2_dct4_instance_f32 S = {N, Nby2, normalize, pTwiddle, pCosFactor, pRfft, pCfft};
 *csky_vdsp2_dct4_instance_q31 S = {N, Nby2, normalize, pTwiddle, pCosFactor, pRfft, pCfft};
 *csky_vdsp2_dct4_instance_q15 S = {N, Nby2, normalize, pTwiddle, pCosFactor, pRfft, pCfft};
 * </pre>
 * where \c N is the length of the DCT4; \c Nby2 is half of the length of the DCT4;
 * \c normalize is normalizing factor used and is equal to <code>sqrt(2/N)</code>;
 * \c pTwiddle points to the twiddle factor table;
 * \c pCosFactor points to the cosFactor table;
 * \c pRfft points to the real FFT instance;
 * \c pCfft points to the complex FFT instance;
 * The CFFT and RFFT structures also needs to be initialized, refer to csky_vdsp2_cfft_f32()
 * and csky_vdsp2_rfft_fast_f32() respectively for details regarding static initialization.
 *
 * \par Fixed-Point Behavior
 * Care must be taken when using the fixed-point versions of the DCT4 transform functions.
 * In particular, the overflow and saturation behavior of the accumulator used in each function must be considered.
 * Refer to the function specific documentation below for usage guidelines.
 */

 /**
 * @addtogroup DCT4_IDCT4
 * @{
 */

/**
 * @brief Processing function for the floating-point DCT4/IDCT4.
 * @param[in]       *S             points to an instance of the floating-point DCT4/IDCT4 structure.
 * @param[in]       *pState        points to state buffer.
 * @param[in,out]   *pInlineBuffer points to the in-place input and output buffer.
 * @return none.
 *
 * \par Recommended Usage
 * For each length of input, the usage is as follows:
 * \code
 * csky_vdsp2_dct4_f32(&csky_vdsp2_dct4_sR_f32_len128, pState, pSrc);
 * csky_vdsp2_dct4_f32(&csky_vdsp2_dct4_sR_f32_len512, pState, pSrc);
 * csky_vdsp2_dct4_f32(&csky_vdsp2_dct4_sR_f32_len2048, pState, pSrc);
 * csky_vdsp2_dct4_f32(&csky_vdsp2_dct4_sR_f32_len8192, pState, pSrc);
 * \endcode
 */

void csky_vdsp2_dct4_f32(
  const csky_vdsp2_dct4_instance_f32 * S,
  float32_t * pState,
  float32_t * pInlineBuffer)
{
  uint32_t i;                                    /* Loop counter */
  float32_t *weights = S->pTwiddle;              /* Pointer to the Weights table */
  float32_t *cosFact = S->pCosFactor;            /* Pointer to the cos factors table */
  float32_t *pS1, *pS2, *pbuff;                  /* Temporary pointers for input buffer and pState buffer */
  float32_t in;                                  /* Temporary variable */


  /* DCT4 computation involves DCT2 (which is calculated using RFFT)
   * along with some pre-processing and post-processing.
   * Computational procedure is explained as follows:
   * (a) Pre-processing involves multiplying input with cos factor,
   *     r(n) = 2 * u(n) * cos(pi*(2*n+1)/(4*N))
   *              where,
   *                 r(n) -- output of preprocessing
   *                 u(n) -- input to preprocessing(actual Source buffer)
   * (b) Calculation of DCT2 using FFT is divided into three steps:
   *                  Step1: Re-ordering of even and odd elements of input.
   *                  Step2: Calculating FFT of the re-ordered input.
   *                  Step3: Taking the real part of the product of FFT output and weights.
   * (c) Post-processing - DCT4 can be obtained from DCT2 output using the following equation:
   *                   Y4(k) = Y2(k) - Y4(k-1) and Y4(-1) = Y4(0)
   *                        where,
   *                           Y4 -- DCT4 output,   Y2 -- DCT2 output
   * (d) Multiplying the output with the normalizing factor sqrt(2/N).
   */

        /*-------- Pre-processing ------------*/
  /* Multiplying input with cos factor i.e. r(n) = 2 * x(n) * cos(pi*(2*n+1)/(4*n)) */
  csky_vdsp2_mult_f32(pInlineBuffer, cosFact, pInlineBuffer, S->N);

  /* ----------------------------------------------------------------
   * Step1: Re-ordering of even and odd elements as,
   *             pState[i] =  pInlineBuffer[2*i] and
   *             pState[N-i-1] = pInlineBuffer[2*i+1] where i = 0 to N/2
   ---------------------------------------------------------------------*/

  /* pS1 initialized to pState */
  pS1 = pState;

  /* pS2 initialized to pState+N-1, so that it points to the end of the state buffer */
  pS2 = pState + (S->N - 1u);

  /* pbuff initialized to input buffer */
  pbuff = pInlineBuffer;

#ifndef CSKY_MATH_NO_SIMD


  /* Initializing the loop counter to N/2 >> 2 for loop unrolling by 4 */
  i = (uint32_t) S->Nby2 >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  do
  {
    /* Re-ordering of even and odd elements */
    /* pState[i] =  pInlineBuffer[2*i] */
    *pS1++ = *pbuff++;
    /* pState[N-i-1] = pInlineBuffer[2*i+1] */
    *pS2-- = *pbuff++;

    *pS1++ = *pbuff++;
    *pS2-- = *pbuff++;

    *pS1++ = *pbuff++;
    *pS2-- = *pbuff++;

    *pS1++ = *pbuff++;
    *pS2-- = *pbuff++;

    /* Decrement the loop counter */
    i--;
  } while(i > 0u);

  /* pbuff initialized to input buffer */
  pbuff = pInlineBuffer;

  /* pS1 initialized to pState */
  pS1 = pState;

  /* Initializing the loop counter to N/4 instead of N for loop unrolling */
  i = (uint32_t) S->N >> 2u;

  /* Processing with loop unrolling 4 times as N is always multiple of 4.
   * Compute 4 outputs at a time */
  do
  {
    /* Writing the re-ordered output back to inplace input buffer */
    *pbuff++ = *pS1++;
    *pbuff++ = *pS1++;
    *pbuff++ = *pS1++;
    *pbuff++ = *pS1++;

    /* Decrement the loop counter */
    i--;
  } while(i > 0u);


  /* ---------------------------------------------------------
   *     Step2: Calculate RFFT for N-point input
   * ---------------------------------------------------------- */
  /* pInlineBuffer is real input of length N , pState is the complex output of length 2N */
  csky_vdsp2_rfft_fast_f32(S->pRfft, pInlineBuffer, pState, 0);

  /* Add the conjugate part for DCT usage.*/
  rfft_result_copy_f32(pState, S->N);

  /*----------------------------------------------------------------------
   *  Step3: Multiply the FFT output with the weights.
   *----------------------------------------------------------------------*/
  csky_vdsp2_cmplx_mult_cmplx_f32(pState, weights, pState, S->Nby2+1);

  csky_vdsp2_cmplx_mult_cmplx_re_f32(pState+S->N+2, &weights[S->N], pState+S->N+2, S->Nby2-1);

  /* ----------- Post-processing ---------- */
  /* DCT-IV can be obtained from DCT-II by the equation,
   *       Y4(k) = Y2(k) - Y4(k-1) and Y4(-1) = Y4(0)
   *       Hence, Y4(0) = Y2(0)/2  */
  /* Getting only real part from the output and Converting to DCT-IV */

  /* Initializing the loop counter to N >> 2 for loop unrolling by 4 */
  i = ((uint32_t) S->N - 1u) >> 2u;

  /* pbuff initialized to input buffer. */
  pbuff = pInlineBuffer;

  /* pS1 initialized to pState */
  pS1 = pState;

  /* Calculating Y4(0) from Y2(0) using Y4(0) = Y2(0)/2 */
  in = *pS1++ * (float32_t) 0.5;
  /* input buffer acts as inplace, so output values are stored in the input itself. */
  *pbuff++ = in;

  /* pState pointer is incremented twice as the real values are located alternatively in the array */
  pS1++;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  do
  {
    /* Calculating Y4(1) to Y4(N-1) from Y2 using equation Y4(k) = Y2(k) - Y4(k-1) */
    /* pState pointer (pS1) is incremented twice as the real values are located alternatively in the array */
    in = *pS1++ - in;
    *pbuff++ = in;
    /* points to the next real value */
    pS1++;

    in = *pS1++ - in;
    *pbuff++ = in;
    pS1++;

    in = *pS1++ - in;
    *pbuff++ = in;
    pS1++;

    in = *pS1++ - in;
    *pbuff++ = in;
    pS1++;

    /* Decrement the loop counter */
    i--;
  } while(i > 0u);

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  i = ((uint32_t) S->N - 1u) % 0x4u;

  while(i > 0u)
  {
    /* Calculating Y4(1) to Y4(N-1) from Y2 using equation Y4(k) = Y2(k) - Y4(k-1) */
    /* pState pointer (pS1) is incremented twice as the real values are located alternatively in the array */
    in = *pS1++ - in;
    *pbuff++ = in;
    /* points to the next real value */
    pS1++;

    /* Decrement the loop counter */
    i--;
  }

        /*------------ Normalizing the output by multiplying with the normalizing factor ----------*/

  /* Initializing the loop counter to N/4 instead of N for loop unrolling */
  i = (uint32_t) S->N >> 2u;

  /* pbuff initialized to the pInlineBuffer(now contains the output values) */
  pbuff = pInlineBuffer;

  /* Processing with loop unrolling 4 times as N is always multiple of 4.  Compute 4 outputs at a time */
  do
  {
    /* Multiplying pInlineBuffer with the normalizing factor sqrt(2/N) */
    in = *pbuff;
    *pbuff++ = in * S->normalize;

    in = *pbuff;
    *pbuff++ = in * S->normalize;

    in = *pbuff;
    *pbuff++ = in * S->normalize;

    in = *pbuff;
    *pbuff++ = in * S->normalize;

    /* Decrement the loop counter */
    i--;
  } while(i > 0u);


#else


  /* Initializing the loop counter to N/2 */
  i = (uint32_t) S->Nby2;

  do
  {
    /* Re-ordering of even and odd elements */
    /* pState[i] =  pInlineBuffer[2*i] */
    *pS1++ = *pbuff++;
    /* pState[N-i-1] = pInlineBuffer[2*i+1] */
    *pS2-- = *pbuff++;

    /* Decrement the loop counter */
    i--;
  } while(i > 0u);

  /* pbuff initialized to input buffer */
  pbuff = pInlineBuffer;

  /* pS1 initialized to pState */
  pS1 = pState;

  /* Initializing the loop counter */
  i = (uint32_t) S->N;

  do
  {
    /* Writing the re-ordered output back to inplace input buffer */
    *pbuff++ = *pS1++;

    /* Decrement the loop counter */
    i--;
  } while(i > 0u);


  /* ---------------------------------------------------------
   *     Step2: Calculate RFFT for N-point input
   * ---------------------------------------------------------- */
  /* pInlineBuffer is real input of length N , pState is the complex output of length 2N */
  csky_vdsp2_rfft_fast_f32(S->pRfft, pInlineBuffer, pState, 0);

  /* Add the conjugate part for DCT usage.*/
  rfft_result_copy_f32(pState, S->N);


  /*----------------------------------------------------------------------
  *  Step3: Multiply the FFT output with the weights.
  *----------------------------------------------------------------------*/
  csky_vdsp2_cmplx_mult_cmplx_f32(pState, weights, pState, S->N);

  /* ----------- Post-processing ---------- */
  /* DCT-IV can be obtained from DCT-II by the equation,
   *       Y4(k) = Y2(k) - Y4(k-1) and Y4(-1) = Y4(0)
   *       Hence, Y4(0) = Y2(0)/2  */
  /* Getting only real part from the output and Converting to DCT-IV */

  /* pbuff initialized to input buffer. */
  pbuff = pInlineBuffer;

  /* pS1 initialized to pState */
  pS1 = pState;

  /* Calculating Y4(0) from Y2(0) using Y4(0) = Y2(0)/2 */
  in = *pS1++ * (float32_t) 0.5;
  /* input buffer acts as inplace, so output values are stored in the input itself. */
  *pbuff++ = in;

  /* pState pointer is incremented twice as the real values are located alternatively in the array */
  pS1++;

  /* Initializing the loop counter */
  i = ((uint32_t) S->N - 1u);

  do
  {
    /* Calculating Y4(1) to Y4(N-1) from Y2 using equation Y4(k) = Y2(k) - Y4(k-1) */
    /* pState pointer (pS1) is incremented twice as the real values are located alternatively in the array */
    in = *pS1++ - in;
    *pbuff++ = in;
    /* points to the next real value */
    pS1++;


    /* Decrement the loop counter */
    i--;
  } while(i > 0u);


        /*------------ Normalizing the output by multiplying with the normalizing factor ----------*/

  /* Initializing the loop counter */
  i = (uint32_t) S->N;

  /* pbuff initialized to the pInlineBuffer(now contains the output values) */
  pbuff = pInlineBuffer;

  do
  {
    /* Multiplying pInlineBuffer with the normalizing factor sqrt(2/N) */
    in = *pbuff;
    *pbuff++ = in * S->normalize;

    /* Decrement the loop counter */
    i--;
  } while(i > 0u);

#endif /* #ifndef CSKY_MATH_NO_SIMD */

}

/**
   * @} end of DCT4_IDCT4 group
   */
