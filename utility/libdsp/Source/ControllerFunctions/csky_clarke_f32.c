/******************************************************************************
 * @file     arm_math.h
 * @brief    Public header file for CMSIS DSP Library
 * version   V1.7.0
 * @date     18.March 2019
******************************************************************************/
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
*-------------------------------------------------------------------- */
/*
* Copyright (C) 2016-2020 T-HEAD Limited. All rights reserved.
*
* This file comes from arm_math.h.
*
*/

/******************************************************************************
 * @file     csky_clarke_f32.c
 * @brief    Floating-point clarke transform function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupController
 */

/**
 * @defgroup clarke Vector Clarke Transform
 * Forward Clarke transform converts the instantaneous stator phases into a two-coordinate time invariant vector.
 * Generally the Clarke transform uses three-phase currents <code>Ia, Ib and Ic</code> to calculate currents
 * in the two-phase orthogonal stator axis <code>Ialpha</code> and <code>Ibeta</code>.
 * When <code>Ialpha</code> is superposed with <code>Ia</code> as shown in the figure below
 * \image html clarke.gif Stator current space vector and its components in (a,b).
 * and <code>Ia + Ib + Ic = 0</code>, in this condition <code>Ialpha</code> and <code>Ibeta</code>
 * can be calculated using only <code>Ia</code> and <code>Ib</code>.
 *
 * The function operates on a single sample of data and each call to the function returns the processed output.
 * The library provides separate functions for Q31 and floating-point data types.
 * \par Algorithm
 * \image html clarkeFormula.gif
 * where <code>Ia</code> and <code>Ib</code> are the instantaneous stator phases and
 * <code>pIalpha</code> and <code>pIbeta</code> are the two coordinates of time invariant vector.
 * \par Fixed-Point Behavior
 * Care must be taken when using the Q31 version of the Clarke transform.
 * In particular, the overflow and saturation behavior of the accumulator used must be considered.
 * Refer to the function specific documentation below for usage guidelines.
 */

/**
 * @addtogroup clarke
 * @{
 */

/**
 *
 * @brief  Floating-point Clarke transform
 * @param[in]  Ia       input three-phase coordinate a
 * @param[in]  Ib       input three-phase coordinate b
 * @param[out] pIalpha  points to output two-phase orthogonal vector axis alpha
 * @param[out] pIbeta   points to output two-phase orthogonal vector axis beta
 */
void csky_clarke_f32(
float32_t Ia,
float32_t Ib,
float32_t * pIalpha,
float32_t * pIbeta)
{
  /* Calculate pIalpha using the equation, pIalpha = Ia */
  *pIalpha = Ia;

  /* Calculate pIbeta using the equation, pIbeta = (1/sqrt(3)) * Ia + (2/sqrt(3)) * Ib */
  *pIbeta = ((float32_t) 0.57735026919 * Ia + (float32_t) 1.15470053838 * Ib);
}

/**
 * @} end of clarke group
 */
