/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_mat_init_q31.c
* Description:  Q31 matrix initialization
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
* This file comes from arm_mat_init_q31.c. It is renamed by replacing arm with csky_vdsp2.
*
*/

/******************************************************************************
 * @file     csky_vdsp2_mat_init_q31.c
 * @brief    Q31 matrix initialization.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/


#include "csky_vdsp2_math.h"

/**
 * @ingroup groupMatrix
 */

/**
 * @defgroup MatrixInit Matrix Initialization
 *
 */

/**
 * @addtogroup MatrixInit
 * @{
 */

  /**
   * @brief  Q31 matrix initialization.
   * @param[in,out] *S             points to an instance of the floating-point matrix structure.
   * @param[in]     nRows          number of rows in the matrix.
   * @param[in]     nColumns       number of columns in the matrix.
   * @param[in]     *pData	   points to the matrix data array.
   * @return        none
   */

void csky_vdsp2_mat_init_q31(
  csky_vdsp2_matrix_instance_q31 * S,
  uint16_t nRows,
  uint16_t nColumns,
  q31_t * pData)
{
  /* Assign Number of Rows */
  S->numRows = nRows;

  /* Assign Number of Columns */
  S->numCols = nColumns;

  /* Assign Data pointer */
  S->pData = pData;
}

/**
 * @} end of MatrixInit group
 */
