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
#include "csky_math.h"


/**
 * @addtogroup LinearInterpolate
 * @{
 */
/**
 *
 * @brief  Process function for the Q7 Linear Interpolation Function.
 * @param[in] pYData   pointer to Q7 Linear Interpolation table
 * @param[in] x        input sample to process
 * @param[in] nValues  number of table values
 * @return y processed output sample.
 *
 * \par
 * Input sample <code>x</code> is in 12.20 format which contains 12 bits for table index and 20 bits for fractional part.
 * This function can support maximum of table size 2^12.
 */
q7_t csky_linear_interp_q7(
q7_t * pYData,
q31_t x,
uint32_t nValues)
{
  q31_t y;                                     /* output */
  q7_t y0, y1;                                 /* Nearest output values */
  q31_t fract;                                 /* fractional part */
  uint32_t index;                              /* Index to read nearest output values */
  /* Input is in 12.20 format */
  /* 12 bits for the table index */
  /* Index value calculation */
  if (x < 0)
  {
    return (pYData[0]);
  }
  index = (x >> 20) & 0xfff;
  if(index >= (nValues - 1))
  {
    return (pYData[nValues - 1]);
  }
  else
  {
    /* 20 bits for the fractional part */
    /* fract is in 12.20 format */
    fract = (x & 0x000FFFFF);
    /* Read two nearest output values from the index and are in 1.7(q7) format */
    y0 = pYData[index];
    y1 = pYData[index + 1];
    /* Calculation of y0 * (1-fract ) and y is in 13.27(q27) format */
    y = ((y0 * (0xFFFFF - fract)));
    /* Calculation of y1 * fract + y0 * (1-fract) and y is in 13.27(q27) format */
    y += (y1 * fract);
    /* convert y to 1.7(q7) format */
    return (q7_t) (y >> 20);
   }
}
/**
 * @} end of LinearInterpolate group
 */
