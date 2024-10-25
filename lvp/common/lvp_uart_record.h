/* Voice Signal Preprocess
 * Copyright (C) 2001-2020 NationalChip Co., Ltd
 * ALL RIGHTS RESERVED!
 *
 * uart_message_v2.h: MCU uart communication protocol driver
 *
 */

#ifndef __LVP_UART_RECORD__
#define __LVP_UART_RECORD__

typedef enum{
    RECORD_CHANNEL_MIC0     = 0x1,
    RECORD_CHANNEL_MIC1     = 0x2,
    RECORD_CHANNEL_FFT      = 0x4,
    RECORD_CHANNEL_LOGFBANK = 0x8,
    RECORD_CHANNEL_G_SENSOR = 0x10,
} RECORD_CHANNEL_MASK;


/**
 * @brief 初始化串口录音功能
 *
 * @param port 选择串口号，1: 串口1，other:串口0
 * @param baudrate 设置串口波特率，0: 使用默认波特率，other:使用该参数
 * @return int  是否初始化成功
 * @retval 0  成功
 * @retval -1 失败
 */
int UartRecordInit(int port, unsigned int baudrate);

/**
 * @brief 设置串口任务
 *
 * @param context 数据源
 * @return int  是否成功
 * @retval 0  成功
 * @retval -1 失败，流控限制
 */
int UartRecordTask(LVP_CONTEXT *context);

/**
 * @brief 根据通道设置串口任务
 *
 * @param sendbuffer 数据源
 * @param sendlen 数据长度
 * @param channel_mask 通道掩码
 * @return int  是否成功
 * @retval 0  成功
 * @retval -1 失败
 */
int UartRecordChannelTask(unsigned char *send_buffer, unsigned int send_len, RECORD_CHANNEL_MASK channel_mask);

/**
 * @brief 后台进程
 *
 * @return int  状态
 * @retval 正常
 */
int UartRecordTick(void);

#endif
