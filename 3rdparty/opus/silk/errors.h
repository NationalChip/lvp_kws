/***********************************************************************
Copyright (c) 2006-2011, Skype Limited. All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
- Neither the name of Internet Society, IETF or IETF Trust, nor the
names of specific contributors, may be used to endorse or promote
products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#ifndef SILK_ERRORS_H
#define SILK_ERRORS_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************/
/* Error messages */
/******************/
#define SILK_NO_ERROR                               0

/**************************/
/* Encoder error messages */
/**************************/

/* Input length is not a multiple of 10 ms, or length is longer than the packet length */

/**************************/
/* Decoder error messages */
/**************************/

/* Output sampling frequency lower than internal decoded sampling frequency */
#define SILK_DEC_INVALID_SAMPLING_FREQUENCY         -200

/* Payload size exceeded the maximum allowed 1024 bytes */
#define SILK_DEC_PAYLOAD_TOO_LARGE                  -201

/* Payload has bit errors */
#define SILK_DEC_PAYLOAD_ERROR                      -202

/* Payload has bit errors */
#define SILK_DEC_INVALID_FRAME_SIZE                 -203

#ifdef __cplusplus
}
#endif

#endif
