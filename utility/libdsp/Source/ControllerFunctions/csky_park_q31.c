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
 * @file     csky_park_q31.c
 * @brief    Q31 Park transform function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @addtogroup park
 * @{
 */
/**
 * @brief  Park transform for Q31 version
 * @param[in]  Ialpha  input two-phase vector coordinate alpha
 * @param[in]  Ibeta   input two-phase vector coordinate beta
 * @param[out] pId     points to output rotor reference frame d
 * @param[out] pIq     points to output rotor reference frame q
 * @param[in]  sinVal  sine value of rotation angle theta
 * @param[in]  cosVal  cosine value of rotation angle theta
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using an internal 32-bit accumulator.
 * The accumulator maintains 1.31 format by truncating lower 31 bits of the intermediate multiplication in 2.62 format.
 * There is saturation on the addition and subtraction, hence there is no risk of overflow.
 */

void csky_park_q31(
q31_t Ialpha,
q31_t Ibeta,
q31_t * pId,
q31_t * pIq,
q31_t sinVal,
q31_t cosVal)
{
  q31_t product1, product2;                    /* Temporary variables used to store intermediate results */
  q31_t product3, product4;                    /* Temporary variables used to store intermediate results */
  /* Intermediate product is calculated by (Ialpha * cosVal) */
  product1 = (q31_t) (((q63_t) (Ialpha) * (cosVal)) >> 31);
  /* Intermediate product is calculated by (Ibeta * sinVal) */
  product2 = (q31_t) (((q63_t) (Ibeta) * (sinVal)) >> 31);
  /* Intermediate product is calculated by (Ialpha * sinVal) */
  product3 = (q31_t) (((q63_t) (Ialpha) * (sinVal)) >> 31);
  /* Intermediate product is calculated by (Ibeta * cosVal) */
  product4 = (q31_t) (((q63_t) (Ibeta) * (cosVal)) >> 31);
  /* Calculate pId by adding the two intermediate products 1 and 2 */
  *pId = __QADD(product1, product2);
  /* Calculate pIq by subtracting the two intermediate products 3 from 4 */
  *pIq = __QSUB(product4, product3);
}

/**
 * @} end of park group
 */
