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
 * @file     csky_pid_reset_q15.c
 * @brief    Q15 PID Control reset function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"

 /**
 * @addtogroup PID
 * @{
 */

/**
* @brief  Reset function for the Q15 PID Control.
* @param[in] *S		Instance pointer of PID control data structure.
* @return none.
* \par Description:
* The function resets the state buffer to zeros.
*/
void csky_pid_reset_q15(
  csky_pid_instance_q15 * S)
{
  /* Reset state to zero, The size will be always 3 samples */
  memset(S->state, 0, 3u * sizeof(q15_t));
}

/**
 * @} end of PID group
 */
