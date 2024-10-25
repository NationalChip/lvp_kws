#include <opus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <interf_dec.h>
#include <dec_if.h>
#include <enc_if.h>
#include <if_rom.h>
#include <interf_enc.h>
#include <string.h>


#define MODE_SILK_ONLY 1000
#define OPUS_SET_FORCE_MODE_REQUEST 11002
#define OPUS_SET_FORCE_MODE(x) OPUS_SET_FORCE_MODE_REQUEST, x

struct {
    char riff[4];
    unsigned int riff_size;
    char riff_format[4];
    char format[4];
    unsigned int format_size;
    unsigned short format_tag;
    unsigned short channels;
    unsigned int sample_rate;
    unsigned int bytes_rate;
    unsigned short block_align;
    unsigned short bits;
    char data_tag[4];
    unsigned int data_size;
} voice_header = {
    .riff = {'R', 'I', 'F', 'F'},
    .riff_size = 0,
    .riff_format = {'O', 'P', 'U', 'S'},
    .format = {'f', 'm', 't', ' '},
    .format_size = 0x10, // 16
    .format_tag = 0x08,
    .channels = 1,
    .sample_rate = 16000,
    .bytes_rate = 16000,
    .block_align = 2,
    .bits = 16,
    .data_tag = {'d', 'a', 't', 'a'},
    .data_size = 0,
};



#define MAX_PACKET 1500
int opus_enc(int sampleRate, char *wavfile, char *opusfile)
{
    int application = OPUS_APPLICATION_AUDIO;
    int sampling_rate = sampleRate;
    int frame_size = sampling_rate / 50;
    int channels = 1;
    int bitrate_bps = 16000;
    int use_vbr = 0;
    int max_payload_bytes = MAX_PACKET;
    int complexity = 10;
    int use_inbandfec = 0;
    int forcechannels = 1;
    int use_dtx = 0;
    int packet_loss_perc = 0;
    int data_size = 0;
    opus_int32 skip = 0;
    FILE *fin = fopen(wavfile, "rb");
    FILE *fout = fopen(opusfile, "wb");
    if (fin == NULL || fout == NULL)
        return -1;

    int err;
    OpusEncoder *enc = opus_encoder_create(sampling_rate, channels, application, &err);
    if (err != OPUS_OK)
        return -1;
    opus_encoder_ctl(enc, OPUS_SET_BITRATE(bitrate_bps));
//    opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(bandwidth));
    opus_encoder_ctl(enc, OPUS_SET_VBR(use_vbr));
    opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(complexity));
    opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(use_inbandfec));
    opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS(forcechannels));
    opus_encoder_ctl(enc, OPUS_SET_DTX(use_dtx));
    opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(packet_loss_perc));
    opus_encoder_ctl(enc, OPUS_GET_LOOKAHEAD(&skip));
    opus_encoder_ctl(enc, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_20_MS));
    opus_encoder_ctl(enc, OPUS_SET_FORCE_MODE(MODE_SILK_ONLY));
    short in[frame_size];
    char out[MAX_PACKET];
    fseek(fin, sizeof(voice_header), SEEK_SET);
    fwrite(&voice_header, 1, sizeof(voice_header), fout);

    while(1) {
        int num_read = fread(in, sizeof(short), frame_size, fin);
        if (num_read == 0)
            break;
        if (num_read != frame_size) {
            for (int i = num_read; i < (frame_size - num_read); i++)
                in[i] = 0;
        }
        int len = opus_encode(enc, in, frame_size, out, max_payload_bytes);
        if (len <= 0)
            break;
        len = fwrite(out, sizeof(char), len, fout);
        data_size += len;
    }
    voice_header.sample_rate = sampleRate;
    voice_header.bytes_rate = bitrate_bps;
    voice_header.format_tag = 0x08; //opus
    voice_header.channels = channels;
    voice_header.riff_size = data_size + sizeof(voice_header) - 8;
    voice_header.data_size = data_size;
    fseek(fout, 0, SEEK_SET);
    fwrite(&voice_header, 1, sizeof(voice_header), fout);
}

