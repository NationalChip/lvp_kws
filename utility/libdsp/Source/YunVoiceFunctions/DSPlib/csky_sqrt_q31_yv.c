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
 * @file     csky_sqrt_q31_yv.c
 * @brief    Q31 square root function.
 * @version  V1.0
 * @date     20. Dec 2016
 ******************************************************************************/

#include "csky_math.h"
#include "csky_common_tables.h"

/**
 * @ingroup DSPlibyv
 */

/**
 * @defgroup SQRTyv Square Root
 *
 * Computes the square root of a number. Only the Q31 format data is supported
 * here.Two methods are used to compute the result.
 * When a CPU with FPU is used, the instruction <code>fsqrts</code> is used to
 * compute the result, while a CPU without FPU, Newton-Raphson algorithm is used.
 * For Newton-Raphson algorithm, this is an iterative algorithm of the form:
 * <pre>
 *      x1 = x0 - f(x0)/f'(x0)
 * </pre>
 * where <code>x1</code> is the current estimate,
 * <code>x0</code> is the previous estimate, and
 * <code>f'(x0)</code> is the derivative of <code>f()</code> evaluated at <code>x0</code>.
 * For the square root function, the algorithm reduces to:
 * <pre>
 *     x0 = in/2                         [initial guess]
 *     x1 = 1/2 * ( x0 + in / x0)        [each iteration]
 * </pre>
 *
 * For both of the two method, the mantissa is longer than 15, so the result will abandon the low
 * 16 bits and be rounded if the flag is set.
 *
 */

/**
 * @addtogroup SQRTyv
 * @{
 */

/**
 * @brief Q31 square root function.
 * @param[in] x         input value, the range of the input value is [0 +1) or 0x00000000 to 0x7FFFFFFF
 * @param[in] rnd_flag  the result should be rounded if this flag is set
 * @return The result of sqrt.
 */

q15_t csky_dsp_lib_sqrt_int32(
  q31_t x,
  uint32_t rnd_flag)
{
  q15_t result;
  q31_t number, temp1, bits_val1, var1, signBits1, half;
  float32_t temp_float1, result_tmp;

#ifdef __FPU_PRESENT
  if(x > 0)
  {
      temp_float1 = (float)x / 2147483648;
      __ASM volatile("fsqrts %0, %1\n\t":"=v"(result_tmp),"=v"(temp_float1):"0"(result_tmp),"1"(temp_float1));
      var1 = (q31_t) (result_tmp * 2147483648);

#else
  union
  {
      q31_t fracval;
      float32_t floatval;
  } tempconv;

  number = x;

  /* If the input is a positive number then compute the signBits. */
  if(number > 0)
  {
    signBits1 = __CLZ(number) - 1;

   /* Shift by the number of signBits1 */
   if((signBits1 % 2) == 0)
   {
     number = number << signBits1;
   }
   else
   {
     number = number << (signBits1 - 1);
   }

    /* Calculate half value of the number */
    half = number >> 1;
    /* Store the number for later use */
    temp1 = number;

    /*Convert to float */
    temp_float1 = number * 4.6566128731e-010f;
    /*Store as integer */
    tempconv.floatval = temp_float1;
    bits_val1 = tempconv.fracval;
    /* Subtract the shifted value from the magic number to give intial guess */
    bits_val1 = 0x5f3759df - (bits_val1 >> 1);  /* gives initial guess */
    /* Store as float */
    tempconv.fracval = bits_val1;
    temp_float1 = tempconv.floatval;
    /* Convert to integer format */
    var1 = (q31_t) (temp_float1 * 1073741824);
#ifdef CSKY_SIMD
    /* 1st iteration */
    q31_t tmp1 = mult_32x32_dext_31(var1, var1);
    tmp1 = mult_32x32_dext_31(tmp1, half);
    tmp1 = mult_32x32_dext_31(0x30000000 - tmp1, var1);
    var1 = tmp1 << 2;

    /* mult_32x32_dext_312nd iteration */
    tmp1 = mult_32x32_dext_31(var1, var1);
    tmp1 = mult_32x32_dext_31(tmp1, half);
    tmp1 = mult_32x32_dext_31(0x30000000 - tmp1, var1);
    var1 = tmp1 << 2;

    /* 3rd iteration */
    tmp1 = mult_32x32_dext_31(var1, var1);
    tmp1 = mult_32x32_dext_31(tmp1, half);
    tmp1 = mult_32x32_dext_31(0x30000000 - tmp1, var1);
    var1 = tmp1 << 2;

    /* Multiply the inverse square root with the original value */
    var1 = mult_32x32_dext_31(temp1, var1) << 1;
#else
    /* 1st iteration */
    var1 = ((q31_t) ((q63_t) var1 * (0x30000000 -
                                     ((q31_t)
                                      ((((q31_t)
                                         (((q63_t) var1 * var1) >> 31)) *
                                        (q63_t) half) >> 31))) >> 31)) << 2;
    /* 2nd iteration */
    var1 = ((q31_t) ((q63_t) var1 * (0x30000000 -
                                     ((q31_t)
                                      ((((q31_t)
                                         (((q63_t) var1 * var1) >> 31)) *
                                        (q63_t) half) >> 31))) >> 31)) << 2;
    /* 3rd iteration */
    var1 = ((q31_t) ((q63_t) var1 * (0x30000000 -
                                     ((q31_t)
                                      ((((q31_t)
                                         (((q63_t) var1 * var1) >> 31)) *
                                        (q63_t) half) >> 31))) >> 31)) << 2;

    /* Multiply the inverse square root with the original value */
    var1 = ((q31_t) (((q63_t) temp1 * var1) >> 31)) << 1;

#endif

    /* Shift the output down accordingly */
    if((signBits1 % 2) == 0)
    {
      var1 = var1 >> (signBits1 / 2);
    }
    else
    {
      var1 = var1 >> ((signBits1 - 1) / 2);
    }

#endif
    result = (q15_t)(var1 >> 16 );
    if(rnd_flag && ((var1 & 0x8000) == 1))
    {
      result = __SADD16(result, 1);
    }
    return result;
  }
}
/**
 * @} end of SQRTyv group
 */
