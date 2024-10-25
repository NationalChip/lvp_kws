/** \addtogroup <scpu>
 *  @{
 */
#ifndef __GX_AUDIO_IN_V1_H__
#define __GX_AUDIO_IN_V1_H__
/**
 * @brief 输入源
 */
typedef enum {
	AUDIO_IN_SADC  = (1 << 0), ///< 输入源 SAR ADC
	AUDIO_IN_PDM   = (1 << 1), ///< 输入源 PDM
	AUDIO_IN_I2S   = (1 << 2), ///< 输入源 I2S
	AUDIO_IN_FSPEC = (1 << 3), ///< 输入源 频谱数据
} GX_AUDIO_IN_SOURCE;

/**
 * @brief 采样率
 */
typedef enum {
	SAMPLE_RATE_16K,///< 16k采样率
	SAMPLE_RATE_8K, ///< 8k采样率, only for i2s in
	SAMPLE_RATE_48K ///< 48k采样率, only for i2s in
} GX_AUDIO_IN_SAMPLE_RATE;

/**
 * @brief 大小端存储
 */
typedef enum {
	ENDIAN_LITTLE_16BIT, ///< 1a2b3c4d
	ENDIAN_BIG_16BIT,    ///< 2b1a4d3c
	ENDIAN_LITTLE_32BIT, ///< 3c4d1a2b
	ENDIAN_BIG_32BIT,    ///< 4d3c2b1a
} GX_AUDIO_IN_ENDIAN;

/**
 * @brief I2S 数据位宽
 */
typedef enum {
	PCM_LENGTH_24BIT,   ///< 数据有效位24bit
	PCM_LENGTH_20BIT,   ///< 数据有效位20bit
	PCM_LENGTH_16BIT,   ///< 数据有效位16bit
	PCM_LENGTH_32BIT = 8,///< 数据有效位32bit
} GX_AUDIO_IN_I2S_PCM_LENGTH;

/**
 * @brief I2S 传输格式
 */
typedef enum {
	DATA_FORMAT_I2S, ///< 标准I2S
	DATA_FORMAT_LEFT_JUSTIFIED, ///< 左对齐
	DATA_FORMAT_RIGHT_JUSTIFIED,///< 右对齐
	DATA_FORMAT_TDM64, ///< TDM格式，bclk 64fs
	DATA_FORMAT_TDM128,///< TDM格式，bclk 128fs
	DATA_FORMAT_TDM256,///< TDM格式，bclk 256fs
} GX_AUDIO_IN_I2S_DATA_FORMAT;

/**
 * @brief I2S主从模式
 */
typedef enum {
	CLK_MODE_MASTER, ///< I2S主模式，输出时钟信号
	CLK_MODE_SLAVE   ///< I2S从模式，接收外部时钟信号
} GX_AUDIO_IN_I2S_CLOCK_MODE;

/**
 * @brief I2S bclk模式
 */
typedef enum {
	BCLK_MODE_32FS,  ///< bclk 32fs
	BCLK_MODE_64FS,  ///< bclk 64fs
	BCLK_MODE_128FS, ///< bclk 128fs, only for tdm
	BCLK_MODE_256FS, ///< bclk 256fs, only for tdm
} GX_AUDIO_IN_I2S_BCLK_MODE;

/**
 * @brief I2S 配置参数
 */
typedef struct {
	GX_AUDIO_IN_I2S_PCM_LENGTH  pcm_length;  ///< 数据位宽
	GX_AUDIO_IN_I2S_DATA_FORMAT data_format; ///< 数据格式
	GX_AUDIO_IN_I2S_CLOCK_MODE  clk_mode;    ///< 主从格式
	GX_AUDIO_IN_I2S_BCLK_MODE   bclk_sel;    ///< bclk模式
	GX_AUDIO_IN_SAMPLE_RATE     i2s_fs;      ///< 采样率
} GX_AUDIO_IN_I2S_PARAM;

/**
 * @brief TDM模式下长、短帧模式
 */
