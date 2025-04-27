#include <autoconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <lvp_context.h>
#include <lvp_param.h>

#include "decoder.h"
#include "kws_strategy.h"

#define LOG_TAG "[##HVAD##]"

int human_vad = 0;

void HumanVadDectectInit(void)
{
}

int HumanVadDectectDone(void)
{
    return 0;
}

int HumanVadDectectGetStatus(void)
{
    return human_vad;
}

int valid_human_vad_cnt = 0;
int invalid_human_vad_cnt = 0;
int HumanVadDectectRun(LVP_CONTEXT *context, float *data)
{
    float *p = data + CONFIG_KWS_MODEL_OUTPUT_LENGTH - 1;
    int cur_vad = 0;

    if (*p * 100.f < CONFIG_LVP_ADVANCE_HUMAN_VAD_THRESHOLD) {
        cur_vad = 1;
    } else {
        cur_vad = 0;
    }

    if (human_vad == 0 && cur_vad == 1) {
        valid_human_vad_cnt++;
        if(valid_human_vad_cnt > CONFIG_LVP_VALID_ADVANCE_HUMAN_VAD_NUM) {
            human_vad = 1;
            invalid_human_vad_cnt = 0;
        }
    } else if (human_vad == 1 && cur_vad == 0) {
        invalid_human_vad_cnt++;
        if(invalid_human_vad_cnt > CONFIG_LVP_INVALID_ADVANCE_HUMAN_VAD_NUM) {
            human_vad = 0;
        }
    } else if (human_vad == 1) {
        invalid_human_vad_cnt = 0;
    } else if (human_vad == 0) {
        valid_human_vad_cnt = 0;
    }
    context->vad = human_vad;
    // printf(LOG_TAG"ctx:%d, blak:%f, vad:%d, human_vad:%d\n", context->ctx_index, *p, cur_vad, human_vad);
    // printf("%f\n\n", *p);

    return 0;
}
