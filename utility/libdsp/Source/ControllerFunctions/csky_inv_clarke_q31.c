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
 * @file     csky_inv_clarke_q31.c
 * @brief    Q31 inverse clarke transform function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @addtogroup inv_clarke
 * @{
 */

/**
 * @brief  Inverse Clarke transform for Q31 version
 * @param[in]  Ialpha  input two-phase orthogonal vector axis alpha
 * @param[in]  Ibeta   input two-phase orthogonal vector axis beta
 * @param[out] pIa     points to output three-phase coordinate <code>a</code>
 * @param[out] pIb     points to output three-phase coordinate <code>b</code>
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using an internal 32-bit accumulator.
 * The accumulator maintains 1.31 format by truncating lower 31 bits of the intermediate multiplication in 2.62 format.
 * There is saturation on the subtraction, hence there is no risk of overflow.
 */
void csky_inv_clarke_q31(
q31_t Ialpha,
q31_t Ibeta,
q31_t * pIa,
q31_t * pIb)
{
  q31_t product1, product2;                    /* Temporary variables used to store intermediate results */

  /* Calculating pIa from Ialpha by equation pIa = Ialpha */
  *pIa = Ialpha;

  /* Intermediate product is calculated by (1/(2*sqrt(3)) * Ia) */
  product1 = (q31_t) (((q63_t) (Ialpha) * (0x40000000)) >> 31);

  /* Intermediate product is calculated by (1/sqrt(3) * pIb) */
  product2 = (q31_t) (((q63_t) (Ibeta) * (0x6ED9EBA1)) >> 31);

  /* pIb is calculated by subtracting the products */
  *pIb = __QSUB(product2, product1);
}

/**
 * @} end of inv_clarke group
 */
