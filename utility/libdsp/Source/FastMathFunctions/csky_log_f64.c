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
 * @file     csky_log_f64.c
 * @brief    Fast log calculation for float64_t values.
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
 * @defgroup log Log Series Functions.
 *
 * Logarithm series functions includes log, log2 and log10, which are
 * represented e based, 2 based and 10 based logarithm function repectively.
 * As exponentional functions, the common methods to compute these functions
 * are ploynomial approximation and table loop-up. And, it is a double
 * floating version for the functions, say, the input and output are both double.
 */

 /**
 * @addtogroup log
 * @{
 */

/**
 * @brief  Fast approximation to the 2 based logarithm function for float64_t
 * -point data.
 * @param[in] x input value in double.
 * @return log(x).
 *
 * The implementation is based on range of abs(x-1), when abs(x-1) < 0.03, series
 * expansion is used, and when abs(x-1) > 0.03 series expansion combining with table
 * loop up is used.
 *
 * <b>The steps used are:</b>
 *  -# First, the processing of special cases.
 *  -# Then,  compute the result when abs(x-1) < 0.03.
 *  -# Finally, compute the result when abs(x-1) > 0.03.
 */

float64_t csky_log_f64 (float64_t x)
{
#define M 4
  static const q31_t pr[M] = {8, 10, 18, 32};
  q31_t i, j, n, ux, dx, p;
  float64_t dbl_n, u, p0, q, r0, w, nln2a, luai, lubi, lvaj, lvbj,
  sij, ssij, ttij, A, B, B0, y, y1, y2, polI, polII, sa, sb,
  t1, t2, t7, t8, t, ra, rb, ww,
  a0, aa0, s1, s2, ss2, s3, ss3, a1, aa1, a, aa, b, bb, c;
  float64_t t3, t4, t5, t6;
  mynumber num;
  mp_no mpx, mpy, mpy1, mpy2, mperr;


  /* Treating special values of x ( x<=0, x=INF, x=NaN etc.). */
  num.x = x;
  ux    = num.i[HIGH_HALF];
  dx    = num.i[LOW_HALF];
  n     = 0;
  if(ux < 0x0010000)
  {
      if(((ux & 0x7fffffff) | dx) == 0)
          return nINF.x;    /* return -INF */
      if (ux < 0)
          return NaN.x;     /* return NaN  */
      n    -= 54;
      x    *= two54.x;      /* scale x */
      num.x = x;
  }
  if(ux >= 0x7ff00000)
      return x + x;         /* INF or NaN  */

  /* Regular values of x */
  w = x - 1.0;
  if (ABS (w) > U03)
      goto case_03;

  /*--- Stage I, the case abs(x-1) < 0.03 */
  t8 = MHALF * w;
  EMULV (t8, w, a, aa, t1, t2, t3, t4, t5);
  EADD (w, a, b, bb);

  /* Evaluate polynomial II */
  polII  = b7.x + w * b8.x;
  polII  = b6.x + w * polII;
  polII  = b5.x + w * polII;
  polII  = b4.x + w * polII;
  polII  = b3.x + w * polII;
  polII  = b2.x + w * polII;
  polII  = b1.x + w * polII;
  polII  = b0.x + w * polII;
  polII *= w * w * w;
  c      = (aa + bb) + polII;

  /* End stage I, case abs(x-1) < 0.03 */
  if ((y = b + (c + b * E2)) == b + (c - b * E2))
      return y;

  /*--- Stage II, the case abs(x-1) < 0.03 */
  a = d19.x + w * d20.x;
  a = d18.x + w * a;
  a = d17.x + w * a;
  a = d16.x + w * a;
  a = d15.x + w * a;
  a = d14.x + w * a;
  a = d13.x + w * a;
  a = d12.x + w * a;
  a = d11.x + w * a;

  EMULV (w, a, s2, ss2, t1, t2, t3, t4, t5);
  ADD2 (d10.x, dd10.x, s2, ss2, s3, ss3, t1, t2);
  MUL2 (w, 0, s3, ss3, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (d9.x, dd9.x, s2, ss2, s3, ss3, t1, t2);
  MUL2 (w, 0, s3, ss3, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (d8.x, dd8.x, s2, ss2, s3, ss3, t1, t2);
  MUL2 (w, 0, s3, ss3, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (d7.x, dd7.x, s2, ss2, s3, ss3, t1, t2);
  MUL2 (w, 0, s3, ss3, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (d6.x, dd6.x, s2, ss2, s3, ss3, t1, t2);
  MUL2 (w, 0, s3, ss3, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (d5.x, dd5.x, s2, ss2, s3, ss3, t1, t2);
  MUL2 (w, 0, s3, ss3, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (d4.x, dd4.x, s2, ss2, s3, ss3, t1, t2);
  MUL2 (w, 0, s3, ss3, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (d3.x, dd3.x, s2, ss2, s3, ss3, t1, t2);
  MUL2 (w, 0, s3, ss3, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (d2.x, dd2.x, s2, ss2, s3, ss3, t1, t2);
  MUL2 (w, 0, s3, ss3, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
  MUL2 (w, 0, s2, ss2, s3, ss3, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (w, 0, s3, ss3, b, bb, t1, t2);

  /* End stage II, case abs(x-1) < 0.03 */
  y = b + (bb + b * E4);
  return y;

  /*--- Stage I, the case abs(x-1) > 0.03 */
case_03:

  /* Find n,u such that x = u*2**n,   1/sqrt(2) < u < sqrt(2)  */
  n += (num.i[HIGH_HALF] >> 20) - 1023;
  num.i[HIGH_HALF] = (num.i[HIGH_HALF] & 0x000fffff) | 0x3ff00000;
  if(num.x > SQRT_2)
  {
      num.x *= HALF;
      n++;
  }
  u      = num.x;
  dbl_n  = (float64_t) n;

  /* Find i such that ui=1+(i-75)/2**8 is closest to u (i= 0,1,2,...,181) */
  num.x += h1.x;
  i      = (num.i[HIGH_HALF] & 0x000fffff) >> 12;

  /* Find j such that vj=1+(j-180)/2**16 is closest to v=u/ui (j= 0,...,361) */
  num.x  = u * Iu[i].x + h2.x;
  j = (num.i[HIGH_HALF] & 0x000fffff) >> 4;

  /* Compute w=(u-ui*vj)/(ui*vj) */
  p0 = (1 + (i - 75) * DEL_U) * (1 + (j - 180) * DEL_V);
  q  = u - p0;
  r0 = Iu[i].x * Iv[j].x;
  w  = q * r0;

  /* Evaluate polynomial I */
  polI = w + (a2.x + a3.x * w) * w * w;

  /* Add up everything */
  nln2a = dbl_n * LN2A;
  luai  = Lu[i][0].x;
  lubi  = Lu[i][1].x;
  lvaj  = Lv[j][0].x;
  lvbj  = Lv[j][1].x;
  EADD (luai, lvaj, sij, ssij);
  EADD (nln2a, sij, A, ttij);
  B0    = (((lubi + lvbj) + ssij) + ttij) + dbl_n * LN2B;
  B     = polI + B0;

  /* End stage I, case abs(x-1) >= 0.03 */
  if((y = A + (B + E1)) == A + (B - E1))
      return y;


  /*--- Stage II, the case abs(x-1) > 0.03 */

  /* Improve the accuracy of r0 */
  EMULV (p0, r0, sa, sb, t1, t2, t3, t4, t5);
  t = r0 * ((1 - sa) - sb);
  EADD (r0, t, ra, rb);

  /* Compute w */
  MUL2 (q, 0, ra, rb, w, ww, t1, t2, t3, t4, t5, t6, t7, t8);

  EADD (A, B0, a0, aa0);

  /* Evaluate polynomial III */
  s1 = (c3.x + (c4.x + c5.x * w) * w) * w;
  EADD (c2.x, s1, s2, ss2);
  MUL2 (s2, ss2, w, ww, s3, ss3, t1, t2, t3, t4, t5, t6, t7, t8);
  MUL2 (s3, ss3, w, ww, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (s2, ss2, w, ww, s3, ss3, t1, t2);
  ADD2 (s3, ss3, a0, aa0, a1, aa1, t1, t2);

  /* End stage II, case abs(x-1) >= 0.03 */
  y = a1 + (aa1 + E3);
  return y;
}
/**
 *@}
 */ // end of log group
