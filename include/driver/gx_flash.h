/** \addtogroup <scpu>
 *  @{
 */

#ifndef __GX_FLASH_H__
#define __GX_FLASH_H__

#include "autoconf.h"
#include "gx_flash_common.h"

typedef struct gx_flash_dev GX_FLASH_DEV;

/**
 * @brief flash 设备定义
 */
typedef struct gx_flash_dev {
	GX_FLASH_DEV *(*init) (int bus, int cs, int speed, int mode);
	int  (*readdata)(unsigned int addr, unsigned char *data, unsigned int len);
	int  (*chiperase) (void);
	int  (*erasedata)(unsigned int addr, unsigned int len);
	int  (*pageprogram)(unsigned int addr, unsigned char *data, unsigned int len);
	void (*sync)(void);
	void (*test)(int argc, char *argv[]);
	void (*calcblockrange)(unsigned int addr, unsigned int len, unsigned int *pstart, unsigned int *pend);
	int  (*badinfo)(void);
	int  (*pageprogram_yaffs2)(unsigned int addr, unsigned char *data, unsigned int len);
	int  (*readoob)(unsigned int addr, unsigned char *data, unsigned int len);
	int  (*writeoob)(unsigned int addr, unsigned char *data, unsigned int len);
	char *(*gettype)(void);
	int (*getinfo)(enum gx_flash_info flash_info);
	int (*write_protect_mode)(void);
	int (*write_protect_status)(unsigned int *len);
	int (*write_protect_lock)(unsigned int addr);
	int (*write_protect_unlock)(void);
#ifdef CONFIG_MTD_TESTS
	int  (*readdata_noskip)(unsigned int addr, unsigned char *data, unsigned int len);
	void (*erasedata_noskip)(unsigned int addr, unsigned int len);
	int  (*pageprogram_noskip)(unsigned int addr, unsigned char *data, unsigned int len);
	int  (*scruberase) (unsigned int addr, unsigned int len);
	int (*block_markbad)(unsigned int addr);
#endif
	int (*otp_lock)(void);
	int (*otp_status)(unsigned char *data);
	int (*otp_erase)(void);
	int (*otp_write)(unsigned int addr, unsigned char *data, unsigned int len);
	int (*otp_read)(unsigned int addr, unsigned char* data, unsigned int len);
	int (*otp_get_region)(unsigned int *region);
	int (*otp_set_region)(unsigned int region);
	int (*otp_get_current_region)(unsigned int *region);
	int (*otp_get_region_size)(unsigned int *size);
	int (*block_isbad)(unsigned int addr);
	int (*calc_phy_offset)(unsigned int start, unsigned int logic_offset, unsigned int *phy_offset);
	int (*uid_read)(unsigned char *buf, int buf_len, int *ret_len);
} GX_FLASH_DEV;

/**
 * @brief flash初始化
 *
 * @param busnum spi总线号
 * @param cs spi片选
 * @param max_hz 支持最大的spi频率
 * @param spi_mode spi工作模式
 *
 * @return GX_FLASH_DEV flash设备
 * @retval NULL 设备初始化失败
 * @retval 非NULL flash设备
 */
GX_FLASH_DEV *gx_spi_flash_probe(unsigned int busnum, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode);

