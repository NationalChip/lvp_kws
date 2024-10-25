/** \addtogroup <scpu>
 *  @{
 */
#ifndef __GX_LDO_H__
#define __GX_LDO_H__

/**
 * @brief ldo fla使能开关
 *
 * @param enable 0:关 1:开
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_ldo_fla_enbale(unsigned int enable);

/**
 * @brief ldo fla输出电压选项
 */
typedef enum {
	LDO_FLA_VOLTAGE_1_8V   =  0, ///< 输出1.8V
	LDO_FLA_VOLTAGE_1_775V =  1, ///< 输出1.775V
	LDO_FLA_VOLTAGE_1_75V  =  2, ///< 输出1.75V
	LDO_FLA_VOLTAGE_1_725V =  3, ///< 输出1.725V
	LDO_FLA_VOLTAGE_1_7V   =  4, ///< 输出1.7V
	LDO_FLA_VOLTAGE_1_675V =  5, ///< 输出1.675V
	LDO_FLA_VOLTAGE_1_65V  =  6, ///< 输出1.65V
	LDO_FLA_VOLTAGE_1_625V =  7, ///< 输出1.625V
	LDO_FLA_VOLTAGE_2_0V   =  8, ///< 输出2.0V
	LDO_FLA_VOLTAGE_1_975V =  9, ///< 输出1.975V
	LDO_FLA_VOLTAGE_1_95V  = 10, ///< 输出1.95V
	LDO_FLA_VOLTAGE_1_925V = 11, ///< 输出1.925V
	LDO_FLA_VOLTAGE_1_9V   = 12, ///< 输出1.9V
	LDO_FLA_VOLTAGE_1_875V = 13, ///< 输出1.875V
	LDO_FLA_VOLTAGE_1_85V  = 14, ///< 输出1.85V
	LDO_FLA_VOLTAGE_1_825V = 15, ///< 输出1.825V

	LDO_FLA_VOLTAGE_INVALID = -1,
} GX_ANALOG_LDO_FLA_VOLTAGE;

/**
 * @brief ldo fla 输出电压设置
 *
 * @param voltage 详见 GX_ANALOG_LDO_FLA_VOLTAGE
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_ldo_fla_voltage(GX_ANALOG_LDO_FLA_VOLTAGE voltage);

/**
 * @brief ldo ana/dig 功能选项
 */
typedef enum {
	LDO_HW_CTRL_ADAPED = 0, ///< 硬件控制，根据电压自动Bypss或Enable
	LDO_SW_CTRL_ENABLE = 1, ///< 软件控制，强制使用Ldo
	LDO_SW_CTRL_BYPASS = 2, ///< 软件控制，强制Bypass Ldo
} GX_ANALOG_LDO_CTRL;

/**
 * @brief ldo ana exbypass
 *
 * @param exbypass 0:不允许soc给出强制bypass信号， 1:允许soc给出强制bypass信号
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_ldo_ana_exbypass(unsigned int bypass);

/**
 * @brief ldo ana bypass
 *
 * @param bypass 0:soc给的bypass信号， 1:soc给出强制bypass信号
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_ldo_ana_bypass(unsigned int bypass);

/**
 * @brief ldo ana ctrl
 *
 * @param ctrl: 详见GX_ANALOG_LDO_CTRL
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_ldo_ana_ctrl(GX_ANALOG_LDO_CTRL ctrl);

/**
 * @brief ldo ana输出电压选项
 */
typedef enum {
	LDO_ANA_VOLTAGE_0_9V   =  0, ///< 输出0.9V
	LDO_ANA_VOLTAGE_0_925V =  1, ///< 输出0.925V
	LDO_ANA_VOLTAGE_0_95V  =  2, ///< 输出0.95V
	LDO_ANA_VOLTAGE_0_975V =  3, ///< 输出0.975V
	LDO_ANA_VOLTAGE_0_7V   =  4, ///< 输出0.7V
	LDO_ANA_VOLTAGE_0_725V =  9, ///< 输出0.725V
	LDO_ANA_VOLTAGE_0_75V  = 10, ///< 输出0.75V
	LDO_ANA_VOLTAGE_0_775V = 11, ///< 输出0.775V
	LDO_ANA_VOLTAGE_0_8V   = 12, ///< 输出0.8V
	LDO_ANA_VOLTAGE_0_825V = 13, ///< 输出0.825V
	LDO_ANA_VOLTAGE_0_85V  = 14, ///< 输出0.85V
	LDO_ANA_VOLTAGE_0_875V = 15, ///< 输出0.875V

	LDO_ANA_VOLTAGE_INVALID = -1,
} GX_ANALOG_LDO_ANA_VOLTAGE;

