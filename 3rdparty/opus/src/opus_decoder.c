/* Copyright (c) 2010 Xiph.Org Foundation, Skype Limited
   Written by Jean-Marc Valin and Koen Vos */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef OPUS_BUILD
# error "OPUS_BUILD _MUST_ be defined to build Opus. This probably means you need other defines as well, as in a config.h. See the included build files for details."
#endif

#if defined(__GNUC__) && (__GNUC__ >= 2) && !defined(__OPTIMIZE__) && !defined(OPUS_WILL_BE_SLOW)
# pragma message "You appear to be compiling without optimization, if so opus will be very slow."
#endif

#include <stdarg.h>
#include "opus.h"
#include "celt.h"
#include "API.h"
#include "stack_alloc.h"
#include "opus_private.h"
#include "os_support.h"
#include "structs.h"
#include "define.h"
#include "cpu_support.h"
#include "arch.h"

struct OpusDecoder {
   int          channels;
   opus_int32   Fs;          /** Sampling rate (at the API level) */
   silk_DecControlStruct DecControl;
   int          arch;

   /* Everything beyond this point gets cleared on a reset */
#define OPUS_DECODER_RESET_START stream_channels
   int          stream_channels;

   int          bandwidth;
   int          mode;
   int          prev_mode;
   int          frame_size;
#ifndef FIXED_POINT
   opus_val16   softclip_mem[2];
#endif
};

static struct OpusDecoder opusDecoder __attribute__((aligned(32)));


#if defined(ENABLE_HARDENING) || defined(ENABLE_ASSERTIONS)
OPUS_EXPORT static void validate_opus_decoder(OpusDecoder *st)
{
   celt_assert(st->channels == 1);
   celt_assert(st->Fs == 16000 || st->Fs == 8000);
   celt_assert(st->DecControl.API_sampleRate == st->Fs);
   celt_assert(st->DecControl.internalSampleRate == 0 || st->DecControl.internalSampleRate == 16000 || st->DecControl.internalSampleRate == 12000 || st->DecControl.internalSampleRate == 8000);
   celt_assert(st->DecControl.nChannelsAPI == st->channels);
   celt_assert(st->DecControl.nChannelsInternal == 0 || st->DecControl.nChannelsInternal == 1 || st->DecControl.nChannelsInternal == 2);
   celt_assert(st->DecControl.payloadSize_ms == 0 || st->DecControl.payloadSize_ms == 10 || st->DecControl.payloadSize_ms == 20 || st->DecControl.payloadSize_ms == 40 || st->DecControl.payloadSize_ms == 60);
#ifdef OPUS_ARCHMASK
   celt_assert(st->arch >= 0);
   celt_assert(st->arch <= OPUS_ARCHMASK);
#endif
   celt_assert(st->stream_channels == 1);
}
#define VALIDATE_OPUS_DECODER(st) validate_opus_decoder(st)
#else
#define VALIDATE_OPUS_DECODER(st)
#endif

OPUS_EXPORT int opus_decoder_init(opus_int32 Fs, int channels)
{
   int ret = -1;
   OpusDecoder *st = &opusDecoder;
   memset(st, 0, sizeof(struct OpusDecoder));
   if ((channels!=1))
      return OPUS_BAD_ARG;
#ifdef CONFIG_USE_OPUS_8K
   if (Fs == 8000)
       ret = 0;
#endif
#ifdef CONFIG_USE_OPUS_16K
   if (Fs == 16000)
       ret = 0;
#endif
   if (ret) {
       printf("can't support sample rate %d\n", Fs);
       return OPUS_BAD_ARG;
   }

   /* Initialize SILK decoder */

   st->stream_channels = st->channels = channels;

   st->Fs = Fs;
   st->DecControl.API_sampleRate = st->Fs;
   st->DecControl.nChannelsAPI      = st->channels;

   /* Reset decoder */
   ret = silk_InitDecoder( );
   if(ret)return OPUS_INTERNAL_ERROR;

   st->prev_mode = 0;
   st->frame_size = Fs/400;
   st->arch = opus_select_arch();
   return OPUS_OK;
}

