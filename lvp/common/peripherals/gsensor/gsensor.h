#pragma once
#include <stdio.h>
#include <common.h>
#include <lvp_pmu.h>
typedef enum
{
    SINGLE_TAP_EVENT =0,
}GSENSOR_EVENT_TYPE;

typedef void (*gsensor_event_cb_t)(GSENSOR_EVENT_TYPE type, void *arg);
typedef struct {
    const char *name;
    int (*GsensorInit)(unsigned char *buffer, int size);
    void (*GsensorRegisterCb)(gsensor_event_cb_t cb, void *arg);
    void (*GsensorEventProc)(void);
    int (*GsensorSuspend)(void *priv);
}gsensor_t;

extern gsensor_t *gsensor_instance;

#define REGISTER_GSENSOR(s)    \
     gsensor_t *gsensor_instance = &s

typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} sample_t;
int GsensorDataFill(const unsigned char *sample_data, unsigned int sample_num);

/*User Interface*/
int GsensorInit(unsigned char *buffer, int size);
int GsensorGetData(unsigned char* buffer, int size);
int GsensorGetDataSize(void);
int GsensorGetInfo(void);
unsigned char *GsensorGetDataPointer(int size);
int GsensorGetConsequentDataSize(void);
void GsensorRegisterCb(gsensor_event_cb_t cb, void *arg);
void GsensorEventProc(void);
