/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_mat_mult_fast_q15.c
* Description:  Q15 matrix multiplication (fast variant)
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
* This file comes from arm_mat_mult_fast_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_mat_mult_fast_q15.c
 * @brief    Q15 matrix multiplication (fast variant).
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
 * @param[in]		*pState points to the array for storing intermediate results
 * @return     		The function returns either
 * <code>CSKY_MATH_SIZE_MISMATCH</code> or <code>CSKY_MATH_SUCCESS</code> based on the outcome of size checking.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * The difference between the function csky_mat_mult_q15() and this fast variant is that
 * the fast variant use a 32-bit rather than a 64-bit accumulator.
 * The result of each 1.15 x 1.15 multiplication is truncated to
 * 2.30 format. These intermediate results are accumulated in a 32-bit register in 2.30
 * format. Finally, the accumulator is saturated and converted to a 1.15 result.
 *
 * \par
 * The fast version has the same overflow behavior as the standard version but provides
 * less precision since it discards the low 16 bits of each multiplication result.
 * In order to avoid overflows completely the input signals must be scaled down.
 * Scale down one of the input matrices by log2(numColsA) bits to
 * avoid overflows, as a total of numColsA additions are computed internally for each
 * output element.
 *
 * \par
 * See <code>csky_mat_mult_q15()</code> for a slower implementation of this function
 * which uses 64-bit accumulation to provide higher precision.
 */

csky_status csky_mat_mult_fast_q15(
  const csky_matrix_instance_q15 * pSrcA,
  const csky_matrix_instance_q15 * pSrcB,
  csky_matrix_instance_q15 * pDst)
{
  q31_t sum;                                     /* accumulator */

#if 0//ndef CSKY_MATH_NO_SIMD
  q15_t *pSrcBT;                                 /* input data matrix pointer for transpose */
  q15_t *pInA = pSrcA->pData;                    /* input data matrix pointer A of Q15 type */
  q15_t *pInB = pSrcB->pData;                    /* input data matrix pointer B of Q15 type */
  q15_t *px;                                     /* Temporary output data matrix pointer */
  uint16_t numRowsA = pSrcA->numRows;            /* number of rows of input matrix A    */
  uint16_t numColsB = pSrcB->numCols;            /* number of columns of input matrix B */
  uint16_t numColsA = pSrcA->numCols;            /* number of columns of input matrix A */
  uint16_t numRowsB = pSrcB->numRows;            /* number of rows of input matrix A    */
  uint16_t col, i = 0u, row = numRowsB, colCnt;  /* loop counters */
  csky_status status;                             /* status of matrix multiplication */

#ifndef UNALIGNED_SUPPORT_DISABLE

  q31_t in;                                      /* Temporary variable to hold the input value */
  q31_t inA1, inA2, inB1, inB2;

#else

  q15_t in;                                      /* Temporary variable to hold the input value */
  q15_t inA1, inA2, inB1, inB2;

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE	*/

  pSrcBT = malloc(2 * numColsB * numRowsB);
#ifdef CSKY_MATH_MATRIX_CHECK
  /* Check for matrix mismatch condition */
  if((pSrcA->numCols != pSrcB->numRows) ||
     (pSrcA->numRows != pDst->numRows) || (pSrcB->numCols != pDst->numCols))
  {
    /* Set status as CSKY_MATH_SIZE_MISMATCH */
    status = CSKY_MATH_SIZE_MISMATCH;
  }
  else
#endif
  {
    /* Matrix transpose */
    do
    {
      /* Apply loop unrolling and exchange the columns with row elements */
      col = numColsB >> 2;

      /* The pointer px is set to starting address of the column being processed */
      px = pSrcBT + i;

      /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
       ** a second loop below computes the remaining 1 to 3 samples. */
      while(col > 0u)
      {
#ifndef UNALIGNED_SUPPORT_DISABLE
        /* Read two elements from the row */
        in = *__SIMD32(pInB)++;

        /* Unpack and store one element in the destination */
#ifndef CSKY_MATH_BIG_ENDIAN

        *px = (q15_t) in;

#else

        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#endif /*    #ifndef CSKY_MATH_BIG_ENDIAN    */

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Unpack and store the second element in the destination */
#ifndef CSKY_MATH_BIG_ENDIAN

        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#else

        *px = (q15_t) in;

#endif /*    #ifndef CSKY_MATH_BIG_ENDIAN    */

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Read two elements from the row */
        in = *__SIMD32(pInB)++;

        /* Unpack and store one element in the destination */
#ifndef CSKY_MATH_BIG_ENDIAN

        *px = (q15_t) in;

#else

        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#endif /*    #ifndef CSKY_MATH_BIG_ENDIAN    */

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Unpack and store the second element in the destination */

#ifndef CSKY_MATH_BIG_ENDIAN

        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#else

        *px = (q15_t) in;

#endif /*    #ifndef CSKY_MATH_BIG_ENDIAN    */

#else

        /* Read one element from the row */
        in = *pInB++;

        /* Store one element in the destination */
        *px = in;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Read one element from the row */
        in = *pInB++;

        /* Store one element in the destination */
        *px = in;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Read one element from the row */
        in = *pInB++;

        /* Store one element in the destination */
        *px = in;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Read one element from the row */
        in = *pInB++;

        /* Store one element in the destination */
        *px = in;

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE	*/

		/* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Decrement the column loop counter */
        col--;
      }

      /* If the columns of pSrcB is not a multiple of 4, compute any remaining output samples here.
       ** No loop unrolling is used. */
      col = numColsB % 0x4u;

      while(col > 0u)
      {
        /* Read and store the input element in the destination */
        *px = *pInB++;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Decrement the column loop counter */
        col--;
      }

      i++;

      /* Decrement the row loop counter */
      row--;

    } while(row > 0u);

    /* Reset the variables for the usage in the following multiplication process */
    row = numRowsA;
    i = 0u;
    px = pDst->pData;

    /* The following loop performs the dot-product of each row in pSrcA with each column in pSrcB */
    /* row loop */
    do
    {
      /* For every row wise process, the column loop counter is to be initiated */
      col = numColsB;

      /* For every row wise process, the pIn2 pointer is set
       ** to the starting address of the transposed pSrcB data */
      pInB = pSrcBT;

      /* column loop */
      do
      {
        /* Set the variable sum, that acts as accumulator, to zero */
        sum = 0;

        /* Apply loop unrolling and compute 2 MACs simultaneously. */
        colCnt = numColsA >> 2;

        /* Initiate the pointer pIn1 to point to the starting address of the column being processed */
        pInA = pSrcA->pData + i;

        /* matrix multiplication */
        while(colCnt > 0u)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */
#ifndef UNALIGNED_SUPPORT_DISABLE

          inA1 = *__SIMD32(pInA)++;
          inB1 = *__SIMD32(pInB)++;
          inA2 = *__SIMD32(pInA)++;
          inB2 = *__SIMD32(pInB)++;

          sum = __SMLAD(inA1, inB1, sum);
          sum = __SMLAD(inA2, inB2, sum);

#else

          inA1 = *pInA++;
          inB1 = *pInB++;
          inA2 = *pInA++;
          sum += inA1 * inB1;
          inB2 = *pInB++;

          inA1 = *pInA++;
          inB1 = *pInB++;
          sum += inA2 * inB2;
          inA2 = *pInA++;
          inB2 = *pInB++;

          sum += inA1 * inB1;
          sum += inA2 * inB2;

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE	*/

          /* Decrement the loop counter */
          colCnt--;
        }

        /* process odd column samples */
        colCnt = numColsA % 0x4u;

        while(colCnt > 0u)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */
          sum += (q31_t) (*pInA++) * (*pInB++);

          colCnt--;
        }

        /* Saturate and store the result in the destination buffer */
        *px = (q15_t) (sum >> 15);
        px++;

        /* Decrement the column loop counter */
        col--;

      } while(col > 0u);

      i = i + numColsA;

      /* Decrement the row loop counter */
      row--;

    } while(row > 0u);