OPUS_EXPORT static int opus_packet_get_mode(const unsigned char *data)
{
   int mode;
   if (data[0]&0x80 || (data[0]&0x60) == 0x60)
   {
       printf("only support silk mode\n");
       mode = MODE_CELT_ONLY;
   } else {
      mode = MODE_SILK_ONLY;
   }
   return mode;
}

OPUS_EXPORT static int opus_decode_frame(OpusDecoder *st, const unsigned char *data,
      opus_int32 len, opus_val16 *pcm, int frame_size, int decode_fec)
{
   int i, silk_ret=0;
   ec_dec dec;
   opus_int32 silk_frame_size;
   int pcm_silk_size;
   VARDECL(opus_int16, pcm_silk);

   int audiosize;
   int mode;
   int bandwidth;
   int F2_5, F5, F10, F20;
   int celt_accum;
   ALLOC_STACK;

   F20 = st->Fs/50;
   F10 = F20>>1;
   F5 = F10>>1;
   F2_5 = F5>>1;
   if (data == NULL || len <= 0)
       return OPUS_BAD_ARG;
   if (frame_size < F2_5)
   {
      RESTORE_STACK;
      return OPUS_BUFFER_TOO_SMALL;
   }
   /* Limit frame_size to avoid excessive stack allocations. */
   frame_size = IMIN(frame_size, st->Fs/25*3);
   /* Payloads of 1 (2 including ToC) or 0 trigger the PLC/DTX */
   if (len<=1)
   {
      data = NULL;
      /* In that case, don't conceal more than what the ToC says */
      frame_size = IMIN(frame_size, st->frame_size);
   }
   audiosize = st->frame_size;
   mode = st->mode;
   bandwidth = st->bandwidth;
   ec_dec_init(&dec,(unsigned char*)data,len);

   /* In fixed-point, we can tell CELT to do the accumulation on top of the
      SILK PCM buffer. This saves some stack space. */
#ifdef FIXED_POINT
   celt_accum = (mode != MODE_CELT_ONLY) && (frame_size >= F10);
#else
   celt_accum = 0;
#endif

   if (audiosize > frame_size)
   {
      /*fprintf(stderr, "PCM buffer too small: %d vs %d (mode = %d)\n", audiosize, frame_size, mode);*/
      RESTORE_STACK;
      return OPUS_BAD_ARG;
   } else {
      frame_size = audiosize;
   }

   /* Don't allocate any memory when in CELT-only mode */
   pcm_silk_size = (mode != MODE_CELT_ONLY && !celt_accum) ? IMAX(F10, frame_size)*st->channels : ALLOC_NONE;
   ALLOC(pcm_silk, pcm_silk_size, opus_int16);

   /* SILK processing */
   if (mode != MODE_CELT_ONLY)
   {
      int lost_flag, decoded_samples;
      opus_int16 *pcm_ptr;
#ifdef FIXED_POINT
      if (celt_accum)
         pcm_ptr = pcm;
      else
#endif
         pcm_ptr = pcm_silk;

      if (st->prev_mode==MODE_CELT_ONLY)
          return OPUS_INVALID_STATE;

      /* The SILK PLC cannot produce frames of less than 10 ms */
      st->DecControl.payloadSize_ms = IMAX(10, 1000 * audiosize / st->Fs);

      if (data != NULL)
      {
        st->DecControl.nChannelsInternal = st->stream_channels;
        if( mode == MODE_SILK_ONLY ) {
           if( bandwidth == OPUS_BANDWIDTH_NARROWBAND ) {
              st->DecControl.internalSampleRate = 8000;
           } else if( bandwidth == OPUS_BANDWIDTH_MEDIUMBAND ) {
              st->DecControl.internalSampleRate = 12000;
           } else if( bandwidth == OPUS_BANDWIDTH_WIDEBAND ) {
              st->DecControl.internalSampleRate = 16000;
           } else {
              st->DecControl.internalSampleRate = 16000;
              celt_assert( 0 );
           }
        } else {
           /* Hybrid mode */
           st->DecControl.internalSampleRate = 16000;
        }
     }

     lost_flag = data == NULL ? 1 : 2 * decode_fec;
     decoded_samples = 0;
     do {
        /* Call SILK decoder */
        int first_frame = decoded_samples == 0;
        silk_ret = silk_Decode( &st->DecControl,
                                lost_flag, first_frame, &dec, pcm_ptr, &silk_frame_size, st->arch );
        if( silk_ret ) {
           if (lost_flag) {
              /* PLC failure should not be fatal */
              silk_frame_size = frame_size;
              for (i=0;i<frame_size*st->channels;i++)
                 pcm_ptr[i] = 0;
           } else {
             RESTORE_STACK;
             return OPUS_INTERNAL_ERROR;
           }
        }
        pcm_ptr += silk_frame_size * st->channels;
        decoded_samples += silk_frame_size;
      } while( decoded_samples < frame_size );
   }

   /* MUST be after PLC */
   if (mode == MODE_SILK_ONLY)
   {
      if (!celt_accum)
      {
         for (i=0;i<frame_size*st->channels;i++)
            pcm[i] = 0;
      }
      /* For hybrid -> SILK transitions, we let the CELT MDCT
         do a fade-out by decoding a silence frame */
   }

   if (mode != MODE_CELT_ONLY && !celt_accum)
   {
#ifdef FIXED_POINT
      for (i=0;i<frame_size*st->channels;i++) {
         pcm[i] = SAT16(ADD32(pcm[i], pcm_silk[i]));
      }
#else
      for (i=0;i<frame_size*st->channels;i++)
         pcm[i] = pcm[i] + (opus_val16)((1.f/32768.f)*pcm_silk[i]);
#endif
   }
   st->prev_mode = mode;

   RESTORE_STACK;
   return silk_ret != 0 ? silk_ret : audiosize;

}