/**
 * @brief 读flash数据
 *
 * @param devp flash设备
 * @param addr 读取flash的起始地址
 * @param data 读缓存
 * @param len 读取flash的长度
 *
 * @return int 读取flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_readdata(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);

/**
 * @brief 擦除整片flash
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 *
 * @return int 擦除整片flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_chiperase (GX_FLASH_DEV *devp);

/**
 * @brief 擦除flash数据
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 擦除flash的起始地址
 * @param len 擦除flash的长度
 *
 * @return int 擦除flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_erasedata(GX_FLASH_DEV *devp, unsigned int addr, unsigned int len);

/**
 * @brief 强力擦除flash数据,不跳坏块
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 擦除flash的起始地址
 * @param len 擦除flash的长度
 *
 * @return int 擦除flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_scruberase(GX_FLASH_DEV *devp, unsigned int addr, unsigned int len);
/**
 * @brief 写flash数据
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 写flash的起始地址
 * @param data 写缓存
 * @param len 写flash的长度
 *
 * @return int 写flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_pageprogram(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);

/**
 * @brief flash数据同步接口，等待flash操作完成
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 */
void gx_spi_flash_sync(GX_FLASH_DEV *devp);
#ifdef CONFIG_MTD_TESTS
/**
 * @brief nand flash 测试接口，读flash数据,采用不跳坏块的方式
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 读取flash的起始地址
 * @param data 读缓存
 * @param len 读取flash的长度
 *
 * @return int 读取flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_readdata_noskip(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);

/**
 * @brief nand flash 测试接口，擦除flash数据,采用不跳坏块的方式
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 擦除flash的起始地址
 * @param len 擦除flash的长度
 *
 * @return int 擦除flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
void gx_spi_flash_erasedata_noskip(GX_FLASH_DEV *devp, unsigned int addr, unsigned int len);

/**
 * @brief nand flash 测试接口，写flash数据,采用不跳坏块的方式
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 写flash的起始地址
 * @param data 写缓存
 * @param len 写flash的长度
 *
 * @return int 写flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_pageprogram_noskip(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);
#endif

/**
 * @brief flash测试
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param argc 参数个数
 * @param argv 参数二维数组
 */
void gx_spi_flash_test(GX_FLASH_DEV *devp, int argc, char *argv[]);

/**
 * @brief 计算flash block对齐的起始地址和结束地址
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 写flash的起始地址
 * @param len 写flash的长度
 * @param pstart block对齐后的flash的起始地址
 * @param pend block对齐后flash的结束地址
 */
void gx_spi_flash_calcblockrange(GX_FLASH_DEV *devp, unsigned int addr, unsigned int len, unsigned int *pstart, unsigned int *pend);

/**
 * @brief 打印flash的坏块信息
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 *
 * @return int 是否调用成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_badinfo(GX_FLASH_DEV *devp);

/**
 * @brief 写flash yaffs2文件系统数据
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 写flash的起始地址
 * @param data 写缓存
 * @param len 写flash的长度
 *
 * @return int 写flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_pageprogram_yaffs2(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);

/**
 * @brief 读flash oob数据
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 读取flash的oob起始地址
 * @param data 读缓存
 * @param len 读取flash oob的长度
 *
 * @return int 读取flash oob是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_readoob(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);

/**
 * @brief 写flash oob数据
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 写flash的oob起始地址
 * @param data 写缓存
 * @param len 写flash oob的长度
 *
 * @return int 写flash oob是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_writeoob(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);

/**
 * @brief 获取flash 型号
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 *
 * @return char flash型号
 * @retval flash型号字符串
 */
char * gx_spi_flash_gettype(GX_FLASH_DEV *devp);

/**
 * @brief 获取flash 的详细信息
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param flash_info flash 信息的枚举, 详细说明请参考 gxdocref gx_flash_info
 *
 * @return int 获取flash详细信息是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_getinfo(GX_FLASH_DEV *devp, enum gx_flash_info flash_info);

/**
 * @brief 获取flash Unique ID
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param buf 缓存
 * @param buf_len 缓存长度
 * @param ret_len 读取的 UID 长度，长度单位字节
 *
 * @return int 获取flash UID是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_getuid(GX_FLASH_DEV *devp, unsigned char *buf, int buf_len, int *ret_len);

/**
 * @brief 获取flash 写保护模式
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 *
 * @return int 写保护模式
 * @retval -1 没有写保护功能
 * @retval 非-1 写保护模式
 */
int gx_spi_flash_write_protect_mode(GX_FLASH_DEV *devp);

/**
 * @brief 获取flash 保护模式状态
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param len 获取的写保护长度
 *
 * @return int 获取 flash  写保护状态是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_write_protect_status(GX_FLASH_DEV *devp, unsigned int *len);

/**
 * @brief 设置flash写保护
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr flash写保护地址
 *
 * @return int 设置falsh写保护是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_write_protect_lock(GX_FLASH_DEV *devp, unsigned long addr);

/**
 * @brief 解除flash写保护
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 *
 * @return int 解除falsh写保护是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_write_protect_unlock(GX_FLASH_DEV *devp);

/**
 * @brief 获取flash指定地址是否是坏块
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 需要判断坏块的地址
 *
 * @return int 是否是坏块
 * @retval 0 不是坏块
 * @retval 1 是坏块
 * @retval -1 调用失败
 */
