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
 * @file     csky_exp_math.h
 * @brief    Public header file for CSI DSP Library.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/


#ifndef _CSKY_EXP_MATH_H
#define _CSKY_EXP_MATH_H

#include "csky_math.h"

#ifdef   __cplusplus
extern "C"
{
#endif

  /**
   * @brief Macro for log , pow and related fast functions.
   */
#define ABS(x) (((x) > 0)   ? (x) : (-x))
#define max(x) (((y) > (x)) ? (y) : (x))
#define min(x) (((y) < (x)) ? (y) : (x))
#define CN                     124217729.0
#define HIGH_HALF              1
#define LOW_HALF               0

/* Exact addition of two single-length floating point numbers.         */
/* The macro produces a double-length number (z,zz) that satisfies     */
/* z+zz = x+y exactly.                                                 */

#define  EADD(x,y,z,zz)                            \
        z=(x)+(y);  zz=(ABS(x)>ABS(y)) ? (((x)-(z))+(y)) : (((y)-(z))+(x));

/* Exact multiplication of two single-length floating point numbers,   */
/*The macro produces a double-length number (z,zz) that                */
/* satisfies z+zz = x*y exactly. p,hx,tx,hy,ty are temporary           */
/* storage variables of type double.                                   */

# define  EMULV(x,y,z,zz,p,hx,tx,hy,ty)            \
        p=CN*(x);  hx=((x)-p)+p;  tx=(x)-hx;       \
        p=CN*(y);  hy=((y)-p)+p;  ty=(y)-hy;       \
        z=(x)*(y); zz=(((hx*hy-z)+hx*ty)+tx*hy)+tx*ty;
/* Exact multiplication of two single-length floating point numbers.         */
/* The macro produces a nearly double-length number (z,zz) (see Dekker)      */
/* that satisfies z+zz = x*y exactly. p,hx,tx,hy,ty,q are temporary          */
/* storage variables of type double.                                         */

# define  MUL12(x,y,z,zz,p,hx,tx,hy,ty,q)          \
        p=CN*(x);  hx=((x)-p)+p;  tx=(x)-hx;       \
        p=CN*(y);  hy=((y)-p)+p;  ty=(y)-hy;       \
        p=hx*hy;  q=hx*ty+tx*hy; z=p+q;  zz=((p-z)+q)+tx*ty;

/* Double-length addition, Dekker. The macro produces a double-length   */
/* number (z,zz) which satisfies approximately   z+zz = x+xx + y+yy.    */
/* An error bound: (abs(x+xx)+abs(y+yy))*4.94e-32. (x,xx), (y,yy)       */
/* are assumed to be double-length numbers. r,s are temporary           */
/* storage variables of type double.                                    */

#define  ADD2(x,xx,y,yy,z,zz,r,s)                  \
        r=(x)+(y);  s=(ABS(x)>ABS(y)) ?            \
                (((((x)-r)+(y))+(yy))+(xx)) :      \
                (((((y)-r)+(x))+(xx))+(yy));       \
        z=r+s;  zz=(r-z)+s;


/* Double-length subtraction, Dekker. The macro produces a double-length  */
/* number (z,zz) which satisfies approximately   z+zz = x+xx - (y+yy).    */
/* An error bound: (abs(x+xx)+abs(y+yy))*4.94e-32. (x,xx), (y,yy)         */
/* are assumed to be double-length numbers. r,s are temporary             */
/* storage variables of type double.                                      */

#define  SUB2(x,xx,y,yy,z,zz,r,s)                  \
        r=(x)-(y);  s=(ABS(x)>ABS(y)) ?            \
                (((((x)-r)-(y))-(yy))+(xx)) :      \
                ((((x)-((y)+r))+(xx))-(yy));       \
        z=r+s;  zz=(r-z)+s;


/* Double-length multiplication, Dekker. The macro produces a double-length  */
/* number (z,zz) which satisfies approximately   z+zz = (x+xx)*(y+yy).       */
/* An error bound: abs((x+xx)*(y+yy))*1.24e-31. (x,xx), (y,yy)               */
/* are assumed to be double-length numbers. p,hx,tx,hy,ty,q,c,cc are         */
/* temporary storage variables of type double.                               */

#define  MUL2(x,xx,y,yy,z,zz,p,hx,tx,hy,ty,q,c,cc) \
        MUL12(x,y,c,cc,p,hx,tx,hy,ty,q)            \
        cc=((x)*(yy)+(xx)*(y))+cc;   z=c+cc;   zz=(c-z)+cc;

  /*double format*/
  typedef union _myNumber
  {
    uint32_t  i[2];
    float64_t x;
  }mynumber;

  /* the coefficient for log2 table looh up*/
  typedef union
  {
    uint32_t i[5800];
    float64_t x[2900];
  }log2_cof1;

  typedef union
  {
    uint32_t i[4350];
    float64_t x[2175];
  }log2_cof2;

  /* the coefficient for exp table looh up*/
 typedef union
  {
    uint32_t i[1424];
    float64_t x[712];
  }exp_cof1;

  typedef union
  {
    uint32_t i[2048];
    float64_t x[1024];
  }exp_cof2;

  union ieee754_double
  {
    float64_t d;

    struct
      {
        unsigned int mantissa1:32;
        unsigned int mantissa0:20;
        unsigned int exponent:11;
        unsigned int negative:1;
      } ieee;
    struct
      {
        unsigned int mantissa1:32;
        unsigned int mantissa0:19;
        unsigned int quiet_nan:1;
        unsigned int exponent:11;
        unsigned int negative:1;
      } ieee_nan;
  };

  typedef struct
  {
    uint32_t e;
    long  d[40];
  }mp_no;

  float64_t csky_pow_f64(
  float64_t x,
  float64_t y);

  float64_t csky_log_f64(
  float64_t x);

  float64_t csky_exp_f64(
  float64_t x);

  float64_t csky_pow2_f64(
  float64_t x);

  float64_t csky_log2_f64(
  float64_t x);

  float64_t csky_log10_f64(
  float64_t x);

/*Table for Fast math pow*/
extern  const log2_cof1  ui;
extern  const log2_cof2  vj;
extern  const exp_cof1 coar;
extern  const exp_cof2 fine;

/*Table for Fast math pow2*/
extern  const float64_t exp2_accuratetable[512];
extern  const float32_t exp2_deltatable[512];

/*Table for Fast math pow2*/
extern  const mynumber Iu[182];
extern  const mynumber Iv[362];
extern  const mynumber Lu[182][2];
extern  const mynumber Lv[362][2];

/*constant for Fast math*/
  const static mynumber
  nZERO	  = {{0, 0x80000000}},	        /* -0.0          */
  INF     = {{0x00000000, 0x7ff00000}}, /* INF           */
  nINF    = {{0x00000000, 0xfff00000}}, /* -INF          */
  sqrt_2  = {{0x667f3bcc, 0x3ff6a09e}}, /* sqrt(2)       */
  ln2a    = {{0xfefa3800, 0x3fe62e42}}, /* ln(2) 43 bits */
  ln2b    = {{0x93c76730, 0x3d2ef357}}, /* ln(2)-ln2a    */
  bigu    = {{0xfffffd2c, 0x4297ffff}}, /* 1.5*2**42 -724*2**-10  */
  bigv    = {{0xfff8016a, 0x4207ffff}}, /* 1.5*2**33-1+362*2**-19 */
  t52     = {{0x00000000, 0x43300000}}, /* 2**52         */
  two52e  = {{0x000003ff, 0x43300000}}, /* 2**52'        */
  NaN     = {{0x00000000, 0x7ff80000}}, /* NAN           */
  t256    = {{0, 0x4ff00000}},          /* 2^256         */
  ln_two1 = {{0xFEFA3800, 0x3FE62E42}}, /* 0.69314718055989033 */
  ln_two2 = {{0x93C76730, 0x3D2EF357}}, /* 5.4979230187083712e-14*/
  log2e   = {{0x652B82FE, 0x3FF71547}}, /* 1.4426950408889634  */
  ep2     = {{0x000004DC, 0x3FE00000}}, /* 0.50000000000013811 */
  ep3     = {{0x55555A0F, 0x3FC55555}}, /* 0.16666666666670024 */
  three33 = {{0, 0x42180000}},          /* 25769803776      */
  three51 = {{0, 0x43380000}};          /* 6755399441055744 */

  const static float64_t
  p2 = -0.5, p3 =  3.3333333333333333333e-1, p4 = -0.25,
  q2 = -0.5, q3 = 3.3333333333331404e-01, q4 =  -2.4999999999996436e-01,
  q5 =  2.0000010500004459e-01, q6 =  -1.6666678916688004e-01,
  r3 =  3.33333333333333333372884096563030E-01,
  r4 = -2.50000000000000000213574153875908E-01,
  r5 =  1.99999999999683593814072199830603E-01,
  r6 = -1.66666666666065494878165510225378E-01,
  r7 =  1.42857517857114380606360005067609E-01,
  r8 = -1.25000449999974370683775964001702E-01,
  s3 =  0.333251953125000000e0,
 ss3 =  8.138020833333333333e-05,
  s4 = -2.500000000000000000e-01,
  s5 =  1.999999999999960937e-01,
  s6 = -1.666666666666592447e-01,
  s7 =  1.428571845238194705e-01,
  s8 = -1.250000500000149097e-01;

  const static float64_t huge = 1.0e300, tiny = 1.0e-300;
  const static float64_t err_0 = 1.000014, err_1 = 0.000016, zero = 0.0;
  const static q31_t bigint = 0x40862002,
               badint = 0x40876000, smallint = 0x3C8fffff;
  const static q31_t hugeint = 0x7fffffff, infint = 0x7ff00000;

static const mynumber
  /* polynomial I */
  a2  = {{0x0001aa8f, 0xbfe00000} }, /* -0.500... */
  a3             = {{0x55588d2e, 0x3fd55555} }, /*  0.333... */
  /*polynomial II */
  b0    = {{0x55555555, 0x3fd55555} }, /*  0.333... */
  b1    = {{0xffffffbb, 0xbfcfffff} }, /* -0.249... */
  b2    = {{0x9999992f, 0x3fc99999} }, /*  0.199... */
  b3    = {{0x556503fd, 0xbfc55555} }, /* -0.166... */
  b4    = {{0x925b3d62, 0x3fc24924} }, /*  0.142... */
  b5    = {{0x160472fc, 0xbfbffffe} }, /* -0.124... */
  b6    = {{0x25db58ac, 0x3fbc71c5} }, /*  0.111... */
  b7    = {{0x11a2a61c, 0xbfb9a4ac} }, /* -0.100... */
  b8    = {{0x0df2b591, 0x3fb75077} }, /*  0.091... */
  /*polynomial III */
  c2    = {{0x00000000, 0xbfe00000} }, /* -1/2      */
  c3    = {{0x55555555, 0x3fd55555} }, /*  1/3      */
  c4    = {{0x00000000, 0xbfd00000} }, /* -1/4      */
  c5    = {{0x9999999a, 0x3fc99999} }, /*  1/5      */
  /*polynomial IV */
  d2    = {{0x00000000, 0xbfe00000} }, /* -1/2      */
  dd2   = {{0x00000000, 0x00000000} }, /* -1/2-d2   */
  d3    = {{0x55555555, 0x3fd55555} }, /*  1/3      */
  dd3   = {{0x55555555, 0x3c755555} }, /*  1/3-d3   */
  d4    = {{0x00000000, 0xbfd00000} }, /* -1/4      */
  dd4   = {{0x00000000, 0x00000000} }, /* -1/4-d4   */
  d5    = {{0x9999999a, 0x3fc99999} }, /*  1/5      */
  dd5   = {{0x9999999a, 0xbc699999} }, /*  1/5-d5   */
  d6    = {{0x55555555, 0xbfc55555} }, /* -1/6      */
  dd6   = {{0x55555555, 0xbc655555} }, /* -1/6-d6   */
  d7    = {{0x92492492, 0x3fc24924} }, /*  1/7      */
  dd7   = {{0x92492492, 0x3c624924} }, /*  1/7-d7   */
  d8    = {{0x00000000, 0xbfc00000} }, /* -1/8      */
  dd8   = {{0x00000000, 0x00000000} }, /* -1/8-d8   */
  d9    = {{0x1c71c71c, 0x3fbc71c7} }, /*  1/9      */
  dd9   = {{0x1c71c71c, 0x3c5c71c7} }, /*  1/9-d9   */
  d10   = {{0x9999999a, 0xbfb99999} }, /* -1/10     */
  dd10  = {{0x9999999a, 0x3c599999} }, /* -1/10-d10 */
  d11   = {{0x745d1746, 0x3fb745d1} }, /*  1/11     */
  d12   = {{0x55555555, 0xbfb55555} }, /* -1/12     */
  d13   = {{0x13b13b14, 0x3fb3b13b} }, /*  1/13     */
  d14   = {{0x92492492, 0xbfb24924} }, /* -1/14     */
  d15   = {{0x11111111, 0x3fb11111} }, /*  1/15     */
  d16   = {{0x00000000, 0xbfb00000} }, /* -1/16     */
  d17   = {{0x1e1e1e1e, 0x3fae1e1e} }, /*  1/17     */
  d18   = {{0x1c71c71c, 0xbfac71c7} }, /* -1/18     */
  d19   = {{0xbca1af28, 0x3faaf286} }, /*  1/19     */
  d20   = {{0x9999999a, 0xbfa99999} }, /* -1/20     */
  /*constants    */
  h1    = {{0x00000000, 0x3fd2e000} }, /* 151/2**9  */
  h2    = {{0x00000000, 0x3f669000} }, /* 361/2**17 */
  delu  = {{0x00000000, 0x3f700000} }, /* 1/2**8    */
  delv  = {{0x00000000, 0x3ef00000} }, /* 1/2**16   */
  e1    = {{0x00000000, 0x3bbcc868} }, /* 6.095e-21 */
  e2    = {{0x00000000, 0x3c1138ce} }, /* 2.334e-19 */
  e3    = {{0x00000000, 0x3aa1565d} }, /* 2.801e-26 */
  e4    = {{0x00000000, 0x39809d88} }, /* 1.024e-31 */
  e[4]  = {{{0x00000000, 0x37da223a} },/* 1.2e-39   */
          {{0x00000000, 0x35c851c4} }, /* 1.3e-49   */
          {{0x00000000, 0x2ab85e51} }, /* 6.8e-103  */
          {{0x00000000, 0x17383827} }},/* 8.1e-197  */
  two54 = {{0x00000000, 0x43500000} }, /* 2**54     */
  u03   = {{0xeb851eb8, 0x3f9eb851} }; /* 0.03      */

#define  SQRT_2    sqrt_2.x
#define  DEL_U     delu.x
#define  DEL_V     delv.x
#define  LN2A      ln2a.x
#define  LN2B      ln2b.x
#define  E1        e1.x
#define  E2        e2.x
#define  E3        e3.x
#define  E4        e4.x
#define  U03       u03.x
#define  HALF      0x1.0p-1     /* 1/2 */
#define  MHALF     -0x1.0p-1    /* -1/2 */

/*coeffient for log2 funtion*/
static const float64_t
  ln2     = 0.69314718055994530942,
  two54_d = 1.80143985094819840000e+16,   /* 43500000 00000000 */
  Lg1     = 6.666666666666735130e-01,     /* 3FE55555 55555593 */
  Lg2     = 3.999999999940941908e-01,     /* 3FD99999 9997FA04 */
  Lg3     = 2.857142874366239149e-01,     /* 3FD24924 94229359 */
  Lg4     = 2.222219843214978396e-01,     /* 3FCC71C5 1D8E78AF */
  Lg5     = 1.818357216161805012e-01,     /* 3FC74664 96CB03DE */
  Lg6     = 1.531383769920937332e-01,     /* 3FC39A09 D078C69F */
  Lg7     = 1.479819860511658591e-01;     /* 3FC2F112 DF3E5244 */

/*coeffient for log10 function*/
static const float64_t
  ivln10    = 4.34294481903251816668e-01,	/* 0x3FDBCB7B, 0x1526E50E */
  log10_2hi = 3.01029995663611771306e-01,	/* 0x3FD34413, 0x509F6000 */
  log10_2lo = 3.69423907715893078616e-13;	/* 0x3D59FEF3, 0x11F12B36 */

/*constant for log10 function*/
static const float64_t
  TWO1023  = 8.988465674311579539e+307,
  TWOM1000 = 9.3326361850321887899e-302;

#ifdef   __cplusplus
}
#endif

#endif
