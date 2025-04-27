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
 * @file     csky_pow_f64.c
 * @brief    Fast x^y calculation for float64_t values.
 * @version  V1.0
 * @date     15. Nov 2017
 ******************************************************************************/
#include"csky_math.h"
#include"csky_exp_math.h"
#include"csky_common_tables.h"

static float64_t _exp1(float64_t x, float64_t xx, float64_t error);
static float64_t _log1(float64_t x, float64_t *delta, float64_t *error);
static q31_t     _checkint(float64_t x);

/**
 * @ingroup DSPlibyv
 */

/**
 * @defgroup Pow Exponential Function
 *
 * This group includes only one function, which is any value based Exponentional
 * function. The common methods to compute the functions are ploynomial approximation
 * and table loop-up. And, it is a double floating similarly version, whose exponention
 * part and mantissa part are seperated.
 *
 */

 /**
 * @addtogroup Pow
 * @{
 */

/**
 * @brief Fast approximation to the exponential function for float64_t-point data.
 * @param[in]  arg_in_x     the mantissa of the base of pow.
 * @param[in]  arg_exp_in_x the exponention of the base of pow.
 * @param[in]  arg_in_y     the mantissa of the exponention of pow.
 * @param[in]  arg_exp_in_y the exponention of the exponention of pow.
 * @param[out] arg_exp_out  the exponention of the exponention of result.
 * @return the mantissa of the result.
 *
 * The implementation is based on the transform x^y = e^(y*log(x)).
 *
 * <b>The steps used are:</b>
 *  -# First, calculation of the nearest value of log(x) with series expansion
 *     or table look up depending on the range of x.
 *  -# Then, compute of the nearest value of e^(y*log(x)) with the function
 *     e^(x+xx), in which, defferent methods are used based on the range of x.
 *  -# Finally, some special cases are processed.
 */

