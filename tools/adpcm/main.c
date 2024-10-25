/*************************************************************************
	> File Name: main.c
	> Author: jialp
	> Mail: jialp@nationalchip.com
	> Created Time: Thu 04 Nov 2021 09:08:11 AM CST
	> Description: 
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "adpcm.h"

static const char *usage =
" Usage:     ADPCM [-options] infile outfile\n\n"
"               Just support 16bit, mono\n"
" Options:  -bn    = samples number of block\n"
"           -d     = decode only\n"
"           -e     = encode only\n"
"           -h     = display this help message\n"
"           -y     = overwrite outfile if it exists\n\n"
;

    int verbosity = 1; // 输出log


typedef struct {
    char ckID [4];
    unsigned int ckSize;
    char formType [4];
} RiffChunkHeader;

typedef struct {
    char ckID [4];
    unsigned int ckSize;
} ChunkHeader;

#define ChunkHeaderFormat "4L" // 标记各字段size，用于大小端转换

typedef struct {
    unsigned short FormatTag, NumChannels;
    unsigned int SampleRate, BytesPerSecond;
    unsigned short BlockAlign, BitsPerSample;
    unsigned short cbSize;
    union {
        unsigned short ValidBitsPerSample;
        unsigned short SamplesPerBlock;
        unsigned short Reserved;
    } Samples;
    unsigned int ChannelMask;
    unsigned short SubFormat;
    char GUID [14];
} WaveHeader;

#define WaveHeaderFormat "SSLLSSSSLS" // 标记各字段size，用于大小端转换

typedef struct {
    char ckID [4];
    unsigned int ckSize;
    unsigned int TotalSamples;
} FactHeader;

#define FactHeaderFormat "4LL" // 标记各字段size，用于大小端转换

#define WAVE_FORMAT_PCM         0x1
#define WAVE_FORMAT_IMA_ADPCM   0x11
#define WAVE_FORMAT_EXTENSIBLE  0xfffe

static void little_endian_to_native (void *data, char *format)
{
    unsigned char *cp = (unsigned char *) data;
    int32_t temp;

    while (*format) {
        switch (*format) {
            case 'L': // format 为 'L' 时按 4 字节重整大小端
                temp = cp [0] + ((int32_t) cp [1] << 8) + ((int32_t) cp [2] << 16) + ((int32_t) cp [3] << 24);
                * (int32_t *) cp = temp;
                cp += 4;
                break;

            case 'S': // format 为 'S' 时按 2 字节重整大小端
                temp = cp [0] + (cp [1] << 8);
                * (short *) cp = (short) temp;
                cp += 2;
                break;

            default: // format 为 数字 时，偏移相应字节数
                if (isdigit ((unsigned char) *format))
                    cp += *format - '0';

                break;
        }

        format++;
    }
}

static void native_to_little_endian (void *data, char *format)
{
    unsigned char *cp = (unsigned char *) data;
    int32_t temp;

    while (*format) {
        switch (*format) {
            case 'L':
                temp = * (int32_t *) cp;
                *cp++ = (unsigned char) temp;
                *cp++ = (unsigned char) (temp >> 8);
                *cp++ = (unsigned char) (temp >> 16);
                *cp++ = (unsigned char) (temp >> 24);
                break;

            case 'S':
                temp = * (short *) cp;
                *cp++ = (unsigned char) temp;
                *cp++ = (unsigned char) (temp >> 8);
                break;

            default:
                if (isdigit ((unsigned char) *format))
                    cp += *format - '0';

                break;
        }

        format++;
    }
}

static int write_adpcm_wav_header (FILE *outfile, int num_channels, size_t num_samples, int sample_rate, int samples_per_block)
{
    RiffChunkHeader riffhdr;
    ChunkHeader datahdr, fmthdr;
    WaveHeader wavhdr;
    FactHeader facthdr;

    int wavhdrsize = 20;
    int block_size = samples_per_block / 2;
    size_t num_blocks = num_samples / samples_per_block;
    int leftover_samples = num_samples % samples_per_block;
    size_t total_data_bytes = num_blocks * block_size;

    if (leftover_samples) {
        int last_block_samples = ((leftover_samples + 6) & ~7) + 1;
        int last_block_size = (last_block_samples - 1) / (num_channels ^ 3) + (num_channels * 4);
        total_data_bytes += last_block_size;
    }

    memset (&wavhdr, 0, sizeof (wavhdr));

    wavhdr.FormatTag = WAVE_FORMAT_IMA_ADPCM;
    wavhdr.NumChannels = num_channels;
    wavhdr.SampleRate = sample_rate;
    wavhdr.BytesPerSecond = sample_rate * block_size / samples_per_block;
    wavhdr.BlockAlign = block_size;
    wavhdr.BitsPerSample = 4;
    wavhdr.cbSize = 2;
    wavhdr.Samples.SamplesPerBlock = samples_per_block;

    strncpy (riffhdr.ckID, "RIFF", sizeof (riffhdr.ckID));
    strncpy (riffhdr.formType, "WAVE", sizeof (riffhdr.formType));
    riffhdr.ckSize = sizeof (riffhdr) + wavhdrsize + sizeof (facthdr) + sizeof (datahdr) + total_data_bytes;
    strncpy (fmthdr.ckID, "fmt ", sizeof (fmthdr.ckID));
    fmthdr.ckSize = wavhdrsize;
    strncpy (facthdr.ckID, "fact", sizeof (facthdr.ckID));
    facthdr.TotalSamples = num_samples;
    facthdr.ckSize = 4;

    strncpy (datahdr.ckID, "data", sizeof (datahdr.ckID));
    datahdr.ckSize = total_data_bytes;

    // write the RIFF chunks up to just before the data starts

    native_to_little_endian (&riffhdr, ChunkHeaderFormat);
    native_to_little_endian (&fmthdr, ChunkHeaderFormat);
    native_to_little_endian (&wavhdr, WaveHeaderFormat);
    native_to_little_endian (&facthdr, FactHeaderFormat);
    native_to_little_endian (&datahdr, ChunkHeaderFormat);

    fprintf(stderr, "adfdasfasdfsadfasdfsdf\nasdfasfasdfs\nasdfasdfasdfasdfas\nfsadfsadfasfasdfasdf\n");

    return fwrite (&riffhdr, sizeof (riffhdr), 1, outfile) &&
        fwrite (&fmthdr, sizeof (fmthdr), 1, outfile) &&
        fwrite (&wavhdr, wavhdrsize, 1, outfile) &&
        fwrite (&facthdr, sizeof (facthdr), 1, outfile) &&
        fwrite (&datahdr, sizeof (datahdr), 1, outfile);
}

static int adpcm_decode_data (FILE *infile, FILE *outfile, size_t num_samples, int block_size)
{
    int samples_per_block = block_size * 2, percent;
    void *pcm_block = malloc (samples_per_block * 2);
    void *adpcm_block = malloc (block_size);
    size_t progress_divider = 0;

    if (!pcm_block || !adpcm_block) {
        fprintf (stderr, "could not allocate memory for buffers!\n");
        return -1;
    }

    while (num_samples > samples_per_block) {
        int this_block_adpcm_samples = samples_per_block;
        int this_block_pcm_samples = samples_per_block;

        if (!fread (adpcm_block, block_size, 1, infile)) {
            fprintf (stderr, "could not read all audio data from input file!\n");
            return -1;
        }

        Adpcm2Pcm (adpcm_block, pcm_block, this_block_pcm_samples);

        if (!fwrite (pcm_block, this_block_pcm_samples * 2, 1, outfile)) {
            fprintf (stderr, "could not write all audio data to output file!\n");
            return -1;
        }

        num_samples -= this_block_pcm_samples;
    }

    if (verbosity >= 0)
        fprintf (stderr, "\rdecode...completed successfully\n");

    free (adpcm_block);
    free (pcm_block);
    return 0;
}

static int adpcm_encode_data (FILE *infile, FILE *outfile, size_t num_samples, int samples_per_block)
{
    int block_size = samples_per_block / 2, percent;
    int16_t *pcm_block = malloc (samples_per_block * 2);
    void *adpcm_block = malloc (block_size);
    size_t progress_divider = 0;
    void *adpcm_cnxt = NULL;

    if (!pcm_block || !adpcm_block) {
        fprintf (stderr, "could not allocate memory for buffers!\n");
        return -1;
    }

    while (num_samples > samples_per_block) {
        int this_block_adpcm_samples = samples_per_block;
        int this_block_pcm_samples = samples_per_block;

        if (!fread (pcm_block, this_block_pcm_samples * 2, 1, infile)) {
            fprintf (stderr, "\rcould not read all audio data from input file!\n");
            return -1;
        }

        Pcm2Adpcm(pcm_block, adpcm_block, this_block_adpcm_samples);

        if (!fwrite (adpcm_block, block_size, 1, outfile)) {
            fprintf (stderr, "\rcould not write all audio data to output file!\n");
            return -1;
        }

        num_samples -= this_block_pcm_samples;
    }

    if (verbosity >= 0)
        fprintf (stderr, "\rencode...completed successfully\n");

    free (adpcm_block);
    free (pcm_block);
    return 0;
}

static int write_pcm_wav_header (FILE *outfile, int num_channels, size_t num_samples, int sample_rate)
{
    RiffChunkHeader riffhdr;
    ChunkHeader datahdr, fmthdr;
    WaveHeader wavhdr;

    int wavhdrsize = 16;
    int bytes_per_sample = 2;
    size_t total_data_bytes = num_samples * bytes_per_sample * num_channels;

    memset (&wavhdr, 0, sizeof (wavhdr));

    wavhdr.FormatTag = WAVE_FORMAT_PCM;
    wavhdr.NumChannels = num_channels;
    wavhdr.SampleRate = sample_rate;
    wavhdr.BytesPerSecond = sample_rate * num_channels * bytes_per_sample;
    wavhdr.BlockAlign = bytes_per_sample * num_channels;
    wavhdr.BitsPerSample = 16;

    strncpy (riffhdr.ckID, "RIFF", sizeof (riffhdr.ckID));
    strncpy (riffhdr.formType, "WAVE", sizeof (riffhdr.formType));
    riffhdr.ckSize = sizeof (riffhdr) + wavhdrsize + sizeof (datahdr) + total_data_bytes;
    strncpy (fmthdr.ckID, "fmt ", sizeof (fmthdr.ckID));
    fmthdr.ckSize = wavhdrsize;

    strncpy (datahdr.ckID, "data", sizeof (datahdr.ckID));
    datahdr.ckSize = total_data_bytes;

    // write the RIFF chunks up to just before the data starts

    native_to_little_endian (&riffhdr, ChunkHeaderFormat);
    native_to_little_endian (&fmthdr, ChunkHeaderFormat);
    native_to_little_endian (&wavhdr, WaveHeaderFormat);
    native_to_little_endian (&datahdr, ChunkHeaderFormat);

    return fwrite (&riffhdr, sizeof (riffhdr), 1, outfile) &&
        fwrite (&fmthdr, sizeof (fmthdr), 1, outfile) &&
        fwrite (&wavhdr, wavhdrsize, 1, outfile) &&
        fwrite (&datahdr, sizeof (datahdr), 1, outfile);
}

int main (int argc, char **argv )
{
    char *infilename = NULL, *outfilename = NULL;

    int samples = 256;
    int encode = 1;
    int decode = 0;
    int asked_help = 0;
    int overwrite = 0;

    FILE *outfile;

    while (--argc) {
#if defined (_WIN32)
        if ((**++argv == '-' || **argv == '/') && (*argv)[1])
#else
            if ((**++argv == '-') && (*argv)[1])
#endif
                while (*++*argv)
                    switch (**argv) {

                    case 'N': case 'n':
                        samples = strtol (++*argv, argv, 10);

                        --*argv;
                        break;

                    case 'D': case 'd':
                        decode = 1;
                        encode = 0;
                        break;

                    case 'E': case 'e':
                        encode = 1;
                        decode = 0;
                        break;

                    case 'H': case 'h':
                        asked_help = 0;
                        break;

                    case 'Y': case 'y':
                        overwrite = 1;
                        break;

                    default:
                        fprintf (stderr, "\nillegal option: %c !\n", **argv);
                        return 1;
                    }
            else if (!infilename) {
                infilename = malloc (strlen (*argv) + 10);
                strcpy (infilename, *argv);
            }
            else if (!outfilename) {
                outfilename = malloc (strlen (*argv) + 10);
                strcpy (outfilename, *argv);
            }
            else {
                fprintf (stderr, "\nextra unknown argument: %s !\n", *argv);
                return 1;
            }
    }

    if (!outfilename || asked_help) {
        printf ("%s", usage);
        return 0;
    }

    if (!strcmp (infilename, outfilename)) {
        fprintf (stderr, "can't overwrite input file (specify different/new output file name)\n");
        return -1;
    }

    if (!overwrite && (outfile = fopen (outfilename, "r"))) {
        fclose (outfile);
        fprintf (stderr, "output file \"%s\" exists (use -y to overwrite)\n", outfilename);
        return -1;
    }


    FILE *infile;

    if (!(infile = fopen (infilename, "rb"))) {
        fprintf (stderr, "can't open file \"%s\" for reading!\n", infilename);
        return -1;
    }

    if (!(outfile = fopen (outfilename, "wb"))) {
        fprintf (stderr, "can't open file \"%s\" for writing!\n", outfilename);
        return -1;
    }




    int format = 0, res = 0, bits_per_sample, sample_rate, num_channels;
    unsigned int fact_samples = 0;
    int num_samples = 0;
    RiffChunkHeader riff_chunk_header;
    ChunkHeader chunk_header;
    WaveHeader WaveHeader;


    // read initial RIFF form header

    int wav_flag = 1;
    if (!fread (&riff_chunk_header, sizeof (RiffChunkHeader), 1, infile) ||
        strncmp (riff_chunk_header.ckID, "RIFF", 4) ||
        strncmp (riff_chunk_header.formType, "WAVE", 4)) {
        wav_flag = 0;
        fprintf (stderr, "%d ", __LINE__);
        fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
    }

    // loop through all elements of the RIFF wav header (until the data chuck)

    while (wav_flag) {

        if (!fread (&chunk_header, sizeof (ChunkHeader), 1, infile)) {
            fprintf (stderr, "%d ", __LINE__);
            fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
            return -1;
        }

        little_endian_to_native (&chunk_header, ChunkHeaderFormat);

        // if it's the format chunk, we want to get some info out of there and
        // make sure it's a .wav file we can handle

        if (!strncmp (chunk_header.ckID, "fmt ", 4)) {
            int supported = 1;

            if (chunk_header.ckSize < 16 || chunk_header.ckSize > sizeof (WaveHeader) ||
                !fread (&WaveHeader, chunk_header.ckSize, 1, infile)) {
                fprintf (stderr, "%d ", __LINE__);
                fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                return -1;
            }

            little_endian_to_native (&WaveHeader, WaveHeaderFormat);

            // 获取文件格式 PCM 0x1，ADPCM 0x11，EXTENSIBLE 0xfffe
            format = (WaveHeader.FormatTag == WAVE_FORMAT_EXTENSIBLE && chunk_header.ckSize == 40) ?
                WaveHeader.SubFormat : WaveHeader.FormatTag;

            bits_per_sample = (chunk_header.ckSize == 40 && WaveHeader.Samples.ValidBitsPerSample) ?
                WaveHeader.Samples.ValidBitsPerSample : WaveHeader.BitsPerSample;

            if (WaveHeader.NumChannels != 1)
                supported = 0;
            else if (format == WAVE_FORMAT_PCM) {
                if (decode) {
                    fprintf (stderr, "%d ", __LINE__);
                    fprintf (stderr, "\"%s\" is PCM .WAV file, invalid in decode-only mode!\n", infilename);
                    return -1;
                }

                if (bits_per_sample != 16) // 位宽
                    supported = 0;

                if (WaveHeader.BlockAlign != 2) // 样点size
                    supported = 0;
            }
            else if (format == WAVE_FORMAT_IMA_ADPCM) {
                if (encode) {
                    fprintf (stderr, "%d ", __LINE__);
                    fprintf (stderr, "\"%s\" is ADPCM .WAV file, invalid in encode-only mode!\n", infilename);
                    return -1;
                }

                if (bits_per_sample != 4)
                    supported = 0;

                // ????
                if (WaveHeader.Samples.SamplesPerBlock != WaveHeader.BlockAlign * 2) {
                    fprintf (stderr, "%d ", __LINE__);
                    fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                    return -1;
                }
            }
            else
                supported = 0;

            if (!supported) {
                fprintf (stderr, "\"%s\" is an unsupported .WAV format!\n", infilename);
                return -1;
            }

            if (verbosity > 0) {
                fprintf (stderr, "format tag size = %d\n", chunk_header.ckSize);
                fprintf (stderr, "FormatTag = 0x%x, NumChannels = %d, BitsPerSample = %d\n",
                         WaveHeader.FormatTag, WaveHeader.NumChannels, WaveHeader.BitsPerSample);
                fprintf (stderr, "BlockAlign = %d, SampleRate = %d, BytesPerSecond = %d\n",
                         WaveHeader.BlockAlign, WaveHeader.SampleRate, WaveHeader.BytesPerSecond);

                if (chunk_header.ckSize > 16) {
                    if (format == WAVE_FORMAT_PCM)
                        fprintf (stderr, "cbSize = %d, ValidBitsPerSample = %d\n", WaveHeader.cbSize,
                                 WaveHeader.Samples.ValidBitsPerSample);
                    else if (format == WAVE_FORMAT_IMA_ADPCM)
                        fprintf (stderr, "cbSize = %d, SamplesPerBlock = %d\n", WaveHeader.cbSize,
                                 WaveHeader.Samples.SamplesPerBlock);
                }

                if (chunk_header.ckSize > 20)
                    fprintf (stderr, "ChannelMask = %x, SubFormat = %d\n",
                             WaveHeader.ChannelMask, WaveHeader.SubFormat);
            }
        }
        else if (!strncmp (chunk_header.ckID, "fact", 4)) {

            if (chunk_header.ckSize < 4 || !fread (&fact_samples, sizeof (fact_samples), 1, infile)) {
                fprintf (stderr, "%d ", __LINE__);
                fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                return -1;
            }

            if (chunk_header.ckSize > 4) {
                int bytes_to_skip = chunk_header.ckSize - 4;
                char dummy;

                while (bytes_to_skip--)
                    if (!fread (&dummy, 1, 1, infile)) {
                        fprintf (stderr, "%d ", __LINE__);
                        fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                        return -1;
                    }
            }
        }
        else if (!strncmp (chunk_header.ckID, "data", 4)) {

            // on the data chunk, get size and exit parsing loop

            if (!WaveHeader.NumChannels) {      // make sure we saw a "fmt" chunk...
                fprintf (stderr, "%d ", __LINE__);
                fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                return -1;
            }

            if (!chunk_header.ckSize) {
                fprintf (stderr, "this .WAV file has no audio samples, probably is corrupt!\n");
                return -1;
            }

            if (format == WAVE_FORMAT_PCM) {
                if (chunk_header.ckSize % WaveHeader.BlockAlign) {
                    fprintf (stderr, "%d ", __LINE__);
                    fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                    return -1;
                }

                num_samples = chunk_header.ckSize / WaveHeader.BlockAlign;
            }
            else {
                int complete_blocks = chunk_header.ckSize / WaveHeader.BlockAlign;
                int leftover_bytes = chunk_header.ckSize % WaveHeader.BlockAlign;
                int samples_last_block;

                num_samples = complete_blocks * WaveHeader.Samples.SamplesPerBlock;


            }
            if (!num_samples) {
                fprintf (stderr, "this .WAV file has no audio samples, probably is corrupt!\n");
                return -1;
            }

            if (verbosity > 0)
                fprintf (stderr, "num samples = %lu\n", (unsigned long) num_samples);

            num_channels = WaveHeader.NumChannels;
            sample_rate = WaveHeader.SampleRate;
            break;
        }
        else {          // just ignore unknown chunks
            int bytes_to_eat = (chunk_header.ckSize + 1) & ~1L;
            char dummy;

            if (verbosity > 0)
                fprintf (stderr, "extra unknown chunk \"%c%c%c%c\" of %d bytes\n",
                         chunk_header.ckID [0], chunk_header.ckID [1], chunk_header.ckID [2],
                         chunk_header.ckID [3], chunk_header.ckSize);

            while (bytes_to_eat--)
                if (!fread (&dummy, 1, 1, infile)) {
                    fprintf (stderr, "%d ", __LINE__);
                    fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                    return -1;
                }
        }
    }

    if (format == WAVE_FORMAT_PCM) {
        int block_size, samples_per_block;

        block_size = 256; // 16000 mono : 256

        samples_per_block = block_size * 2; // ???

        num_samples -= num_samples % samples_per_block;

        if (!write_adpcm_wav_header (outfile, num_channels, num_samples, sample_rate, samples_per_block)) {
            fprintf (stderr, "can't write header to file \"%s\" !\n", outfilename);
            return -1;
        }

        if (verbosity >= 0) fprintf (stderr, "encoding PCM file \"%s\" to%sADPCM file \"%s\"...\n",
                                     infilename, overwrite ? " raw " : " ", outfilename);

        res = adpcm_encode_data (infile, outfile, num_samples, samples_per_block);
    }
    else if (format == WAVE_FORMAT_IMA_ADPCM) {
        if (!(overwrite) && !write_pcm_wav_header (outfile, num_channels, num_samples, sample_rate)) {
            fprintf (stderr, "can't write header to file \"%s\" !\n", outfilename);
            return -1;
        }

        if (verbosity >= 0) fprintf (stderr, "decoding ADPCM file \"%s\" to%sPCM file \"%s\"...\n",
                                     infilename, overwrite ? " raw " : " ", outfilename);

        res = adpcm_decode_data (infile, outfile, num_samples, WaveHeader.BlockAlign);
    }


    if (wav_flag == 0) {
        fclose (infile);
        if (!(infile = fopen (infilename, "rb"))) {
            fprintf (stderr, "can't open file \"%s\" for reading!\n", infilename);
            return -1;
        }

        if (encode) {
            int block_size = samples * sizeof(short);
            short *in_buff = malloc(block_size);
            int out_size = block_size / 4;
            short *out_buff = malloc(out_size);



            if (!in_buff || !out_buff) {
                fprintf (stderr, "could not allocate memory for buffers!\n");
                return -1;
            }

            while (1) {
                if (!fread (in_buff, block_size, 1, infile)) {
                    fprintf (stderr, "\"%s\" Encode Stop!\n", infilename);
                    return -1;
                }

                Pcm2Adpcm(in_buff, out_buff, samples);

                if (!fwrite (out_buff, out_size, 1, outfile)) {
                    fprintf (stderr, "\rcould not write all audio data to output file!\n");
                    return -1;
                }
            }

        } else if (decode) {

            int block_size = samples / 2;
            short *in_buff = malloc(block_size);
            int out_size = samples * sizeof(short);
            short *out_buff = malloc(out_size);

            if (!in_buff || !out_buff) {
                fprintf (stderr, "could not allocate memory for buffers!\n");
                return -1;
            }

            while (1) {
                if (!fread (in_buff, block_size, 1, infile)) {
                    fprintf (stderr, "\"%s\" Decode Stop!\n", infilename);
                    return -1;
                }

                Adpcm2Pcm(in_buff, out_buff, samples);

                if (!fwrite (out_buff, out_size, 1, outfile)) {
                    fprintf (stderr, "\rcould not write all audio data to output file!\n");
                    return -1;
                }
            }
        }
    }


    fclose (outfile);
    fclose (infile);


    return 0;
}