/**
 * @brief ldo ana 输出电压设置
 *
 * @param voltage: 详见 GX_ANALOG_LDO_ANA_VOLTAGE
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_ldo_ana_voltage(GX_ANALOG_LDO_ANA_VOLTAGE voltage);

/**
 * @brief ldo dig exbypass
 *
 * @param exbypass 0:默认， 1:允许soc给出强制bypass信号
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_ldo_dig_exbypass(unsigned int bypass);

/**
 * @brief ldo dig bypass
 *
 * @param exbypass 0:默认, 1:oc给出强制bypass信号
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_ldo_dig_bypass(unsigned int bypass);

/**
 * @brief ldo dig ctrl
 *
 * @param ctrl: 详见GX_ANALOG_LDO_CTRL
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_ldo_dig_ctrl(GX_ANALOG_LDO_CTRL ctrl);

/**
 * @brief ldo dig ctrl
 *
 * @return GX_ANALOG_LDO_CTRL
 */
int gx_analog_get_ldo_dig_ctrl(void);

/**
 * @brief ldo dig dummyload开关
 *
 * @param enable 0:正常工作, 1:打开dummyload 增加内部消耗
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_ldo_dig_dummyload(unsigned int enable);

/**
 * @brief ldo dig输出电压选项
 */
typedef enum {
	LDO_DIG_VOLTAGE_0_950V = 0, ///< 输出0.950V
	LDO_DIG_VOLTAGE_0_924V = 1, ///< 输出0.924V
	LDO_DIG_VOLTAGE_0_897V = 2, ///< 输出0.897V
	LDO_DIG_VOLTAGE_0_871V = 3, ///< 输出0.871V
	LDO_DIG_VOLTAGE_0_845V = 4, ///< 输出0.845V
	LDO_DIG_VOLTAGE_0_819V = 5, ///< 输出0.819V
	LDO_DIG_VOLTAGE_0_792V = 6, ///< 输出0.792V
	LDO_DIG_VOLTAGE_0_766V = 7, ///< 输出0.767V
	LDO_DIG_VOLTAGE_0_740V = 8, ///< 输出0.740V
	LDO_DIG_VOLTAGE_0_714V = 9, ///< 输出0.714V
	LDO_DIG_VOLTAGE_0_687V = 10, ///< 输出0.687V
	LDO_DIG_VOLTAGE_0_661V = 11, ///< 输出0.661V

	LDO_DIG_VOLTAGE_INVALID = -1,
} GX_ANALOG_LDO_DIG_VOLTAGE;
/**
 * @brief ldo dig 输出电压设置
 *
 * @param voltage: 详见GX_ANALOG_LDO_DIG_VOLTAGE
 *
 * @return int 设置是否成功
 * @retval  0 成功
 * @retval -1 失败
 */
int gx_analog_set_ldo_dig_voltage(GX_ANALOG_LDO_DIG_VOLTAGE voltage);

/**
 * @brief ldo dig 获取输出电压
 *
 * @return voltage  电压值
 */
GX_ANALOG_LDO_DIG_VOLTAGE gx_analog_get_ldo_dig_voltage(void);

/**
 * @brief 获取ldo fla bypass状态
 *
 * @return int
 * @retval 0 flash 有ldo_fla供电1.8V
 * @retval 1 外部电压<=2V, flash供电由外部直通
 */
int gx_analog_get_ldo_fla_bypass(void);

/**
 * @brief 获取ldo core bypass状态
 *
 * @return int
 * @retval 0  ldo_ana和ldo_dig都工作，输出0.9V
 * @retval 1  ldo_ana和ldo_dig都进入Bypass模式，外部供电0.9V直通内核
 */
int gx_analog_get_ldo_core_bypass(void);

/**
 * @brief 获取ldo dig bypass状态
 *
 * @return int
 * @retval 0  ldo_dig都工作，输出目标电压
 * @retval 1  ldo_dig都进入Bypass模式，外部供电直通内核
 */
int gx_analog_get_ldo_dig_bypass(void);

/**
 * @brief 获取vadd12 ov10 状态
 *
 * @return int
 * @retval 0 判断输入电源是0.9V
 * @retval 1 判断输入电源是1.2V
 */
int gx_analog_get_ldo_vdd12_ov10(void);
#endif

/** @}*/
