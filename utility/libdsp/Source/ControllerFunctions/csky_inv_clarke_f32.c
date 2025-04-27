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
 * @file     csky_inv_clarke_f32.c
 * @brief    Floating-point inverse clarke transform function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupController
 */
/**
 * @defgroup inv_clarke Vector Inverse Clarke Transform
 * Inverse Clarke transform converts the two-coordinate time invariant vector into instantaneous stator phases.
 *
 * The function operates on a single sample of data and each call to the function returns the processed output.
 * The library provides separate functions for Q31 and floating-point data types.
 * \par Algorithm
 * \image html clarkeInvFormula.gif
 * where <code>pIa</code> and <code>pIb</code> are the instantaneous stator phases and
 * <code>Ialpha</code> and <code>Ibeta</code> are the two coordinates of time invariant vector.
 * \par Fixed-Point Behavior
 * Care must be taken when using the Q31 version of the Clarke transform.
 * In particular, the overflow and saturation behavior of the accumulator used must be considered.
 * Refer to the function specific documentation below for usage guidelines.
 */

/**
 * @addtogroup inv_clarke
 * @{
 */

 /**
 * @brief  Floating-point Inverse Clarke transform
 * @param[in]  Ialpha  input two-phase orthogonal vector axis alpha
 * @param[in]  Ibeta   input two-phase orthogonal vector axis beta
 * @param[out] pIa     points to output three-phase coordinate <code>a</code>
 * @param[out] pIb     points to output three-phase coordinate <code>b</code>
 */
void csky_inv_clarke_f32(
float32_t Ialpha,
float32_t Ibeta,
float32_t * pIa,
float32_t * pIb)
{
  /* Calculating pIa from Ialpha by equation pIa = Ialpha */
  *pIa = Ialpha;

  /* Calculating pIb from Ialpha and Ibeta by equation pIb = -(1/2) * Ialpha + (sqrt(3)/2) * Ibeta */
  *pIb = -0.5f * Ialpha + 0.8660254039f * Ibeta;
}


/**
 * @} end of inv_clarke group
 */