q31_t csky_dsp_lib_pow_int32(
  q31_t arg_in_x,
  q15_t arg_exp_in_x,
  q31_t arg_in_y,
  q15_t arg_exp_in_y,
  q31_t *arg_exp_out)
{
  float64_t z,a,aa,error, t,a1,a2,y1,y2, x, y;
  mynumber u,v;
  q31_t k, high_half, low_half, exp;
  q31_t qx,qy;

  x              = 1 << arg_exp_in_x;
  arg_exp_in_x  += 1023;
  high_half      = ((q31_t)(arg_exp_in_x) << 20) | (arg_in_x >> 12);
  low_half       = (arg_in_x & 0xFFF) << 20;
  u.i[LOW_HALF]  = high_half;
  u.i[HIGH_HALF] = low_half;
  u.x           -= x;

  y              = 1 << arg_exp_in_x;
  arg_exp_in_y  += 1023;
  high_half      = ((q31_t)(arg_exp_in_y) << 20) | (arg_in_y >> 12);
  low_half       = (arg_in_y & 0xFFF) << 20;
  v.i[LOW_HALF]  = high_half;
  v.i[HIGH_HALF] = low_half;
  u.x           -= y;

  x = u.x;
  y = v.x;
  if (v.i[LOW_HALF] == 0)
  {   /* of y */
      qx = u.i[HIGH_HALF] & hugeint;
      /* Is x a NaN?  */
      if (y == 0) return 1.0;
      if (((qx == 0x7ff00000) && (u.i[LOW_HALF] != 0)) || (qx > 0x7ff00000))
          return x;
      if (y == 1.0) return x;
      if (y == 2.0) return x*x;
      if (y == -1.0) return 1.0/x;
  }
  /* else */
  if(((u.i[HIGH_HALF] > 0 && u.i[HIGH_HALF] < 0x7ff00000)||/* x>0 and not x->0 */
     (u.i[HIGH_HALF] == 0 && u.i[LOW_HALF] != 0)) &&
    /* 2^-1023< x<= 2^-1023 * 0x1.0000ffffffff */
    (v.i[HIGH_HALF]&hugeint) < 0x4ff00000)
  { /* if y<-1 or y>1 */
      float64_t retval;

      /* Avoid internal underflow for tiny y.  The exact value of y does
      not matter if |y| <= 2**-64.  */
      if (ABS (y) < 0x1p-64)
      {
          y = y < 0 ? -0x1p-64 : 0x1p-64;
      }
      z      = _log1(x, &aa, &error);  /* x^y  =e^(y log (x)) */
      t      = y * CN;
      y1     = t - (t - y);
      y2     = y - y1;
      t      = z * CN;
      a1     = t - (t - z);
      a2     = (z - a1) + aa;
      a      = y1 * a1;
      aa     = y2 * a1 + y * a2;
      a1     = a + aa;
      a2     = (a - a1) + aa;
      error  = error * ABS(y);
      u.x    = _exp1(a1, a2, 1.9e16*error);

      high_half   = u.i[HIGH_HALF];
      low_half    = u.i[LOW_HALF];
      exp         = (high_half >> 20) - 1022;
      retval      = (((high_half & 0xFFFFF) << 12) | (low_half >> 20)) >> 1;
      arg_exp_out = &exp;
      return retval;
  }

  if (x == 0)
  {
      if (((v.i[HIGH_HALF] & hugeint) == 0x7ff00000 && v.i[LOW_HALF] != 0)
      || (v.i[HIGH_HALF] & hugeint) > 0x7ff00000) /* NaN */
          return y;

      if (ABS(y) > 1.0e20) return (y>0)?0 : INF.x;

      k = _checkint(y);
      if (k == -1)
      {
          return y < 0 ? 1.0/x : x;
      }
      else
      {
          return y < 0 ? INF.x : 0.0;/* return 0 */
      }
  }

  qx = u.i[HIGH_HALF]&hugeint;  /* no sign */
  qy = v.i[HIGH_HALF]&hugeint;  /* no sign */

  if (qx >= 0x7ff00000 && (qx > 0x7ff00000 || u.i[LOW_HALF] != 0)) /* NaN */
      return x;
  if (qy >= 0x7ff00000 && (qy > 0x7ff00000 || v.i[LOW_HALF] != 0)) /* NaN */
      return x == 1.0 ? 1.0 : y;

  /* if x<0 */
  if (u.i[HIGH_HALF] < 0)
  {
      k = _checkint(y);
      if (k == 0)
      {
          if (qy == 0x7ff00000)
          {
              if (x == -1.0) return 1.0;
              else if (x > -1.0) return v.i[HIGH_HALF] < 0 ? INF.x : 0.0;
              else return v.i[HIGH_HALF] < 0 ? 0.0 : INF.x;
          }
          else if (qx == 0x7ff00000)
              return y < 0 ? 0.0 : INF.x;
          return NaN.x;   /* y not integer and x<0 */
      }
      else if (qx == 0x7ff00000)
      {
          if (k < 0)
              return y < 0 ? nZERO.x : nINF.x;
          else
              return y < 0 ? 0.0 : INF.x;
      }
      return (k == 1) ? csky_pow_f64(-x,y):-csky_pow_f64(-x,y); /* if y even or odd */
  }
  /* x>0 */

  if (qx == 0x7ff00000) /* x= 2^-0x3ff */
      return y > 0 ? x : 0;

  if (qy > 0x45f00000 && qy < 0x7ff00000)
  {
      if (x == 1.0) return 1.0;
      if (y > 0) return (x > 1.0) ? huge*huge : tiny*tiny;
      if (y < 0) return (x < 1.0) ? huge*huge : tiny*tiny;
  }

  if (x == 1.0) return 1.0;
  if (y > 0) return (x > 1.0) ? INF.x : 0;
  if (y < 0) return (x < 1.0) ? INF.x : 0;
  return 0;     /* unreachable, to make the compiler happy */
}
/**
 * @} end of Pow group
 */


/**
 * @brief Fast approximation to the logarithm function for float64_t-point data.
 * @param[in]  x input value in double.
 * @param[out] *delta output value in double.
 * @param[out] *error output value in double.
 * @return log(x).
 */
