/* Grus
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * lvp_context.h:
 *
 */

#ifndef __LVP_CONTEXT_H__
#define __LVP_CONTEXT_H__

#include <lvp_attr.h>

#define LVP_CONTEXT_VERSION 0x20191222

//=================================================================================================
// Audio Process Context shared with MCU, NPU

typedef struct {
    unsigned int        version;

    unsigned int        fft_vad_en;             // 0:Disable FFTVad, 1:Enable FFTVad
    unsigned int        mic_num;                // 1 - 2
    unsigned int        ref_num;                // 0 - 1
    unsigned int        fft_num;                // 0 - 2
    unsigned int        logfbank_num;           // 0 - 1
    unsigned int        out_num;                // 0 - 1
    unsigned int        frame_length;           // 10ms, 16ms
    unsigned int        sample_rate;            // 8000, 16000
    unsigned int        pcm_frame_num_per_context;       // the FRAME num in a CONTEXT
    unsigned int        pcm_frame_num_per_channel;       // the total FRMAE num in a CHANNEL
    unsigned int        fft_frame_num_per_channel;       // the total FRMAE num in a CHANNEL
    unsigned int        logfbank_frame_num_per_channel;  // the total FRMAE num in a CHANNEL
    unsigned int        out_frame_num_per_channel;       // the total FRMAE num in a CHANNEL

    // CTX buffer for GLOBAL
    void               *ctx_buffer;             // Context Buffer header point
    unsigned int        ctx_num;                // Context number
    unsigned int        ctx_size;               // Context size

    // SNPU and OUT buffer for CONTEXT
    unsigned int        snpu_buffer_size;       // Bytes

    // OUT buffer for CLOBAL
    short              *out_buffer;
    unsigned int        out_buffer_size;        // Bytes

    // MIC buffer for GLOBAL
    short              *mic_buffer;             // DEVICE ADDRESS
    unsigned int        mic_buffer_size;        // Bytes

    // REF buffer for GLOBAL
    short              *ref_buffer;             // DEVICE ADDRESS
    unsigned int        ref_buffer_size;        // Bytes

    // FFT buffer for GLOBAL
    unsigned int        fft_dim_per_frame;      // FFT Dim Per Frame
    int              *fft_buffer;             // Address
    unsigned int        fft_buffer_size;        // Bytes

    // Logfbank buffer for GLOBAL
    unsigned int        logfbank_dim_per_frame; // Logfbank Dim Per Frame
    short              *logfbank_buffer;        // Address
    unsigned int        logfbank_buffer_size;   // Bytes

} LVP_CONTEXT_HEADER;

typedef struct {
    LVP_CONTEXT_HEADER *ctx_header;             // DEVICE ADDRESS
    unsigned int        frame_index;            // FRAME index of the first frame in CONTEXT
    unsigned int        ctx_index;              // CONTEXT index from 0 - (2^32 - 1)
    unsigned int        fft_vad:3;
    unsigned int        vad:3;
    unsigned int        G_vad:2;
    unsigned int        kws:8;
    unsigned int        env_noise:3;
    unsigned int        sed:3;
    unsigned int        reserve:10;
    void               *snpu_buffer;            // DEVICE ADDRESS
} ALIGNED_ATTR(16) LVP_CONTEXT;

//=================================================================================================
#endif /* __LVP_CONTEXT_H__ */
