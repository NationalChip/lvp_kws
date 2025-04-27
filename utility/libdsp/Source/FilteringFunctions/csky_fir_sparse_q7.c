/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_fir_sparse_q7.c
* Description:  Q7 sparse FIR filter processing function
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
* This file comes from arm_fir_sparse_q7.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_fir_sparse_q7.c
 * @brief    Q7 sparse FIR filter processing function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"


/**
 * @ingroup groupFilters
 */

/**
 * @brief Q7 Circular write function.
 */
static void csky_circularWrite_q7(
q7_t * circBuffer,
int32_t L,
uint16_t * writeOffset,
int32_t bufferInc,
const q7_t * src,
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
 * @brief Q7 Circular Read function.
 */
static void csky_circularRead_q7(
q7_t * circBuffer,
int32_t L,
int32_t * readOffset,
int32_t bufferInc,
q7_t * dst,
q7_t * dst_base,
int32_t dst_length,
int32_t dstInc,
uint32_t blockSize)
{
  uint32_t i = 0;
  int32_t rOffset;
  q7_t *dst_end;
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
    /* Circularly update rOffset.  Watch out for positive and negative value */
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
 * @addtogroup FIR_Sparse
 * @{
 */


/**
 * @brief Processing function for the Q7 sparse FIR filter.
 * @param[in]  *S           points to an instance of the Q7 sparse FIR structure.
 * @param[in]  *pSrc        points to the block of input data.
 * @param[out] *pDst        points to the block of output data
 * @param[in]  *pScratchIn  points to a temporary buffer of size blockSize.
 * @param[in]  *pScratchOut points to a temporary buffer of size blockSize.
 * @param[in]  blockSize    number of input samples to process per call.
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using a 32-bit internal accumulator.
 * Both coefficients and state variables are represented in 1.7 format and multiplications yield a 2.14 result.
 * The 2.14 intermediate results are accumulated in a 32-bit accumulator in 18.14 format.
 * There is no risk of internal overflow with this approach and the full precision of intermediate multiplications is preserved.
 * The accumulator is then converted to 18.7 format by discarding the low 7 bits.
 * Finally, the result is truncated to 1.7 format.
 */

void csky_fir_sparse_q7(
  csky_fir_sparse_instance_q7 * S,
  q7_t * pSrc,
  q7_t * pDst,
  q7_t * pScratchIn,
  q31_t * pScratchOut,
  uint32_t blockSize)
{

  q7_t *pState = S->pState;                      /* State pointer */
  q7_t *pCoeffs = S->pCoeffs;                    /* Coefficient pointer */
  q7_t *px;                                      /* Scratch buffer pointer */
  q7_t *py = pState;                             /* Temporary pointers for state buffer */
  q7_t *pb = pScratchIn;                         /* Temporary pointers for scratch buffer */
  q7_t *pOut = pDst;                             /* Destination pointer */
  int32_t *pTapDelay = S->pTapDelay;             /* Pointer to the array containing offset of the non-zero tap values. */
  uint32_t delaySize = S->maxDelay + blockSize;  /* state length */
  uint16_t numTaps = S->numTaps;                 /* Filter order */
  int32_t readIndex;                             /* Read index of the state buffer */
  uint32_t tapCnt, blkCnt;                       /* loop counters */
  q7_t coeff = *pCoeffs++;                       /* Read the coefficient value */
  q31_t *pScr2 = pScratchOut;                    /* Working pointer for scratch buffer of output values */
  q31_t in;


#ifndef CSKY_MATH_NO_SIMD


  q7_t in1, in2, in3, in4;

  /* BlockSize of Input samples are copied into the state buffer */
  /* StateIndex points to the starting position to write in the state buffer */
  csky_circularWrite_q7(py, (int32_t) delaySize, &S->stateIndex, 1, pSrc, 1,
                       blockSize);

  /* Loop over the number of taps. */
  tapCnt = numTaps;

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = ((int32_t) S->stateIndex - (int32_t) blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if(readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Working pointer for state buffer is updated */
  py = pState;

  /* blockSize samples are read from the state buffer */
  csky_circularRead_q7(py, (int32_t) delaySize, &readIndex, 1, pb, pb,
                      (int32_t) blockSize, 1, blockSize);

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
  readIndex = ((int32_t) S->stateIndex - (int32_t) blockSize) - *pTapDelay++;

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
    csky_circularRead_q7(py, (int32_t) delaySize, &readIndex, 1, pb, pb,
                        (int32_t) blockSize, 1, blockSize);

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
      in = *pScratchOut + ((q31_t) * px++ * coeff);
      *pScratchOut++ = in;
      in = *pScratchOut + ((q31_t) * px++ * coeff);
      *pScratchOut++ = in;
      in = *pScratchOut + ((q31_t) * px++ * coeff);
      *pScratchOut++ = in;
      in = *pScratchOut + ((q31_t) * px++ * coeff);
      *pScratchOut++ = in;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* If the blockSize is not a multiple of 4,
     * compute the remaining samples */
    blkCnt = blockSize % 0x4u;

    while(blkCnt > 0u)
    {
      /* Perform Multiply-Accumulate */
      in = *pScratchOut + ((q31_t) * px++ * coeff);
      *pScratchOut++ = in;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* Load the coefficient value and
     * increment the coefficient buffer for the next set of state values */
    coeff = *pCoeffs++;

    /* Read Index, from where the state buffer should be read, is calculated. */
    readIndex = ((int32_t) S->stateIndex -
                 (int32_t) blockSize) - *pTapDelay++;

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
	csky_circularRead_q7(py, (int32_t) delaySize, &readIndex, 1, pb, pb,
											(int32_t) blockSize, 1, blockSize);

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
		in = *pScratchOut + ((q31_t) * px++ * coeff);
		*pScratchOut++ = in;
		in = *pScratchOut + ((q31_t) * px++ * coeff);
		*pScratchOut++ = in;
		in = *pScratchOut + ((q31_t) * px++ * coeff);
		*pScratchOut++ = in;
		in = *pScratchOut + ((q31_t) * px++ * coeff);
		*pScratchOut++ = in;

		/* Decrement the loop counter */
		blkCnt--;
	}

	/* If the blockSize is not a multiple of 4,
	 * compute the remaining samples */
	blkCnt = blockSize % 0x4u;

	while(blkCnt > 0u)
	{
		/* Perform Multiply-Accumulate */
		in = *pScratchOut + ((q31_t) * px++ * coeff);
		*pScratchOut++ = in;

		/* Decrement the loop counter */
		blkCnt--;
	}

  /* All the output values are in pScratchOut buffer.
     Convert them into 1.15 format, saturate and store in the destination buffer. */
  /* Loop over the blockSize. */
  blkCnt = blockSize >> 2;

  while(blkCnt > 0u)
  {
    in1 = (q7_t) __SSAT_8(*pScr2++ >> 7);
    in2 = (q7_t) __SSAT_8(*pScr2++ >> 7);
    in3 = (q7_t) __SSAT_8(*pScr2++ >> 7);
    in4 = (q7_t) __SSAT_8(*pScr2++ >> 7);

    *__SIMD32(pOut)++ = __PACKq7(in1, in2, in3, in4);

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4,
     remaining samples are processed in the below loop */
  blkCnt = blockSize % 0x4u;

  while(blkCnt > 0u)
  {
    *pOut++ = (q7_t) __SSAT_8(*pScr2++ >> 7);

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

#else


  /* BlockSize of Input samples are copied into the state buffer */
  /* StateIndex points to the starting position to write in the state buffer */
  csky_circularWrite_q7(py, (int32_t) delaySize, &S->stateIndex, 1, pSrc, 1,
                       blockSize);

  /* Loop over the number of taps. */
  tapCnt = numTaps;

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = ((int32_t) S->stateIndex - (int32_t) blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if(readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Working pointer for state buffer is updated */
  py = pState;

  /* blockSize samples are read from the state buffer */
  csky_circularRead_q7(py, (int32_t) delaySize, &readIndex, 1, pb, pb,
                      (int32_t) blockSize, 1, blockSize);

  /* Working pointer for the scratch buffer of state values */
  px = pb;

  /* Working pointer for scratch buffer of output values */
  pScratchOut = pScr2;

  /* Loop over the blockSize */
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
  readIndex = ((int32_t) S->stateIndex - (int32_t) blockSize) - *pTapDelay++;

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
    csky_circularRead_q7(py, (int32_t) delaySize, &readIndex, 1, pb, pb,
                        (int32_t) blockSize, 1, blockSize);

    /* Working pointer for the scratch buffer of state values */
    px = pb;

    /* Working pointer for scratch buffer of output values */
    pScratchOut = pScr2;

    /* Loop over the blockSize */
    blkCnt = blockSize;

    while(blkCnt > 0u)
    {
      /* Perform Multiply-Accumulate */
      in = *pScratchOut + ((q31_t) * px++ * coeff);
      *pScratchOut++ = in;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* Load the coefficient value and
     * increment the coefficient buffer for the next set of state values */
    coeff = *pCoeffs++;

    /* Read Index, from where the state buffer should be read, is calculated. */
    readIndex =
      ((int32_t) S->stateIndex - (int32_t) blockSize) - *pTapDelay++;

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
	csky_circularRead_q7(py, (int32_t) delaySize, &readIndex, 1, pb, pb,
											(int32_t) blockSize, 1, blockSize);

	/* Working pointer for the scratch buffer of state values */
	px = pb;

	/* Working pointer for scratch buffer of output values */
	pScratchOut = pScr2;

	/* Loop over the blockSize */
	blkCnt = blockSize;

	while(blkCnt > 0u)
	{
		/* Perform Multiply-Accumulate */
		in = *pScratchOut + ((q31_t) * px++ * coeff);
		*pScratchOut++ = in;

		/* Decrement the loop counter */
		blkCnt--;
	}

  /* All the output values are in pScratchOut buffer.
     Convert them into 1.15 format, saturate and store in the destination buffer. */
  /* Loop over the blockSize. */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    *pOut++ = (q7_t) __SSAT_8(*pScr2++ >> 7);

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

#endif /*   #ifndef CSKY_MATH_NO_SIMD */

}

/**
 * @} end of FIR_Sparse group
 */