typedef enum {
	FSYNC_MODE_LONG, ///< tdm模式下，fsync为长帧模式
	FSYNC_MODE_SHORT,///< tdm模式下，fsync为短帧模式
} GX_AUDIO_IN_FSYNC_MODE;

/**
 * @brief I2S/TDM In 配置参数
 */
typedef struct {
	GX_AUDIO_IN_I2S_PARAM  i2s;
	GX_AUDIO_IN_FSYNC_MODE fsync_mode;
} GX_AUDIO_IN_INPUT_I2S;

/**
 * @brief PDM 时钟模式
 */
typedef enum {
	AUDIO_IN_PDM_1M, ///< PDM工作在1.048MHz频率下
	AUDIO_IN_PDM_2M  ///< PDM工作在2.048MHz频率下
} GX_AUDIO_IN_PDM_CLK;

/**
 * @brief PDM 输入配置
 */
typedef struct {
	GX_AUDIO_IN_PDM_CLK clk; ///< pdm时钟配置
} GX_AUDIO_IN_INPUT_PDM;

/**
 * @brief SAR ADC 输入配置
 */
typedef struct {
	unsigned int pga_enable; ///< pga使能开关
} GX_AUDIO_IN_INPUT_SADC;

/**
 * @brief 设置SAR ADC输入
 *
 * @param sadc sar adc相关参数, 详细说明请参考 gxdocref GX_AUDIO_IN_INPUT_SADC
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_input_sadc(GX_AUDIO_IN_INPUT_SADC sadc);

/**
 * @brief 设置PDM输入
 *
 * @param pdm pdm工作相关参数, 详细说明请参考 gxdocref GX_AUDIO_IN_INPUT_PDM
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_input_pdm(GX_AUDIO_IN_INPUT_PDM pdm);

/**
 * @brief 设置I2S输入
 *
 * @param i2s i2sin相关参数, 详细说明请参考 gxdocref GX_AUDIO_IN_INPUT_I2S
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_input_i2s(GX_AUDIO_IN_INPUT_I2S i2s);

/**
 * @brief 设置输入声道
 *
 * @param source 输入源, 详细说明请参考 gxdocref GX_AUDIO_IN_SOURCE
 * @param left_channel  左声道选择 0,1
 * @param right_channel 右声道选择 0,1
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_input_channel(GX_AUDIO_IN_SOURCE source, unsigned int left_channel, unsigned int right_channel);

/**
 * @brief 设置i2s in模式
 *
 * @param mode i2s in模式, 详细说明请参考 gxdocref GX_AUDIO_IN_I2S_CLOCK_MODE
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_i2sin_mode(GX_AUDIO_IN_I2S_CLOCK_MODE mode);

/******************************************************/
/**
 * @brief 输出通道
 */
typedef enum {
	AUDIO_IN_CHANNEL_PCM0     = (1 << 0),  ///< PCM0 FIFO
	AUDIO_IN_CHANNEL_PCM1     = (1 << 1),  ///< PCM1 FIFO
	AUDIO_IN_CHANNEL_LOGFBANK = (1 << 2),  ///< LOGFBANK FIFO
	AUDIO_IN_CHANNEL_I2SOUT   = (1 << 3),  ///< I2S OUT
} GX_AUDIO_IN_CHANNEL;


/**
 * @brief i2s out 通道数据来源
 */
typedef enum {
	I2S_SOURCE_SADC = 0,  ///< 数据源来自SADC
	I2S_SOURCE_PDM_LEFT,  ///< 数据源来自PDM左声道
	I2S_SOURCE_PDM_RIGHT, ///< 数据源来自PDM右声道
	I2S_SOURCE_I2SIN_LEFT, ///< 数据源来自I2S IN左声道
	I2S_SOURCE_I2SIN_RIGHT ///< 数据源来自I2S IN右声道
} GX_AUDIO_IN_I2S_SOURCE;

/**
 * @brief i2s输出配置
 */
