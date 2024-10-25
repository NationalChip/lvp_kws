#ifndef __GX_AUDIO_IN_V1_H__
#define __GX_AUDIO_IN_V1_H__

typedef enum {
	AIN_AMIC = 0,
	AIN_DMIC,
	AIN_OUT_ADC,
	AIN_IN_I2S,
	AIN_OUT_I2S,
	AIN_NULL = 0xff
} GX_AUDIO_IN_SOURCE;

typedef enum {
	AIN_SAMPLE_RATE_16KHZ,
	AIN_SAMPLE_RATE_08KHZ,
	AIN_SAMPLE_RATE_48KHZ,
} GX_AUDIO_IN_SAMPLE_RATE;

typedef enum {
	AIN_AVAD_FREEZE = 0,
	AIN_AVAD_ACTIVE,
} GX_AUDIO_IN_AVAD_STATE;

#ifdef CONFIG_ARCH_LEO
typedef enum {
	AIN_BIG_ENDIAN = 0,
	AIN_LITTLE_ENDIAN = 1
} GX_AUDIO_IN_ENDIAN;
#elif defined CONFIG_ARCH_LEO_MINI
typedef enum {
	AIN_16BIT = 0,
	AIN_24BIT_LOWEST  = 1,
	AIN_24BIT_HIGHEST = 2,
} GX_AUDIO_IN_BIT_SIZE;

typedef enum {
	AIN_16BIT_LITTLE_ENDIAN = 0,
	AIN_24BIT_LITTLE_ENDIAN = 0,
	AIN_24BIT_BIG_ENDIAN = 1,
	AIN_16BIT_BIG_ENDIAN = 3
} GX_AUDIO_IN_ENDIAN;
#endif

typedef enum {
	AIN_AVAD_ITPARA_HARD,
	AIN_AVAD_ITPARA_SOFT
} GX_AUDIO_IN_AVAD_ITPARA_MODE;

typedef enum {
	AIN_AVAD_ZCR_ENABLE,
	AIN_AVAD_ZCR_DISABLE
} GX_AUDIO_IN_AVAD_ZCR_MODE;

typedef enum {
	AIN_AGC_DISABLE = 0,
	AIN_AGC_ENABLE
} GX_AUDIO_IN_AGC_ENABLE_CTRL;

typedef enum {
	AIN_AGC_AMIC_STEP_2DB = 0,
	AIN_AGC_AMIC_STEP_4DB,
	AIN_AGC_AMIC_STEP_6DB,
	AIN_AGC_AMIC_STEP_8DB,
	AIN_AGC_AMIC_STEP_10DB,
	AIN_AGC_AMIC_STEP_12DB,
} GX_AUDIO_IN_AGC_AMIC_STEP;

typedef enum {
	AIN_AGC_CH_0   = 0,
	AIN_AGC_CH_1   = 1,
	AIN_AGC_CH_2   = 2,
	AIN_AGC_CH_3   = 3,
#ifdef CONFIG_ARCH_LEO
	AIN_AGC_CH_4   = 4,
	AIN_AGC_CH_5   = 5,
	AIN_AGC_CH_6   = 6,
	AIN_AGC_CH_7   = 7,
#endif
	AIN_AGC_ECHO_0 = 8,
	AIN_AGC_ECHO_1 = 9,
} GX_AUDIO_IN_AGC_CH_SEL;

typedef enum {
	AIN_AGC_SOFT = 0,
	AIN_AGC_HARD,
} GX_AUDIO_IN_AGC_UPDATA_MODE;

typedef enum {
	AIN_AVAD_CH_NULL = 0,
	AIN_AVAD_CH_0    = (1 << 0),
	AIN_AVAD_CH_1    = (1 << 1),
	AIN_AVAD_CH_2    = (1 << 2),
	AIN_AVAD_CH_3    = (1 << 3),
	AIN_AVAD_CH_4    = (1 << 4),
	AIN_AVAD_CH_5    = (1 << 5),
	AIN_AVAD_CH_6    = (1 << 6),
	AIN_AVAD_CH_7    = (1 << 7),
	AIN_AVAD_CH_ALL  = 0xff,
} GX_AUDIO_IN_AVAD_CH_EN;

