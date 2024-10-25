/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_I2C_H__
#define __GX_I2C_H__

#define I2C_STATIC_MAX_COUNT (100)


/**
 * @brief i2c slave事件
 */
typedef enum gx_i2c_slave_event {
	I2C_SLAVE_RECEIVE_DATA,   ///< i2c slave接收数据
	I2C_SLAVE_REQUESTED_DATA, ///< i2c slave发送数据
	I2C_SLAVE_STOP,           ///< i2c slave关闭
	I2C_SLAVE_DMA_TX_DONE,    ///< i2c slave dma传输完成
	I2C_SLAVE_DMA_RX_DONE,    ///< i2c slave dma传输完成
} GX_I2C_SLAVE_EVENT;

/**
 * @brief i2c消息
 */
typedef struct gx_i2c_msg {
	unsigned short addr;  ///< i2c slave地址
	unsigned short flags; ///< 消息标志

#define I2C_M_RD           0x0001 ///< 读取slave标志
#define I2C_M_START_BYTE   0x0002 ///< START byte标志
#define I2C_M_GENERAL_CALL 0x0004 ///< General call标志
#define I2C_M_TEN          0x0010 ///< 10bit slave地址标志
#define I2C_M_RECV_LEN     0x0400 ///< 第一byte为长度标志
#define I2C_M_NO_RD_ACK    0x0800 ///< 读取操作不产生ACK标志
#define I2C_M_IGNORE_NAK   0x1000 ///< 忽略NACK标志
#define I2C_M_REV_DIR_ADDR 0x2000 ///< 读写切换标志
#define I2C_M_NOSTART      0x4000 ///< no start标志

	unsigned short len;   ///< 消息长度
	unsigned char  *buf;  ///< 消息内容
} GX_I2C_MSG;

/**
 * @brief i2c slave回调函数
 *
 * @param event i2c slave事件, 详细说明请参考 gxdocref GX_I2C_SLAVE_EVENT
 * @param val 读取或传输的值
 * @param private 私有参数
 *
 * @retval 0 成功
 * @retval -1 失败
 */
typedef int (*gx_i2c_slave_cb_t)(GX_I2C_SLAVE_EVENT event, unsigned char *val, void *private);

/**
 * @brief i2c 读写异步回调函数
 *
 * @param private 私有参数
 *
 * @retval 无
 */
typedef void (*gx_i2c_async_done_cb_t)(void *private);

/**
 * @brief 打开i2c模块时钟
 *
 * @param bus_id i2c总线id
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_init(unsigned char bus_id);

/**
 * @brief 关闭i2c模块时钟
 *
 * @param bus_id i2c总线id
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_exit(unsigned char bus_id);

/**
 * @brief i2c模块进入低功耗
 *
 * @param bus_id i2c总线id
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_suspend(unsigned char bus_id);

/**
 * @brief i2c模块退出低功耗
 *
 * @param bus_id i2c总线id
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_resume(unsigned char bus_id);

/**
 * @brief 初始化gpio i2c master
 *
 * @param bus_id i2c总线id
 * @param scl_port scl线对应的gpio
 * @param sda_port sda线对应的gpio
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_gpio_i2c_init(unsigned char bus_id, unsigned int scl_port, unsigned int sda_port);

/**
 * @brief 打开i2c master
 *
 * @param bus_id i2c总线id
 *
 * @retval NULL 失败
 * @retval OTHER 成功
 */
void *gx_i2c_open(unsigned char bus_id);

