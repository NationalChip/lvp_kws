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
 * @file     csky_exp_f64.c
 * @brief    Fast e^x calculation for float64_t values.
 * @version  V1.0
 * @date     25. Oct 2017
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
 * @brief  Fast approximation to the exponential function based on e for float64_t
 * -point_t data.
 * @param[in] x input value in double.
 * @return e^x.
 *
 * The implementation is based on table lookup using 712 coarse values together
 * with 1024 fine values which are both in double format.
 *
 * <b>The steps used are:</b>
 *  -# First, compute the result with table lookup when smallint < x < bigint.
 *  -# Then, process the special case namely x > badint or x < smallint.
 *  -# Finally, compute the result with table loopup when bigint < x < badint.
 */

float64_t csky_exp_f64(float64_t x)
{
  float64_t bexp, t, eps, del, base, y, al, bet, res, rem, cor;
  mynumber junk1, junk2, binexp  = {{0,0}};
  q31_t i,j,m,n,ex;
  float64_t retval;

  junk1.x = x;
  m       = junk1.i[HIGH_HALF];
  n       = m & hugeint;

  if(n > smallint && n < bigint)
  {
      y       = x*log2e.x + three51.x;
      bexp    = y - three51.x;       /* multiply the result by 2**bexp    */
      junk1.x = y;
      eps     = bexp*ln_two2.x;      /* x = bexp*ln(2) + t - eps          */
      t       = x - bexp*ln_two1.x;
      y       = t + three33.x;
      base    = y - three33.x;       /* t rounded to a multiple of 2**-18 */
      junk2.x = y;
      del     = (t - base) - eps;    /* x = bexp*ln(2) + base + del       */
      eps     = del + del*del*(ep3.x*del + ep2.x);
      binexp.i[HIGH_HALF] =(junk1.i[LOW_HALF]+1023)<<20;
      i       = ((junk2.i[LOW_HALF]>>8)&0xfffffffe)+356;
      j       = (junk2.i[LOW_HALF]&511)<<1;
      al      = coar.x[i]*fine.x[j];
      bet     = (coar.x[i]*fine.x[j+1] + coar.x[i+1]*fine.x[j]) + coar.x[i+1]*fine.x[j+1];
      rem     = (bet + bet*eps)+al*eps;
      res     = al + rem;
      cor     = (al - res) + rem;

      retval = res*binexp.x;
      goto ret;
  }

  if (n <= smallint)
  {
      retval = 1.0;
      goto ret;
  }

  if (n >= badint)
  {
      if (n > infint)
      {
          retval = x+x;
          goto ret;
      }/* x is NaN */
      if (n < infint)
      {
          retval = (x>0) ? (huge*huge) : (tiny*tiny);
          goto ret;
      }
      /* x is finite,  cause either overflow or underflow  */
      if (junk1.i[LOW_HALF] != 0)
      {
          retval = x+x;
          goto ret;
      } /*  x is NaN  */
      retval = (x>0) ? INF.x:zero; /* |x| = inf;  return either inf or 0 */
      goto ret;
  }

  y       = x*log2e.x + three51.x;
  bexp    = y - three51.x;
  junk1.x = y;
  eps     = bexp*ln_two2.x;
  t       = x - bexp*ln_two1.x;
  y       = t + three33.x;
  base    = y - three33.x;
  junk2.x = y;
  del     = (t - base) - eps;
  eps     = del + del*del*(ep3.x*del + ep2.x);
  i       = ((junk2.i[LOW_HALF]>>8)&0xfffffffe)+356;
  j       = (junk2.i[LOW_HALF]&511)<<1;
  al      = coar.x[i]*fine.x[j];
  bet     = (coar.x[i]*fine.x[j+1] + coar.x[i+1]*fine.x[j]) + coar.x[i+1]*fine.x[j+1];
  rem     = (bet + bet*eps)+al*eps;
  res     = al + rem;
  cor     = (al - res) + rem;
  if (m >> 31)
  {
      ex = junk1.i[LOW_HALF];
      if (res < 1.0)
      {
          res += res;
          cor += cor;
          ex  -= 1;
      }
      if (ex >= -1022)
      {
          binexp.i[HIGH_HALF] = (1023+ex)<<20;

          retval = res*binexp.x;
          goto ret;
      }

      ex   = -(1022+ex);
      binexp.i[HIGH_HALF] = (1023-ex)<<20;
      res *= binexp.x;
      cor *= binexp.x;
      eps  = 1.0000000001+err_0*binexp.x;
      t    = 1.0+res;
      y    = ((1.0-t)+res)+cor;
      res  = t+y;
      cor  = (t-res)+y;

      binexp.i[HIGH_HALF] = 0x00100000;
      retval = (res-1.0)*binexp.x;
      goto ret;
  }
  else
  {
      binexp.i[HIGH_HALF] =(junk1.i[LOW_HALF]+767)<<20;

      retval = res*binexp.x*t256.x;
      goto ret;
  }
  ret:
  return retval;
}
/**
 *@}
 */ // end of ExponFun group
