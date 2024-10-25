#include <stdint.h>
#include <string.h>
#include "printf.h"

#include "uart_sendboot.h"
#include <driver/gx_delay.h>

#ifdef DEBUG
#define DBG(fmt, ...) printf (fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...)
#endif

/* The following API need to be ported */
/*
 * xx_uart_config :
 *  @baudrate : 波特率
 *  @databits : 数据位
 *  @stopbits : 停止位
 *  @parity : 奇偶校验
 *  return : 配置成功返回 0，配置失败返回负值
 */
extern int xx_uart_config(int baudrate);
/*
 * xx_uart_read : 阻塞接收串口数据
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
extern int xx_uart_read(uint8_t *buf, int len);
/*
 * xx_uart_try_read : 尝试从串口中读取数据（立即返回）
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
extern int xx_uart_try_read(uint8_t *buf, int len);
/*
 * xx_uart_read_nonblock : 超时从串口中读取数据
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  @timeout_ms : 超时时间设置
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
extern int xx_uart_read_nonblock(uint8_t *buf, int len, int timeout_ms);
/*
 * xx_uart_write : 串口写数据
 *  @buf : 串口数据发送缓冲
 *  @len : 待发送数据的长度
 *  return : 返回串口实际发送的数据长度，出错则返回负值
 */
extern int xx_uart_write(const uint8_t *buf, int len);
/*
 * xx_msleep : 睡眠
 */
extern void xx_msleep(int ms);
/*
 * xx_get_time_ms : 获取当前系统时间
 */
extern uint64_t xx_get_time_ms(void);
/*
 * xx_reboot_chip : 重启 leo/leo mini
 */
extern void xx_reboot_chip(void);

extern uint32_t crc32 (uint32_t crc, const unsigned char/*Bytef*/ *p, unsigned int/*uInt*/ len);

#define BOOT_STAGE2_BAUDRATE 115200
#define BOOT_HEADER_LEN 32
#define BOOT_STAGE1_LEN (32*1024)
#define BOOT_BUF_LEN 64

#define BOOT_STAGE1_BAUD_GRUS 230400 // 大部分芯片都可以支持到 576000，不过处于通用的考虑，选择 230400
#define BOOT_STAGE2_BAUD_GRUS 576000 // 可配置的范围 115200 ~ 1500000 波特率太高stage发送会失败

typedef enum {
    CHIP_NONE = 0,
    CHIP_LEO,
    CHIP_LEO_MINI,
    CHIP_GRUS,
} CHIP;

typedef enum {
    CHIP_ID_GX8010 = 0x8010, // leo
    CHIP_ID_GX8008 = 0X8008, // leo mini
    CHIP_ID_GX8002 = 0x8002, // grus
} CHIP_ID;

typedef enum {
    CHIP_VERSION_V1 = 0x01,
} CHIP_VERSION;

struct boot_header {
    uint16_t chip_id;
    uint8_t  chip_type;
    uint8_t  chip_version;

    uint16_t boot_delay;
    uint8_t  baudrate;
    uint8_t  reserved_1;

    uint32_t stage1_size;
    uint32_t stage2_baud_rate; // baudrate config when downloading stage2 and coming into stage2 cmdline
    uint32_t stage2_size;
    uint32_t stage2_checksum;
    uint8_t  resv[8];
}__attribute__((packed));

static int uart_wait_strings(const char *str, int timeout_ms)
{
    int i;
    int stringlen = strlen(str);
    uint64_t start_time, current_time, remain_time;
    uint8_t ch;
    start_time = xx_get_time_ms();
    current_time = start_time;
    while (current_time < start_time + timeout_ms) {
        remain_time = start_time + timeout_ms - current_time;
        for (i = 0; i < stringlen; i++) {
            if (xx_uart_read_nonblock(&ch, 1, remain_time) == 1) {
                if (ch == '\r')
                    ch = '\n';
                if (ch != str[i])
                    break;
            } else {
                break;
            }

            if (i + 1 == stringlen)
                return 0;
        }
        current_time = xx_get_time_ms();
    }

    return -1;
}

