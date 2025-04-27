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
 * @file     csky_inv_park_f32.c
 * @brief    Floating-point inverse Park transform function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupController
 */
/**
 * @defgroup inv_park Vector Inverse Park transform
 * Inverse Park transform converts the input flux and torque components to two-coordinate vector.
 *
 * The function operates on a single sample of data and each call to the function returns the processed output.
 * The library provides separate functions for Q31 and floating-point data types.
 * \par Algorithm
 * \image html parkInvFormula.gif
 * where <code>pIalpha</code> and <code>pIbeta</code> are the stator vector components,
 * <code>Id</code> and <code>Iq</code> are rotor vector components and <code>cosVal</code> and <code>sinVal</code> are the
 * cosine and sine values of theta (rotor flux position).
 * \par Fixed-Point Behavior
 * Care must be taken when using the Q31 version of the Park transform.
 * In particular, the overflow and saturation behavior of the accumulator used must be considered.
 * Refer to the function specific documentation below for usage guidelines.
 */
/**
 * @addtogroup inv_park
 * @{
 */
 /**
 * @brief  Floating-point Inverse Park transform
 * @param[in]  Id       input coordinate of rotor reference frame d
 * @param[in]  Iq       input coordinate of rotor reference frame q
 * @param[out] pIalpha  points to output two-phase orthogonal vector axis alpha
 * @param[out] pIbeta   points to output two-phase orthogonal vector axis beta
 * @param[in]  sinVal   sine value of rotation angle theta
 * @param[in]  cosVal   cosine value of rotation angle theta
 */
void csky_inv_park_f32(
float32_t Id,
float32_t Iq,
float32_t * pIalpha,
float32_t * pIbeta,
float32_t sinVal,
float32_t cosVal)
{
  /* Calculate pIalpha using the equation, pIalpha = Id * cosVal - Iq * sinVal */
  *pIalpha = Id * cosVal - Iq * sinVal;
  /* Calculate pIbeta using the equation, pIbeta = Id * sinVal + Iq * cosVal */
  *pIbeta = Id * sinVal + Iq * cosVal;
}

/**
 * @} end of inv_park group
 */
