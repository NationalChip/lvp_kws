/* Copyright (c) 2001-2011 Timothy B. Terriberry
   Copyright (c) 2008-2009 Xiph.Org Foundation */
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

#if !defined(_entdec_H)
# define _entdec_H (1)
# include <limits.h>
# include "entcode.h"

/*Initializes the decoder.
  _buf: The input buffer to use.
  Return: 0 on success, or a negative value on error.*/
OPUS_EXPORT void ec_dec_init(ec_dec *_this,unsigned char *_buf,opus_uint32 _storage);

/* Decode a bit that has a 1/(1<<_logp) probability of being a one */
DRAM0_STAGE2_SRAM_ATTR int ec_dec_bit_logp(ec_dec *_this,unsigned _logp);

/*Decodes a symbol given an "inverse" CDF table.
  No call to ec_dec_update() is necessary after this call.
  _icdf: The "inverse" CDF, such that symbol s falls in the range
          [s>0?ft-_icdf[s-1]:0,ft-_icdf[s]), where ft=1<<_ftb.
         The values must be monotonically non-increasing, and the last value
          must be 0.
  _ftb: The number of bits of precision in the cumulative distribution.
  Return: The decoded symbol s.*/
DRAM0_STAGE2_SRAM_ATTR int ec_dec_icdf(ec_dec *_this,const unsigned char *_icdf,unsigned _ftb);
#endif
