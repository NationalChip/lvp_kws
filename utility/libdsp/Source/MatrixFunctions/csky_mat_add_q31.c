/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_mat_add_q15.c
* Description:  Q31 matrix addition
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
* This file comes from arm_mat_add_q31.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_mat_add_q31.c
 * @brief    Q31 matrix addition.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupMatrix
 */

/**
 * @addtogroup MatrixAdd
 * @{
 */

/**
 * @brief Q31 matrix addition.
 * @param[in]       *pSrcA points to the first input matrix structure
 * @param[in]       *pSrcB points to the second input matrix structure
 * @param[out]      *pDst points to output matrix structure
 * @return     		The function returns either
 * <code>CSKY_MATH_SIZE_MISMATCH</code> or <code>CSKY_MATH_SUCCESS</code> based on the outcome of size checking.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q31 range [0x80000000 0x7FFFFFFF] will be saturated.
 */

csky_status csky_mat_add_q31(
  const csky_matrix_instance_q31 * pSrcA,
  const csky_matrix_instance_q31 * pSrcB,
  csky_matrix_instance_q31 * pDst)
{
  q31_t *pIn1 = pSrcA->pData;                    /* input data matrix pointer A */
  q31_t *pIn2 = pSrcB->pData;                    /* input data matrix pointer B */
  q31_t *pOut = pDst->pData;                     /* output data matrix pointer */
  q31_t inA1, inB1;                              /* temporary variables */

#ifndef CSKY_MATH_NO_SIMD

  q31_t inA2, inB2;                              /* temporary variables */
  q31_t out1, out2;                              /* temporary variables */

#endif //      #ifndef CSKY_MATH_NO_SIMD

  uint32_t numSamples;                           /* total number of elements in the matrix  */
  uint32_t blkCnt;                               /* loop counters */
  csky_status status;                             /* status of matrix addition */

#ifdef CSKY_MATH_MATRIX_CHECK
  /* Check for matrix mismatch condition */
  if((pSrcA->numRows != pSrcB->numRows) ||
     (pSrcA->numCols != pSrcB->numCols) ||
     (pSrcA->numRows != pDst->numRows) || (pSrcA->numCols != pDst->numCols))
  {
    /* Set status as CSKY_MATH_SIZE_MISMATCH */
    status = CSKY_MATH_SIZE_MISMATCH;
  }
  else
#endif
  {
    /* Total number of samples in the input matrix */
    numSamples = (uint32_t) pSrcA->numRows * pSrcA->numCols;

#ifndef CSKY_MATH_NO_SIMD


    /* Loop Unrolling */
    blkCnt = numSamples >> 2u;


    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while(blkCnt > 0u)
    {
      /* C(m,n) = A(m,n) + B(m,n) */
      /* Add, saturate and then store the results in the destination buffer. */
      /* Read values from source A */
      inA1 = pIn1[0];

      /* Read values from source B */
      inB1 = pIn2[0];

      /* Read values from source A */
      inA2 = pIn1[1];

      /* Add and saturate */
      out1 = __QADD(inA1, inB1);

      /* Read values from source B */
      inB2 = pIn2[1];

      /* Read values from source A */
      inA1 = pIn1[2];

      /* Add and saturate */
      out2 = __QADD(inA2, inB2);

      /* Read values from source B */
      inB1 = pIn2[2];

      /* Store result in destination */
      pOut[0] = out1;
      pOut[1] = out2;

      /* Read values from source A */
      inA2 = pIn1[3];

      /* Read values from source B */
      inB2 = pIn2[3];

      /* Add and saturate */
      out1 = __QADD(inA1, inB1);
      out2 = __QADD(inA2, inB2);

      /* Store result in destination */
      pOut[2] = out1;
      pOut[3] = out2;

      /* update pointers to process next sampels */
      pIn1 += 4u;
      pIn2 += 4u;
      pOut += 4u;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* If the numSamples is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = numSamples % 0x4u;

#else


    /* Initialize blkCnt with number of samples */
    blkCnt = numSamples;


#endif /* #ifndef CSKY_MATH_NO_SIMD */

    while(blkCnt > 0u)
    {
      /* C(m,n) = A(m,n) + B(m,n) */
      /* Add, saturate and then store the results in the destination buffer. */
      inA1 = *pIn1++;
      inB1 = *pIn2++;

      inA1 = __QADD(inA1, inB1);

      /* Decrement the loop counter */
      blkCnt--;

      *pOut++ = inA1;

    }

    /* set status as CSKY_MATH_SUCCESS */
    status = CSKY_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);
}

/**
 * @} end of MatrixAdd group
 */
