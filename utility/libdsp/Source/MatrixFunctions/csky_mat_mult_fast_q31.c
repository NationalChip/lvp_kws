/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_mat_mult_fast_q31.c
* Description:  Q31 matrix multiplication (fast variant)
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
* This file comes from arm_mat_mult_fast_q31.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_mat_mult_fast_q31.c
 * @brief    Q31 matrix multiplication (fast variant).
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupMatrix
 */

/**
 * @addtogroup MatrixMult
 * @{
 */

/**
 * @param[in]       *pSrcA points to the first input matrix structure
 * @param[in]       *pSrcB points to the second input matrix structure
 * @param[out]      *pDst points to output matrix structure
 * @return     		The function returns either
 * <code>CSKY_MATH_SIZE_MISMATCH</code> or <code>CSKY_MATH_SUCCESS</code> based on the outcome of size checking.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * The difference between the function csky_mat_mult_q31() and this fast variant is that
 * the fast variant use a 32-bit rather than a 64-bit accumulator.
 * The result of each 1.31 x 1.31 multiplication is truncated to
 * 2.30 format. These intermediate results are accumulated in a 32-bit register in 2.30
 * format. Finally, the accumulator is saturated and converted to a 1.31 result.
 *
 * \par
 * The fast version has the same overflow behavior as the standard version but provides
 * less precision since it discards the low 32 bits of each multiplication result.
 * In order to avoid overflows completely the input signals must be scaled down.
 * Scale down one of the input matrices by log2(numColsA) bits to
 * avoid overflows, as a total of numColsA additions are computed internally for each
 * output element.
 *
 * \par
 * See <code>csky_mat_mult_q31()</code> for a slower implementation of this function
 * which uses 64-bit accumulation to provide higher precision.
 */

csky_status csky_mat_mult_fast_q31(
  const csky_matrix_instance_q31 * pSrcA,
  const csky_matrix_instance_q31 * pSrcB,
  csky_matrix_instance_q31 * pDst)
{
  q31_t *pIn1 = pSrcA->pData;                    /* input data matrix pointer A */
  q31_t *pIn2 = pSrcB->pData;                    /* input data matrix pointer B */
  q31_t *pInA = pSrcA->pData;                    /* input data matrix pointer A */
//  q31_t *pSrcB = pSrcB->pData;                    /* input data matrix pointer B */
  q31_t *pOut = pDst->pData;                     /* output data matrix pointer */
  q31_t *px;                                     /* Temporary output data matrix pointer */
  q31_t sum;                                     /* Accumulator */
  uint16_t numRowsA = pSrcA->numRows;            /* number of rows of input matrix A    */
  uint16_t numColsB = pSrcB->numCols;            /* number of columns of input matrix B */
  uint16_t numColsA = pSrcA->numCols;            /* number of columns of input matrix A */
  uint16_t col, i = 0u, j, row = numRowsA, colCnt;      /* loop counters */
  csky_status status;                             /* status of matrix multiplication */
  q31_t inA1, inA2, inA3, inA4, inB1, inB2, inB3, inB4;

#ifdef CSKY_MATH_MATRIX_CHECK


  /* Check for matrix mismatch condition */
  if((pSrcA->numCols != pSrcB->numRows) ||
     (pSrcA->numRows != pDst->numRows) || (pSrcB->numCols != pDst->numCols))
  {
    /* Set status as CSKY_MATH_SIZE_MISMATCH */
    status = CSKY_MATH_SIZE_MISMATCH;
  }
  else
#endif /*      #ifdef CSKY_MATH_MATRIX_CHECK    */

  {
    /* The following loop performs the dot-product of each row in pSrcA with each column in pSrcB */
    /* row loop */
    do
    {
      /* Output pointer is set to starting address of the row being processed */
      px = pOut + i;

      /* For every row wise process, the column loop counter is to be initiated */
      col = numColsB;

      /* For every row wise process, the pIn2 pointer is set
       ** to the starting address of the pSrcB data */
      pIn2 = pSrcB->pData;

      j = 0u;

      /* column loop */
      do
      {
        /* Set the variable sum, that acts as accumulator, to zero */
        sum = 0;

        /* Initiate the pointer pIn1 to point to the starting address of pInA */
        pIn1 = pInA;

        /* Apply loop unrolling and compute 4 MACs simultaneously. */
        colCnt = numColsA >> 2;


        /* matrix multiplication */
        while(colCnt > 0u)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */
          /* Perform the multiply-accumulates */
          inB1 = *pIn2;
          pIn2 += numColsB;

          inA1 = pIn1[0];
          inA2 = pIn1[1];

          inB2 = *pIn2;
          pIn2 += numColsB;

          inB3 = *pIn2;
          pIn2 += numColsB;

          sum = (q31_t) ((((q63_t) sum << 32) + ((q63_t) inA1 * inB1)) >> 32);
          sum = (q31_t) ((((q63_t) sum << 32) + ((q63_t) inA2 * inB2)) >> 32);

          inA3 = pIn1[2];
          inA4 = pIn1[3];

          inB4 = *pIn2;
          pIn2 += numColsB;

          sum = (q31_t) ((((q63_t) sum << 32) + ((q63_t) inA3 * inB3)) >> 32);
          sum = (q31_t) ((((q63_t) sum << 32) + ((q63_t) inA4 * inB4)) >> 32);

          pIn1 += 4u;

          /* Decrement the loop counter */
          colCnt--;
        }

        /* If the columns of pSrcA is not a multiple of 4, compute any remaining output samples here.
         ** No loop unrolling is used. */
        colCnt = numColsA % 0x4u;

        while(colCnt > 0u)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */
          /* Perform the multiply-accumulates */
          sum = (q31_t) ((((q63_t) sum << 32) +
                          ((q63_t) * pIn1++ * (*pIn2))) >> 32);
          pIn2 += numColsB;

          /* Decrement the loop counter */
          colCnt--;
        }

        /* Convert the result from 2.30 to 1.31 format and store in destination buffer */
        *px++ = sum << 1;

        /* Update the pointer pIn2 to point to the  starting address of the next column */
        j++;
        pIn2 = pSrcB->pData + j;

        /* Decrement the column loop counter */
        col--;

      } while(col > 0u);

      /* Update the pointer pInA to point to the  starting address of the next row */
      i = i + numColsB;
      pInA = pInA + numColsA;

      /* Decrement the row loop counter */
      row--;

    } while(row > 0u);

    /* set status as CSKY_MATH_SUCCESS */
    status = CSKY_MATH_SUCCESS;
  }
  /* Return to application */
  return (status);
}

/**
 * @} end of MatrixMult group
 */
