/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_mode_record.c:
 *
 */
#include <autoconf.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <csi_core.h>

#include <driver/gx_audio_in.h>
#include <driver/gx_pmu_ctrl.h>
#include <driver/gx_watchdog.h>
#include <driver/gx_delay.h>
#include <driver/gx_clock.h>
#include <driver/gx_irq.h>
#include <driver/gx_snpu.h>
#include <driver/gx_timer.h>
#include <driver/gx_gpio.h>

#include <lvp_context.h>
#include <lvp_buffer.h>
#include <lvp_board.h>

#include "common/lvp_queue.h"
#include "common/lvp_audio_in.h"
#include "common/uart_message_v2.h"
#include "common/lvp_uart_record.h"
#include "lvp_app_core.h"

#include "lvp_mode.h"
#include "lvp_kws.h"

#include "gsensor.h"

#define LOG_TAG "[LVP_RECORD]"

//=================================================================================================
static int _LvpAudioInRecordCallback(int ctx_index, void *priv)
{
   if (ctx_index > 0) {
      LVP_CONTEXT *context;
      unsigned int ctx_size;
      LvpGetContext(ctx_index - 1, &context, &ctx_size);
      context->ctx_index = ctx_index - 1;
      context->kws        = 0;
      context->vad        = 0;


      if (context->ctx_index%15 == 0) {
          printf (LOG_TAG"Ctx:%d, Vad:%d\n", context->ctx_index, context->G_vad);
      }

      APP_EVENT plc_event = {
          .event_id = AUDIO_IN_RECORD_DONE_EVENT_ID,
          .ctx_index = context->ctx_index
      };
      LvpTriggerAppEvent(&plc_event);

      LvpAudioInUpdateReadIndex(1);
   }

   return 0;
}

//-------------------------------------------------------------------------------------------------

static int _RecordModeInit(LVP_MODE_TYPE mode)
{
    if (0 != LvpAudioInInit(_LvpAudioInRecordCallback)) {
        printf(LOG_TAG"LvpAudioInInit Failed\n");
        return -1;
    }

    return 0;
}

static void _RecordModeTick(void)
{
}

static void _RecordModeDone(LVP_MODE_TYPE next_mode)
{
    printf(LOG_TAG"Exit RECORD mode\n");
}

static int _RecordModeBufferInit(void)
{
    return LvpInitBuffer();
}
//-------------------------------------------------------------------------------------------------

const LVP_MODE_INFO lvp_record_mode_info = {
    .type = LVP_MODE_RECORD,
    .buffer_init = _RecordModeBufferInit,
    .init = _RecordModeInit,
    .done = _RecordModeDone,
    .tick = _RecordModeTick,
};
