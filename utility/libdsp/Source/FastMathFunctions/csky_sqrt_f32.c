/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        arm_sqrt_f32.c
* Description:  Floating-point square root function
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
* This file comes from arm_sqrt_f32.c. It is renamed by replacing arm with csky.
*
*/

/******************************************************************************
 * @file     csky_sqrt_f32.c
 * @brief    f32 square root function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"
#include "csky_common_tables.h"


/**
 * @ingroup groupFastMath
 */

/**
 * @defgroup SQRT Square Root
 *
 * Computes the square root of a number.
 * There are separate functions for Q15, Q31, and floating-point data types.
 * When a CPU with FPU is used, the instruction <code>fsqrts</code> is used to
 * compute the result, while a CPU without FPU, Newton-Raphson algorithm is used.
 * For Newton-Raphson algorithm, this is an iterative algorithm of the form:
 * <pre>
 *      x1 = x0 - f(x0)/f'(x0)
 * </pre>
 * where <code>x1</code> is the current estimate,
 * <code>x0</code> is the previous estimate, and
 * <code>f'(x0)</code> is the derivative of <code>f()</code> evaluated at <code>x0</code>.
 * For the square root function, the algorithm reduces to:
 * <pre>
 *     x0 = in/2                         [initial guess]
 *     x1 = 1/2 * ( x0 + in / x0)        [each iteration]
 * </pre>
 */

/**
 * @addtogroup SQRT
 * @{
 */

/**
 * @brief  Floating-point square root function.
 * @param[in]  in    input value.
 * @param[out] pOut  square root of input value.
 * @return The function returns CSKY_MATH_SUCCESS if input value is positive value or CSKY_MATH_ARGUMENT_ERROR if
 * <code>in</code> is negative value and returns zero output for negative values.
 */
#if 1
csky_status csky_sqrt_f32(
float32_t in,
float32_t * pOut)
{
	float x = in;
	float xhalf = 0.5f * in;
	int i = *(int *)&x;

	if (in >= 0.0f)
	{
#ifdef __FPU_PRESENT
		__ASM volatile("fsqrts %0, %1\n\t":"=v"(x),"=v"(in):"0"(x),"1"(in));
		*pOut = x;
#else
		i = 0x5f375a86 - (i >> 1);
		x = *(float *)&i;
		x = x * (1.5f - xhalf * x * x);
		x = x * (1.5f - xhalf * x * x);
		x = x * (1.5f - xhalf * x * x);

		*pOut = x * in;
#endif
		return (CSKY_MATH_SUCCESS);
	}
	else
	{
		*pOut = 0.0f;
		return (CSKY_MATH_ARGUMENT_ERROR);
	}
}
#else
csky_status csky_sqrt_f32(
float32_t in,
float32_t * pOut)
{
  if(in >= 0.0f)
  {
    *pOut = (double)sqrt((double)in);

    return (CSKY_MATH_SUCCESS);
  }
  else
  {
    *pOut = 0.0f;
    return (CSKY_MATH_ARGUMENT_ERROR);
  }
}
#endif
/**
 * @} end of SQRT group
 */