static int32_t is_little_endian(void)
{
	uint32_t a = 0x11223344;
	uint8_t *p = (unsigned char *)&a;

	return (*p == 0x44) ? 1 : 0;
}

static uint32_t switch_endian(uint32_t v)
{
	return (((v >> 0) & 0xff) << 24) | (((v >> 8) & 0xff) << 16) | (((v >> 16) & 0xff) << 8) | (((v >> 24) & 0xff) << 0);
}

static uint32_t be32_to_cpu(uint32_t v)
{
	if (is_little_endian())
		return switch_endian(v);
	else
		return v;
}

__attribute__((unused)) static uint32_t cpu_to_be32(uint32_t v)
{
	if (is_little_endian())
		return switch_endian(v);
	else
		return v;
}

/*
 * boot file format:
 * ---------------------------------------------------------
 * | boot_header |  stage1    |         stage2             |
 * | (32 Bytes)  |  (32 KB)   |        variable            |
 * ---------------------------------------------------------
 */
static int uart_sendboot(const uint8_t *boot, int len)
{
    int i = 0;
    int stage1_size = 0;
    uint32_t stage2_offset = 0;
    uint32_t stage2_size = 0;
    uint32_t checksum = 0;
    int baudrate[] = {300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
    struct boot_header *boot_header;
    unsigned char wait_char, get_char;
    uint8_t buf[BOOT_BUF_LEN];
    unsigned char send_char = 0xef;
    const uint8_t *pdata;
    int stage1_crc_pass = 0;
    CHIP chip;
#define MAX_TIMES 5
    int try_times = 0;
    int ret;
    unsigned int stage2_baud_rate = BOOT_STAGE2_BAUD_GRUS;;

    if (!boot || len < BOOT_HEADER_LEN)
        return -1;

    boot_header = (struct boot_header *)boot;
    pdata = boot;

    switch (boot_header->chip_id) {
    case CHIP_ID_GX8002:
    case CHIP_ID_GX8008:
        if (boot_header->chip_id == CHIP_ID_GX8008)
                chip = CHIP_LEO_MINI;
            else
                chip = CHIP_GRUS;

            if (boot_header->chip_version == CHIP_VERSION_V1) {
                stage1_size = be32_to_cpu(boot_header->stage1_size);
                wait_char = 'M';
            } else {
                printf("Unsupported chip version!\n");
                return -1;
            }
            break;
    case CHIP_ID_GX8010:
        chip = CHIP_LEO;
        if (boot_header->chip_version == CHIP_VERSION_V1) {
            stage1_size = BOOT_STAGE1_LEN;
            wait_char = 'M';
        } else {
            printf("Unsupported chip version!\n");
            return -1;
        }
        break;
    default:
        printf("Unsupported chip id!\n");
        return -1;
    }

    if (len < (BOOT_HEADER_LEN + stage1_size))
        return -1;

    switch (chip) {
    case CHIP_LEO_MINI:
    case CHIP_GRUS:
        if (xx_uart_config(BOOT_STAGE1_BAUD_GRUS) < 0) {
            printf("Uart config error!\n");
            return -1;
        }
        break;
    default:
        if (xx_uart_config(baudrate[boot_header->baudrate])) {
            printf("Uart config error!\n");
            return -1;
        }
        break;
    }

    xx_reboot_chip();
    printf("board reboot finsh! wait bootfile send finish...\n");
    while (1) {
        ret = xx_uart_try_read(&get_char, 1);
        if (ret == 0)
            if (get_char == wait_char)
                break;
        // for leo mini
        xx_uart_write(&send_char, 1);
        xx_msleep(1);
    }
    stage1_size = stage1_size / 4;
    buf[0] = 'Y';
    buf[1] = (stage1_size >>  0) & 0xFF;
    buf[2] = (stage1_size >>  8) & 0xFF;
    buf[3] = (stage1_size >> 16) & 0xFF;
    buf[4] = (stage1_size >> 24) & 0xFF;
    xx_uart_write(buf, 5);

    /* Send boot stage1 */
    if (boot_header->chip_type == 0x01) {
        stage1_size = stage1_size * 4;
        while (1) {
            ret = xx_uart_write(pdata + sizeof(struct boot_header), stage1_size);
            if (ret != (int)stage1_size) {
                printf ("uart write failed! except : %d actual : %d\n", stage1_size, ret);
                return -1;
            }
            while (1) {
                ret = xx_uart_try_read(&get_char, 1);
                if (ret == 0) {
                    DBG ("recv : %x\n", get_char);
                    if (get_char == 'E' || get_char == 'F') {
                        if (get_char == 'E') {
                            printf ("CRC FAILED, recv : %c %x\n", get_char, get_char);
                            break;
                        } else {
                            DBG ("CRC SUCCESS\n");
                            stage1_crc_pass = 1;
                            break;
                        }
                    }
                }

                // 根据应用需要添加超时
                ;
            }
            if (stage1_crc_pass)
                break;

            if (++try_times >= MAX_TIMES) {
                printf ("stage1 send failed\n");
                return -1;
            }
        }
    } else {
        printf("Unsupported chip type!\n");
        return -1;
    }

    /* Send boot stage2 */
    switch (chip) {
        case CHIP_LEO_MINI:
        case CHIP_GRUS:
            // wait for stage1 to start
            if (uart_wait_strings("wfb", 4000) != 0) {
                printf ("Wait for \"wfb\",please check UART receive !\n");
                return -1;
            }

            // ACK
            sprintf ((char *)buf, "OK");
            xx_uart_write(buf, strlen((char *)buf));

            buf[0] = (stage2_baud_rate >>  0) & 0xff;
            buf[1] = (stage2_baud_rate >>  8) & 0xff;
            buf[2] = (stage2_baud_rate >> 16) & 0xff;
            buf[3] = (stage2_baud_rate >> 24) & 0xff;
            xx_uart_write(&buf[0], 4);

            // Wait for the completion of the previous data transmission
            if (uart_wait_strings("OK", 4000) != 0) {
                printf ("Wait for \"OK\",please check UART receive !\n");
                return -1;
            }

            DBG ("baud :  %d\n", stage2_baud_rate);
            if (xx_uart_config(stage2_baud_rate) < 0) {
                printf("Uart config error!\n");
                return -1;
            }
            break;
        default:
            /* Wait until boot stage1 send complete */
            for (i = 0; i < boot_header->boot_delay; i++)
                xx_msleep(1);

            xx_uart_config(BOOT_STAGE2_BAUDRATE); // 配置stage2波特率

            break;
    }
    if (uart_wait_strings("GET", 4000) < 0) {
        printf("Wait 'GET' string error!\n");
        return -1;
    }
    sprintf ((char *)buf, "OK");
    xx_uart_write(buf, strlen((char *)buf));

    DBG ("stage1 finished\n");

    buf[0] = 'S';
    xx_uart_write(buf, 1);

    stage2_offset = BOOT_HEADER_LEN + stage1_size;
    stage2_size = len - BOOT_HEADER_LEN - stage1_size;
    switch (chip) {
        case CHIP_LEO_MINI:
        case CHIP_GRUS:
            stage2_size = be32_to_cpu(boot_header->stage2_size);
            checksum    = be32_to_cpu(boot_header->stage2_checksum);
            break;
        default:
            for (i = 0, checksum = 0; i < stage2_size; i++)
                checksum += pdata[stage2_offset + i];
            break;
    }
    if (boot_header->chip_type == 0x01) {
        xx_uart_write((uint8_t *)&checksum, 4);
        xx_uart_write((uint8_t *)&stage2_size, 4);
    } else {
        printf("Unsupported chip type!\n");
        return -1;
    }
    while (1) {
        switch (chip) {
            case CHIP_LEO_MINI:
            case CHIP_GRUS:
                // 由于下载速率的提高，pc 需要等待 mcu 准备好 dma 后，再发送数据，不然 mcu 会来不及处理
                if (uart_wait_strings("ready", 1000) < 0) {
                    printf("Wait ready string error!\n");
                    return -1;
                }
                break;
            default:
                break;
        }

        xx_uart_write(pdata + stage2_offset, stage2_size);
        xx_uart_read(buf, 1);
        if (buf[0] == 'O') {
            break;
        } else if (buf[0] == 'E') {
            continue;
        } else {
            printf("Unknown character! : %c 0x%x\n", buf[0], buf[0]);
            return -1;
        }
    }
    if (uart_wait_strings("boot> ", 3000) < 0) {
        printf("Wait 'boot> ' string error!\n");
        return -1;
    }

    DBG ("stage2 finished\n");

    return 0;
}

#define MAX_TIMEOUT_MS 10000
#define MAX_SIZE_EACH_TIME 0x80000 //512k
#define SERIAL_RESULT_HEADER "[Result]:"
#define START_FLAG  "~sta~"
#define FINISH_FLAG "~fin~"

#include "leo_mini_boot.h"


static int pack_size = 0;
char  uart_cmd[128];


int DownloadInit(int32_t pkg_size)
{
    pack_size = pkg_size;
    xx_uart_init(57600);

    // env prepare
    int ret = uart_sendboot(leo_mini_boot, leo_mini_boot_len);
    if (ret != 0) {
        printf ("send boot failed\n");
        return -1;
    }

    printf ("boot file send finish\n");
    return 0;
}


int DownloadFirmware(uint32_t flash_addr, unsigned char *pkg_buff)
{

    uint8_t *buff;
    uint32_t offset;
    int32_t total_size, xfer_size;
    int32_t ret;
    uint32_t crc_cal;
    int32_t addr = flash_addr;

    buff       = pkg_buff;
    total_size = pack_size;

    memset(uart_cmd, 0, sizeof(uart_cmd));
    // serial down cmd : serialdown offset len each_size
    sprintf(uart_cmd, "serialdown 0x%x %d %d\n", addr, total_size, MAX_SIZE_EACH_TIME);
    DBG ("uart cmd : %s\n", uart_cmd);
    ret = xx_uart_write((const uint8_t *)uart_cmd, strlen(uart_cmd));
    if (ret != strlen(uart_cmd)) {
        printf ("uart send failed\n");
        return -1;
    }
    // send crc
    crc_cal = crc32(0, (unsigned char *)buff, total_size);
    xx_uart_write((uint8_t *)&crc_cal, 4);
    offset = 0;
    while (total_size > 0) {
        xfer_size = total_size > MAX_SIZE_EACH_TIME ? MAX_SIZE_EACH_TIME : total_size;

        // start
        ret = uart_wait_strings(START_FLAG, MAX_TIMEOUT_MS);
        if (ret < 0) {
            printf ("wait %s failed, cmd inde : %d\n", START_FLAG, 0);
            return -1;
        }

        ret = xx_uart_write(buff + offset, xfer_size);
        if (ret != xfer_size) {
            printf ("uart write failed, except : %d, actual : %d\n", xfer_size, ret);
            return -1;
        }
        // finish
        ret = uart_wait_strings(FINISH_FLAG, MAX_TIMEOUT_MS);
        if (ret < 0) {
            printf ("wait %s failed, cmd inde : %d\n", FINISH_FLAG, 0);
            return -1;
        }

        total_size -= xfer_size;
        offset     += xfer_size;
    }

    if (uart_wait_strings(SERIAL_RESULT_HEADER, 10000) != 0 || uart_wait_strings("SUCC", 4000) != 0) {
        printf ("download failed\n");
        return -1;
    }
    return 0;
}



int DownloadReboot(void)
{
    printf ("Download finish\n");

    // reboot
    memset(uart_cmd, 0, sizeof(uart_cmd));
    sprintf(uart_cmd, "reboot\n");
    int ret = xx_uart_write((const uint8_t *)uart_cmd, strlen(uart_cmd));
    if (ret != strlen(uart_cmd)) {
        printf ("send reboot cmd failed\n");
        return -1;
    }

    // xx_uart_done();     // 串口关闭
    printf ("reboot finish\n");
    return 0;
}

