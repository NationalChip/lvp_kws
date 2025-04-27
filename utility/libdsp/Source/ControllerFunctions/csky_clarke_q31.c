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
 * @file     csky_clarke_q31.c
 * @brief    Q31 clarke transform function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @addtogroup clarke
 * @{
 */

/**
 * @brief  Clarke transform for Q31 version
 * @param[in]  Ia       input three-phase coordinate a
 * @param[in]  Ib       input three-phase coordinate b
 * @param[out] pIalpha  points to output two-phase orthogonal vector axis alpha
 * @param[out] pIbeta   points to output two-phase orthogonal vector axis beta
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using an internal 32-bit accumulator.
 * The accumulator maintains 1.31 format by truncating lower 31 bits of the intermediate multiplication in 2.62 format.
 * There is saturation on the addition, hence there is no risk of overflow.
 */
void csky_clarke_q31(
q31_t Ia,
q31_t Ib,
q31_t * pIalpha,
q31_t * pIbeta)
{
  q31_t product1, product2;                    /* Temporary variables used to store intermediate results */

  /* Calculating pIalpha from Ia by equation pIalpha = Ia */
  *pIalpha = Ia;

  /* Intermediate product is calculated by (1/(sqrt(3)) * Ia) */
  product1 = (q31_t) (((q63_t) Ia * 0x24F34E8B) >> 30);

  /* Intermediate product is calculated by (2/sqrt(3) * Ib) */
  product2 = (q31_t) (((q63_t) Ib * 0x49E69D16) >> 30);

  /* pIbeta is calculated by adding the intermediate products */
  *pIbeta = __QADD(product1, product2);
}


/**
 * @} end of clarke group
 */