int opus_dec(int sampleRate, char *opusfile, char *wavfile)
{
    int sampling_rate = sampleRate;
    int frame_size = sampling_rate / 50;
    int channels = 1;
    int opus_size = 40;
    short out[frame_size];
    char in[opus_size];
    int data_size = 0;

    FILE *fin = fopen(opusfile, "rb");
    FILE *fout = fopen(wavfile, "wb");
    if (fin == NULL || fout == NULL) {
        printf("%s fopen file fail\n", __func__);
        return -1;
    }

    int err;
    OpusDecoder *dec = opus_decoder_create(sampling_rate, channels, &err);
    if (err != OPUS_OK) {
        printf("%s create decoder fail\n", __func__);
        return -1;
    }
    fseek(fin, sizeof(voice_header), SEEK_SET);
    fwrite(&voice_header, 1, sizeof(voice_header), fout);

    while(1) {
        int num_read = fread(in, sizeof(char), opus_size, fin);
        if (num_read != opus_size) {
            break;
        }
        int len = opus_decode(dec, in, opus_size, out, frame_size, 0);
        if (len <= 0)
            break;
        len = fwrite(out, sizeof(short), len, fout);
        data_size += len * sizeof(short);
    }
    voice_header.riff_format[0] = 'W';
    voice_header.riff_format[1] = 'A';
    voice_header.riff_format[2] = 'V';
    voice_header.riff_format[3] = 'E';
    voice_header.sample_rate = sampleRate;
    voice_header.bytes_rate = sampleRate * channels * 16 / 8;
    voice_header.channels = channels;
    voice_header.format_tag = 0x01;
    voice_header.riff_size = data_size + sizeof(voice_header) - 8;
    voice_header.data_size = data_size;
    fseek(fout, 0, SEEK_SET);
    fwrite(&voice_header, 1, sizeof(voice_header), fout);
}

int amr_nb_dec(char *amrfile, char *wavfile)
{
    int size = 12;
	FILE *fin;
    FILE *fout;
	char header[6];
	int n;
	void *amr;
    short out[160] = {0};
    char in[200] = {0};
    int data_size = 0;
	fin = fopen(amrfile, "rb");
    fout = fopen(wavfile, "wb");
	if (!fin || !fout) {
        printf("%s fopen file fail\n", __func__);
		return 1;
	}

    fread(&voice_header, 1, sizeof(voice_header), fin);

    if (strncmp(voice_header.riff_format, "AMR ", 4) != 0 || voice_header.format_tag != 0x09) {
        printf("%s is not amr file\n", amrfile);
        return -1;
    }

    if (voice_header.sample_rate != 8000 || voice_header.data_size <= 0 || voice_header.channels != 1 || voice_header.bytes_rate != 4750) {
        printf("%s can't decode this format amr\n", __func__);
        return -1;
    }
    fwrite(&voice_header, 1, sizeof(voice_header), fout);
	amr = Decoder_Interface_init();
	while (1) {
        memset(out, 0, sizeof(out));
        memset(in, 0, sizeof(in));
		/* Read the mode byte */
		n = fread(in, 1, 1, fin);
		if (n <= 0)
			break;
		/* Find the packet size */
        if (((in[0] >> 3) & 0x0f) != 0) {
            printf("%s unsupport bits rate\n", __func__);
            break;
        }
		n = fread(in + 1, 1, size, fin);
		if (n != size)
			break;

		/* Decode the packet */
		Decoder_Interface_Decode(amr, in, out, 0);
        fwrite(out, sizeof(short), sizeof(out)/sizeof(short), fout);
        data_size += sizeof(out);

		/* Convert to little endian and write to wav */
	}
    voice_header.riff_format[0] = 'W';
    voice_header.riff_format[1] = 'A';
    voice_header.riff_format[2] = 'V';
    voice_header.riff_format[3] = 'E';
    voice_header.sample_rate = 8000;
    voice_header.bytes_rate = 16000;
    voice_header.channels = 1;
    voice_header.format_tag = 0x01;
    voice_header.riff_size = data_size + sizeof(voice_header) - 8;
    voice_header.data_size = data_size;
    fseek(fout, 0, SEEK_SET);
    fwrite(&voice_header, 1, sizeof(voice_header), fout);
	fclose(fin);
	fclose(fout);
	Decoder_Interface_exit(amr);
	return 0;
}

