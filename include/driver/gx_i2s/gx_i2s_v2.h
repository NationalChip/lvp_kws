/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_I2S_V1_H__
#define __GX_I2S_V1_H__

/**
 * @brief I2S模块
 */
typedef enum {
	MODULE_I2S_IN,  ///< Audio In I2S In
	MODULE_REC_OUT, ///< Audio In I2S Out
	MODULE_I2S_OUT  ///< Audio Out I2S Out
} GX_I2S_MODULE;

/**
 * @brief I2S单一模式
 */
typedef enum {
	I2S_MODE_SLAVE_COMPLETE,
	I2S_MODE_SLAVE_SIMPLE,
	I2S_MODE_MASTER,
} GX_I2S_SINGLE_MODE;

/**
 * @brief I2S五线模式
 */
typedef enum {
	I2S_MODE_I2S_IN_M_REC_OUT_S, ///< Audio In I2S in master, I2S out slave
	I2S_MODE_I2S_IN_S_REC_OUT_M, ///< Audio In I2S in slave, I2S out master
	I2S_MODE_I2S_IN_S_REC_OUT_S_FROM_I2S_IN, ///< Audio In I2S in slave, I2S out slave, clock from i2s in
	I2S_MODE_I2S_IN_S_REC_OUT_S_FROM_REC_OUT,///< Audio In I2S in slave, I2S out slave, clock from i2s out

	I2S_MODE_I2S_IN_M_I2S_OUT_S, ///< Audio In I2S in master, Audio Out I2S out slave
	I2S_MODE_I2S_IN_S_I2S_OUT_M, ///< Audio In I2S in slave, Audio Out I2S out master
	I2S_MODE_I2S_IN_S_I2S_OUT_S_FROM_I2S_IN, ///< Audio In I2S in slave, Audio Out I2S out slave, clock from i2s in
	I2S_MODE_I2S_IN_S_I2S_OUT_S_FROM_I2S_OUT,///< Audio In I2S in slave, Audio Out I2S out slave, clock from i2s out

	I2S_MODE_REC_OUT_M_I2S_OUT_S, ///< Audio In I2S out master, Audio Out I2S out slave
	I2S_MODE_REC_OUT_S_I2S_OUT_M, ///< Audio In I2S out slave, Audio Out I2S out master
	I2S_MODE_REC_OUT_S_I2S_OUT_S_FROM_REC_OUT, ///< Audio In I2S out slave, Audio Out I2S out slave, clock from Audio In I2S out
	I2S_MODE_REC_OUT_S_I2S_OUT_S_FROM_I2S_OUT, ///< Audio In I2S out master, Audio Out I2S out slave, clock from Audio Out I2S out
} GX_I2S_FIVE_WIRE_MODE;

/**
 * @brief I2S MCLK类型
 */
typedef enum {
	I2S_IN_MCLK_IN,
	I2S_IN_MCLK_OUT,
	REC_OUT_MCLK_IN,
	REC_OUT_MCLK_OUT,
	I2S_OUT_MCLK_IN,
} GX_I2S_MCLK_TYPE;

/**
 * @brief I2S BCLK类型
 */
typedef enum {
	I2S_IN_BCLK_IN,
	I2S_IN_BCLK_OUT,
	REC_OUT_BCLK_IN,
	REC_OUT_BCLK_OUT,
	I2S_OUT_BCLK_IN,
	I2S_OUT_BCLK_OUT
} GX_I2S_BCLK_TYPE;

/**
 * @brief I2S LRCLK类型
 */
typedef enum {
	I2S_IN_LRCLK_IN,
	I2S_IN_LRCLK_OUT,
	REC_OUT_LRCLK_IN,
	REC_OUT_LRCLK_OUT,
	I2S_OUT_LRCLK_IN,
	I2S_OUT_LRCLK_OUT
} GX_I2S_LRCLK_TYPE;

/**
 * @brief 设置单一I2S工作模式
 *
 * @param module i2s 模块，详细说明请参考 gxdocref GX_I2S_MODULE
 * @param mode   工作模式，详细说明请参考 gxdocref GX_I2S_SINGLE_MODE
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_i2s_set_single_mode(GX_I2S_MODULE module, GX_I2S_SINGLE_MODE mode);

/**
 * @brief 设置五线I2S工作模式
 *
 * @param mode  工作模式，详细说明请参考 gxdocref GX_I2S_FIVE_WIRE_MODE
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_i2s_set_five_wire_mode(GX_I2S_FIVE_WIRE_MODE mode);
#endif

/** @}*/