typedef struct {
	GX_AUDIO_IN_I2S_PARAM i2s; ///< i2s参数

	GX_AUDIO_IN_I2S_SOURCE left_source;  ///< 左声道数据来源
	GX_AUDIO_IN_I2S_SOURCE right_source; ///< 右声道数据来源
} GX_AUDIO_IN_OUTPUT_I2S;

/**
 * @brief PCM 通道数据来源
 */
typedef enum {
	PCM_SOURCE_SADC = 0, ///< PCM FIFO中数据来自SADC
	PCM_SOURCE_PDM,      ///< PCM FIFO中数据来自PDM
	PCM_SOURCE_I2SIN,    ///< PCM FIFO中数据来自I2S IN
	PCM_SOURCE_FSPEC,    ///< PCM FIFO中数据来自频率数据
} GX_AUDIO_IN_PCM_SOURCE;

/**
 * @brief pcm 输出配置
 */
typedef struct {
	unsigned int left_buffer; ///< 左声道buffer起始地址
	unsigned int right_buffer;///< 右声道buffer起始地址
	unsigned int size;        ///< buffer大小

	unsigned int frame_num;        ///< 帧数
	GX_AUDIO_IN_ENDIAN endian;     ///< 大小端
	GX_AUDIO_IN_PCM_SOURCE source; ///< 数据源
} GX_AUDIO_IN_OUTPUT_PCM;

/**
 * @brief logfbank 运算数据来源
 */
typedef enum {
	LOGFBANK_SOURCE_SADC = 0,  ///< Logfbank运算的数据来自SADC
	LOGFBANK_SOURCE_PDM_LEFT,  ///< Logfbank运算的数据来自pdm左声道
	LOGFBANK_SOURCE_PDM_RIGHT, ///< Logfbank运算的数据来自pdm右声道
	LOGFBANK_SOURCE_I2SIN_LEFT,///< Logfbank运算的数据来自i2s in左声道
	LOGFBANK_SOURCE_I2SIN_RIGHT///< Logfbank运算的数据来自i2s in右声道
} GX_AUDIO_IN_LOGFBANK_SOURCE;

/**
 * @brief logfbank 输出配置
 */
typedef struct {
	unsigned int buffer;     ///< buffer起始地址
	unsigned int size;       ///< buffer大小

	unsigned int frame_num;             ///< 帧数
	GX_AUDIO_IN_ENDIAN endian;          ///< 大小端
	GX_AUDIO_IN_LOGFBANK_SOURCE source; ///< 数据源
} GX_AUDIO_IN_OUTPUT_LOGFBANK;

/**
 * @brief frequence spectrum 输出配置
 */
typedef struct {
	unsigned int buffer;     ///< buffer起始地址
	unsigned int size;       ///< buffer大小

	unsigned int frame_num;             ///< 帧数
	GX_AUDIO_IN_ENDIAN endian;          ///< 大小端
	GX_AUDIO_IN_LOGFBANK_SOURCE source; ///< 数据源
} GX_AUDIO_IN_OUTPUT_SPECTRUM;
/**
 * @brief 设置PCM输出
 *
 * @param channel 输出通道, 详细说明请参考 gxdocref GX_AUDIO_IN_CHANNEL
 * @param pcm     pcm输出通道配置参数, 详细说明请参考 gxdocref GX_AUDIO_IN_OUTPUT_PCM
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_output_pcm(GX_AUDIO_IN_CHANNEL channel, GX_AUDIO_IN_OUTPUT_PCM pcm);

/**
 *@brief 停止pcm通路工作
 */
int gx_audio_in_set_output_pcm_stop(GX_AUDIO_IN_CHANNEL channel);

