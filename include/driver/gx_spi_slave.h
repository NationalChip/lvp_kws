/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_SPI_SLAVE_H__
#define __GX_SPI_SLAVE_H__

#include <common.h>

#define START_STATE     (0)
#define RUNNING_STATE   (1)
#define DONE_STATE      (2)
#define ERROR_STATE     (-1)

typedef void (*completion_cb)(struct spi_device *spi);

 /**
 * @brief 初始化SPI slave 模块
 *
 * @param spi slave设备, 详细说明请参考 gxdocref spi_device
 *
 * @return int 初始化是否成功
 * @retval 0 成功
 * @retval -1 失败
 */

int spi_slave_init(struct spi_device* spi);

 /**
 * @brief SPI slave 接收数据
 *
 * @param spi slave设备, 详细说明请参考 gxdocref spi_device
 *
 * @param buffer 发送数据地址
 *
 * @param len 发送数据长度, byte为单位
 *
 * @param slave_completetion_cb 发送完成回调接口，如果为NULL, 则使用阻塞模式发送; 回调函数在中断中执行;
 *
 * @return int 发送是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int spi_slave_rx(struct spi_device *spi, void* buffer, int len, completion_cb slave_completetion_cb);

 /**
 * @brief SPI slave 发送数据
 *
 * @param spi slave设备, 详细说明请参考 gxdocref spi_device
 *
 * @param buffer 接收数据地址
 *
 * @param len 接收数据长度, byte为单位
 *
 * @param slave_completetion_cb 接收完成回调接口，如果为NULL, 则使用阻塞模式接收; 回调函数在中断中执行;
 *
 * @return int 接收是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int spi_slave_tx(struct spi_device *spi, const void* buffer, int len, completion_cb slave_completetion_cb);

/**
* @brief 终止SPI传输
*
* @param spi slave设备, 详细说明请参考 gxdocref spi_device
*
* @return int 是否成功
* @retval 0 成功
* @retval -1 失败
*/
int spi_slave_abort(struct spi_device *spi);

#endif /* __GX_SPI_SLAVE_H__ */

/** @}*/