#else


  q15_t *pIn1 = pSrcA->pData;                    /* input data matrix pointer A */
  q15_t *pIn2 = pSrcB->pData;                    /* input data matrix pointer B */
  q15_t *pInA = pSrcA->pData;                    /* input data matrix pointer A of Q15 type */
  q15_t *pInB = pSrcB->pData;                    /* input data matrix pointer B of Q15 type */
  q15_t *pOut = pDst->pData;                     /* output data matrix pointer */
  q15_t *px;                                     /* Temporary output data matrix pointer */
  uint16_t numColsB = pSrcB->numCols;            /* number of columns of input matrix B */
  uint16_t numColsA = pSrcA->numCols;            /* number of columns of input matrix A */
  uint16_t numRowsA = pSrcA->numRows;            /* number of rows of input matrix A    */
  uint16_t col, i = 0u, row = numRowsA, colCnt;  /* loop counters */
  csky_status status;                             /* status of matrix multiplication */

#ifdef CSKY_MATH_MATRIX_CHECK

  /* Check for matrix mismatch condition */
  if((pSrcA->numCols != pSrcB->numRows) ||
     (pSrcA->numRows != pDst->numRows) || (pSrcB->numCols != pDst->numCols))
  {
    /* Set status as CSKY_MATH_SIZE_MISMATCH */
    status = CSKY_MATH_SIZE_MISMATCH;
  }
  else
#endif /*    #ifdef CSKY_MATH_MATRIX_CHECK    */

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

      /* column loop */
      do
      {
        /* Set the variable sum, that acts as accumulator, to zero */
        sum = 0;

        /* Initiate the pointer pIn1 to point to the starting address of pSrcA */
        pIn1 = pInA;

        /* Matrix A columns number of MAC operations are to be performed */
        colCnt = numColsA;

        /* matrix multiplication */
        while(colCnt > 0u)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */
          /* Perform the multiply-accumulates */
          sum += (q31_t) * pIn1++ * *pIn2;
          pIn2 += numColsB;

          /* Decrement the loop counter */
          colCnt--;
        }

        /* Convert the result from 34.30 to 1.15 format and store the saturated value in destination buffer */
        /* Saturate and store the result in the destination buffer */
        *px++ = (q15_t)((sum >> 15));

        /* Decrement the column loop counter */
        col--;

        /* Update the pointer pIn2 to point to the  starting address of the next column */
        pIn2 = pInB + (numColsB - col);

      } while(col > 0u);

      /* Update the pointer pSrcA to point to the  starting address of the next row */
      i = i + numColsB;
      pInA = pInA + numColsA;

      /* Decrement the row loop counter */
      row--;

    } while(row > 0u);

#endif
    /* set status as CSKY_MATH_SUCCESS */
    status = CSKY_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);
}

/**
 * @} end of MatrixMult group
 */
