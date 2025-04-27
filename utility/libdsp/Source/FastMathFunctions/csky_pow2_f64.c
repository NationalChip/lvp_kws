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
 * @file     csky_pow2_f64.c
 * @brief    Fast 2^x calculation for float64_t values.
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
 * @addtogroup ExponFun
 * @{
 */

/**
 * @brief  Fast approximation to the 2 based exponential function for float64_t
 * -point data.
 * @param[in] x input value in double.
 * @return 2^x.
 *
 * The implementation is based on table lookup using 512 delta values combining
 * with 512 accurate values.
 *
 * <b>The steps used are:</b>
 *  -# processing of special cases.
 *  -# argument reduction. Choose integers ex, -256 <= t < 256, and some real
 *        -1/1024 <= x1 <= 1024 so that <pre> x = ex + t/512 + x1</pre>
 *  -# adjust for accurate table entry. Find e so that
 *        <pre> x = ex + t/512 + e + x2</pre>
 *        where -1e6 < e < 1e6, and (float64_t)(2^(t/512+e)) is accurate to one
 *        part in 2^-64.
 *  -# approximate 2^x2 - 1, using a fourth-degree polynomial, with maximum
 *        error in [-2^-10-2^-30,2^-10+2^-30] less than 10^-19.
 */

float64_t csky_pow2_f64 (float64_t x)
{
  static const float64_t himark = (float64_t) DBL_MAX_EXP;
  static const float64_t lomark = (float64_t) (DBL_MIN_EXP - DBL_MANT_DIG - 1);

  /* Check for usual case.  */
  if (x <  himark)
  {
      /* Exceptional cases:  */
      if (x < lomark)
      {
          if (isinf (x))
              /* e^-inf == 0, with no error.  */
              return 0;
          else
              /* Underflow */
              return TWOM1000 * TWOM1000;
      }

      static const float64_t THREEp42 = 13194139533312.0;
      q31_t tval, unsafe;
      float64_t rx, x22, result;
      union ieee754_double ex2_u, scale_u;


      /*First, calculate rx = ex + t/512.*/
      rx  = x + THREEp42;
      rx -= THREEp42;
      x  -= rx;  /* Compute x=x1. */
      /* Compute tval = (ex*512 + t)+256.
      Now, t = (tval mod 512)-256 and ex=tval/512  [that's mod, NOT %;
      and /-round-to-nearest not the usual c integer /].  */
      tval = (q31_t) (rx * 512.0 + 256.0);

      /* 'tval & 511' is the same as 'tval%512' except that it's always
      positive.
      Compute x = x2.  */
      x -= exp2_deltatable[tval & 511];

      /* Compute ex2 = 2^(t/512+e+ex).  */
      ex2_u.d = exp2_accuratetable[tval & 511];
      tval >>= 9;
      unsafe = abs(tval) >= -DBL_MIN_EXP - 1;
      ex2_u.ieee.exponent += tval >> unsafe;
      scale_u.d = 1.0;
      scale_u.ieee.exponent += tval - (tval >> unsafe);

      x22 = (((.0096181293647031180
      * x + .055504110254308625)
      * x + .240226506959100583)
      * x + .69314718055994495) * ex2_u.d;

      /*Return (2^x2-1) * 2^(t/512+e+ex) + 2^(t/512+e+ex).*/
      result = x22 * x + ex2_u.d;

      if(!unsafe)
          return result;
      else
          return result * scale_u.d;
  }
  else
      /* Return x, if x is a NaN or Inf; or overflow, otherwise.  */
      return TWO1023*x;
}
/**
 *@}
 */ // end of ExponFun group
