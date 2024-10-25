#include <autoconf.h>
#include <base_addr.h>
#include <board_config.h>
#include <common.h>
#include <spl/spl.h>
#include <base_addr.h>
#include <misc_regs.h>
#include <soc.h>
#include <driver/gx_clock.h>

#define PMU_HW_I2C_REG_BASE     0xa0005000
#define PMU_I2C_SLAVE_ADDR_SELECT 0xa0010028

#define PMU_HW_I2C_COMMON_REG1  (PMU_HW_I2C_REG_BASE + 0xa0)
#define PMU_HW_I2C_COMMON_REG2  (PMU_HW_I2C_REG_BASE + 0xa4)

#define I2C_SLAVE_READY_ID  0x55
#define I2C_SLAVE_WRITE_ID  0xef
#define I2C_SLAVE_READ_ID   0x78
#define I2C_SLAVE_OK        0x46
#define I2C_SLAVE_FATAL     0x45
#define I2C_MASTER_NO_NEED_WRITE_FLASH   0x59
#define I2C_MASTER_NEED_WRITE_FLASH      0x58

#define DW_IC_CON           0x00
#define DW_IC_SAR           0x08
#define DW_IC_DATA_CMD      0x10
#define DW_IC_INTR_STAT     0x2c
#define DW_IC_INTR_MASK     0x30
#define DW_IC_RX_TL         0x38
#define DW_IC_TX_TL         0x3c
#define DW_IC_CLR_INTR      0x40
#define DW_IC_CLR_RD_REQ    0x50
#define DW_IC_CLR_TX_ABRT   0x54
#define DW_IC_CLR_STOP_DET  0x60
#define DW_IC_ENABLE        0x6c
#define DW_IC_RXFLR         0x78

#define DW_IC_INTR_RX_FULL   0x004
#define DW_IC_INTR_RD_REQ    0x020
#define DW_IC_INTR_TX_ABRT   0x040

#define DW_RX_FIFO_FULL_HLD  (1 << 9)

#define DW_IC_INTR_SLAVE_MASK       (DW_IC_INTR_RX_FULL | \
                    DW_IC_INTR_TX_ABRT | \
                    DW_IC_INTR_RD_REQ)

#define LENGTH_BYTE 4
#define CRC_BYTE    4
#define MASTER_WRITE_TARGET_ADDR 0x0
#define MASTER_READ_TARGET_ADDR 0x1

typedef enum i2c_slave_state {
	I2C_SLAVE_ERROR        = -1,
	I2C_SLAVE_FINISH       = 0,
	I2C_SLAVE_WAIT_CONNECT = 1,
	I2C_SLAVE_WAIT_LENGTH  = 2,
	I2C_SLAVE_WAIT_FILE    = 3,
	I2C_SLAVE_WAIT_ACK     = 4,
	I2C_SLAVE_TIMEOUT      = 5,
	I2C_SLAVE_WAIT_CRC     = 6,
} I2C_SLAVE_STATE;

typedef enum i2c_slave_event {
	I2C_SLAVE_RECEIVE_DATA,
	I2C_SLAVE_REQUESTED_DATA,
	I2C_SLAVE_STOP
} I2C_SLAVE_EVENT;

struct dw_i2c_dev {
	unsigned int base;
	unsigned int buffer_index;
	unsigned int length;
	unsigned int bytes;
	unsigned int crc;
	bool         crc_fail;
	I2C_SLAVE_STATE state;
	volatile unsigned char *buffer;
};

static struct dw_i2c_dev device;
static inline unsigned int dw_readl(struct dw_i2c_dev *dev, int offset)
{
	return readl(dev->base + offset);
}

static inline void dw_writel(struct dw_i2c_dev *dev, unsigned int b, int offset)
{
	writel(b, dev->base + offset);
}

