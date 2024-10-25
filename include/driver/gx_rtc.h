/** \addtogroup <scpu>
 *  @{
 */

/* Define to prevent recursive inclusion */
#ifndef __GX_RTC_H__
#define __GX_RTC_H__
/* Cooperation with C and C++ */
#ifdef __cplusplus
extern "C" {
#endif

#include <driver/gx_irq.h>

/**
 * @brief RTC 日期时间结构体
 */
typedef struct RtcDateTime_s {
	int TmYear;  ///< 年,BCD码
	int TmMonth; ///< 月,BCD码
	int TmDay;   ///< 日,BCD码
	int TmWeek;  ///< 周,BCD码
	int TmHour;  ///< 时,BCD码
	int TmMin;   ///< 分,BCD码
	int TmSec;   ///< 秒,BCD码
	int TmMs;    ///< 毫秒,DEC值
	int TmUs;    ///< 微秒,DEC值
} GX_RTC_DATETIME;

/**
 * @brief RTC 闹钟日期 mask
 */
typedef struct GXRTC_AlarmMask_s {
	unsigned char AlarmMaskYear;      ///< 年
	unsigned char AlarmMaskMonth;     ///< 月
	unsigned char AlarmMaskDay;       ///< 日
	unsigned char AlarmMaskWeek;      ///< 周
	unsigned char AlarmMaskHour;      ///< 时
	unsigned char AlarmMaskMin;       ///< 分
	unsigned char AlarmMaskSec;       ///< 秒
	unsigned char AlarmMaskMs;        ///< 毫秒
	unsigned char AlarmMaskUs;        ///< 微秒
} GX_RTC_ALARM_MASK;

/**
 * @brief RTC 初始化
 */
void gx_rtc_init(void);

/**
 * @brief 启动 tick 计数
 */
void gx_rtc_start_tick(void);

/**
 * @brief 停止 tick 计数
 */
void gx_rtc_stop_tick(void);

/**
 * @brief 设置 tick 值
 *
 * @param time_duration tick 值, 单位秒
 */
void gx_rtc_set_tick(unsigned long time_duration);

/**
 * @brief 获取 tick 值
 *
 * @param time_duration 获取到的 tick 值, 单位秒
 */
void gx_rtc_get_tick(unsigned long *time_duration);

/**
 * @brief 开启时钟功能
 */
void gx_rtc_start_time(void);

/**
 * @brief 关闭时钟功能
 */
void gx_rtc_stop_time(void);

/**
 * @brief 获取当前日期和时间
 *
 * @param DateTime 获取到的日期和时间, 详细说明请参考 gxdocref GX_RTC_DATETIME
 * @return int 是否获取成功
 * @retval 0 成功
 * @retval -1 失败
 */
int  gx_rtc_get_time(GX_RTC_DATETIME *DateTime);

/**
 * @brief 设置当前日期和时间
 *
 * @param DateTime 要设置的日期和时间, 详细说明请参考 gxdocref GX_RTC_DATETIME
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int  gx_rtc_set_time(GX_RTC_DATETIME *DateTime);

/**
 * @brief 启动定时器
 */
void gx_rtc_start_alarm(void);

/**
 * @brief 关闭定时器
 */
void gx_rtc_stop_alarm(void);

/**
 * @brief 获取定时时间
 *
 * @param DateTime 获取到的定时时间, 详细说明请参考 gxdocref GX_RTC_DATETIME
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int  gx_rtc_get_alarm(GX_RTC_DATETIME *DateTime);

/**
 * @brief 设置定时时间
 *
 * @param DateTime 要设置的的定时时间, 详细说明请参考 gxdocref GX_RTC_DATETIME
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int  gx_rtc_set_alarm(GX_RTC_DATETIME *DateTime);

/**
 * @brief mask 相应的闹钟日期字段
 *
 * @param Mask 需要 mask 的字段, 详细说明请参考 gxdocref GX_RTC_ALARM_MASK
 */
void gx_rtc_mask_alarm(GX_RTC_ALARM_MASK *Mask);

/**
 * @brief 打印日期
 *
 * @param DateTime 需要打印的数据, 详细说明请参考 gxdocref GX_RTC_DATETIME
 */
void gx_rtc_print_time(GX_RTC_DATETIME DateTime);

/**
 * @brief 设置中断回调函数
 *
 * @param callback 中断回调函数
 * @param priv 中断回调参数
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_rtc_set_interrupt( irq_handler_t callback, void *priv);

/**
 * @brief 设置定时时间
 *
 * @param alarm_after_s 定时时间
 * @return int 是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_rtc_set_alarm_time_s(int alarm_after_s);

/**
 * @brief rtc 退出
 *
 * @return int 退出是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_rtc_exit(void);

/**
 * @brief rtc 暂停
 *
 * @return int 暂停是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_rtc_suspend(void);

/**
 * @brief rtc 恢复
 *
 * @return int 恢复是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_rtc_resume(void);

#ifdef __cplusplus
}
#endif

#endif

/** @}*/