typedef enum {
	AIN_AVAD_ECHO_CH_NULL = 0,
	AIN_AVAD_ECHO_CH_0    = (1 << 0),
	AIN_AVAD_ECHO_CH_1    = (1 << 1),
	AIN_AVAD_ECHO_CH_ALL  = 0x3
} GX_AUDIO_IN_AVAD_ECHO_CH_EN;

typedef enum {
	AIN_AVAD_ADC = 0,
	AIN_AVAD_AIN
} GX_AUDIO_IN_AVAD_DETECT_MODE;

typedef enum {
	AIN_AVAD_ENABLE = 0,
	AIN_AVAD_DISABLE
} GX_AUDIO_IN_AVAD_MODE;

#ifdef CONFIG_ARCH_LEO
typedef enum {
	AIN_DC_DISABLE = 0,
	AIN_DC_ENABLE
} GX_AUDIO_IN_DC_ENABLE_CTRL;

typedef enum {
	AIN_DC_HARD = 0,
	AIN_DC_SOFT,
	AIN_DC_BYPASS
} GX_AUDIO_IN_DC_FILTER_MODE;
#elif defined CONFIG_ARCH_LEO_MINI
typedef enum {
	AIN_DC_ENABLE  = 0x0,
	AIN_DC_DISABLE = 0xf
} GX_AUDIO_IN_DC_CTRL;

typedef enum {
	AIN_DC_HARD = 0x0,
	AIN_DC_SOFT = 0xf,
} GX_AUDIO_IN_DC_MODE;

typedef enum {
	AIN_ECHO_DC_ENABLE  = 0x0,
	AIN_ECHO_DC_DISABLE = 0x3,
} GX_AUDIO_IN_ECHO_DC_CTRL;

typedef enum {
	AIN_ECHO_DC_HARD = 0x0,
	AIN_ECHO_DC_SOFT = 0x3,
} GX_AUDIO_IN_ECHO_DC_MODE;
#endif

typedef enum {
	AIN_DIGITAL_DB_0 = 0,
	AIN_DIGITAL_DB_6,
	AIN_DIGITAL_DB_12,
	AIN_DIGITAL_DB_18,
	AIN_DIGITAL_DB_24,
	AIN_DIGITAL_DB_30,
	AIN_DIGITAL_DB_36,
	AIN_DIGITAL_DB_42,
	AIN_DIGITAL_DB_48,
	AIN_DIGITAL_DB_54
} GX_AUDIO_IN_DIGITAL_GAIN;

typedef enum {
	AIN_I2S_OUT_ADC = 0,
	AIN_I2S_AUDIO_PLAY,
	AIN_I2S_OUT_I2S,
	AIN_I2S_NULL,
} GX_AUDIO_IN_I2S_SRC_SEL;

typedef enum {
	AIN_OUT_I2S_MAGIF_EN = 0,
	AIN_OUT_ADC_MAGIF_EN = 0,
	AIN_IN_I2S_MAGIF_EN  = 0,
	AIN_DMIC_MAGIF_EN = 1,
	AIN_AMIC_MAGIF_EN = 2,
} GX_AUDIO_IN_CONFIG_MAGIF_EN;

typedef enum {
   AIN_ADC_I2S = 0,
   AIN_ADC_LEFT_JUSTIFIED,
   AIN_ADC_RIGHT_JUSTIFIED,
   AIN_ADC_TDM
} GX_AUDIO_IN_ADC_FORMAT;

typedef enum {
	AIN_ADC_SOFT = 0,
	AIN_ADC_HARD,
} GX_AUDIO_IN_ADC_CTRL_MODE;

