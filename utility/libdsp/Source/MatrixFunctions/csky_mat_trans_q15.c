/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_mat_trans_q15.c
* Description:  Q15 matrix transpose
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
* This file comes from arm_mat_trans_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_mat_trans_q15.c
 * @brief    Q15 matrix transpose.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupMatrix
 */

/**
 * @addtogroup MatrixTrans
 * @{
 */

/*
 * @brief Q15 matrix transpose.
 * @param[in]  *pSrc points to the input matrix
 * @param[out] *pDst points to the output matrix
 * @return 	The function returns either  <code>CSKY_MATH_SIZE_MISMATCH</code>
 * or <code>CSKY_MATH_SUCCESS</code> based on the outcome of size checking.
 */

csky_status csky_mat_trans_q15(
  const csky_matrix_instance_q15 * pSrc,
  csky_matrix_instance_q15 * pDst)
{
  q15_t *pSrcA = pSrc->pData;                    /* input data matrix pointer */
  q15_t *pOut = pDst->pData;                     /* output data matrix pointer */
  uint16_t nRows = pSrc->numRows;                /* number of nRows */
  uint16_t nColumns = pSrc->numCols;             /* number of nColumns */
  uint16_t col, row = nRows, i = 0u;             /* row and column loop counters */
  csky_status status;                             /* status of matrix transpose */

#ifndef CSKY_MATH_NO_SIMD

#ifndef UNALIGNED_SUPPORT_DISABLE

  q31_t in;                                      /* variable to hold temporary output  */

#else

  q15_t in;

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE	*/

#ifdef CSKY_MATH_MATRIX_CHECK


  /* Check for matrix mismatch condition */
  if((pSrc->numRows != pDst->numCols) || (pSrc->numCols != pDst->numRows))
  {
    /* Set status as CSKY_MATH_SIZE_MISMATCH */
    status = CSKY_MATH_SIZE_MISMATCH;
  }
  else
#endif /*      #ifdef CSKY_MATH_MATRIX_CHECK    */

  {
    /* Matrix transpose by exchanging the rows with columns */
    /* row loop     */
    do
    {

      /* Apply loop unrolling and exchange the columns with row elements */
      col = nColumns >> 2u;

      /* The pointer pOut is set to starting address of the column being processed */
      pOut = pDst->pData + i;

      /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
       ** a second loop below computes the remaining 1 to 3 samples. */
      while(col > 0u)
      {
#ifndef UNALIGNED_SUPPORT_DISABLE

        /* Read two elements from the row */
        in = *__SIMD32(pSrcA)++;

        /* Unpack and store one element in the destination */
#ifndef CSKY_MATH_BIG_ENDIAN

        *pOut = (q15_t) in;

#else

        *pOut = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#endif /*    #ifndef CSKY_MATH_BIG_ENDIAN    */

        /* Update the pointer pOut to point to the next row of the transposed matrix */
        pOut += nRows;

        /* Unpack and store the second element in the destination */

#ifndef CSKY_MATH_BIG_ENDIAN

        *pOut = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#else

        *pOut = (q15_t) in;

#endif /*    #ifndef CSKY_MATH_BIG_ENDIAN    */

        /* Update the pointer pOut to point to the next row of the transposed matrix */
        pOut += nRows;

        /* Read two elements from the row */
#ifndef CSKY_MATH_BIG_ENDIAN

        in = *__SIMD32(pSrcA)++;

#else

        in = *__SIMD32(pSrcA)++;

#endif /*    #ifndef CSKY_MATH_BIG_ENDIAN    */

        /* Unpack and store one element in the destination */
#ifndef CSKY_MATH_BIG_ENDIAN

        *pOut = (q15_t) in;

#else

        *pOut = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#endif /*    #ifndef CSKY_MATH_BIG_ENDIAN    */

        /* Update the pointer pOut to point to the next row of the transposed matrix */
        pOut += nRows;

        /* Unpack and store the second element in the destination */
#ifndef CSKY_MATH_BIG_ENDIAN

        *pOut = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);

#else

        *pOut = (q15_t) in;

#endif /*    #ifndef CSKY_MATH_BIG_ENDIAN    */

#else
        /* Read one element from the row */
        in = *pSrcA++;

        /* Store one element in the destination */
        *pOut = in;

        /* Update the pointer px to point to the next row of the transposed matrix */
        pOut += nRows;

        /* Read one element from the row */
        in = *pSrcA++;

        /* Store one element in the destination */
        *pOut = in;

        /* Update the pointer px to point to the next row of the transposed matrix */
        pOut += nRows;

        /* Read one element from the row */
        in = *pSrcA++;

        /* Store one element in the destination */
        *pOut = in;

        /* Update the pointer px to point to the next row of the transposed matrix */
        pOut += nRows;

        /* Read one element from the row */
        in = *pSrcA++;

        /* Store one element in the destination */
        *pOut = in;

#endif	/*	#ifndef UNALIGNED_SUPPORT_DISABLE	*/

        /* Update the pointer pOut to point to the next row of the transposed matrix */
        pOut += nRows;

        /* Decrement the column loop counter */
        col--;
      }

      /* Perform matrix transpose for last 3 samples here. */
      col = nColumns % 0x4u;

#else


#ifdef CSKY_MATH_MATRIX_CHECK

  /* Check for matrix mismatch condition */
  if((pSrc->numRows != pDst->numCols) || (pSrc->numCols != pDst->numRows))
  {
    /* Set status as CSKY_MATH_SIZE_MISMATCH */
    status = CSKY_MATH_SIZE_MISMATCH;
  }
  else
#endif /*    #ifdef CSKY_MATH_MATRIX_CHECK    */

  {
    /* Matrix transpose by exchanging the rows with columns */
    /* row loop     */
    do
    {
      /* The pointer pOut is set to starting address of the column being processed */
      pOut = pDst->pData + i;

      /* Initialize column loop counter */
      col = nColumns;

#endif /* #ifndef CSKY_MATH_NO_SIMD */

      while(col > 0u)
      {
        /* Read and store the input element in the destination */
        *pOut = *pSrcA++;

        /* Update the pointer pOut to point to the next row of the transposed matrix */
        pOut += nRows;

        /* Decrement the column loop counter */
        col--;
      }

      i++;

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
 * @} end of MatrixTrans group
 */
