/******************************************************************************
 * @file     core_systimer.c
 * @brief    the systimer for the Dhrystone
 * @version  V1.0
 * @date     20. July 2016
 ******************************************************************************/
/* ---------------------------------------------------------------------------
 * Copyright (C) 2016 CSKY Limited. All rights reserved.
 *
 * Redistribution and use of this software in source and binary forms,
 * with or without modification, are permitted provided that the following
 * conditions are met:
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *   * Neither the name of CSKY Ltd. nor the names of CSKY's contributors may
 *     be used to endorse or promote products derived from this software without
 *     specific prior written permission of CSKY Ltd.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * -------------------------------------------------------------------------- */
#include "drv_timer.h"
#include "soc.h"


/* APB frequence definition */
#define APB_FREQ       SYSTEM_CLOCK /* 20M Hz */
#define CORETIM_LOADTIMER           10000000 /* 10s */
#define CORETIM_LOADCOUNT           CORETIM_LOADTIMER * 20



static unsigned int SysTimer_LoopCount = 0;

static timer_handle_t timer_handle;

/*
 * Callback function for TIMER0 interrupt, set timer_flag.
 */
static void timer_cb_fun(timer_event_e event, void *arg)
{
	SysTimer_LoopCount++;
}

unsigned  long long Timer_CurrentValue()
{
  unsigned int cv;
  csi_timer_get_current_value (timer_handle, &cv);
  *(int*)(0x10002044) =1;

  return (unsigned long long)(SysTimer_LoopCount + 1) * (CORETIM_LOADCOUNT + 1) - cv -1;
}

/*
 *start  systimer
 *
 */
void Timer_Open()
{
    timer_handle = csi_timer_initialize(0, timer_cb_fun, (void *)NULL);
    csi_timer_config(timer_handle, TIMER_MODE_RELOAD);
    csi_timer_set_timeout(timer_handle, CORETIM_LOADTIMER);
}

void Timer_Start()
{
    csi_timer_start(timer_handle, SYSTEM_CLOCK);
}
void Timer_Stop()
{
    csi_timer_stop(timer_handle);
}

void Timer_Close()
{
    csi_timer_uninitialize(timer_handle);
}

/* time(uS) since run */
unsigned long long  clock()
{

    unsigned long long  systimer_val, systimer_us;
    systimer_val = Timer_CurrentValue();
    systimer_us  = systimer_val * 1000000/ APB_FREQ;
    return  systimer_us;
}

/* run time since run (seconds)*/
 long time()
{
    unsigned long long systimer_val;
    systimer_val = Timer_CurrentValue();
    return (long)(systimer_val / APB_FREQ);
}
void Sleep(unsigned long long msecond)
{
      unsigned long long systimer_val;
      systimer_val = Timer_CurrentValue();
      systimer_val=systimer_val+msecond * APB_FREQ / 1000;
      while(Timer_CurrentValue() < systimer_val);	
}
