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
 * @file     csky_pid_f32.c
 * @brief    Floating-point PID Control function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

/**
  * @ingroup groupController
  */

/**
 * @defgroup PID PID Motor Control
 *
 * A Proportional Integral Derivative (PID) controller is a generic feedback control
 * loop mechanism widely used in industrial control systems.
 * A PID controller is the most commonly used type of feedback controller.
 *
 * This set of functions implements (PID) controllers
 * for Q15, Q31, and floating-point data types.  The functions operate on a single sample
 * of data and each call to the function returns a single processed value.
 * <code>S</code> points to an instance of the PID control data structure.  <code>in</code>
 * is the input sample value. The functions return the output value.
 *
 * \par Algorithm:
 * <pre>
 *    y[n] = y[n-1] + A0 * x[n] + A1 * x[n-1] + A2 * x[n-2]
 *    A0 = Kp + Ki + Kd
 *    A1 = (-Kp ) - (2 * Kd )
 *    A2 = Kd  </pre>
 *
 * \par
 * where \c Kp is proportional constant, \c Ki is Integral constant and \c Kd is Derivative constant
 *
 * \par
 * \image html PID.gif "Proportional Integral Derivative Controller"
 *
 * \par
 * The PID controller calculates an "error" value as the difference between
 * the measured output and the reference input.
 * The controller attempts to minimize the error by adjusting the process control inputs.
 * The proportional value determines the reaction to the current error,
 * the integral value determines the reaction based on the sum of recent errors,
 * and the derivative value determines the reaction based on the rate at which the error has been changing.
 *
 * \par Instance Structure
 * The Gains A0, A1, A2 and state variables for a PID controller are stored together in an instance data structure.
 * A separate instance structure must be defined for each PID Controller.
 * There are separate instance structure declarations for each of the 3 supported data types.
 *
 * \par Reset Functions
 * There is also an associated reset function for each data type which clears the state array.
 *
 * \par Initialization Functions
 * There is also an associated initialization function for each data type.
 * The initialization function performs the following operations:
 * - Initializes the Gains A0, A1, A2 from Kp,Ki, Kd gains.
 * - Zeros out the values in the state buffer.
 *
 * \par
 * Instance structure cannot be placed into a const data section and it is recommended to use the initialization function.
 *
 * \par Fixed-Point Behavior
 * Care must be taken when using the fixed-point versions of the PID Controller functions.
 * In particular, the overflow and saturation behavior of the accumulator used in each function must be considered.
 * Refer to the function specific documentation below for usage guidelines.
 */

/**
 * @addtogroup PID
 * @{
 */

/**
 * @brief  Process function for the floating-point PID Control.
 * @param[in,out] S   is an instance of the floating-point PID Control structure
 * @param[in]     in  input sample to process
 * @return out processed output sample.
 */
float32_t csky_pid_f32(
csky_pid_instance_f32 * S,
float32_t in)
{
  float32_t out;

  /* y[n] = y[n-1] + A0 * x[n] + A1 * x[n-1] + A2 * x[n-2]  */
  out = (S->A0 * in) +
    (S->A1 * S->state[0]) + (S->A2 * S->state[1]) + (S->state[2]);

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
