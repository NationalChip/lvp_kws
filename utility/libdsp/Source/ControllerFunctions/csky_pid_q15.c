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
 * @file     csky_pid_q15.c
 * @brief    Q15 PID Control function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @addtogroup PID
 * @{
 */
/**
 * @brief  Process function for the Q15 PID Control.
 * @param[in,out] S   points to an instance of the Q15 PID Control structure
 * @param[in]     in  input sample to process
 * @return out processed output sample.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using a 64-bit internal accumulator.
 * Both Gains and state variables are represented in 1.15 format and multiplications yield a 2.30 result.
 * The 2.30 intermediate results are accumulated in a 64-bit accumulator in 34.30 format.
 * There is no risk of internal overflow with this approach and the full precision of intermediate multiplications is preserved.
 * After all additions have been performed, the accumulator is truncated to 34.15 format by discarding low 15 bits.
 * Lastly, the accumulator is saturated to yield a result in 1.15 format.
 */
q15_t csky_pid_q15(
csky_pid_instance_q15 * S,
q15_t in)
{
  q63_t acc;
  q15_t out;

  /* acc = A0 * x[n]  */
  acc = ((q31_t) S->A0) * in;

  /* acc += A1 * x[n-1] + A2 * x[n-2]  */
  acc += (q31_t) S->A1 * S->state[0];
  acc += (q31_t) S->A2 * S->state[1];

  /* acc += y[n-1] */
  acc += (q31_t) S->state[2] << 15;

  /* saturate the output */
  out = (q15_t) (__SSAT_16((acc >> 15)));

  /* Update state */
  S->state[1] = S->state[0];
  S->state[0] = in;
  S->state[2] = out;

  /* return to application */
  return (out);
}


/**
 * @} end of PID group
 */