typedef enum {
	AIN_ADC_FRONT_BOOST_16DB = 0x0000,
	AIN_ADC_FRONT_BOOST_30DB = 0xffff,

	AIN_ADC_FRONT_BOOST_0DB  = 0xffffffff,
} GX_AUDIO_IN_ADC_FRONT_BOOST_GAIN;

typedef enum {
	AIN_ADC_POST_BOOST_0DB,
	AIN_ADC_POST_BOOST_16DB,
	AIN_ADC_POST_BOOST_30DB,
} GX_AUDIO_IN_ADC_POST_BOOST_GAIN;

typedef enum {
	AIN_ADC_PGA_0DB  = 0x00000000,

	AIN_ADC_PGA_2DB  = 0x66666666,
	AIN_ADC_PGA_4DB  = 0x77777777,
	AIN_ADC_PGA_6DB  = 0x88888888,
	AIN_ADC_PGA_8DB  = 0x99999999,
	AIN_ADC_PGA_10DB = 0xaaaaaaaa,
	AIN_ADC_PGA_12DB = 0xbbbbbbbb,
	AIN_ADC_PGA_14DB = 0xcccccccc,
	AIN_ADC_PGA_16DB = 0xdddddddd,
	AIN_ADC_PGA_18DB = 0xeeeeeeee,
	AIN_ADC_PGA_20DB = 0xffffffff,
} GX_AUDIO_IN_ADC_PGA_GAIN;

typedef enum {
	AIN_ADC_CHOPPER_CLK_0    = 0x00,
	AIN_ADC_CHOPPER_CLK_2    = 0x01,
	AIN_ADC_CHOPPER_CLK_4    = 0x02,
	AIN_ADC_CHOPPER_CLK_8    = 0x03,
	AIN_ADC_CHOPPER_CLK_16   = 0x04,
	AIN_ADC_CHOPPER_CLK_32   = 0x05,
	AIN_ADC_CHOPPER_CLK_64   = 0x06,
	AIN_ADC_CHOPPER_CLK_128  = 0x07,
} GX_AUDIO_IN_ADC_CHOPPER_CLK;

typedef enum {
	AIN_ADC_CHOPPER_DISABLE  = 0,
	AIN_ADC_CHOPPER_ENABLE   = 1,
} GX_AUDIO_IN_ADC_CHOPPER_CTRL;

typedef enum {
	AIN_MEM_PINGPONG = 0,
	AIN_MEM_LOOP,
} GX_AUDIO_IN_MEM_TYPE;

typedef struct {
	GX_AUDIO_IN_SOURCE mic;
	GX_AUDIO_IN_SOURCE ref;
} GX_AUDIO_IN_HANDLE;

typedef struct {
	GX_AUDIO_IN_ADC_FRONT_BOOST_GAIN  adcBoostGain;
	GX_AUDIO_IN_ADC_PGA_GAIN          adcPgaGain;
} GX_AUDIO_IN_ADC_FRONT_CTRL;

typedef struct {
	GX_AUDIO_IN_ADC_CTRL_MODE          adcMode;
	GX_AUDIO_IN_ADC_POST_BOOST_GAIN    adcBoostGain;
	GX_AUDIO_IN_ADC_PGA_GAIN           adcPgaGain;
	unsigned int               adcChSel;
} GX_AUDIO_IN_ADC_POST_CTRL;

typedef struct {
#ifdef CONFIG_ARCH_LEO
#define AIN_CHANNEL_NUM_COM  (8)
#define AIN_CHANNEL_NUM_ECHO (2)
#elif defined CONFIG_ARCH_LEO_MINI
#define AIN_CHANNEL_NUM_COM  (4)
#define AIN_CHANNEL_NUM_ECHO (2)
#endif
	struct {
		unsigned int  enable;
		unsigned char *buffer;
	} micChannel[AIN_CHANNEL_NUM_COM];

	struct {
		unsigned int  enable;
		unsigned char *buffer;
	} refChannel[AIN_CHANNEL_NUM_ECHO];
	unsigned int size;
} GX_AUDIO_IN_READ;

