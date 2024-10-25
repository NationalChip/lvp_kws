#include "gsensor.h"
#include <driver/gx_dcache.h>
#include <lvp_attr.h>
#include <board_misc_config.h>

#ifndef BOARD_HAS_GSNEOR
gsensor_t *gsensor_instance = NULL;
#endif

static int gsensor_workstate = 1;
static struct {
    unsigned int   read_p;
    unsigned int   write_p;
    unsigned char *buffer;
    unsigned int   size;
} s_G_handle;

DRAM0_STAGE2_SRAM_ATTR int GsensorInit(unsigned char *buffer, int size)
{
    if(!gsensor_instance)
        return -1;

    memset(buffer, 0 ,size);
    s_G_handle.read_p = 0;
    s_G_handle.write_p = 0;
    s_G_handle.size = size;
    s_G_handle.buffer = buffer;

    gsensor_instance -> GsensorInit(buffer, size);

    LVP_SUSPEND_INFO info;
    info.suspend_callback = gsensor_instance -> GsensorSuspend;
    info.priv = NULL;
    LvpSuspendInfoRegist(&info);
    gsensor_workstate = 1;
    return 0;
}

DRAM0_STAGE2_SRAM_ATTR int GsensorGetData(unsigned char* buffer, int size)
{
    if (GsensorGetDataSize() < size)
        return -1;

    if (s_G_handle.read_p + size <= s_G_handle.size) {
        unsigned char *f = (unsigned char*)(s_G_handle.buffer + s_G_handle.read_p);
        unsigned char *t = (unsigned char*)buffer;
        unsigned int   s = size;

        if (s >= 128) {
            memcpy((void*)t, (const void*)f, s);
        } else {
            for (int i = 0; i < s; i ++) {
                t[i] = f[i];
            }
        }
    } else {
        unsigned char *f = (unsigned char*)(s_G_handle.buffer + s_G_handle.read_p);
        unsigned char *t = (unsigned char*)buffer;
        unsigned int   s = s_G_handle.size - s_G_handle.read_p;

        if (s >= 128) {
            memcpy((void*)t, (const void*)f, s);
        } else {
            for (int i = 0; i < s; i ++) {
                t[i] = f[i];
            }
        }

        f = (unsigned char*)(s_G_handle.buffer);
        t = (unsigned char*)(buffer + s_G_handle.size - s_G_handle.read_p);
        s = size - (s_G_handle.size - s_G_handle.read_p);

        if (s >= 128) {
            memcpy((void*)t, (const void*)f, s);
        } else {
            for (int i = 0; i < s; i ++) {
                t[i] = f[i];
            }
        }
    }

    s_G_handle.read_p += size;
    if (s_G_handle.read_p > s_G_handle.size) {
        s_G_handle.read_p -= s_G_handle.size;
        s_G_handle.write_p -= s_G_handle.size;
    }
    return size;
}

DRAM0_STAGE2_SRAM_ATTR unsigned char *GsensorGetDataPointer(int size)
{
    if (GsensorGetConsequentDataSize() < size)
        return NULL;

    unsigned int _read_p = s_G_handle.read_p;

    s_G_handle.read_p += size;
    if (s_G_handle.read_p > s_G_handle.size) {
        s_G_handle.read_p -= s_G_handle.size;
        s_G_handle.write_p -= s_G_handle.size;
    }
    return s_G_handle.buffer + _read_p;
}

DRAM0_STAGE2_SRAM_ATTR int GsensorGetDataSize(void)
{
    return s_G_handle.write_p - s_G_handle.read_p;
}

DRAM0_STAGE2_SRAM_ATTR int GsensorGetConsequentDataSize(void)
{
    return (s_G_handle.size > s_G_handle.write_p ? s_G_handle.write_p : s_G_handle.size) - s_G_handle.read_p;
}

DRAM0_STAGE2_SRAM_ATTR void GsensorRegisterCb(gsensor_event_cb_t cb, void *arg)
{
    if(gsensor_instance)
    {
        return gsensor_instance -> GsensorRegisterCb(cb, arg);
    }
}

DRAM0_STAGE2_SRAM_ATTR void GsensorEventProc(void)
{
    if(gsensor_instance)
    {
        return gsensor_instance -> GsensorEventProc();
    }
}

DRAM0_STAGE2_SRAM_ATTR int GsensorDataFill(const unsigned char *sample_data, unsigned int sample_num)
{
    if(gsensor_workstate)
        gsensor_workstate = 0;
    if ((s_G_handle.size - GsensorGetDataSize()) < (sample_num * sizeof(sample_t)))
    {
        //gx_console_putc('$');
        // printf("[G_sensor buffer fill!]\n");
    }

    int need_write =    (s_G_handle.size - GsensorGetDataSize()) > (sample_num * sizeof(sample_t))
        ?
        (sample_num * sizeof(sample_t))
        :
        (s_G_handle.size - GsensorGetDataSize());

    if ((s_G_handle.write_p % s_G_handle.size) + need_write <= s_G_handle.size) {
        unsigned char *f = (unsigned char*)sample_data;
        unsigned char *t = (unsigned char*)(s_G_handle.buffer + (s_G_handle.write_p % s_G_handle.size));
        unsigned int   s = need_write;

        if (s >= 128) {
            memcpy((void*)t, (const void*)f, s);
        } else {
            for (int i = 0; i < s; i ++) {
                t[i] = f[i];
            }
        }
        gx_dcache_clean_range((unsigned int*)(t), s);
    } else {
        unsigned char *f = (unsigned char*)sample_data;
        unsigned char *t = (unsigned char*)(s_G_handle.buffer + (s_G_handle.write_p % s_G_handle.size));
        unsigned int   s = s_G_handle.size - (s_G_handle.write_p % s_G_handle.size);

        if (s >= 128) {
            memcpy((void*)t, (const void*)f, s);
        } else {
            for (int i = 0; i < s; i ++) {
                t[i] = f[i];
            }
        }
        gx_dcache_clean_range((unsigned int*)(t), s);

        f = (unsigned char*)(((unsigned char*)sample_data) + s_G_handle.size - (s_G_handle.write_p % s_G_handle.size));
        t = (unsigned char*)s_G_handle.buffer;
        s = need_write - (s_G_handle.size - (s_G_handle.write_p % s_G_handle.size));

        if (s >= 128) {
            memcpy((void*)t, (const void*)f, s);
        } else {
            for (int i = 0; i < s; i ++) {
                t[i] = f[i];
            }
        }
        gx_dcache_clean_range((unsigned int*)(t), s);
    }
    s_G_handle.write_p += need_write;
    return need_write;
}

int GsensorGetInfo(void)
{
    printf("gsensor_workstate =%d\n", gsensor_workstate);
    return gsensor_workstate;
}