/**
 * @brief 设置i2s输出
 *
 * @param i2s i2s输出通道配置参数, 详细说明请参考 gxdocref GX_AUDIO_IN_OUTPUT_I2S
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_output_i2s(GX_AUDIO_IN_OUTPUT_I2S i2s);

/**
 * @brief 设置logfbank输出
 *
 * @param logfbank logfbank输出通道配置参数, 详细说明请参考 gxdocref GX_AUDIO_IN_OUTPUT_LOGFBANK
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_output_logfbank(GX_AUDIO_IN_OUTPUT_LOGFBANK logfbank);

/**
 * @brief 设置frequence spectrum输出
 *
 * @param channel 输出通道, 详细说明请参考 gxdocref GX_AUDIO_IN_CHANNEL
 * @param spectrum 频谱输出通道配置参数, 详细说明请参考 gxdocref GX_AUDIO_IN_OUTPUT_SPECTRUM
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_output_spectrum(GX_AUDIO_IN_CHANNEL channel, GX_AUDIO_IN_OUTPUT_SPECTRUM spectrum);

/**
 * @brief 设置buffer切换地址
 *
 * @param channel 输出通道, 详细说明请参考 gxdocref GX_AUDIO_IN_CHANNEL
 * @param value   切换目标地址
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_update_addr(GX_AUDIO_IN_CHANNEL channel, void *value);

/**
 * @brief 设置i2s输出模式
 *
 * @param i2s i2s输出模式, 详细说明请参考 gxdocref GX_AUDIO_IN_I2S_CLOCK_MODE
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_i2sout_mode(GX_AUDIO_IN_I2S_CLOCK_MODE mode);

/******************************************************/
/**
 * @brief 粗调增益
 */
typedef enum {
	AUDIO_IN_GAIN_0dB, ///< 增益0dB
	AUDIO_IN_GAIN_6dB, ///< 增益6dB
	AUDIO_IN_GAIN_12dB,///< 增益12dB
	AUDIO_IN_GAIN_18dB,///< 增益18dB
	AUDIO_IN_GAIN_24dB,///< 增益24dB
	AUDIO_IN_GAIN_30dB,///< 增益30dB
	AUDIO_IN_GAIN_36dB,///< 增益36dB
	AUDIO_IN_GAIN_42dB,///< 增益42dB
	AUDIO_IN_GAIN_48dB,///< 增益48dB
	AUDIO_IN_GAIN_54dB,///< 增益54dB
} GX_AUDIO_IN_GAIN;

/**
 * @brief 设置粗调增益
 *
 * @param source 输入源, 详细说明请参考 gxdocref GX_AUDIO_IN_SOURCE
 * @param gain 增益，步进6dB, 详细说明请参考 gxdocref GX_AUDIO_IN_GAIN
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_rough_gain(GX_AUDIO_IN_SOURCE source, GX_AUDIO_IN_GAIN gain);

/**
 * @brief 设置精调增益
 *
 * @param source 输入源, 详细说明请参考 gxdocref GX_AUDIO_IN_SOURCE
 * @param gain 步进0.1dB
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_refined_gain(GX_AUDIO_IN_SOURCE source, int gain);

/**
 * @brief 获取精调增益
 *
 * @param source 输入源, 详细说明请参考 gxdocref GX_AUDIO_IN_SOURCE
 *
 * @return int
 * @retval  增益值
 * @retval -1 失败
 */
int gx_audio_in_get_rough_gain(GX_AUDIO_IN_SOURCE source);

/**
 * @brief 获取当前帧增益
 *
 * @param channel 输出通道, 详细说明请参考 gxdocref GX_AUDIO_IN_CHANNEL
 *
 * @return int
 * @retval  增益值
 * @retval -1 失败
 */
int gx_audio_in_get_frame_rough_gain(GX_AUDIO_IN_CHANNEL channel);

/******************************************************/
/**
 * @brief fftvad中线削波值
 */
typedef struct {
	unsigned short chipping_1;
	unsigned short chipping_2;
	unsigned short chipping_3;
	unsigned short chipping_4;
} GX_AUDIO_IN_FFTVAD_CHIPPING;

typedef struct {
	unsigned char w1;
	unsigned char w2;
	unsigned char w3;
} GX_AUDIO_IN_FFTVAD_W;

/**
 * @brief fftvad状态参数
 */
