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
 * @file     csky_park_f32.c
 * @brief    Floating-point Park transform function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @ingroup groupController
 */
/**
 * @defgroup park Vector Park Transform
 *
 * Forward Park transform converts the input two-coordinate vector to flux and torque components.
 * The Park transform can be used to realize the transformation of the <code>Ialpha</code> and the <code>Ibeta</code> currents
 * from the stationary to the moving reference frame and control the spatial relationship between
 * the stator vector current and rotor flux vector.
 * If we consider the d axis aligned with the rotor flux, the diagram below shows the
 * current vector and the relationship from the two reference frames:
 * \image html park.gif "Stator current space vector and its component in (a,b) and in the d,q rotating reference frame"
 *
 * The function operates on a single sample of data and each call to the function returns the processed output.
 * The library provides separate functions for Q31 and floating-point data types.
 * \par Algorithm
 * \image html parkFormula.gif
 * where <code>Ialpha</code> and <code>Ibeta</code> are the stator vector components,
 * <code>pId</code> and <code>pIq</code> are rotor vector components and <code>cosVal</code> and <code>sinVal</code> are the
 * cosine and sine values of theta (rotor flux position).
 * \par Fixed-Point Behavior
 * Care must be taken when using the Q31 version of the Park transform.
 * In particular, the overflow and saturation behavior of the accumulator used must be considered.
 * Refer to the function specific documentation below for usage guidelines.
 */
/**
 * @addtogroup park
 * @{
 */
/**
 * @brief Floating-point Park transform
 * @param[in]  Ialpha  input two-phase vector coordinate alpha
 * @param[in]  Ibeta   input two-phase vector coordinate beta
 * @param[out] pId     points to output   rotor reference frame d
 * @param[out] pIq     points to output   rotor reference frame q
 * @param[in]  sinVal  sine value of rotation angle theta
 * @param[in]  cosVal  cosine value of rotation angle theta
 *
 * The function implements the forward Park transform.
 *
 */
void csky_park_f32(
float32_t Ialpha,
float32_t Ibeta,
float32_t * pId,
float32_t * pIq,
float32_t sinVal,
float32_t cosVal)
{
  /* Calculate pId using the equation, pId = Ialpha * cosVal + Ibeta * sinVal */
  *pId = Ialpha * cosVal + Ibeta * sinVal;
  /* Calculate pIq using the equation, pIq = - Ialpha * sinVal + Ibeta * cosVal */
  *pIq = -Ialpha * sinVal + Ibeta * cosVal;
}

/**
 * @} end of park group
 */
