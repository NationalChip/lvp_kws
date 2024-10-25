#ifndef _GX_AUDIO_OUT_V1_H__
#define _GX_AUDIO_OUT_V1_H__

typedef enum {
	AOUT_PCM_0 = 0,
	AOUT_PCM_1 = 1,
	AOUT_PCM_2 = 2,
	AOUT_I2SIN = 3,
	AOUT_DUMP  = 4
} GX_AUDIO_OUT_HANDLE;

typedef enum {
	BIT_32,
	BIT_24,
	BIT_20,
	BIT_16,
} GX_AUDIO_OUT_PCM_BIT;

typedef enum {
	SAMPLERATE_48K,
	SAMPLERATE_44K,
	SAMPLERATE_32K,
	SAMPLERATE_24K,
	SAMPLERATE_22K,
	SAMPLERATE_16K,
	SAMPLERATE_11K,
	SAMPLERATE_8K,
} GX_AUDIO_OUT_SAMPLE_RATE;

typedef enum {
	SIGNAL_CHANNEL,
	DOUBLE_CHANNLE
} GX_AUDIO_OUT_CHANNELS;

typedef enum {
	CHANNEL_LEFT = 0,
	CHANNEL_RIGHT
} GX_AUDIO_OUT_CHANNEL_SEL;

typedef enum {
	OUT_I2S   = (1 << 0),
	OUT_SPDIF = (1 << 1)
} GX_AUDIO_OUT_OUT_MODE;

typedef enum {
	ENDIAN_BIG = 0,
	ENDIAN_LITTLE
} GX_AUDIO_OUT_ENDIAN;

typedef enum {
	SPD_OFF,
	SPD_IDLE,
	SPD_FROM_I2S
} GX_AUDIO_OUT_SPD_MODE;

typedef enum {
	SPD_RUNNING,
	SPD_STOP
} GX_AUDIO_OUT_SPD_STATUS;

typedef enum {
	GRADUAL_MODE,
	DIRECT_MODE,
} GX_AUDIO_OUT_VOLUME_MODE;

typedef enum {
	UNINTERLACE,
	INTERLACE
} GX_AUDIO_OUT_STORE_MODE;

typedef enum {
	SRC_ENABLE,
	SRC_BYPASS
} GX_AUDIO_OUT_SRC_ENABLE;

typedef enum {
	SOURCE_PCM = 0,
	SOURCE_I2S
} GX_AUDIO_OUT_DUMP_SOURCE;

typedef enum {
	LOW_THRESHOLD  = 0,
	HIGH_THRESHOLD = 1,
} GX_AUDIO_OUT_DAC_THRESHOLD;

typedef struct {
	GX_AUDIO_OUT_DAC_THRESHOLD threshold;
	unsigned int value;
} GX_AUDIO_OUT_DAC_PARA;

typedef struct {
	GX_AUDIO_OUT_OUT_MODE   outMode;      //输出方式
	GX_AUDIO_OUT_SPD_MODE   spdMode;      //spdif的工作模式
	GX_AUDIO_OUT_SPD_STATUS spdStatus;    //spdif是否停止数据输出
} GX_AUDIO_OUT_PORT_CONFIG;

typedef struct {
	int spdMute;
	int i2sMute;
} GX_AUDIO_OUT_MUTE;

typedef struct {
	int volumedB;
	GX_AUDIO_OUT_VOLUME_MODE mode;
} GX_AUDIO_OUT_VOLUME;

typedef struct {
	GX_AUDIO_OUT_CHANNELS    channelNum;  //混音后录制到内存的声道数
	GX_AUDIO_OUT_ENDIAN      endian;      //混音后录制到内存的数据存储大小端
	GX_AUDIO_OUT_STORE_MODE  interlace;   //通路数据存储格式 交织和非交织
	GX_AUDIO_OUT_DUMP_SOURCE source;
#if (defined CONFIG_ARCH_LEO)
	GX_AUDIO_OUT_SAMPLE_RATE sampleRate;  //录制到内存数据采样率选择
#endif
	int frameSamples;             //录制时一帧单声道的采样点数
} GX_AUDIO_OUT_DUMP_CONFIG;

