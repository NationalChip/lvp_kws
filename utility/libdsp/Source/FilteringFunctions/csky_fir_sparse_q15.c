/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_fir_sparse_q15.c
* Description:  Q15 sparse FIR filter processing function
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
* This file comes from arm_fir_sparse_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_fir_sparse_q15.c
 * @brief    Q15 sparse FIR filter processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @addtogroup FIR_Sparse
 * @{
 */
/**
 * @brief Q15 Circular write function.
 */
static void csky_circularWrite_q15(
q15_t * circBuffer,
int32_t L,
uint16_t * writeOffset,
int32_t bufferInc,
const q15_t * src,
int32_t srcInc,
uint32_t blockSize)
{
  uint32_t i = 0u;
  int32_t wOffset;
  /* Copy the value of Index pointer that points
   * to the current location where the input samples to be copied */
  wOffset = *writeOffset;
  /* Loop over the blockSize */
  i = blockSize;
  while(i > 0u)
  {
    /* copy the input sample to the circular buffer */
    circBuffer[wOffset] = *src;
    /* Update the input pointer */
    src += srcInc;
    /* Circularly update wOffset.  Watch out for positive and negative value */
    wOffset += bufferInc;
    if(wOffset >= L)
      wOffset -= L;
    /* Decrement the loop counter */
    i--;
  }
  /* Update the index pointer */
  *writeOffset = (uint16_t)wOffset;
}

/**
 * @brief Q15 Circular Read function.
 */
static void csky_circularRead_q15(
q15_t * circBuffer,
int32_t L,
int32_t * readOffset,
int32_t bufferInc,
q15_t * dst,
q15_t * dst_base,
int32_t dst_length,
int32_t dstInc,
uint32_t blockSize)
{
  uint32_t i = 0;
  int32_t rOffset;
  q15_t *dst_end;
  /* Copy the value of Index pointer that points
   * to the current location from where the input samples to be read */
  rOffset = *readOffset;
  dst_end = dst_base + dst_length;
  /* Loop over the blockSize */
  i = blockSize;
  while(i > 0u)
  {
    /* copy the sample from the circular buffer to the destination buffer */
    *dst = circBuffer[rOffset];
    /* Update the input pointer */
    dst += dstInc;
    if(dst == dst_end)
    {
      dst = dst_base;
    }
    /* Circularly update wOffset.  Watch out for positive and negative value */
    rOffset += bufferInc;
    if(rOffset >= L)
    {
      rOffset -= L;
    }
    /* Decrement the loop counter */
    i--;
  }
  /* Update the index pointer */
  *readOffset = rOffset;
}

/**
 * @brief Processing function for the Q15 sparse FIR filter.
 * @param[in]  *S           points to an instance of the Q15 sparse FIR structure.
 * @param[in]  *pSrc        points to the block of input data.
 * @param[out] *pDst        points to the block of output data
 * @param[in]  *pScratchIn  points to a temporary buffer of size blockSize.
 * @param[in]  *pScratchOut points to a temporary buffer of size blockSize.
 * @param[in]  blockSize    number of input samples to process per call.
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using an internal 32-bit accumulator.
 * The 1.15 x 1.15 multiplications yield a 2.30 result and these are added to a 2.30 accumulator.
 * Thus the full precision of the multiplications is maintained but there is only a single guard bit in the accumulator.
 * If the accumulator result overflows it will wrap around rather than saturate.
 * After all multiply-accumulates are performed, the 2.30 accumulator is truncated to 2.15 format and then saturated to 1.15 format.
 * In order to avoid overflows the input signal or coefficients must be scaled down by log2(numTaps) bits.
 */


void csky_fir_sparse_q15(
  csky_fir_sparse_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  q15_t * pScratchIn,
  q31_t * pScratchOut,
  uint32_t blockSize)
{

  q15_t *pState = S->pState;                     /* State pointer */
  q15_t *pIn = pSrc;                             /* Working pointer for input */
  q15_t *pOut = pDst;                            /* Working pointer for output */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *px;                                     /* Temporary pointers for scratch buffer */
  q15_t *pb = pScratchIn;                        /* Temporary pointers for scratch buffer */
  q15_t *py = pState;                            /* Temporary pointers for state buffer */
  int32_t *pTapDelay = S->pTapDelay;             /* Pointer to the array containing offset of the non-zero tap values. */
  uint32_t delaySize = S->maxDelay + blockSize;  /* state length */
  uint16_t numTaps = S->numTaps;                 /* Filter order */
  int32_t readIndex;                             /* Read index of the state buffer */
  uint32_t tapCnt, blkCnt;                       /* loop counters */
  q15_t coeff = *pCoeffs++;                      /* Read the first coefficient value */
  q31_t *pScr2 = pScratchOut;                    /* Working pointer for pScratchOut */


#ifndef CSKY_MATH_NO_SIMD


  q31_t in1, in2;                                /* Temporary variables */


  /* BlockSize of Input samples are copied into the state buffer */
  /* StateIndex points to the starting position to write in the state buffer */
  csky_circularWrite_q15(py, delaySize, &S->stateIndex, 1, pIn, 1, blockSize);

  /* Loop over the number of taps. */
  tapCnt = numTaps;

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = (S->stateIndex - blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if(readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Working pointer for state buffer is updated */
  py = pState;

  /* blockSize samples are read from the state buffer */
  csky_circularRead_q15(py, delaySize, &readIndex, 1,
                       pb, pb, blockSize, 1, blockSize);

  /* Working pointer for the scratch buffer of state values */
  px = pb;

  /* Working pointer for scratch buffer of output values */
  pScratchOut = pScr2;

  /* Loop over the blockSize. Unroll by a factor of 4.
   * Compute 4 multiplications at a time. */
  blkCnt = blockSize >> 2;

  while(blkCnt > 0u)
  {
    /* Perform multiplication and store in the scratch buffer */
    *pScratchOut++ = ((q31_t) * px++ * coeff);
    *pScratchOut++ = ((q31_t) * px++ * coeff);
    *pScratchOut++ = ((q31_t) * px++ * coeff);
    *pScratchOut++ = ((q31_t) * px++ * coeff);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4,
   * compute the remaining samples */
  blkCnt = blockSize % 0x4u;

  while(blkCnt > 0u)
  {
    /* Perform multiplication and store in the scratch buffer */
    *pScratchOut++ = ((q31_t) * px++ * coeff);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Load the coefficient value and
   * increment the coefficient buffer for the next set of state values */
  coeff = *pCoeffs++;

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = (S->stateIndex - blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if(readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Loop over the number of taps. */
  tapCnt = (uint32_t) numTaps - 2u;

  while(tapCnt > 0u)
  {
    /* Working pointer for state buffer is updated */
    py = pState;

    /* blockSize samples are read from the state buffer */
    csky_circularRead_q15(py, delaySize, &readIndex, 1,
                         pb, pb, blockSize, 1, blockSize);

    /* Working pointer for the scratch buffer of state values */
    px = pb;

    /* Working pointer for scratch buffer of output values */
    pScratchOut = pScr2;

    /* Loop over the blockSize. Unroll by a factor of 4.
     * Compute 4 MACS at a time. */
    blkCnt = blockSize >> 2;

    while(blkCnt > 0u)
    {
      /* Perform Multiply-Accumulate */
      *pScratchOut++ += (q31_t) * px++ * coeff;
      *pScratchOut++ += (q31_t) * px++ * coeff;
      *pScratchOut++ += (q31_t) * px++ * coeff;
      *pScratchOut++ += (q31_t) * px++ * coeff;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* If the blockSize is not a multiple of 4,
     * compute the remaining samples */
    blkCnt = blockSize % 0x4u;

    while(blkCnt > 0u)
    {
      /* Perform Multiply-Accumulate */
      *pScratchOut++ += (q31_t) * px++ * coeff;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* Load the coefficient value and
     * increment the coefficient buffer for the next set of state values */
    coeff = *pCoeffs++;

    /* Read Index, from where the state buffer should be read, is calculated. */
    readIndex = (S->stateIndex - blockSize) - *pTapDelay++;

    /* Wraparound of readIndex */
    if(readIndex < 0)
    {
      readIndex += (int32_t) delaySize;
    }

    /* Decrement the tap loop counter */
    tapCnt--;
  }

	/* Compute last tap without the final read of pTapDelay */

	/* Working pointer for state buffer is updated */
	py = pState;

	/* blockSize samples are read from the state buffer */
	csky_circularRead_q15(py, delaySize, &readIndex, 1,
											 pb, pb, blockSize, 1, blockSize);

	/* Working pointer for the scratch buffer of state values */
	px = pb;

	/* Working pointer for scratch buffer of output values */
	pScratchOut = pScr2;

	/* Loop over the blockSize. Unroll by a factor of 4.
	 * Compute 4 MACS at a time. */
	blkCnt = blockSize >> 2;

	while(blkCnt > 0u)
	{
		/* Perform Multiply-Accumulate */
		*pScratchOut++ += (q31_t) * px++ * coeff;
		*pScratchOut++ += (q31_t) * px++ * coeff;
		*pScratchOut++ += (q31_t) * px++ * coeff;
		*pScratchOut++ += (q31_t) * px++ * coeff;

		/* Decrement the loop counter */
		blkCnt--;
	}

	/* If the blockSize is not a multiple of 4,
	 * compute the remaining samples */
	blkCnt = blockSize % 0x4u;

	while(blkCnt > 0u)
	{
		/* Perform Multiply-Accumulate */
		*pScratchOut++ += (q31_t) * px++ * coeff;

		/* Decrement the loop counter */
		blkCnt--;
	}

  /* All the output values are in pScratchOut buffer.
     Convert them into 1.15 format, saturate and store in the destination buffer. */
  /* Loop over the blockSize. */
  blkCnt = blockSize >> 2;

  while(blkCnt > 0u)
  {
    in1 = *pScr2++;
    in2 = *pScr2++;

#ifndef  CSKY_MATH_BIG_ENDIAN

    *__SIMD32(pOut)++ =
      __PKHBT((q15_t) __SSAT_16(in1 >> 15), (q15_t) __SSAT_16(in2 >> 15),
              16);

#else
    *__SIMD32(pOut)++ =
      __PKHBT((q15_t) __SSAT_16(in2 >> 15), (q15_t) __SSAT_16(in1 >> 15),
              16);

#endif /*      #ifndef  CSKY_MATH_BIG_ENDIAN    */

    in1 = *pScr2++;

    in2 = *pScr2++;

#ifndef  CSKY_MATH_BIG_ENDIAN

    *__SIMD32(pOut)++ =
      __PKHBT((q15_t) __SSAT_16(in1 >> 15), (q15_t) __SSAT_16(in2 >> 15),
              16);

#else

    *__SIMD32(pOut)++ =
      __PKHBT((q15_t) __SSAT_16(in2 >> 15), (q15_t) __SSAT_16(in1 >> 15),
              16);

#endif /*      #ifndef  CSKY_MATH_BIG_ENDIAN    */


    blkCnt--;

  }

  /* If the blockSize is not a multiple of 4,
     remaining samples are processed in the below loop */
  blkCnt = blockSize % 0x4u;

  while(blkCnt > 0u)
  {
    *pOut++ = (q15_t) __SSAT_16(*pScr2++ >> 15);
    blkCnt--;
  }

#else


  /* BlockSize of Input samples are copied into the state buffer */
  /* StateIndex points to the starting position to write in the state buffer */
  csky_circularWrite_q15(py, delaySize, &S->stateIndex, 1, pIn, 1, blockSize);

  /* Loop over the number of taps. */
  tapCnt = numTaps;

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = (S->stateIndex - blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if(readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Working pointer for state buffer is updated */
  py = pState;

  /* blockSize samples are read from the state buffer */
  csky_circularRead_q15(py, delaySize, &readIndex, 1,
                       pb, pb, blockSize, 1, blockSize);

  /* Working pointer for the scratch buffer of state values */
  px = pb;

  /* Working pointer for scratch buffer of output values */
  pScratchOut = pScr2;

  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* Perform multiplication and store in the scratch buffer */
    *pScratchOut++ = ((q31_t) * px++ * coeff);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Load the coefficient value and
   * increment the coefficient buffer for the next set of state values */
  coeff = *pCoeffs++;

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = (S->stateIndex - blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if(readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Loop over the number of taps. */
  tapCnt = (uint32_t) numTaps - 2u;

  while(tapCnt > 0u)
  {
    /* Working pointer for state buffer is updated */
    py = pState;

    /* blockSize samples are read from the state buffer */
    csky_circularRead_q15(py, delaySize, &readIndex, 1,
                         pb, pb, blockSize, 1, blockSize);

    /* Working pointer for the scratch buffer of state values */
    px = pb;

    /* Working pointer for scratch buffer of output values */
    pScratchOut = pScr2;

    blkCnt = blockSize;

    while(blkCnt > 0u)
    {
      /* Perform Multiply-Accumulate */
      *pScratchOut++ += (q31_t) * px++ * coeff;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* Load the coefficient value and
     * increment the coefficient buffer for the next set of state values */
    coeff = *pCoeffs++;

    /* Read Index, from where the state buffer should be read, is calculated. */
    readIndex = (S->stateIndex - blockSize) - *pTapDelay++;

    /* Wraparound of readIndex */
    if(readIndex < 0)
    {
      readIndex += (int32_t) delaySize;
    }

    /* Decrement the tap loop counter */
    tapCnt--;
  }

	/* Compute last tap without the final read of pTapDelay */

	/* Working pointer for state buffer is updated */
	py = pState;

	/* blockSize samples are read from the state buffer */
	csky_circularRead_q15(py, delaySize, &readIndex, 1,
											 pb, pb, blockSize, 1, blockSize);

	/* Working pointer for the scratch buffer of state values */
	px = pb;

	/* Working pointer for scratch buffer of output values */
	pScratchOut = pScr2;

	blkCnt = blockSize;

	while(blkCnt > 0u)
	{
		/* Perform Multiply-Accumulate */
		*pScratchOut++ += (q31_t) * px++ * coeff;

		/* Decrement the loop counter */
		blkCnt--;
	}

  /* All the output values are in pScratchOut buffer.
     Convert them into 1.15 format, saturate and store in the destination buffer. */
  /* Loop over the blockSize. */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    *pOut++ = (q15_t) __SSAT_16(*pScr2++ >> 15);
    blkCnt--;
  }

#endif /*   #ifndef CSKY_MATH_NO_SIMD */

}

/**
 * @} end of FIR_Sparse group
 */