typedef struct {
	unsigned int vad_state_l; ///< 0~31帧状态
	unsigned int vad_state_m; ///< 32~63帧状态
	unsigned int vad_state_h; ///< 64~95帧状态
	unsigned short vad_state_index; ///< 当前更新到的帧状态索引
} GX_AUDIO_IN_FFTVAD_STATE;

/**
 * @brief fftvad curve_1参数配置
 *
 * @param curve_a fftvad算法中阈值拟合曲线参数
 * @param curve_b fftvad算法中阈值拟合曲线参数
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_fftvad_curve_1(unsigned short curve_a, unsigned short curve_b);

/**
 * @brief fftvad curve_2参数配置
 *
 * @param curve_a fftvad算法中阈值拟合曲线参数
 * @param curve_b fftvad算法中阈值拟合曲线参数
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_fftvad_curve_2(unsigned short curve_a, unsigned short curve_b);
/**
 * @brief fftvad curve_3参数配置
 *
 * @param curve_a fftvad算法中阈值拟合曲线参数
 * @param curve_b fftvad算法中阈值拟合曲线参数
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_fftvad_curve_3(short curve_a, short curve_b);
/**
 * @brief fftvad curve_4参数配置
 *
 * @param curve_a fftvad算法中阈值拟合曲线参数
 * @param curve_b fftvad算法中阈值拟合曲线参数
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_fftvad_curve_4(unsigned short curve_a, unsigned short curve_b);

/**
 * @brief fftvad curve_5参数配置
 *
 * @param curve fftvad算法中阈值拟合曲线参数
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_fftvad_curve_5(unsigned short curve);

/**
 * @brief fftvad curve_5参数配置
 *
 * @param silence_num fftvad算法中语音结束端点检测参数[0,50]
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_fftvad_silence_num(unsigned char silence_num);

/**
 * @brief fftvad curve_5参数配置
 *
 * @param count_num fftvad算法中语音结束端点检测参数[0,80]
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_fftvad_count_num(unsigned char count_num);

/**
 * @brief fftvad chipping参数配置
 *
 * @param chipping fftvad算法中中心削波z值参数, 详细说明请参考 gxdocref GX_AUDIO_IN_FFTVAD_CHIPPING
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_fftvad_chipping(GX_AUDIO_IN_FFTVAD_CHIPPING chipping);

/**
 * @brief fftvad 频谱距离参数配置
 *
 * @param chipping fftvad算法中频谱距离参数, 详细说明请参考 gxdocref GX_AUDIO_IN_FFTVAD_W
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_fftvad_w(GX_AUDIO_IN_FFTVAD_W fftvad_w);

/**
 * @brief fftvad 语音开始端点参数配置
 *
 * @param start_num fftvad算法中语音开始端点参数, 详细说明请参考 gxdocref GX_AUDIO_IN_FFTVAD_START_NUM
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_fftvad_startnum(unsigned short start_num);

/**
 * @brief fftvad 功能使能开关
 *
 * @param vad_enable fftvad使能开关，0:关，1:开
 * @param zcr_enable 过零率使能开关，0:关，1:开
 * @param frame_num 帧数，建议和logfbank buffer配置成相同帧数，最大值96
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_fftvad_enable(unsigned int vad_enable, unsigned int zcr_enable, unsigned int frame_num);

/**
 * @brief 获取fftvad激活状态
 *
 * @param state 状态参数, 详细说明请参考 gxdocref GX_AUDIO_IN_FFTVAD_STATE
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_get_fftvad_state(GX_AUDIO_IN_FFTVAD_STATE *state);

/**
 * @brief 获取FFTVAD算法中每单元帧的频域特征量
 *
 * @return unsigned int
 * @retval 频域特征量
 */
unsigned int gx_audio_in_get_distance_city(void);

/**
 * @brief 获取FFTVAD算法中每单元帧的底噪平滑值
 *
 * @return unsigned int
 * @retval 底噪平滑值
 */
unsigned int gx_audio_in_get_distance_noise_smooth(void);

/**
 * @brief 获取FFTVAD算法中每单元帧的判断的过零率
 *
 * @return unsigned int
 * @retval 过去9帧的过零率之和
 */