int amr_nb_enc(char *wavfile, char *amrfile)
{
	FILE *fout;
    FILE *fin;
	void *amr;
    short in[160];
    const int size = 12;
    char out[200];
    int data_size = 0;

    fin = fopen(wavfile, "rb");
    fout = fopen(amrfile, "wb");
    if (!fin || !fout) {
        printf("%s open file fail\n", __func__);
        return -1;
    }

	amr = Encoder_Interface_init(0);

    fread(&voice_header, 1, sizeof(voice_header), fin);

    if (strncmp(voice_header.riff_format, "WAVE", 4) != 0 || voice_header.format_tag != 0x01) {
        printf("%s is not wav file\n", amrfile);
        return -1;
    }

    if (voice_header.sample_rate != 8000 || voice_header.data_size <= 0 || voice_header.channels != 1) {
        printf("%s can't decode this format amr\n", __func__);
        return -1;
    }
    fwrite(&voice_header, 1, sizeof(voice_header), fout);
    if (amr == NULL) {
        printf("%s get encoder fail\n", __func__);
        return -1;
    }

	while (1) {
        int len = fread(in, sizeof(short), sizeof(in)/sizeof(short), fin);
        if (len != (sizeof(in)/sizeof(short)))
            break;
		len = Encoder_Interface_Encode(amr, 0, in, out, 0);
        data_size += len;
        if ((data_size % (size + 1)) != 0)
            printf("%s [WARNING] encode data may not correct\n", __func__);
		fwrite(out, 1, len, fout);
	}
    voice_header.riff_format[0] = 'A';
    voice_header.riff_format[1] = 'M';
    voice_header.riff_format[2] = 'R';
    voice_header.riff_format[3] = ' ';
    voice_header.sample_rate = 8000;
    voice_header.bytes_rate = 4750;
    voice_header.channels = 1;
    voice_header.format_tag = 0x09;
    voice_header.riff_size = data_size + sizeof(voice_header) - 8;
    voice_header.data_size = data_size;
    fseek(fout, 0, SEEK_SET);
    fwrite(&voice_header, 1, sizeof(voice_header), fout);
	fclose(fout);
    fclose(fin);
	Encoder_Interface_exit(amr);

	return 0;

}

int amr_wb_dec(char *amrfile, char *wavfile)
{
    int size = 17;
	FILE *fin;
    FILE *fout;
	char header[6];
	int n;
	void *amr;
    short out[320] = {0};
    char in[200] = {0};
    int data_size = 0;
	fin = fopen(amrfile, "rb");
    fout = fopen(wavfile, "wb");
	if (!fin || !fout) {
        printf("%s fopen file fail\n", __func__);
		return 1;
	}

    fread(&voice_header, 1, sizeof(voice_header), fin);

    if (strncmp(voice_header.riff_format, "AMR ", 4) != 0 || voice_header.format_tag != 0x09) {
        printf("%s is not amr file\n", amrfile);
        return -1;
    }

    if (voice_header.sample_rate != 16000 || voice_header.data_size <= 0 || voice_header.channels != 1 || voice_header.bytes_rate != 6600) {
        printf("%s can't decode this format amr\n", __func__);
        return -1;
    }
    fwrite(&voice_header, 1, sizeof(voice_header), fout);
	amr = D_IF_init();
	while (1) {
        memset(out, 0, sizeof(out));
        memset(in, 0, sizeof(in));
		/* Read the mode byte */
		n = fread(in, 1, 1, fin);
		if (n <= 0)
			break;
		/* Find the packet size */
        if (((in[0] >> 3) & 0x0f) != 0) {
            printf("%s unsupport bits rate\n", __func__);
            break;
        }
		n = fread(in + 1, 1, size, fin);
		if (n != size)
			break;

		/* Decode the packet */
		D_IF_decode(amr, in, out, 0);
        fwrite(out, sizeof(short), sizeof(out)/sizeof(short), fout);
        data_size += sizeof(out);

		/* Convert to little endian and write to wav */
	}
    voice_header.riff_format[0] = 'W';
    voice_header.riff_format[1] = 'A';
    voice_header.riff_format[2] = 'V';
    voice_header.riff_format[3] = 'E';
    voice_header.sample_rate = 16000;
    voice_header.bytes_rate = 32000;
    voice_header.channels = 1;
    voice_header.format_tag = 0x01;
    voice_header.riff_size = data_size + sizeof(voice_header) - 8;
    voice_header.data_size = data_size;
    fseek(fout, 0, SEEK_SET);
    fwrite(&voice_header, 1, sizeof(voice_header), fout);
	fclose(fin);
	fclose(fout);
	D_IF_exit(amr);
	return 0;
}