/**
 * @brief 获取i2c master句柄
 *
 * @param bus_id i2c总线id
 * @param speed i2c总线速度
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_set_speed(unsigned char bus_id, unsigned int speed);

/**
 * @brief 处理i2c消息
 *
 * @param dev i2c master句柄
 * @param msgs 需要发送的消息, 详细说明请参考 gxdocref GX_I2C_MSG
 * @param num 消息数量
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_transfer(void *dev, GX_I2C_MSG *msgs, int num);

/**
 * @brief i2c发送数据
 *
 * @param bus_id i2c总线id
 * @param slv_addr i2c slave地址
 * @param buf 需要发送的数据
 * @param len 发送数据长度
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_write(unsigned char bus_id, unsigned char slv_addr, unsigned char *buf, unsigned short len);

/**
 * @brief i2c发送数据
 *
 * @param bus_id i2c总线id
 * @param slv_addr i2c slave地址
 * @param reg_address i2c slave寄存器地址
 * @param address_width i2c slave寄存器地址宽度
 * @param tx_data 需要发送的数据
 * @param len 发送数据长度
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_tx(unsigned char bus_id, unsigned char slv_addr, unsigned int reg_address, unsigned char address_width, unsigned char *tx_data, unsigned short len);

/**
 * @brief i2c异步发送数据
 *
 * @param bus_id i2c总线id
 * @param slv_addr i2c slave地址
 * @param buf 需要发送的数据
 * @param len 发送数据长度
 * @param callback 完成时回调函数, 详细说明请参考 gxdocref gx_i2c_async_done_cb_t
 * @param private  回调函数参数
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_async_write(unsigned char bus_id, unsigned char slv_addr, unsigned char *buf,
			unsigned short len, gx_i2c_async_done_cb_t callback, void *private);

/**
 * @brief i2c异步发送数据
 *
 * @param bus_id i2c总线id
 * @param slv_addr i2c slave地址
 * @param reg_address i2c slave寄存器地址
 * @param address_width i2c slave寄存器地址宽度
 * @param tx_data 需要发送的数据
 * @param len 发送数据长度
 * @param callback 完成时回调函数, 详细说明请参考 gxdocref gx_i2c_async_done_cb_t
 * @param private  回调函数参数
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_async_tx(unsigned char bus_id, unsigned char slv_addr, unsigned int reg_address, unsigned char address_width,
			unsigned char *tx_data, unsigned short len, gx_i2c_async_done_cb_t callback, void *private);

/**
 * @brief i2c读取数据
 *
 * @param bus_id i2c总线id
 * @param slv_addr i2c slave地址
 * @param buf 读取的数据
 * @param len 读取数据长度
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_read(unsigned char bus_id, unsigned char slv_addr, unsigned char *buf, unsigned short len);

/**
 * @brief i2c读取数据
 *
 * @param bus_id i2c总线id
 * @param slv_addr i2c slave地址
 * @param reg_address i2c slave寄存器地址
 * @param address_width i2c slave寄存器地址宽度
 * @param rx_data 读取的数据
 * @param len 读取数据长度
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_rx(unsigned char bus_id, unsigned char slv_addr, unsigned int reg_address, unsigned char address_width, unsigned char *rx_data, unsigned short len);

/**
 * @brief i2c异步读取数据
 *
 * @param bus_id i2c总线id
 * @param slv_addr i2c slave地址
 * @param buf 读取的数据
 * @param len 读取数据长度
 * @param callback 完成时回调函数, 详细说明请参考 gxdocref gx_i2c_async_done_cb_t
 * @param private  回调函数参数
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_async_read(unsigned char bus_id, unsigned char slv_addr, unsigned char *buf,
			unsigned short len, gx_i2c_async_done_cb_t callback, void *private);

/**
 * @brief i2c异步读取数据
 *
 * @param bus_id i2c总线id
 * @param slv_addr i2c slave地址
 * @param reg_address i2c slave寄存器地址
 * @param address_width i2c slave寄存器地址宽度
 * @param rx_data 读取的数据
 * @param len 读取数据长度
 * @param callback 完成时回调函数, 详细说明请参考 gxdocref gx_i2c_async_done_cb_t
 * @param private  回调函数参数
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_async_rx(unsigned char bus_id, unsigned char slv_addr, unsigned int reg_address, unsigned char address_width,
			unsigned char *rx_data, unsigned short len, gx_i2c_async_done_cb_t callback, void *private);

/**
 * @brief 关闭i2c master
 *
 * @param dev i2c master句柄
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_close(void *dev);

/**
 * @brief 打开i2c slave
 *
 * @param bus_id i2c总线id
 * @param slv_addr i2c slave地址
 * @param callback i2c slave回调函数, 详细说明请参考 gxdocref gx_i2c_slave_cb_t
 * @param private 回调函数私有参数
 *
 * @retval NULL 失败
 * @retval OTHER 成功
 */
void *gx_i2c_slave_open(unsigned char bus_id, unsigned char slv_addr, gx_i2c_slave_cb_t callback, void *private);

/**
 * @brief 关闭i2c slave
 *
 * @param dev i2c slave句柄
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_slave_close(void *dev);

/**
 * @brief 使能 i2c_slave DMA 快速发送，用于与没有时钟延展的主机通信
 *
 * @param dev i2c slave句柄
 * @param buf 接收buf
 * @param size 接收数据大小
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_slave_enable_fast_dma_tx(unsigned char bus_id);

/**
 * @brief i2c slave异步接收数据
 *
 * @param dev i2c slave句柄
 * @param buf 接收buf
 * @param size 接收数据大小
 *
 * @retval 0 成功
 * @retval -1 失败
 */

int gx_i2c_slave_async_rx(void *dev, unsigned char *buf, unsigned int size);

/**
 * @brief i2c slave异步发送数据
 *
 * @param dev i2c slave句柄
 * @param buf 发送buf
 * @param size 发送数据大小
 *
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_i2c_slave_async_tx(void *dev, unsigned char *buf, unsigned int size);

#endif
