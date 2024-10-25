/** \addtogroup <scpu>
 *  @{
 */

#ifndef __SPI_H__
#define __SPI_H__

#include <list.h>
#include <string.h>

/**
 * @brief 传输单元配置
 */
struct spi_transfer {
	const void    *tx_buf;          ///< 发送数据内存地址
	void          *rx_buf;          ///< 接收数据内存地址
	unsigned      len;              ///< 发送/接收数据长度

	unsigned      cs_change:1;      ///< 片选是否变化
	unsigned      tx_nbits:3;       ///< 发送时每个时钟传输bit数
	unsigned      rx_nbits:3;       ///< 接收时每个时钟传输bit数
#define SPI_NBITS_SINGLE        0x01    /* 1bit transfer */
#define SPI_NBITS_DUAL          0x02    /* 2bits transfer */
#define SPI_NBITS_QUAD          0x04    /* 4bits transfer */
	unsigned char bits_per_word;    ///< 每个word位宽
	unsigned int  delay_usecs;      ///< 数据传输完成后与CS片选间隔时间
	unsigned int  speed_hz;         ///< 始终频率

	struct list_head transfer_list; ///< 传输单元链表，驱动内部使用
};

struct spi_device ;

/**
 * @brief 传输消息配置 */
struct spi_message {
	struct list_head      transfers;                   ///< 传输单元链表
	struct spi_device     *spi;                        ///< 从机设备
	unsigned              is_dma_mapped:1;             ///< 是否可以使用DMA
	void                  (*complete)(void *context);  ///< 传输完成回调函数
	void                  *context;                    ///< complete的参数
	unsigned              actual_length;               ///< 传输完成长度
	int                   status;                      ///< 执行结果
	struct list_head      queue;                       ///< spi_message所在队列
	void                  *state;                      ///< 运行状态
};

/**
 * @brief 主机配置
 */
struct spi_master {
	int              bus_num;                               ///< 总线序号
	unsigned int     num_chipselect;                        ///< 最大从机数
	int              (*setup)(struct spi_device *spi);      ///< 配置master接口
	int              (*transfer)(struct spi_device *spi,struct spi_message *mesg); ///< 传输数据
	void             (*cleanup)(struct spi_device *spi);    ///< 清除从机
	void             *data;                                 ///< 驱动私有数据
	void             *driver_data;                          ///< 驱动私有数据
	struct list_head list;                                  ///< 主机链表
};

/**
 * @brief 从机配置
 */
struct spi_device {
	struct spi_master *master;                           ///< 主机
	unsigned int      max_speed_hz;                      ///< 最大频率
	unsigned char     chip_select;                       ///< 片选
	unsigned char     bits_per_word;                     ///< word的bit数
	unsigned short    mode;                              ///< 模式
#define SPI_TX_DUAL     0x100   /* transmit with 2 wires */
#define SPI_TX_QUAD     0x200   /* transmit with 4 wires */
#define SPI_RX_DUAL     0x400   /* receive with 2 wires */
#define SPI_RX_QUAD     0x800   /* receive with 4 wires */
	void              *controller_state;                 ///< 控制器状态
	unsigned char     data_format;                       ///< 数据格式,stand(0),dual(1),quad(2),octal(3)
};

/**
 * @brief 系统中从设备的信息
 *
 * 这里代指类似Flash的从机设备
 */
struct sflash_master {
	int    bus_num;               ///< 所属总线编号
	struct spi_device spi;        ///< 配置信息
	struct list_head  list;       ///< 设备链表
};

struct spi_config
{
	unsigned char mode;
	unsigned char data_width;
	unsigned char dma_mode;
	unsigned char reserved;

	unsigned int max_hz;
};


/**
 * @brief 初始化消息
 *
 * @param m 消息体指针
 */
static inline void spi_message_init(struct spi_message *m)
{
	memset(m, 0, sizeof *m);
	INIT_LIST_HEAD(&m->transfers);
}

/**
 * @brief 添加传输单元
 *
 * @param t 传输单元
 * @param m 消息
 */
static inline void
spi_message_add_tail(struct spi_transfer *t, struct spi_message *m)
{
	list_add_tail(&t->transfer_list, &m->transfers);
}

/**
 * @brief 删除传输单元
 *
 * @param t 传输单元
 */
static inline void
spi_transfer_del(struct spi_transfer *t)
{
	list_del(&t->transfer_list);
}

/**
 * @brief 配置控制器
 *
 * @param spi 从机配置
 *
 * @return int
 * @retval 0 执行成功
 * @retval -12 内存不足
 */
static inline int
spi_setup(struct spi_device *spi)
{
	return spi->master->setup(spi);
}

/**
 * @brief 关闭控制器
 *
 * @param spi 从机配置
 *
 */
static inline void
spi_cleanup(struct spi_device *spi)
{
	spi->master->cleanup(spi);
}

/**
 * @brief 传输数据
 *
 * @param spi 从机设备
 * @param message 传输的消息
 *
 * @return int
 * @retval 0 执行成功
 * @retval -22 参数无效
 */
static inline int
spi_sync(struct spi_device *spi, struct spi_message *message)
{
	message->spi = spi;
	return spi->master->transfer(spi, message);
}

/**
 * @brief 发送数据
 * @param spi 从机设备
 * @param buf 发送数据内存地址
 * @param len 发送数据长度
 *
 * @return int
 * @retval 0 执行成功
 * @retval -22 参数无效
 */
static inline int
spi_write(struct spi_device *spi, const unsigned char *buf, size_t len)
{
	struct spi_transfer	t = {
			.tx_buf		= buf,
			.len		= len,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spi_sync(spi, &m);
}

/**
 * @brief 接收数据
 * @param spi 从机设备
 * @param buf 接收数据内存地址
 * @param len 接收数据长度
 *
 * @return int
 * @retval 0 执行成功
 * @retval -22 参数无效
 */
static inline int
spi_read(struct spi_device *spi, unsigned char *buf, size_t len)
{
	struct spi_transfer	t = {
			.rx_buf		= buf,
			.len		= len,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spi_sync(spi, &m);
}

/**
 * @brief 发送数据后接收数据
 * @param spi 从机设备
 * @param txbuf 发送数据内存地址
 * @param n_tx 发送数据长度
 * @param rxbuf 接收数据内存地址
 * @param n_rx 接收数据长度
 *
 * @return int
 * @retval 0 执行成功
 * @retval -22 参数无效
 */
int spi_write_then_read(struct spi_device *spi,
		const unsigned char *txbuf, unsigned n_tx,
		unsigned char *rxbuf, unsigned n_rx);

/**
 * @brief 注册GX SPI主机
 */
void gx_spi_probe(void);

/**
 * @brief 注册DW SPI主机
 */
void spi_master_v3_probe(void);
void dw_spi_probe(void);

#endif

/** @}*/
