/** \addtogroup <scpu>
 *  @{
 */
#ifndef _GX_AUDIO_OUT_V2_H__
#define _GX_AUDIO_OUT_V2_H__

typedef void (*GX_AUDIO_OUT_FRAME_CB)(unsigned int saddr, unsigned int eaddr);
typedef void (*GX_AUDIO_OUT_FRAME_OVER_CB)(void);

/**
 * @brief 左右通道的起始地址和通道大小
 */
typedef struct {
	unsigned char *buffer0; ///< 左声道起始地址 8字节对齐
	unsigned char *buffer1; ///< 右声道起始地址 8字节对齐
	unsigned int   size;    ///< 通路buffer大小,同时对左右声道buffer起作用 128字节对齐
} GX_AUDIO_OUT_BUFFER;

/**
 * @brief PCM参数配置
 */
typedef struct {
	unsigned int  sample_rate; ///< PCM采样率
	unsigned char channels;   ///< PCM通道数
	unsigned char bits;       ///< PCM位深16/32bit
	unsigned char interlace;  ///< PCM交织:1 非交织:0
	unsigned char endian;     ///< PCM大小端0:小端,1:大端

	unsigned int  module_freq;  ///< Audio Out的模块时钟
} GX_AUDIO_OUT_PCM;

/**
 * @brief I2S数据有效位宽
 */
typedef enum {
	GX_AUDIO_OUT_PCM_LENGTH_16BIT = 0,///< 数据有效位宽16bit
	GX_AUDIO_OUT_PCM_LENGTH_20BIT = 1,///< 数据有效位宽20bit
	GX_AUDIO_OUT_PCM_LENGTH_24BIT = 2,///< 数据有效位宽24bit
} GX_AUDIO_OUT_I2S_PCM_LENGTH;

/**
 * @brief I2S数据格式
 */
typedef enum {
	GX_AUDIO_OUT_DATA_FORMAT_I2S = 0, ///< 标准I2S格式
	GX_AUDIO_OUT_DATA_FORMAT_LEFT_JUSTIFIED = 1, ///< 标准左对齐
	GX_AUDIO_OUT_DATA_FORMAT_RIGHT_JUSTIFIED = 2, ///< 标准右对齐
} GX_AUDIO_OUT_I2S_DATA_FORMAT;

/**
 * @brief bclk频率选择
 */
typedef enum {
	GX_AUDIO_OUT_BCLK_64FS = 0, ///< bclk频率64fs
	GX_AUDIO_OUT_BCLK_32FS = 1, ///< bclk频率32fs
} GX_AUDIO_OUT_I2S_BCLK;

/**
 * @brief I2S参数配置
 */
typedef struct {
	GX_AUDIO_OUT_I2S_BCLK        bclk; ///< bclk模式
	GX_AUDIO_OUT_I2S_PCM_LENGTH  pcm_length; ///< 数据位宽
	GX_AUDIO_OUT_I2S_DATA_FORMAT data_format;///< 数据格式
} GX_AUDIO_OUT_I2S;

/**
 * @brief DAC检测使能配置
 */
typedef enum {
	GX_AUDIO_OUT_DAC_CHECK_OFF = 0,  ///< DAC检测功能关闭
	GX_AUDIO_OUT_DAC_CHECK_ON  = 1,  ///< DAC检测功能打开
} GX_AUDIO_OUT_DAC_CHECK;

/**
 * @brief DAC检测参数配置
 */
typedef struct {
	GX_AUDIO_OUT_DAC_CHECK lp_enable; ///< 低能量检测使能
	unsigned short         lp_num;    ///< 检测样点
	unsigned short         lp_value;  ///< 检测阈值
	unsigned short         lp_detect_channel; ///< 检测声道,0:左声道, 1:右声道, 2:左和右声道, 3:左或右声道
	GX_AUDIO_OUT_DAC_CHECK hp_enable; ///< 高能量检测使能
	unsigned short         hp_num;    ///< 检测样点
	unsigned short         hp_value;  ///< 检测阈值
	unsigned short         hp_detect_channel; ///< 检测声道,0:左声道, 1:右声道, 2:左和右声道, 3:左或右声道
} GX_AUDIO_OUT_DAC;

/**
 * @brief 回调函数
 */
typedef struct {
	GX_AUDIO_OUT_FRAME_CB      new_frame_callback; ///< 回调,该函数实现不能阻塞
	GX_AUDIO_OUT_FRAME_OVER_CB frame_over_callback; ///< 帧数据全部消耗完回调,该函数实现不能阻塞
} GX_AUDIO_OUT_CALLBACK;

/**
 * @brief 播放帧配置
 */
typedef struct {
	unsigned int  saddr;       ///< 帧起始地址
	unsigned int  eaddr;       ///< 帧结束地址
} GX_AUDIO_OUT_FRAME;

/**
 * @brief 播放通道的选择
 */
typedef enum {
	GX_AUDIO_OUT_STEREO_C = 0,///< 双声道立体声,左声道选0th,右声道选1th,不混合
	GX_AUDIO_OUT_LEFT_C,     ///< 左声道,左声道选0th,右声道选0th,不混合
	GX_AUDIO_OUT_RIGHT_C,    ///< 右声道,左声道选1th,右声道选1th,不混合
	GX_AUDIO_OUT_MONO_C,     ///< mono,左声道选0th,右声道选1th,混合
} GX_AUDIO_OUT_CHANNEL;

