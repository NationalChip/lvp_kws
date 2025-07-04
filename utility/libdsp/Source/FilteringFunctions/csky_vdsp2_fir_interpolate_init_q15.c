/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_fir_lattice_init_q15.c
* Description:  Q15 FIR Lattice filter initialization function
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
* This file comes from arm_fir_interpolate_init_q15.c. It is renamed by replacing arm with csky_vdsp2.
*
*/

/******************************************************************************
 * @file     csky_vdsp2_fir_interpolate_init_q15.c
 * @brief    Q15 FIR interpolator initialization function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_vdsp2_math.h"

/**
 * @ingroup groupFilters
 */

/**
 * @addtogroup FIR_Interpolate
 * @{
 */

/**
 * @brief  Initialization function for the Q15 FIR interpolator.
 * @param[in,out] *S        points to an instance of the Q15 FIR interpolator structure.
 * @param[in]     L         upsample factor.
 * @param[in]     numTaps   number of filter coefficients in the filter.
 * @param[in]     *pCoeffs  points to the filter coefficient buffer.
 * @param[in]     *pState   points to the state buffer.
 * @param[in]     blockSize number of input samples to process per call.
 * @return        The function returns CSKY_MATH_SUCCESS if initialization was successful or CSKY_MATH_LENGTH_ERROR if
 * the filter length <code>numTaps</code> is not a multiple of the interpolation factor <code>L</code>.
 *
 * <b>Description:</b>
 * \par
 * <code>pCoeffs</code> points to the array of filter coefficients stored in time reversed order:
 * <pre>
 *    {b[numTaps-1], b[numTaps-2], b[numTaps-2], ..., b[1], b[0]}
 * </pre>
 * The length of the filter <code>numTaps</code> must be a multiple of the interpolation factor <code>L</code>.
 * \par
 * <code>pState</code> points to the array of state variables.
 * <code>pState</code> is of length <code>(numTaps/L)+blockSize-1</code> words
 * where <code>blockSize</code> is the number of input samples processed by each call to <code>csky_vdsp2_fir_interpolate_q15()</code>.
 */

csky_vdsp2_status csky_vdsp2_fir_interpolate_init_q15(
  csky_vdsp2_fir_interpolate_instance_q15 * S,
  uint8_t L,
  uint16_t numTaps,
  q15_t * pCoeffs,
  q15_t * pState,
  uint32_t blockSize)
{
  csky_vdsp2_status status;

  /* The filter length must be a multiple of the interpolation factor */
  if((numTaps % L) != 0u)
  {
    /* Set status as CSKY_MATH_LENGTH_ERROR */
    status = CSKY_VDSP2_MATH_LENGTH_ERROR;
  }
  else
  {

    /* Assign coefficient pointer */
    S->pCoeffs = pCoeffs;

    /* Assign Interpolation factor */
    S->L = L;

    /* Assign polyPhaseLength */
    S->phaseLength = numTaps / L;

    /* Clear state buffer and size of buffer is always phaseLength + blockSize - 1 */
    memset(pState, 0,
           (blockSize + ((uint32_t) S->phaseLength - 1u)) * sizeof(q15_t));

    /* Assign state pointer */
    S->pState = pState;

    status = CSKY_VDSP2_MATH_SUCCESS;
  }

  return (status);

}

 /**
  * @} end of FIR_Interpolate group
  */