extern void spl_clk_set_gate_enable(GX_CLOCK_MODULE module, unsigned int enable);
void i2c_slave_init(unsigned int i2c_info)
{
	unsigned int slave_addr = 0x35, select = i2c_info & 0x1;
	unsigned int id = (i2c_info >> 1) & 0x1;

	if (select)
		slave_addr = 0x36;
	else
		slave_addr = 0x35;

	if (id) {
		device.base = GX_REG_BASE_I2C1;
		spl_clk_set_gate_enable(CLOCK_MODULE_I2C1, 1);
	} else {
		device.base = GX_REG_BASE_I2C0;
		spl_clk_set_gate_enable(CLOCK_MODULE_I2C0, 1);
	}

	device.state = I2C_SLAVE_WAIT_CONNECT;
	device.buffer = (unsigned char *)CONFIG_STAGE2_DRAM_BASE;
	device.buffer_index = 0;
	device.length = 0;
	device.bytes = 0;
	device.crc_fail = false;

	dw_writel(&device, 0, DW_IC_ENABLE);

	/* Configure Tx/Rx FIFO threshold levels */
	dw_writel(&device, 0, DW_IC_TX_TL);
	dw_writel(&device, 0, DW_IC_RX_TL);

	/* configure the i2c slave */
	dw_writel(&device, DW_RX_FIFO_FULL_HLD, DW_IC_CON);

	/* Config slave adress */
	dw_writel(&device, slave_addr, DW_IC_SAR);

	/* Config irq */
	dw_writel(&device, DW_IC_INTR_SLAVE_MASK, DW_IC_INTR_MASK);

	/* Clear all interrupts */
	dw_readl(&device, DW_IC_CLR_INTR);

	/* Enable the adapter */
	dw_writel(&device, 1, DW_IC_ENABLE);

	writel(I2C_SLAVE_READY_ID, PMU_HW_I2C_COMMON_REG1);
}

static void i2c_slave_check_finish(struct dw_i2c_dev *dev)
{
	unsigned int i = 0, cal_crc = 0;

	if (dev->buffer_index >= dev->length) {
		for (i = 0; i < dev->length; i++)
			cal_crc += dev->buffer[i];

		if (cal_crc == dev->crc) {
			dev->state = I2C_SLAVE_WAIT_ACK;
			writel(I2C_SLAVE_OK, PMU_HW_I2C_COMMON_REG2);
		} else {
			dev->crc_fail = true;
			dev->state = I2C_SLAVE_WAIT_CRC;
			dev->length = 0;
			dev->bytes = 0;
			dev->buffer_index = 0;
			writel(I2C_SLAVE_FATAL, PMU_HW_I2C_COMMON_REG2);
		}
	}
}

static void i2c_slave_event(struct dw_i2c_dev *dev, I2C_SLAVE_EVENT event, unsigned char *val)
{
	if (event == I2C_SLAVE_RECEIVE_DATA) {
		switch (dev->state) {
			case I2C_SLAVE_WAIT_CONNECT:
				if (*val == I2C_SLAVE_WRITE_ID)
					dev->state = I2C_SLAVE_WAIT_CRC;
				break;
			case I2C_SLAVE_WAIT_CRC:
				dev->crc >>= 8;
				dev->crc |= ((unsigned int)(*val)) << 24;
				dev->bytes++;
				if (dev->bytes == CRC_BYTE) {
					dev->state = I2C_SLAVE_WAIT_LENGTH;
					dev->bytes = 0;
					writel(0x00, PMU_HW_I2C_COMMON_REG2);
				}
				break;
			case I2C_SLAVE_WAIT_LENGTH:
				dev->length >>= 8;
				dev->length |= ((unsigned int)(*val)) << 24;
				dev->bytes++;
				if (dev->bytes == LENGTH_BYTE) {
					dev->state = I2C_SLAVE_WAIT_FILE;
				}
				break;
			case I2C_SLAVE_WAIT_ACK:
				if (*val == I2C_MASTER_NEED_WRITE_FLASH) {
					boot_info.info.i2c.i2c_info |= 1 << 2;
					dev->state = I2C_SLAVE_FINISH;
				} else if (*val == I2C_MASTER_NO_NEED_WRITE_FLASH) {
					boot_info.info.i2c.i2c_info &= ~(1 << 2);
					dev->state = I2C_SLAVE_FINISH;
				}
				break;
			default:
				break;
		}
	} else if (event == I2C_SLAVE_REQUESTED_DATA) {
		switch (dev->state) {
			case I2C_SLAVE_WAIT_CONNECT:
				*val = 0x0;
				break;
			case I2C_SLAVE_WAIT_CRC:
			case I2C_SLAVE_WAIT_LENGTH:
			case I2C_SLAVE_WAIT_FILE:
				if (dev->crc_fail) {
					*val = I2C_SLAVE_FATAL;
					dev->crc_fail = false;
				} else {
					*val = I2C_SLAVE_READ_ID;
				}
				break;
			case I2C_SLAVE_ERROR:
				*val = I2C_SLAVE_FATAL;
				dev->state = I2C_SLAVE_WAIT_LENGTH;
				break;
			case I2C_SLAVE_WAIT_ACK:
				*val = I2C_SLAVE_OK;
				break;
			default:
				break;
		}
	}
}