int amr_wb_enc(char *wavfile, char *amrfile)
{
	FILE *fout;
    FILE *fin;
	void *amr;
    short in[320];
    const int size = 17;
    char out[200];
    int data_size = 0;

    fin = fopen(wavfile, "rb");
    fout = fopen(amrfile, "wb");
    if (!fin || !fout) {
        printf("%s open file fail\n", __func__);
        return -1;
    }

	amr = E_IF_init();
    if (amr == NULL) {
        printf("%s get encoder fail\n", __func__);
        return -1;
    }

    fread(&voice_header, 1, sizeof(voice_header), fin);

    if (strncmp(voice_header.riff_format, "WAVE", 4) != 0 || voice_header.format_tag != 0x01) {
        printf("%s is not wav file\n", amrfile);
        return -1;
    }

    if (voice_header.sample_rate != 16000 || voice_header.data_size <= 0 || voice_header.channels != 1) {
        printf("%s can't encode this format amr\n", __func__);
        return -1;
    }
    fwrite(&voice_header, 1, sizeof(voice_header), fout);
    if (amr == NULL) {
        printf("%s get encoder fail\n", __func__);
        return -1;
    }

	while (1) {
        int len = fread(in, sizeof(short), sizeof(in)/sizeof(short), fin);
        if (len != (sizeof(in)/sizeof(short)))
            break;
		len = E_IF_encode(amr, 0, in, out, 0);
        data_size += len;
        if ((data_size % (size + 1)) != 0)
            printf("%s [WARNING] encode data may not correct\n", __func__);
		fwrite(out, 1, len, fout);
	}
    voice_header.riff_format[0] = 'A';
    voice_header.riff_format[1] = 'M';
    voice_header.riff_format[2] = 'R';
    voice_header.riff_format[3] = ' ';
    voice_header.sample_rate = 16000;
    voice_header.bytes_rate = 6600;
    voice_header.channels = 1;
    voice_header.format_tag = 0x09;
    voice_header.riff_size = data_size + sizeof(voice_header) - 8;
    voice_header.data_size = data_size;
    fseek(fout, 0, SEEK_SET);
    fwrite(&voice_header, 1, sizeof(voice_header), fout);
	fclose(fout);
    fclose(fin);
	E_IF_exit(amr);

	return 0;
}

int main(int chrc, char *argv[])
{
    if (chrc != 6) {
        printf("%s [opus|amr] [dec|enc] [sampleRate(8000|16000)] [infile(encode file|decode file)] [outfile(decode file|encode file)]\n", argv[0]);
        return -1;
    }
    printf("%s %s %s %s\n", argv[1], argv[2], argv[4], argv[5]);
    if (strcmp(argv[1], "opus") == 0)
    {
        if (strcmp(argv[2], "dec") == 0)
            opus_dec(atoi(argv[3]), argv[4], argv[5]);
        else if (strcmp(argv[2], "enc") == 0)
            opus_enc(atoi(argv[3]), argv[4], argv[5]);
        else
            printf("%s [opus|amr] [dec|enc] [sampleRate(8000|16000)] [infile(encode file|decode file)] [outfile(decode file|encode file)]\n", argv[0]);
    }
    else if (strcmp(argv[1], "amr") == 0)
    {
        if (strcmp(argv[2], "dec") == 0)
        {
            if (atoi(argv[3]) == 8000)
                amr_nb_dec(argv[4], argv[5]);
            else if (atoi(argv[3]) == 16000)
                amr_wb_dec(argv[4], argv[5]);
            else
                printf("unsupport %d sample rate\n", atoi(argv[3]));
        }
        else if (strcmp(argv[2], "enc") == 0)
        {
            if (atoi(argv[3]) == 8000)
                amr_nb_enc(argv[4], argv[5]);
            else if (atoi(argv[3]) == 16000)
                amr_wb_enc(argv[4], argv[5]);
            else
                printf("unsupport %d sample rate\n", atoi(argv[3]));
        }
        else
        {
            printf("%s [opus|amr] [dec|enc] [sampleRate(8000|16000)] [infile(encode file|decode file)] [outfile(decode file|encode file)]\n", argv[0]);
        }
    }
    else
    {
        printf("%s [opus|amr] [dec|enc] [sampleRate(8000|16000)] [infile(encode file|decode file)] [outfile(decode file|encode file)]\n", argv[0]);
    }
    return 0;
}