typedef struct {
	GX_AUDIO_IN_AVAD_STATE channel[8];
	GX_AUDIO_IN_AVAD_STATE echoChannel[2];
} GX_AUDIO_IN_AVAD_STATUS;

typedef struct {
	GX_AUDIO_IN_DIGITAL_GAIN micGain;

	int        mic0;
	int        mic1;
	int        mic2;
	int        mic3;
	int        mic4;
	int        mic5;
	int        mic6;
	int        mic7;
	char       micChEn;
} GX_AUDIO_IN_MIC_SEL;

typedef struct {
	GX_AUDIO_IN_DIGITAL_GAIN refGain;

	int  refLeft;
	int  refRight;
	char refChEn;
} GX_AUDIO_IN_REF_SEL;

#ifdef CONFIG_ARCH_LEO_MINI
typedef struct {
	GX_AUDIO_IN_BIT_SIZE    bitSize;
	GX_AUDIO_IN_ENDIAN      endian;
	GX_AUDIO_IN_ENDIAN      echoEndian;
	GX_AUDIO_IN_SAMPLE_RATE sampleRate;
} GX_AUDIO_IN_PCM_FORMAT;
#else
typedef struct {
	GX_AUDIO_IN_ENDIAN      endian;
	GX_AUDIO_IN_SAMPLE_RATE sampleRate;
} GX_AUDIO_IN_PCM_FORMAT;
#endif

typedef struct {
	GX_AUDIO_IN_AVAD_ITPARA_MODE  avadItParaMode;
	GX_AUDIO_IN_AVAD_MODE         avadEnable;
	GX_AUDIO_IN_AVAD_ZCR_MODE     avadZcrMode;
	GX_AUDIO_IN_AVAD_CH_EN        avadChSel;
	GX_AUDIO_IN_AVAD_ECHO_CH_EN   avadEchoChSel;
	GX_AUDIO_IN_AVAD_DETECT_MODE  avadChDetectMode;
	GX_AUDIO_IN_AVAD_DETECT_MODE  avadEchoChDetectMode;
} GX_AUDIO_IN_AVAD_CONFIG;

typedef struct {
	GX_AUDIO_IN_AGC_ENABLE_CTRL agcEnableCtrl;
	GX_AUDIO_IN_AGC_CH_SEL      agcChSel;
	GX_AUDIO_IN_AGC_UPDATA_MODE agcUpdataMode;
	unsigned int        agcMax;
	unsigned int        agcMin;
	unsigned int        agcStep;
	unsigned int        agcCheckTime;
} GX_AUDIO_IN_AGC_CONFIG;

#ifdef CONFIG_ARCH_LEO
typedef struct {
	GX_AUDIO_IN_DC_ENABLE_CTRL dcCtrl;
	GX_AUDIO_IN_DC_FILTER_MODE dcMode;
	unsigned int       dcOffset;
} GX_AUDIO_IN_DC_CONFIG;
#elif defined CONFIG_ARCH_LEO_MINI
typedef struct {
	GX_AUDIO_IN_DC_CTRL      dcFrontCtrl;
	GX_AUDIO_IN_DC_CTRL      dcPostCtrl;
	GX_AUDIO_IN_DC_MODE      dcMode;
	GX_AUDIO_IN_ECHO_DC_CTRL dcEchoFrontCtrl;
	GX_AUDIO_IN_ECHO_DC_CTRL dcEchoPostCtrl;
	GX_AUDIO_IN_ECHO_DC_MODE dcEchoMode;
	unsigned int     dcChSel;
	unsigned int     dcOffset;
} GX_AUDIO_IN_DC_CONFIG;
#endif

typedef struct {
#ifdef CONFIG_ARCH_LEO
	unsigned int bufferSaddr;
#elif defined CONFIG_ARCH_LEO_MINI
	unsigned int micBufferSaddr;
	unsigned int refBufferSaddr;
#endif
	unsigned int oneChannelLength;
} GX_AUDIO_IN_BUFFER_CONFIG;