int gx_spi_flash_block_isbad(GX_FLASH_DEV *devp, unsigned int addr);

/**
 * @brief 标记flash指定地址为坏块
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 需要标记坏块的地址
 *
 * @return int 标记坏块是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_block_markbad(GX_FLASH_DEV *devp, unsigned int addr);

/**
 * @brief 锁住当前flash otp 区域
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 *
 * @return int 是否锁flash otp 成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_otp_lock(GX_FLASH_DEV *devp);

/**
 * @brief 获取flash otp 区域锁状态
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param data 输出参数，写保护状态，0 未锁住，1 锁住
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_otp_status(GX_FLASH_DEV *devp, unsigned char *data);

/**
 * @brief 擦除当前flash otp 区域数据
 *
 * @param devp flash设备
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_otp_erase(GX_FLASH_DEV *devp);

/**
 * @brief 写当前flash otp 区域数据
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 写当前otp 区域的起始地址
 * @param data 写缓存
 * @param len 写flash otp的长度
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_otp_write(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);

/**
 * @brief 读当前flash otp 区域数据
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param addr 读当前otp 区域的起始地址
 * @param data 读缓存
 * @param len 读flash otp的长度
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_otp_read(GX_FLASH_DEV *devp, unsigned int addr, unsigned char *data, unsigned int len);

/**
 * @brief 获取flash otp区域数量
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param region 输出参数，otp区域数量
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_otp_get_region(GX_FLASH_DEV *devp, unsigned int *region);

/**
 * @brief 设置当前操作的flash otp区域
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param region 需要设置的otp区域
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_otp_set_region(GX_FLASH_DEV *devp, unsigned int region);

/**
 * @brief 获取当前所在的 otp 区域
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param region 当前所在的 otp 区域
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_otp_get_current_region(GX_FLASH_DEV *devp, unsigned int *region);

/**
 * @brief 获取 otp 区域大小
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param size otp 区域大小
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_otp_get_region_size(GX_FLASH_DEV *devp, unsigned int *size);
/**
 * @brief 从一个物理地址开始，计算逻辑偏移对应flash中的实际物理偏移
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param phy_start 物理起始地址
 * @param logic_offset 相对于起始地址的逻辑偏移
 * @param phy_start 输出参数，计算出来的相对于起始地址的物理偏移
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_calc_phy_offset(GX_FLASH_DEV *devp, unsigned int phy_start, unsigned int logic_offset, unsigned int *phy_offset);

/**
 * @brief 从一个物理地址开始，计算逻辑偏移，从逻辑偏移地址读取数据
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param phy_start 物理起始地址
 * @param logic_offs 相对于起始地址的逻辑偏移
 * @param buf 读缓存
 * @param len 读flash数据长度
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_logic_read(GX_FLASH_DEV *devp, unsigned int phy_start, unsigned int logic_offs, void *buf, unsigned int len);

/**
 * @brief 从一个物理地址开始，计算逻辑偏移，从逻辑偏移地址写入数据
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param phy_start 物理起始地址
 * @param logic_offs 相对于起始地址的逻辑偏移
 * @param buf 写缓存
 * @param len 写flash数据长度
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_logic_program(GX_FLASH_DEV *devp, unsigned int phy_start, unsigned int logic_offs, void *buf, unsigned int len);

/**
 * @brief 从物理起始地址开始，擦除到物理结束地址
 *
 * @param devp flash设备, 详细说明请参考 gxdocref GX_FLASH_DEV
 * @param phy_start 物理起始地址
 * @param phy_end 物理结束地址
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spi_flash_area_erase(GX_FLASH_DEV *devp, unsigned int phy_start, unsigned int phy_end);

/**
 * @brief flash的xip功能初始化
 *
 * @param cmd 读Flash命令
 * @param cmd_len 读Flash命令长度(bit为单位)
 * @param cmd_mode 命令模式, 1(标准模式)/2(双倍数)/4(四倍数)
 * @param addr_len 地址长度(bit为单位)
 * @param addr_mode 地址模式,1(标准模式)/2(双倍数)/4(四倍数)
 * @param mode_code reserved
 * @param mode_code_enable XIP模式位使能,请设置为1
 * @param wait_cycles 命令地址与数据之间的等待时钟数
 * @param spi_mode 数据传输模式,1(标准模式)/2(双倍数)/4(四倍数)
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_xip_init(unsigned int cmd,  unsigned int cmd_len,  unsigned int cmd_mode,
            unsigned int addr_len, unsigned int addr_mode,
            unsigned int mode_code, unsigned int mode_code_enable,
            unsigned int wait_cycles, unsigned int spi_mode);

#ifdef CONFIG_ARCH_GRUS
/**
 * @brief flash初始化
 *
 * @return int flash初始化是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spinor_flash_init(void);

/**
 * @brief 读flash数据
 *
 * @param addr 读取flash的起始地址
 * @param data 读缓存
 * @param len 读取flash的长度
 *
 * @return int 读取flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spinor_flash_readdata(unsigned int addr, unsigned char *data, unsigned int len);

/**
 * @brief 获取flash 型号
 *
 * @param devp flash设备
 *
 * @return char flash型号
 * @retval flash型号字符串
 */