static float64_t _log1(float64_t x, float64_t *delta, float64_t *error)
{
  q31_t i,j,m;
  float64_t uu,vv,eps,nx,e,e1,e2,t,t1,t2,res,add=0;
  mynumber u,v;

  mynumber two52  = {{0x00000000, 0x43300000}}; /* 2**52         */

  u.x    = x;
  m      = u.i[HIGH_HALF];
  *error = 0;
  *delta = 0;

  if (m < 0x00100000) /* 1<x<2^-1007 */
  {
      x   = x*t52.x;
      add = -52.0;
      u.x = x;
      m   = u.i[HIGH_HALF];
  }

  if ((m&0x000fffff) < 0x0006a09e)/* 0x3fff6a09e = sqrt(2)*/
  {
      u.i[HIGH_HALF]    = (m&0x000fffff) | 0x3ff00000;
      two52.i[LOW_HALF] = (m >> 20);
  }
  else
  {
      u.i[HIGH_HALF]    = (m&0x000fffff) | 0x3fe00000;
      two52.i[LOW_HALF] = (m >> 20) + 1;
  }

  v.x = u.x + bigu.x;
  uu  = v.x - bigu.x;
  i   = (v.i[LOW_HALF]&0x000003ff) << 2;
  if (two52.i[LOW_HALF] == 1023)  /* nx = 0 */
  {
      if (i > 1192 && i < 1208)   /* |x-1| < 1.5*2**-10  */
      {
          t      = x - 1.0;
          t1     = (t + 5.0e6) - 5.0e6; /*round the low word*/
          t2     = t - t1;
          e1     = t - 0.5*t1*t1;
          e2     = t*t*t*(r3+t*(r4+t*(r5+t*(r6+t*(r7+t*r8)))))-0.5*t2*(t+t1);
          res    = e1+e2;
          *error = 1.0e-21*ABS(t);
          *delta = (e1-res)+e2;
          return res;
      }/* |x-1| < 1.5*2**-10  */
      else
      {
          v.x    = u.x*(ui.x[i]+ui.x[i+1])+bigv.x;
          vv     = v.x-bigv.x;
          j      = v.i[LOW_HALF]&0x0007ffff;
          j      = j+j+j;
          eps    = u.x - uu*vv;
          e1     = eps*ui.x[i];
          e2     = eps*(ui.x[i+1]+vj.x[j]*(ui.x[i]+ui.x[i+1]));
          e      = e1+e2;
          e2     =  ((e1-e)+e2);
          t      = ui.x[i+2]+vj.x[j+1];
          t1     = t+e;
          t2     = (((t-t1)+e)+(ui.x[i+3]+vj.x[j+2]))+e2+e*e*(p2+e*(p3+e*p4));
          res    = t1+t2;
          *error = 1.0e-24;
          *delta = (t1-res)+t2;
          return res;
      }
  }/* nx = 0 */
  else /* nx != 0 */
  {
      eps    = u.x - uu;
      nx     = (two52.x - two52e.x)+add;
      e1     = eps*ui.x[i];
      e2     = eps*ui.x[i+1];
      e      = e1 + e2;
      e2     = (e1-e)+e2;
      t      = nx*ln2a.x+ui.x[i+2];
      t1     = t + e;
      t2     = (((t-t1)+e)+nx*ln2b.x+ui.x[i+3]+e2)+e*e*(q2+e*(q3+e*(q4+e*(q5+e*q6))));
      res    = t1+t2;
      *error = 1.0e-21;
      *delta = (t1-res)+t2;
      return res;
  }   /* nx != 0 */
}

/**
 * @brief Fast approximation to the exponential function for float64_t-point data.
 * @param[in]  x  input value in double.
 * @param[in]  xx input value in double.
 * @param[out] *error output value in double.
 * @return e^(x+xx).
 */

