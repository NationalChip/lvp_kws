/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_lms_norm_init_q15.c
* Description:  Q15 NLMS filter initialization function
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
* This file comes from arm_lms_norm_init_q15.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_lms_norm_init_q15.c
 * @brief    Q15 NLMS initialization function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"
#include "csky_common_tables.h"

/**
 * @addtogroup LMS_NORM
 * @{
 */

  /**
   * @brief Initialization function for Q15 normalized LMS filter.
   * @param[in] *S points to an instance of the Q15 normalized LMS filter structure.
   * @param[in] numTaps  number of filter coefficients.
   * @param[in] *pCoeffs points to coefficient buffer.
   * @param[in] *pState points to state buffer.
   * @param[in] mu step size that controls filter coefficient updates.
   * @param[in] blockSize number of samples to process.
   * @param[in] postShift bit shift applied to coefficients.
   * @return none.
 *
 * <b>Description:</b>
 * \par
 * <code>pCoeffs</code> points to the array of filter coefficients stored in time reversed order:
 * <pre>
 *    {b[numTaps-1], b[numTaps-2], b[N-2], ..., b[1], b[0]}
 * </pre>
 * The initial filter coefficients serve as a starting point for the adaptive filter.
 * <code>pState</code> points to the array of state variables and size of array is
 * <code>numTaps+blockSize-1</code> samples, where <code>blockSize</code> is the number of input samples processed
 * by each call to <code>csky_lms_norm_q15()</code>.
 */

void csky_lms_norm_init_q15(
  csky_lms_norm_instance_q15 * S,
  uint16_t numTaps,
  q15_t * pCoeffs,
  q15_t * pState,
  q15_t mu,
  uint32_t blockSize,
  uint8_t postShift)
{
  /* Assign filter taps */
  S->numTaps = numTaps;

  /* Assign coefficient pointer */
  S->pCoeffs = pCoeffs;

  /* Clear state buffer and size is always blockSize + numTaps - 1 */
  memset(pState, 0, (numTaps + (blockSize - 1u)) * sizeof(q15_t));

  /* Assign post Shift value applied to coefficients */
  S->postShift = postShift;

  /* Assign state pointer */
  S->pState = pState;

  /* Assign Step size value */
  S->mu = mu;

  /* Initialize reciprocal pointer table */
  S->recipTable = (q15_t *) cskyRecipTableQ15;

  /* Initialise Energy to zero */
  S->energy = 0;

  /* Initialise x0 to zero */
  S->x0 = 0;

}

/**
 * @} end of LMS_NORM group
 */
