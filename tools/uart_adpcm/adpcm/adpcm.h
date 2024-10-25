#ifndef __ADPCM_H__
#define __ADPCM_H__

/**
 * @brief 编码 ADPCM 数据，无头部，位宽16bit
 *
 * @param in_pcm    输入pcm数据
 * @param simples   待编码的采样点数
 * @param out_adpcm 输出adpcm数据
 *
 * @retval
 */
void Pcm2Adpcm (short *in_pcm, short *out_adpcm, int simples);

/**
 * @brief 解码 Pcm2Adpcm 编码的 adpcm，无头部，位宽 16bit
 *
 * @param in_adpcm  输入adpcm数据
 * @param simples   待编码的采样点数
 * @param out_pcm   输出pcm数据
 *
 * @retval
 */
void Adpcm2Pcm (short *in_adpcm, short *out_pcm, int simples);

/**
 * @brief 清理编码状态量
 *
 * @retval
 */
void AdpcmClearEncode(void);

/**
 * @brief 清理解码状态量
 *
 * @retval
 */
void AdpcmClearDecode(void);


int AdpcmGetEnPredict(void);

int AdpcmGetEnPredictIdx(void);

int AdpcmGetDePredict(void);

int AdpcmGetDePredictIdx(void);


int AdpcmSetEnPredict(int data);

int AdpcmSetEnPredictIdx(int data);

int AdpcmSetDePredict(int data);

int AdpcmSetDePredictIdx(int data);


// get adpcm size
#define ADPCM_EN_ZIZE(samples)  ((samples + 1) / 2)

// get pcm samples num
#define ADPCM_DE_NUM(size) (size * 2)

#endif /* __ADPCM_H__ */
