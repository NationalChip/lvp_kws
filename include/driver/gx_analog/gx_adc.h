/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_ADC_H__
#define __GX_ADC_H__

/**
 * @brief 设置pga偏置电流
 *
 * @param itrim 偏置电流量  0-63，默认32
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_pga_itrim(unsigned int itrim);

/**
 * @brief pga bypass开关
 *
 * @param bypass开关 1:bypass
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_pga_bypass(unsigned int bypass);

/**
 * @brief pga 使能开关
 *
 * @param 1:使能
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_pga_enable(unsigned int enbale);

/**
 * @brief 共模电阻设置
 *
 * @param sel 0: 1Mohm, 1: 100Kohm
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_pga_cmr_sel(unsigned int sel);

/**
 * @brief 设置直流失调
 *
 * @param dcc 0: 关闭pga直流失调, 0x20:正失调, 0x40:负失调
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_pga_dcc(unsigned char pga_dcc);

/**
 * @brief 设置pga增益
 *
 * @param gain 增益
 *             0-18dB, 6dB递增
 *             19-48dB, 1dB递增
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_pga_gain(unsigned char gain);

/**
 * @brief 设置pga测试模式
 *
 * @param enable 0:关闭 1: 使能
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_pga_tstbuf(unsigned int enable);

/**
 * @brief 设置adc采样时机
 *
 * @param clk_sel 0:SAR转换结束后自动采样
 *                1:固定在adc_clk的低电平时采样
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_adc_sample_clk_sel(unsigned int clk_sel);

/**
 * @brief 设置adc输出时机
 *
 * @param out_clk 0:下一个adc_clk的上沿输出adc_out
 *                1:当前adc_clk的下降沿输出adc_out
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_adc_out_at_clk(unsigned int out_clk);

/**
 * @brief 设置adc的输入
 *
 * @param sel 0:采样pga的输出
 *            1:采样pga的输入
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_adc_in_sel(unsigned int sel);

/**
 * @brief 设置adc复位
 *
 * @param rstn
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_adc_rstn(unsigned int rstn);

#endif

/** @}*/