static int i2c_slave_handler(struct dw_i2c_dev *dev)
{
	unsigned int stat, cnt, i;
	unsigned char val = 0;

	stat = dw_readl(dev, DW_IC_INTR_STAT);

	if (stat & DW_IC_INTR_RD_REQ) {
		dw_readl(dev, DW_IC_CLR_RD_REQ);

		i2c_slave_event(dev, I2C_SLAVE_REQUESTED_DATA, &val);
		dw_writel(dev, val, DW_IC_DATA_CMD);
	}

	if (stat & DW_IC_INTR_RX_FULL) {
		if (dev->state == I2C_SLAVE_WAIT_FILE) {
			cnt = dw_readl(dev, DW_IC_RXFLR);
			for (i = 0; i < cnt; i++) {
				dev->buffer[dev->buffer_index] = dw_readl(dev, DW_IC_DATA_CMD);
				dev->buffer_index++;
			}
			i2c_slave_check_finish(dev);
		} else {
			val = dw_readl(dev, DW_IC_DATA_CMD);
			i2c_slave_event(dev, I2C_SLAVE_RECEIVE_DATA, &val);
		}
	}

	if (stat & DW_IC_INTR_TX_ABRT) {
		dw_readl(dev, DW_IC_CLR_TX_ABRT);
		dev->state = I2C_SLAVE_ERROR;
	}

	return dev->state;
}

int i2c_slave_load_image(void)
{
	int ret;
	struct dw_i2c_dev *dev = &device;

	writel(I2C_SLAVE_READ_ID, PMU_HW_I2C_COMMON_REG1);

	while (1) {
		ret = i2c_slave_handler(&device);

		if (ret == I2C_SLAVE_FINISH) {
			dw_writel(dev, 0, DW_IC_INTR_MASK);
			dw_writel(dev, 0, DW_IC_ENABLE);
			writel(0, PMU_HW_I2C_COMMON_REG1);
			writel(0, PMU_HW_I2C_COMMON_REG2);
			return ret;
		}
	}

	return I2C_SLAVE_ERROR;
}

void i2c_slave_wait_connect(void)
{
	struct dw_i2c_dev *dev = &device;

	while (1) {
		i2c_slave_handler(dev);

		if (dev->state == I2C_SLAVE_WAIT_CRC)
			return;
	}
}

void i2c_slave_disable(void)
{
	unsigned int base = GX_REG_BASE_I2C0;

	readl(base + DW_IC_CLR_INTR);
	writel(0, base + DW_IC_INTR_MASK);
	writel(0, base + DW_IC_ENABLE);

	base = GX_REG_BASE_I2C1;
	readl(base + DW_IC_CLR_INTR);
	writel(0, base + DW_IC_INTR_MASK);
	writel(0, base + DW_IC_ENABLE);

	spl_clk_set_gate_enable(CLOCK_MODULE_I2C0, 0);
	spl_clk_set_gate_enable(CLOCK_MODULE_I2C1, 0);
}
