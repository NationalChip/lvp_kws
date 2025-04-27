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
 * @file     csky_inv_park_q31.c
 * @brief    Q31 inverse Park transform function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @addtogroup inv_park
 * @{
 */
/**
 * @brief  Inverse Park transform for   Q31 version
 * @param[in]  Id       input coordinate of rotor reference frame d
 * @param[in]  Iq       input coordinate of rotor reference frame q
 * @param[out] pIalpha  points to output two-phase orthogonal vector axis alpha
 * @param[out] pIbeta   points to output two-phase orthogonal vector axis beta
 * @param[in]  sinVal   sine value of rotation angle theta
 * @param[in]  cosVal   cosine value of rotation angle theta
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using an internal 32-bit accumulator.
 * The accumulator maintains 1.31 format by truncating lower 31 bits of the intermediate multiplication in 2.62 format.
 * There is saturation on the addition, hence there is no risk of overflow.
 */
void csky_inv_park_q31(
q31_t Id,
q31_t Iq,
q31_t * pIalpha,
q31_t * pIbeta,
q31_t sinVal,
q31_t cosVal)
{
  q31_t product1, product2;                    /* Temporary variables used to store intermediate results */
  q31_t product3, product4;                    /* Temporary variables used to store intermediate results */
  /* Intermediate product is calculated by (Id * cosVal) */
  product1 = (q31_t) (((q63_t) (Id) * (cosVal)) >> 31);
  /* Intermediate product is calculated by (Iq * sinVal) */
  product2 = (q31_t) (((q63_t) (Iq) * (sinVal)) >> 31);
  /* Intermediate product is calculated by (Id * sinVal) */
  product3 = (q31_t) (((q63_t) (Id) * (sinVal)) >> 31);
  /* Intermediate product is calculated by (Iq * cosVal) */
  product4 = (q31_t) (((q63_t) (Iq) * (cosVal)) >> 31);
  /* Calculate pIalpha by using the two intermediate products 1 and 2 */
  *pIalpha = __QSUB(product1, product2);
  /* Calculate pIbeta by using the two intermediate products 3 and 4 */
  *pIbeta = __QADD(product4, product3);
}

/**
 * @} end of inv_park group
 */