static float64_t _exp1(float64_t x, float64_t xx, float64_t error)
{
  float64_t bexp, t, eps, del, base, y, al, bet, res, rem, cor;
  mynumber junk1, junk2, binexp  = {{0,0}};
  q31_t i,j,m,n,ex;

  junk1.x = x;
  m       = junk1.i[HIGH_HALF];
  n       = m&hugeint;                 /* no sign */

  if (n > smallint && n < bigint)
  {
      y       = x*log2e.x + three51.x;
      bexp    = y - three51.x;      /*  multiply the result by 2**bexp        */
      junk1.x = y;
      eps     = bexp*ln_two2.x;      /* x = bexp*ln(2) + t - eps               */
      t       = x - bexp*ln_two1.x;
      y       = t + three33.x;
      base    = y - three33.x;      /* t rounded to a multiple of 2**-18      */
      junk2.x = y;
      del     = (t - base) + (xx-eps);    /*  x = bexp*ln(2) + base + del      */
      eps     = del + del*del*(ep3.x*del + ep2.x);
      binexp.i[HIGH_HALF] =(junk1.i[LOW_HALF]+1023)<<20;
      i       = ((junk2.i[LOW_HALF]>>8)&0xfffffffe)+356;
      j       = (junk2.i[LOW_HALF]&511)<<1;
      al      = coar.x[i]*fine.x[j];
      bet     = (coar.x[i]*fine.x[j+1] + coar.x[i+1]*fine.x[j]) + coar.x[i+1]*fine.x[j+1];
      rem     = (bet + bet*eps)+al*eps;
      res     = al + rem;
      cor     = (al - res) + rem;
      return res*binexp.x;
  }

  if (n <= smallint) return 1.0; /*  if x->0 e^x=1 */

  if (n >= badint)
  {
      if (n > infint) return(zero/zero);    /* x is NaN,  return invalid */
      if (n < infint) return ( (x>0) ? (huge*huge) : (tiny*tiny) );
      /* x is finite,  cause either overflow or underflow  */
      if (junk1.i[LOW_HALF] != 0)  return NaN.x;        /*  x is NaN  */
      return ((x>0) ? INF.x : zero );   /* |x| = INF;  return either INF or 0 */
  }

  y       = x*log2e.x + three51.x;
  bexp    = y - three51.x;
  junk1.x = y;
  eps     = bexp*ln_two2.x;
  t       = x - bexp*ln_two1.x;
  y       = t + three33.x;
  base    = y - three33.x;
  junk2.x = y;
  del     = (t - base) + (xx-eps);
  eps     = del + del*del*(ep3.x*del + ep2.x);
  i       = ((junk2.i[LOW_HALF]>>8)&0xfffffffe)+356;
  j       = (junk2.i[LOW_HALF]&511)<<1;
  al      = coar.x[i]*fine.x[j];
  bet     = (coar.x[i]*fine.x[j+1] + coar.x[i+1]*fine.x[j]) + coar.x[i+1]*fine.x[j+1];
  rem     = (bet + bet*eps)+al*eps;
  res     = al + rem;
  cor     = (al - res) + rem;
  if (m>>31)
  {
      ex = junk1.i[LOW_HALF];
      if (res < 1.0)
      {
          res += res;
          cor += cor;
          ex  -= 1;
      }
      if (ex >=-1022)
      {
          binexp.i[HIGH_HALF] = (1023+ex) << 20;
          return res*binexp.x;
      }
      ex   = -(1022+ex);
      binexp.i[HIGH_HALF] = (1023-ex)<<20;
      res *= binexp.x;
      cor *= binexp.x;
      eps  = 1.00000000001+(error+err_1)*binexp.x;
      t    = 1.0+res;
      y    = ((1.0-t)+res)+cor;
      res  = t+y;
      cor  = (t-res)+y;
      binexp.i[HIGH_HALF] = 0x00100000;
      return (res-1.0)*binexp.x;
  }
  else
  {
      binexp.i[HIGH_HALF] =(junk1.i[LOW_HALF]+767)<<20;
      return res*binexp.x*t256.x;
  }
}

/**
 * @brief a function to check the double value is intger, odd, even or neither.
 * @param[in]  x  input value in double.
 * @return flag.
 */
static q31_t _checkint(float64_t x)
{
  union
  {
      q31_t i[2];
      float64_t x;
  } u;
  q31_t k,m,n;

  u.x = x;
  m   = u.i[HIGH_HALF]&hugeint;     /* no sign */
  if (m >= 0x7ff00000) return 0;    /* x is +/-inf or NaN  */
  if (m >= 0x43400000) return 1;    /* |x| >= 2**53 */
  if (m < 0x40000000) return 0;     /* |x| < 2,  can not be 0 or 1  */
  n   = u.i[LOW_HALF];
  k   = (m>>20)-1023;               /*  1 <= k <= 52   */
  if (k == 52) return (n&1)? -1:1;  /* odd or even*/
  if (k > 20)
  {
      if (n << (k-20)) return 0;    /* if not integer */
      return (n << (k-21)) ? -1 : 1;
  }
  if (n) return 0;                  /*if not integer*/
  if (k == 20) return (m&1) ? -1:1;
  if (m << (k+12)) return 0;
  return (m << (k+11)) ? -1:1;
}
