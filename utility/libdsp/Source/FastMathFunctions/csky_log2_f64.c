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
 * @file     csky_log2_f64.c
 * @brief    Log2 calculation for float64_t values.
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
 * @brief  Fast approximation to the 2 based logarithm function for float64_t
 * -point data.
 * @param[in] x input value in double.
 * @return log2(x).
 *
 * The implementation is based on the transform
 * <pre>
 * log2(x)  = log2^k*(1+f)       where sqrt(2)/2 < 1+f < sqrt(2)
 *          = k + log(1+f)/log2. And then
 * log(1+f) = log(1+s) - log(1-s), where s = f/(1+f)
 *          = 2s + 2/3 s**3 + 2/5 s**5 + .....,
 *          = 2s + s*R
 * where R(z) ~ Lg1*s +Lg2*s +Lg3*s +Lg4*s +Lg5*s  +Lg6*s  +Lg7*s
 * </pre>
 * <b>So the steps used are:</b>
 *  -# First, compute the value of k, while (1+f) should between sqrt(2)/2 and
 *     sqrt(2).
 *  -# Then,  compute the value of R.
 *  -# Finally, compute the log2(x), and the processing of special case.
 */

float64_t csky_log2_f64 (float64_t x)
{
  float64_t hfsq, f, s, z, R, w, t1, t2, dk;
  q31_t k, hx, i, j;
  uint32_t lx;

  EXTRACT_WORDS (hx, lx, x);

  k = 0;
  if (hx < 0x00100000)
  {                           /* x < 2**-1022  */
      if ((((hx & 0x7fffffff) | lx) == 0))
          return nINF.x;      /* log(+-0)=-inf */
      if ((hx < 0))
          return NaN.x;       /* log(-#) = NaN */
      k -= 54;
      x *= two54_d;           /* subnormal number, scale up x */
      GET_HIGH_WORD (hx, x);
  }
  if ((hx >= 0x7ff00000))
      return x + x;
  k  += (hx >> 20) - 1023;
  hx &= 0x000fffff;
  i   = (hx + 0x95f64) & 0x100000;
  SET_HIGH_WORD (x, hx | (i ^ 0x3ff00000)); /* normalize x or x/2 */
  k  += (i >> 20);
  dk  = (float64_t) k;
  f   = x - 1.0;
  if ((0x000fffff & (2 + hx)) < 3)
  {                           /* |f| < 2**-20 */
      if (f == zero)
      {
          return dk;
      }
      R = f * f * (0.5 - 0.33333333333333333 * f);
      return dk - (R - f) / ln2;
  }
  s  = f / (2.0 + f);
  z  = s * s;
  i  = hx - 0x6147a;
  w  = z * z;
  j  = 0x6b851 - hx;
  t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
  t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
  i |= j;
  R  = t2 + t1;
  if (i > 0)
  {
      hfsq = 0.5 * f * f;
      return dk - ((hfsq - (s * (hfsq + R))) - f) / ln2;
  }
  else
  {
      return dk - ((s * (f - R)) - f) / ln2;
  }
}
/**
 *@}
 */ // end of log group