/**
 * @brief 播放 需要在初始化后调用
 *
 * @param route 选择播放的通路,为0即可
 * @return int  是否初始化成功
 * @retval handle 成功
 * @retval -1 失败
 */
int gx_audio_out_alloc_playback(unsigned char route);

/**
 * @brief 播放结束后,释放空间
 *
 * @param handle  播放操作句柄
 * @return int    是否释放成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_audio_out_free(int handle);

/**
 * @brief 配置通路左右声道的buffer
 *
 * @param handle  播放操作句柄
 * @param conf    配置buffer, 详细说明请参考 gxdocref GX_AUDIO_OUT_BUFFER
 * @return int    是否配置成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_audio_out_config_buffer(int handle, GX_AUDIO_OUT_BUFFER *conf);

/**
 * @brief 配置通路pcm的参数
 *
 * @param handle  播放操作句柄
 * @param conf    pcm参数, 详细说明请参考 gxdocref GX_AUDIO_OUT_PCM
 * @return int    是否配置成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_audio_out_config_pcm(int handle, GX_AUDIO_OUT_PCM *conf);

/**
 * @brief 配置通路i2s的参数
 * 	该组参数具备默认值，按需配置
 *
 * @param handle  播放操作句柄
 * @param i2s     i2s参数, 详细说明请参考 gxdocref GX_AUDIO_OUT_I2S
 * @return int    是否配置成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_audio_out_config_i2s(int handle, GX_AUDIO_OUT_I2S *i2s);

/**
 * @brief 配置dac检测的参数
 * 	该组参数具备默认值，非必要配置
 *
 * @param handle  播放操作句柄
 * @param dac     dac参数, 详细说明请参考 gxdocref GX_AUDIO_OUT_DAC
 * @return int    是否配置成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_audio_out_config_dac(int handle, GX_AUDIO_OUT_DAC *dac);

/**
 * @brief 配置播放中断回调
 *
 * @param handle  播放操作句柄
 * @param cb      回调, 详细说明请参考 gxdocref GX_AUDIO_OUT_CALLBACK
 * @return int    是否配置成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_audio_out_config_cb(int handle, GX_AUDIO_OUT_CALLBACK *cb);

/**
 * @brief 播放一帧数据
 *
 * @param handle  播放操作句柄
 * @param frame    数据帧参数, 详细说明请参考 gxdocref GX_AUDIO_OUT_FRAME
 * @return int    是否push成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_audio_out_push_frame(int handle, GX_AUDIO_OUT_FRAME *frame);

/**
 * @brief 等待帧数据全部消耗完
 *
 * @param handle  播放操作句柄
 * @return int    是否消耗完
 * @retval 0 消耗完成
 * @retval -1  push失败
 */
int gx_audio_out_drain_frame(int handle);

/**
 * @brief 设置db -18到18db
 *
 * @param handle  播放操作句柄
 * @param db      设置的db
 * @return int    设置是否成功
 * @retval 0  成功
 * @retval -1  失败
 */
int gx_audio_out_set_db(int handle, short db);

/**
 * @brief 获取当前db -18到18db
 *
 * @param handle  播放操作句柄
 * @param db      传出db值
 * @return int 获取当前db
 * @retval db  获取的db
 * @retval -1  失败
 */
int gx_audio_out_get_db(int handle, short *db);

/**
 * @brief 设置静音
 *
 * @param handle  播放操作句柄
 * @param mute    1:静音 0:取消静音
 * @return int    设置是否成功
 * @retval 0  成功
 * @retval -1  失败
 */
int gx_audio_out_set_mute(int handle, int mute);

/**
 * @brief 获取是否静音
 *
 * @param handle  播放操作句柄
 * @return int    是否静音
 * @retval 0  不静音
 * @retval 1  静音
 * @retval -1  失败
 */
int gx_audio_out_get_mute(int handle);

/**
 * @brief 设置左右声道通道的选择
 *
 * @param handle  播放操作句柄
 * @param channel  选择的模式, 详细说明请参考 gxdocref GX_AUDIO_OUT_CHANNEL
 * @return int    是否成功
 * @retval 0  成功
 * @retval -1  失败
 */
int gx_audio_out_set_channel(int handle, GX_AUDIO_OUT_CHANNEL channel);

/**
 * @brief  I2S和DAC输出固定的值
 *
 * @param handle  播放操作句柄
 * @param value   目标值
 * @return int    是否成功
 * @retval 0  成功
 * @retval -1  失败
 */
int gx_audio_out_set_fixed_src(int handle, short value);

/**
 * @brief 获取SDC Address
 *
 * @param handle  播放操作句柄
 * @return unsigned int SDC Address
 * @retval 0  成功
 * @retval -1  失败
 */
unsigned int gx_audio_out_get_sdc_addr(int handle);

/**
 * @brief 初始化播放
 *
 * @param aout_mode audio play 模式 0:master, 1:slave complete, 2:slave simple
 * @return int  是否初始化成功
 * @retval handle 成功
 * @retval -1 失败
 */
int gx_audio_out_init(int aout_mode);

/**
 * @brief audio out 退出
 *
 * @return int 退出是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_audio_out_exit(void);

/**
 * @brief audio out 挂起
 *
 * @param handle  播放操作句柄
 * @return int 挂起是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_audio_out_suspend(int handle);

/**
 * @brief audio out 恢复
 *
 * @return int 恢复是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_audio_out_resume(int handle);
#endif


/** @}*/
