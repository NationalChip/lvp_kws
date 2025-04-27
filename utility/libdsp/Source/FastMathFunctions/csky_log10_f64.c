/*
 * Copyright (C) 2016-2019 C-SKY Limited. All rights reserved.
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
 */

/******************************************************************************
 * @file     csky_log10_f64.c
 * @brief    Fast log10 calculation for float64_t values.
 * @version  V1.0
 * @date     19. Oct 2017
 ******************************************************************************/
#include"csky_math.h"
#include"csky_exp_math.h"
#include"csky_common_tables.h"

/**
 * @ingroup groupFastMath
 */

 /**
 * @addtogroup log
 * @{
 */

/**
 * @brief  Fast approximation to the 10 base logarithm function for float64_t
 * -point data.
 * @param[in] x input value in double.
 * @return log10(x).
 *
 * The implementation is based on base transform from 10 to e.
 *
 * <b>The steps used are:</b>
 *  -# Processing of the special cases.
 *  -# The base transform is used to compute the result of log10(x).
 *
 */

float64_t csky_log10_f64(float64_t x)
{
  float64_t y, z;
  q31_t i, k, hx;
  uint32_t lx;

  EXTRACT_WORDS (hx, lx, x);

  k = 0;
  if (hx < 0x00100000)
  {     /* x < 2**-1022  */
      if (((hx & 0x7fffffff) | lx) == 0)
          return nINF.x;    /* log(+-0)=-inf */
      if (hx < 0)
          return NaN.x;     /* log(-#) = NaN */
      k -= 54;
      x *= two54_d;         /* subnormal number, scale up x */
      GET_HIGH_WORD (hx, x);
  }
  if(hx >= 0x7ff00000)
      return x + x;

  k += (hx >> 20) - 1023;
  i = ((uint32_t) k & 0x80000000) >> 31;
  hx = (hx & 0x000fffff) | ((0x3ff - i) << 20);
  y = (float64_t) (k + i);
  SET_HIGH_WORD (x, hx);
  z = y * log10_2lo + ivln10 *csky_log_f64(x);
  return z + y * log10_2hi;
}
/**
 *@}
 */ // end of log group