char *gx_spinor_flash_gettype(void);

/**
 * @brief 获取flash 的详细信息
 *
 * @param flash_info flash 信息的枚举, 详细说明请参考 gxdocref gx_flash_info
 *
 * @return int 获取flash详细信息是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spinor_flash_getinfo(enum gx_flash_info flash_info);

/**
 * @brief 获取flash Unique ID
 *
 * @param buf 缓存
 * @param buf_len 缓存长度
 * @param ret_len 读取的 UID 长度，长度单位字节
 *
 * @return int 获取flash UID是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spinor_flash_getuid(unsigned char *buf, int buf_len, int *ret_len);

/**
 * @brief 擦除整片flash
 *
 * @return int 擦除整片flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spinor_flash_chiperase(void);

/**
 * @brief 擦除flash数据
 *
 * @param addr 擦除flash的起始地址
 * @param len 擦除flash的长度
 *
 * @return int 擦除flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spinor_flash_erasedata(unsigned int addr, unsigned int len);

/**
 * @brief 写flash数据
 *
 * @param addr 写flash的起始地址
 * @param data 写缓存
 * @param len 写flash的长度
 *
 * @return int 写flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spinor_flash_pageprogram(unsigned int addr, unsigned char *data, unsigned int len);
/**
 * @brief flash 进入低功耗模式
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spinor_flash_deep_powerdown(void);

/**
 * @brief flash 退出低功耗模式
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spinor_flash_release_powerdown(void);

/**
 * @brief flash 清理操作
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spinor_flash_exit(void);

/**
 * @brief flash 挂起进入低功耗模式
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spinor_flash_suspend(void);

/**
 * @brief flash 退出低功耗模式
 *
 * @return int 调用是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_spinor_flash_resume(void);

/**
 * @brief flash初始化
 *
 * @return int flash初始化是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_generic_spi_norflash_init(void);

/**
 * @brief 读flash数据
 *
 * @param addr 读取flash的起始地址
 * @param data 读缓存
 * @param len 读取flash的长度
 *
 * @return int 读取flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_generic_spi_norflash_readdata(unsigned int addr, unsigned char *data, unsigned int len);

/**
 * @brief 擦除flash数据
 *
 * @param addr 擦除flash的起始地址
 * @param len 擦除flash的长度
 *
 * @return int 擦除flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_generic_spi_norflash_erasedata(unsigned int addr, unsigned int len);

/**
 * @brief 写flash数据
 *
 * @param addr 写flash的起始地址
 * @param data 写缓存
 * @param len 写flash的长度
 *
 * @return int 写flash是否成功
 * @retval 0 成功
 * @retval -1 失败
 */
int gx_gereric_spi_norflash_pageprogram(unsigned int addr, unsigned char *data, unsigned int len);
#endif
#endif

/** @}*/