unsigned int gx_audio_in_get_fftvad_zcr_reg(void);

/******************************************************/

/**
 * @brief 中断类型
 */
typedef enum {
	AUDIO_IN_IRQ_PCM0_DONE           = (1 << 0),  ///< pcm0录制中断
	AUDIO_IN_IRQ_PCM1_DONE           = (1 << 1),  ///< pcm1录制中断
	AUDIO_IN_IRQ_LOGFBANK_DONE       = (1 << 2),  ///< logfbank录制中断
	AUDIO_IN_IRQ_FFTVAD_START        = (1 << 16), ///< fftvad激活起始中断
	AUDIO_IN_IRQ_FFTVAD_FINISH       = (1 << 17), ///< fftvad激活结束中断
	AUDIO_IN_IRQ_SADC_EVAD_DONE      = (1 << 18), ///< sadc路evad激活中断
	AUDIO_IN_IRQ_PDM_EVAD_DONE       = (1 << 19), ///< pdm路evad激活中断
	AUDIO_IN_IRQ_I2S_EVAD_DONE       = (1 << 20), ///< i2s路evad激活中断
	AUDIO_IN_IRQ_WRITE_START         = (1 << 21), ///< 三路写通路实际开始工作中断
} GX_AUDIO_IN_IRQ_TYPE;

/**
 * @brief 中断使能开关
 *
 * @param type 中断类型, 详细说明请参考 gxdocref GX_AUDIO_IN_IRQ_TYPE
 * @param enable 0:关 1:开
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_interrupt_enable(GX_AUDIO_IN_IRQ_TYPE type, unsigned int enable);
GX_AUDIO_IN_IRQ_TYPE gx_audio_in_get_interrupt_enable_state(void);

/**
 * @brief 实时获取写指针
 *
 * @param channel 写通道类型, 详细说明请参考 gxdocref GX_AUDIO_IN_CHANNEL
 * @param sdc_addr 实时写指针，接口中修改其内容
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_get_sdc_addr(GX_AUDIO_IN_CHANNEL channel, unsigned int *sdc_addr);

/******************************************************/
/**
 * @brief energy vad使能参数
 */
typedef struct {
	unsigned int evad_enable; ///< evad使能开关
	unsigned int zcr_enable;  ///< 过零率使能开关

	unsigned int state_valid_enable; ///< 该输入evad检测是否对录制生效
} GX_AUDIO_IN_EVAD;

/**
 * @brief energy 检测门限参数
 */
typedef struct {
	unsigned int evad_low_threshold; ///< evad检测低门限
	unsigned int evad_high_threshold;///< evad检测高门限
	unsigned int evad_zcr_threshold; ///< 过零率检测门限
} GX_AUDIO_IN_EVAD_THRESHOLD;

/**
 * @brief energy vad 使能开关
 *
 * @param source 输入源, 详细说明请参考 gxdocref GX_AUDIO_IN_SOURCE
 * @param evad   evad配置参数, 详细说明请参考 gxdocref GX_AUDIO_IN_EVAD
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_evad_enable(GX_AUDIO_IN_SOURCE source, GX_AUDIO_IN_EVAD evad);

/**
 * @brief energy vad 检测门限
 *
 * @param source 输入源, 详细说明请参考 gxdocref GX_AUDIO_IN_SOURCE
 * @param threshold  evad门限参数, 详细说明请参考 gxdocref GX_AUDIO_IN_EVAD_THRESHOLD
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_evad_threshold(GX_AUDIO_IN_SOURCE source, GX_AUDIO_IN_EVAD_THRESHOLD threshold);

/******************************************************/
/**
 * @brief adc输入源
 */
typedef enum {
	AUDIO_IN_ADCIN_PGA_REAR  = 0, ///< adc数据来自pga之后
	AUDIO_IN_ADCIN_PGA_FRONT = 1, ///< adc数据来自pga之前
} GX_AUDIO_IN_ADC_IN;

