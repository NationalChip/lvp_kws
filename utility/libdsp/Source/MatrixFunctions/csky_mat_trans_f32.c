/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_mat_trans_f32.c
* Description:  Floating-point matrix transpose
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
* This file comes from arm_mat_trans_f32.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_mat_trans_f32.c
 * @brief    Floating-point matrix transpose.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

/**
 * @defgroup MatrixTrans Matrix Transpose
 *
 * Tranposes a matrix.
 * Transposing an <code>M x N</code> matrix flips it around the center diagonal and results in an <code>N x M</code> matrix.
 * \image html MatrixTranspose.gif "Transpose of a 3 x 3 matrix"
 */

#include "csky_math.h"

/**
 * @ingroup groupMatrix
 */

/**
 * @addtogroup MatrixTrans
 * @{
 */

/**
  * @brief Floating-point matrix transpose.
  * @param[in]  *pSrc points to the input matrix
  * @param[out] *pDst points to the output matrix
  * @return 	The function returns either  <code>CSKY_MATH_SIZE_MISMATCH</code>
  * or <code>CSKY_MATH_SUCCESS</code> based on the outcome of size checking.
  */


csky_status csky_mat_trans_f32(
  const csky_matrix_instance_f32 * pSrc,
  csky_matrix_instance_f32 * pDst)
{
  float32_t *pIn = pSrc->pData;                  /* input data matrix pointer */
  float32_t *pOut = pDst->pData;                 /* output data matrix pointer */
  float32_t *px;                                 /* Temporary output data matrix pointer */
  uint16_t nRows = pSrc->numRows;                /* number of rows */
  uint16_t nColumns = pSrc->numCols;             /* number of columns */

#ifndef CSKY_MATH_NO_SIMD


  uint16_t blkCnt, i = 0u, row = nRows;          /* loop counters */
  csky_status status;                             /* status of matrix transpose  */


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
      /* Loop Unrolling */
      blkCnt = nColumns >> 2;

      /* The pointer px is set to starting address of the column being processed */
      px = pOut + i;

      /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
       ** a second loop below computes the remaining 1 to 3 samples. */
      while(blkCnt > 0u)        /* column loop */
      {
        /* Read and store the input element in the destination */
        *px = *pIn++;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += nRows;

        /* Read and store the input element in the destination */
        *px = *pIn++;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += nRows;

        /* Read and store the input element in the destination */
        *px = *pIn++;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += nRows;

        /* Read and store the input element in the destination */
        *px = *pIn++;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += nRows;

        /* Decrement the column loop counter */
        blkCnt--;
      }

      /* Perform matrix transpose for last 3 samples here. */
      blkCnt = nColumns % 0x4u;

      while(blkCnt > 0u)
      {
        /* Read and store the input element in the destination */
        *px = *pIn++;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += nRows;

        /* Decrement the column loop counter */
        blkCnt--;
      }

#else


  uint16_t col, i = 0u, row = nRows;             /* loop counters */
  csky_status status;                             /* status of matrix transpose  */


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
      /* The pointer px is set to starting address of the column being processed */
      px = pOut + i;

      /* Initialize column loop counter */
      col = nColumns;

      while(col > 0u)
      {
        /* Read and store the input element in the destination */
        *px = *pIn++;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += nRows;

        /* Decrement the column loop counter */
        col--;
      }

#endif /* #ifndef CSKY_MATH_NO_SIMD */

      i++;

      /* Decrement the row loop counter */
      row--;

    } while(row > 0u);          /* row loop end  */

    /* Set status as CSKY_MATH_SUCCESS */
    status = CSKY_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);
}

/**
 * @} end of MatrixTrans group
 */