OPUS_EXPORT int opus_decode_native(const unsigned char *data,
      opus_int32 len, opus_val16 *pcm, int frame_size, int decode_fec,
      int self_delimited, opus_int32 *packet_offset)
{
   int i, nb_samples;
   int count, offset;
   unsigned char toc;
   OpusDecoder *st = &opusDecoder;
   int packet_frame_size, packet_bandwidth, packet_mode, packet_stream_channels;
   /* 48 x 2.5 ms = 120 ms */
   opus_int16 size[48];
   VALIDATE_OPUS_DECODER(st);
   if (decode_fec<0 || decode_fec>1)
      return OPUS_BAD_ARG;
   /* For FEC/PLC, frame_size has to be to have a multiple of 2.5 ms */
   if ((decode_fec || len==0 || data==NULL) && frame_size%(st->Fs/400)!=0)
      return OPUS_BAD_ARG;
   if (len <= 0 || data==NULL)
      return OPUS_BAD_ARG;

   packet_mode = opus_packet_get_mode(data); //SILK
   packet_bandwidth = opus_packet_get_bandwidth(data); //1103
   packet_frame_size = opus_packet_get_samples_per_frame(data, st->Fs); //320
   packet_stream_channels = opus_packet_get_nb_channels(data); // 1

   count = opus_packet_parse_impl(data, len, self_delimited, &toc, NULL,
                                  size, &offset, packet_offset);
   if (count<0)
      return count;

   data += offset;

   if (decode_fec)
   {
       printf("decode_fec should be 0\n");
       return OPUS_BAD_ARG;
   }

   if (count*packet_frame_size > frame_size)
      return OPUS_BUFFER_TOO_SMALL;

   /* Update the state as the last step to avoid updating it on an invalid packet */
   st->mode = packet_mode;
   st->bandwidth = packet_bandwidth;
   st->frame_size = packet_frame_size;
   st->stream_channels = packet_stream_channels;

   nb_samples=0;
   for (i=0;i<count;i++)
   {
      int ret;
      ret = opus_decode_frame(st, data, size[i], pcm+nb_samples*st->channels, frame_size-nb_samples, 0);
      if (ret<0)
         return ret;
      celt_assert(ret==packet_frame_size);
      data += size[i];
      nb_samples += ret;
   }
   if (OPUS_CHECK_ARRAY(pcm, nb_samples*st->channels))
      OPUS_PRINT_INT(nb_samples);
#ifndef FIXED_POINT
   if (soft_clip)
      opus_pcm_soft_clip(pcm, nb_samples, st->channels, st->softclip_mem);
   else
      st->softclip_mem[0]=st->softclip_mem[1]=0;
#endif
   return nb_samples;
}