/**
 * @brief 设置pga增益
 *
 * @param gain 目标增益dB
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_pga_gain(unsigned int gain);

/**
 * @brief pga开关
 *
 * @param enable 0:关, 1:开
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_pga_enable(unsigned int enable);

/**
 * @brief 设置adc输入数据源
 *
 * @param adc_in adc输入源, 详细说明请参考 gxdocref GX_AUDIO_IN_ADC_IN
 *
 * @return int
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_adc_in(GX_AUDIO_IN_ADC_IN adc_in);

/******************************************************/
/**
 * @brief fftvad状态
 */
typedef enum {
	AUDIO_IN_FFTVAD_START  = (1 << 0), ///< FFT Vad检测到语音起始端点
	AUDIO_IN_FFTVAD_FINISH = (1 << 1), ///< FFT Vad检测到语音结束端点
} GX_AUDIO_IN_FFTVAD_STATUS;

typedef int (*GX_AUDIO_IN_CONFIG_CB)(void);
typedef int (*GX_AUDIO_IN_RECORD_CB)(GX_AUDIO_IN_CHANNEL channel, unsigned int *sdc_addr);
typedef int (*GX_AUDIO_IN_UPDATE_CB)(GX_AUDIO_IN_CHANNEL channel, unsigned int *saved_addr);
typedef int (*GX_AUDIO_IN_ENGVAD_CB)(GX_AUDIO_IN_SOURCE  source);
typedef int (*GX_AUDIO_IN_FFTVAD_CB)(GX_AUDIO_IN_FFTVAD_STATUS status);

/**
 * @brief I2S工作时钟
 */
typedef enum {
	AUDIO_IN_I2SCLK_1M  = 1, ///< 时钟1MHz
	AUDIO_IN_I2SCLK_2M  = 2, ///< 时钟2MHz
	AUDIO_IN_I2SCLK_12M = 3, ///< 时钟12MHz
	AUDIO_IN_I2SCLK_4M  = 4, ///< 时钟4MHz
} GX_AUDIO_IN_I2S_CLOCK;

/**
 * @brief 初始化配置
 */
typedef struct {
	GX_AUDIO_IN_RECORD_CB record_callback; ///< 录制中断回调
	GX_AUDIO_IN_CONFIG_CB config_callback; ///< 初始化配置回调
	GX_AUDIO_IN_UPDATE_CB update_callback; ///< buffer切换中断回调
	GX_AUDIO_IN_ENGVAD_CB engvad_callback; ///< engvad中断回调
	GX_AUDIO_IN_FFTVAD_CB fftvad_callback; ///< fftvad中断回调
} GX_AUDIO_IN_CONFIG;

/**
 * @brief audio in 初始化
 *
 * @param config 初始化配置项, 详细说明请参考 gxdocref GX_AUDIO_IN_CONFIG
 *
 */
int gx_audio_in_init(GX_AUDIO_IN_CONFIG config);

/**
 * @brief audio in 反初始化
 */
int gx_audio_in_exit(void);

/**
 * @brief 设置i2s时钟
 *
 * @param clock 目标时钟, 详细说明请参考 gxdocref GX_AUDIO_IN_I2S_CLOCK
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_i2s_clock(GX_AUDIO_IN_I2S_CLOCK clock);

/**
 * @brief 去直流开关
 *
 * @param source 输入源, 详细说明请参考 gxdocref GX_AUDIO_IN_SOURCE
 * @param enable 0:关，1:开
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_dc_enable(GX_AUDIO_IN_SOURCE source, unsigned int enable);

/**
 *获取logfbank工作状态
 */
int gx_audio_in_get_logfbank_status(void);

/**
 * @brief 硬件logfbank开关
 *
 * @param module_enable logfbank开关
 * @param hmt_enable    汉明窗开关
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_logfbank_enable(unsigned int module_enable, unsigned int hmt_enable);

/**
 * @brief audio in 自测试模式开关
 *
 * @param enable 开关
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_audio_in_set_data_auto_produce_enable(int enable);
#endif

/** @}*/