typedef struct {
	GX_AUDIO_IN_ADC_CHOPPER_CTRL   adcChopperCtrl;
	GX_AUDIO_IN_ADC_FRONT_CTRL     adcFrontCtrl;
	GX_AUDIO_IN_ADC_POST_CTRL      adcPostCtrl;
} GX_AUDIO_IN_ADC_CONFIG;

typedef struct {
	unsigned char ch0_delay_level;
	unsigned char ch1_delay_level;
	unsigned char ch2_delay_level;
	unsigned char ch3_delay_level;
	unsigned char echo_ch0_delay_level;
	unsigned char echo_ch1_delay_level;
} GX_AUDIO_IN_SYNC_CONFIG;

typedef struct {
	GX_AUDIO_IN_I2S_SRC_SEL i2sSrcSel;
	GX_AUDIO_IN_ADC_FORMAT  outAdcFmt;
} GX_AUDIO_IN_I2S_CONFIG;

typedef struct {
	unsigned int ch0_vol_level;
	unsigned int ch1_vol_level;
	unsigned int ch2_vol_level;
	unsigned int ch3_vol_level;
	unsigned int ch4_vol_level;
	unsigned int ch5_vol_level;
	unsigned int ch6_vol_level;
	unsigned int ch7_vol_level;
	unsigned int echo_ch0_vol_level;
	unsigned int echo_ch1_vol_level;
} GX_AUDIO_IN_CHANNEL_GAIN;

typedef int (*GX_AUDIO_IN_RECORD_CB)(unsigned int sdcAddr, void *priv);

typedef struct {
	GX_AUDIO_IN_MIC_SEL       micSel;
	GX_AUDIO_IN_REF_SEL       refSel;
	GX_AUDIO_IN_PCM_FORMAT    format;
	GX_AUDIO_IN_MEM_TYPE      memType;
	GX_AUDIO_IN_AVAD_CONFIG   avad;
#ifdef CONFIG_ARCH_LEO
	GX_AUDIO_IN_I2S_CONFIG    i2s;
#endif
	GX_AUDIO_IN_AGC_CONFIG    agc;
	GX_AUDIO_IN_DC_CONFIG     dc;
#ifdef CONFIG_ARCH_LEO_MINI
	GX_AUDIO_IN_SYNC_CONFIG   sync;
#endif
	GX_AUDIO_IN_BUFFER_CONFIG buffer;
	GX_AUDIO_IN_ADC_CONFIG    adc;
	GX_AUDIO_IN_RECORD_CB     recordCallBack;
} GX_AUDIO_IN_CONFIG;

int gx_audio_in_init  (void);
int gx_audio_in_deinit(void);
int gx_audio_in_open  (GX_AUDIO_IN_HANDLE handle);
int gx_audio_in_config(GX_AUDIO_IN_HANDLE handle, GX_AUDIO_IN_CONFIG *config);
int gx_audio_in_run   (GX_AUDIO_IN_HANDLE handle);
int gx_audio_in_stop  (GX_AUDIO_IN_HANDLE handle);
int gx_audio_in_close (GX_AUDIO_IN_HANDLE handle);
int gx_audio_in_get_frame_length(GX_AUDIO_IN_HANDLE handle);
int gx_audio_in_get_avad_status (GX_AUDIO_IN_HANDLE handle, GX_AUDIO_IN_AVAD_STATUS *status);
int gx_audio_in_reset_avad(GX_AUDIO_IN_HANDLE handle);
int gx_audio_in_set_channel_gain(GX_AUDIO_IN_HANDLE handle, GX_AUDIO_IN_CHANNEL_GAIN gain);
int gx_audio_in_suspend(GX_AUDIO_IN_HANDLE handle);
int gx_audio_in_resume(GX_AUDIO_IN_HANDLE handle);
int gx_audio_in_read(GX_AUDIO_IN_HANDLE handle, GX_AUDIO_IN_READ *read);

#endif