#ifdef FIXED_POINT

OPUS_EXPORT int opus_decode(const unsigned char *data,
      opus_int32 len, opus_val16 *pcm, int frame_size, int decode_fec)
{
   if(frame_size<=0)
      return OPUS_BAD_ARG;
   return opus_decode_native(data, len, pcm, frame_size, decode_fec, 0, NULL);
}

#ifndef DISABLE_FLOAT_API
#endif

#else
#error "no this function"
#endif

OPUS_EXPORT int opus_decoder_ctl(int request, ...)
{
   int ret = OPUS_OK;
   va_list ap;
   OpusDecoder *st = &opusDecoder;

   va_start(ap, request);

   switch (request)
   {
   case OPUS_GET_BANDWIDTH_REQUEST:
   {
      opus_int32 *value = va_arg(ap, opus_int32*);
      if (!value)
      {
         goto bad_arg;
      }
      *value = st->bandwidth;
   }
   break;
   case OPUS_RESET_STATE:
   {
      OPUS_CLEAR((char*)&st->OPUS_DECODER_RESET_START,
            sizeof(OpusDecoder)-
            ((char*)&st->OPUS_DECODER_RESET_START - (char*)st));

      silk_InitDecoder( );
      st->stream_channels = st->channels;
      st->frame_size = st->Fs/400;
   }
   break;
   case OPUS_GET_SAMPLE_RATE_REQUEST:
   {
      opus_int32 *value = va_arg(ap, opus_int32*);
      if (!value)
      {
         goto bad_arg;
      }
      *value = st->Fs;
   }
   break;
   case OPUS_GET_PITCH_REQUEST:
   {
      opus_int32 *value = va_arg(ap, opus_int32*);
      if (!value)
      {
         goto bad_arg;
      }
      if (st->prev_mode == MODE_CELT_ONLY)
      {
        printf("only support silk decode\n");
        return OPUS_BAD_ARG;
      }
      else
         *value = st->DecControl.prevPitchLag;
   }
   break;
   default:
      /*fprintf(stderr, "unknown opus_decoder_ctl() request: %d", request);*/
      ret = OPUS_UNIMPLEMENTED;
      break;
   }

   va_end(ap);
   return ret;
bad_arg:
   va_end(ap);
   return OPUS_BAD_ARG;
}


OPUS_EXPORT int opus_packet_get_bandwidth(const unsigned char *data)
{
   int bandwidth;
   if (data[0]&0x80)
   {
      bandwidth = OPUS_BANDWIDTH_MEDIUMBAND + ((data[0]>>5)&0x3);
      if (bandwidth == OPUS_BANDWIDTH_MEDIUMBAND)
         bandwidth = OPUS_BANDWIDTH_NARROWBAND;
   } else if ((data[0]&0x60) == 0x60)
   {
      bandwidth = (data[0]&0x10) ? OPUS_BANDWIDTH_FULLBAND :
                                   OPUS_BANDWIDTH_SUPERWIDEBAND;
   } else {
      bandwidth = OPUS_BANDWIDTH_NARROWBAND + ((data[0]>>5)&0x3);
   }
   return bandwidth;
}

OPUS_EXPORT int opus_packet_get_nb_channels(const unsigned char *data)
{
   return (data[0]&0x4) ? 2 : 1;
}