typedef struct {
	GX_AUDIO_OUT_CHANNELS    channelNum;  //通路声道数选择
	GX_AUDIO_OUT_PCM_BIT     bit;         //通路数据长度选择
	GX_AUDIO_OUT_ENDIAN      endian;      //通路内数据大小端存储选择
	GX_AUDIO_OUT_STORE_MODE  interlace;   //通路数据存储格式 交织和非交织
	GX_AUDIO_OUT_SRC_ENABLE  srcEnable;   //通路是否进行采样率转换
	GX_AUDIO_OUT_SAMPLE_RATE sampleRate;  //通路内数据采样率选择
	GX_AUDIO_OUT_VOLUME      volume;

	int frameSamples;             //新一帧单声道采样点数 帧长 = 采样点数 × 16 / 8 × 声道数  采样点数必须大于512
} GX_AUDIO_OUT_PLAY_CONFIG;

typedef enum {
	BUFFER_MODE_USER,
	BUFFER_MODE_KERN,
} GX_AUDIO_OUT_BUFFER_MODE;

typedef struct {
	int size;
	int fullGate;     //通路内数据满门限
	int emptyGate;    //通路内数据空门限
} GX_AUDIO_OUT_BUFFER_KERN;

typedef struct {
	void* bufferSaddr;
	int bufferSize;
} GX_AUDIO_OUT_BUFFER_USER;

typedef struct {
	GX_AUDIO_OUT_BUFFER_MODE mode;
	union {
		GX_AUDIO_OUT_BUFFER_USER user;
		GX_AUDIO_OUT_BUFFER_KERN kern;
	};
} GX_AUDIO_OUT_BUFFER_CONFIG;

typedef int (*GX_AUDIO_OUT_PLAY_CB)(unsigned int sdc_addr, void *priv);

typedef struct {
	union {
		GX_AUDIO_OUT_DUMP_CONFIG  dump;
		GX_AUDIO_OUT_PLAY_CONFIG  play;
	};
	GX_AUDIO_OUT_PORT_CONFIG   port;
	GX_AUDIO_OUT_BUFFER_CONFIG buffer;

	GX_AUDIO_OUT_PLAY_CB playCallBack;
} GX_AUDIO_OUT_CONFIG;

int gx_audio_out_init         (void);
int gx_audio_out_deinit       (void);
int gx_audio_out_open         (GX_AUDIO_OUT_HANDLE handle);
int gx_audio_out_close        (GX_AUDIO_OUT_HANDLE handle);
int gx_audio_out_config       (GX_AUDIO_OUT_HANDLE handle, GX_AUDIO_OUT_CONFIG *config);
int gx_audio_out_run          (GX_AUDIO_OUT_HANDLE handle);
int gx_audio_out_stop         (GX_AUDIO_OUT_HANDLE handle);
int gx_audio_out_pause        (GX_AUDIO_OUT_HANDLE handle);
int gx_audio_out_resume       (GX_AUDIO_OUT_HANDLE handle);
int gx_audio_out_read         (GX_AUDIO_OUT_HANDLE handle, void *buf, int size);
int gx_audio_out_write        (GX_AUDIO_OUT_HANDLE handle, void *buf, int size);
int gx_audio_out_set_mute      (GX_AUDIO_OUT_HANDLE handle, GX_AUDIO_OUT_MUTE  mute);
int gx_audio_out_get_mute      (GX_AUDIO_OUT_HANDLE handle, GX_AUDIO_OUT_MUTE *mute);
int gx_audio_out_set_volume    (GX_AUDIO_OUT_HANDLE handle, GX_AUDIO_OUT_VOLUME  volume);
int gx_audio_out_get_volume    (GX_AUDIO_OUT_HANDLE handle, GX_AUDIO_OUT_VOLUME *volume);
int gx_audio_out_set_dacpara   (GX_AUDIO_OUT_HANDLE handle, GX_AUDIO_OUT_DAC_PARA para);
int gx_audio_out_set_channelsel(GX_AUDIO_OUT_HANDLE handle, GX_AUDIO_OUT_CHANNEL_SEL left, GX_AUDIO_OUT_CHANNEL_SEL right);
int gx_audio_out_play_frame    (GX_AUDIO_OUT_HANDLE handle, void* frame, int size);

#endif

