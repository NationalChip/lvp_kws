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
 * @file     csky_pid_q31.c
 * @brief    Q31 PID Control function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
 * @addtogroup PID
 * @{
 */

/**
 * @brief  Process function for the Q31 PID Control.
 * @param[in,out] S  points to an instance of the Q31 PID Control structure
 * @param[in]     in  input sample to process
 * @return out processed output sample.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using an internal 64-bit accumulator.
 * The accumulator has a 2.62 format and maintains full precision of the intermediate multiplication results but provides only a single guard bit.
 * Thus, if the accumulator result overflows it wraps around rather than clip.
 * In order to avoid overflows completely the input signal must be scaled down by 2 bits as there are four additions.
 * After all multiply-accumulates are performed, the 2.62 accumulator is truncated to 1.32 format and then saturated to 1.31 format.
 */
q31_t csky_pid_q31(
csky_pid_instance_q31 * S,
q31_t in)
{
  q63_t acc;
  q31_t out;

  /* acc = A0 * x[n]  */
  acc = (q63_t) S->A0 * in;

  /* acc += A1 * x[n-1] */
  acc += (q63_t) S->A1 * S->state[0];

  /* acc += A2 * x[n-2]  */
  acc += (q63_t) S->A2 * S->state[1];

  /* convert output to 1.31 format to add y[n-1] */
  out = (q31_t) (acc >> 31u);

  /* out += y[n-1] */
  out += S->state[2];

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
