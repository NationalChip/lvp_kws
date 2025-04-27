/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_common_tables.c
 * Description:  common tables like fft twiddle factors, Bitreverse, reciprocal etc
 *
 * $Date:        18. March 2019
 * $Revision:    V1.6.0
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
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
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2016-2020 T-HEAD Limited. All rights reserved.
 *
 * This file comes from arm_common_tables.c. It includes q15 and q31 reciprocal
 * tables.
 *
 */

#include "csky_math.h"
#include "csky_common_tables.h"

/*
* @brief  Q15 table for reciprocal
*/
const q15_t ALIGN4 cskyRecipTableQ15[64] = {
 0x7F03, 0x7D13, 0x7B31, 0x795E, 0x7798, 0x75E0,
 0x7434, 0x7294, 0x70FF, 0x6F76, 0x6DF6, 0x6C82,
 0x6B16, 0x69B5, 0x685C, 0x670C, 0x65C4, 0x6484,
 0x634C, 0x621C, 0x60F3, 0x5FD0, 0x5EB5, 0x5DA0,
 0x5C91, 0x5B88, 0x5A85, 0x5988, 0x5890, 0x579E,
 0x56B0, 0x55C8, 0x54E4, 0x5405, 0x532B, 0x5255,
 0x5183, 0x50B6, 0x4FEC, 0x4F26, 0x4E64, 0x4DA6,
 0x4CEC, 0x4C34, 0x4B81, 0x4AD0, 0x4A23, 0x4978,
 0x48D1, 0x482D, 0x478C, 0x46ED, 0x4651, 0x45B8,
 0x4521, 0x448D, 0x43FC, 0x436C, 0x42DF, 0x4255,
 0x41CC, 0x4146, 0x40C2, 0x4040
};

/*
* @brief  Q31 table for reciprocal
*/
const q31_t cskyRecipTableQ31[64] = {
  0x7F03F03F, 0x7D137420, 0x7B31E739, 0x795E9F94, 0x7798FD29, 0x75E06928,
  0x7434554D, 0x72943B4B, 0x70FF9C40, 0x6F760031, 0x6DF6F593, 0x6C8210E3,
  0x6B16EC3A, 0x69B526F6, 0x685C655F, 0x670C505D, 0x65C4952D, 0x6484E519,
  0x634CF53E, 0x621C7E4F, 0x60F33C61, 0x5FD0EEB3, 0x5EB55785, 0x5DA03BEB,
  0x5C9163A1, 0x5B8898E6, 0x5A85A85A, 0x598860DF, 0x58909373, 0x579E1318,
  0x56B0B4B8, 0x55C84F0B, 0x54E4BA80, 0x5405D124, 0x532B6E8F, 0x52556FD0,
  0x5183B35A, 0x50B618F3, 0x4FEC81A2, 0x4F26CFA2, 0x4E64E64E, 0x4DA6AA1D,
  0x4CEC008B, 0x4C34D010, 0x4B810016, 0x4AD078EF, 0x4A2323C4, 0x4978EA96,
  0x48D1B827, 0x482D77FE, 0x478C1657, 0x46ED801D, 0x4651A2E5, 0x45B86CE2,
  0x4521CCE1, 0x448DB244, 0x43FC0CFA, 0x436CCD78, 0x42DFE4B4, 0x42554426,
  0x41CCDDB6, 0x4146A3C6, 0x40C28923, 0x40408102
};


