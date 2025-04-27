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
* This file comes from arm_fir_lattice_init_q15.c. It is renamed by replacing arm with csky_vdsp2.
*
*/

/******************************************************************************
 * @file     csky_vdsp2_fir_lattice_init_q15.c
 * @brief    Q15 FIR Lattice filter initialization function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_vdsp2_math.h"

/**
 * @ingroup groupFilters
 */

/**
 * @addtogroup FIR_Lattice
 * @{
 */

  /**
   * @brief Initialization function for the Q15 FIR lattice filter.
   * @param[in] *S points to an instance of the Q15 FIR lattice structure.
   * @param[in] numStages  number of filter stages.
   * @param[in] *pCoeffs points to the coefficient buffer.  The array is of length numStages.
   * @param[in] *pState points to the state buffer.  The array is of length numStages.
   * @return none.
   */

void csky_vdsp2_fir_lattice_init_q15(
  csky_vdsp2_fir_lattice_instance_q15 * S,
  uint16_t numStages,
  q15_t * pCoeffs,
  q15_t * pState)
{
  /* Assign filter taps */
  S->numStages = numStages;

  /* Assign coefficient pointer */
  S->pCoeffs = pCoeffs;

  /* Clear state buffer and size is always numStages */
  memset(pState, 0, (numStages) * sizeof(q15_t));

  /* Assign state pointer */
  S->pState = pState;

}

/**
 * @} end of FIR_Lattice group
 */
