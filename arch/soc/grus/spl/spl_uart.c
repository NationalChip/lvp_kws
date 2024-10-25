#include <autoconf.h>
#include <base_addr.h>
#include <board_config.h>
#include <common.h>
#include <spl/spl.h>
#include <base_addr.h>
#include <misc_regs.h>
#include <soc.h>
#include <driver/gx_clock.h>

extern int spl_clk_get_frequence(GX_CLOCK_MODULE module);

#define  DSW_RBR  0x0
#define  DSW_DLL  0x0
#define  DSW_THR  0x0
#define  DSW_DLH  0x4
#define  DSW_IER  0x4
#define  DSW_FCR  0x8
#define  DSW_LSR  0x14
#define  DSW_MCR  0x10
#define  DSW_USR  0x7c
#define  DSW_LCR  0xc
#define  DSW_DLF  0xc0
#define  DSW_DMASA 0xA8 /* DMA Software Acknowledge Register */

#define  DSW_USR_BUSY 0x0
#define  DSW_LSR_TEMT 0x6
#define  DSW_LSR_DR   0x0

#define DW_WAIT_IDLE(base, offset, state, bit) do {\
	state = readl(((base) + (offset)));\
} while( ((state) & (1 << (bit))) )

#if (defined CONFIG_SERIAL_PORT) && (CONFIG_SERIAL_PORT == 0)
static unsigned int dw_serialbase = GX_REG_BASE_UART0;
#elif (defined CONFIG_SERIAL_PORT) && (CONFIG_SERIAL_PORT == 1)
static unsigned int dw_serialbase = GX_REG_BASE_UART1;
#else
static unsigned int dw_serialbase = GX_REG_BASE_UART0;
#endif

void serial_init_port(int baudrate, int use_rom_div)
{
#define UART_DLF_SIZE (1 << 4)
#define DLF_FACTOR 100

	int clkdiv = 0;
	int clkdlf = 0;
	unsigned int _usr = 0;
	unsigned int v;
	unsigned int uart_clk;

	writel(0x0, (dw_serialbase + DSW_IER));
	writel(0x3, (dw_serialbase + DSW_MCR));

	if (!use_rom_div) {
		if (dw_serialbase == GX_REG_BASE_UART0)
			uart_clk = spl_clk_get_frequence(CLOCK_MODULE_UART0);
		else
			uart_clk = spl_clk_get_frequence(CLOCK_MODULE_UART1);

		clkdiv = uint32_div(uart_clk, 16 * baudrate);
		v = uint32_mod(uart_clk, 16 * baudrate);
		v = uint32_div(v * DLF_FACTOR, 16 * baudrate);
		clkdlf = uint32_div(v * UART_DLF_SIZE, DLF_FACTOR);

		DW_WAIT_IDLE(dw_serialbase, DSW_USR, _usr, DSW_USR_BUSY);
		writel(0x80, (dw_serialbase + DSW_LCR));
		writel(clkdiv & 0xff, (dw_serialbase + DSW_DLL));
		writel((clkdiv >> 8) & 0xff, (dw_serialbase + DSW_DLH));
		writel(clkdlf & 0xff, (dw_serialbase + DSW_DLF));
	}

	DW_WAIT_IDLE(dw_serialbase, DSW_USR, _usr, DSW_USR_BUSY);
	writel(0x03, (dw_serialbase + DSW_LCR));
	writel(0x4f, (dw_serialbase + DSW_FCR));
}

static inline void serial_dma_init(void)
{
	writel(0x1, (dw_serialbase + DSW_DMASA));
	return ;
}

void serial_put(int ch)
{
	unsigned int  state;
	do {
		state = readl(dw_serialbase + DSW_LSR);
	} while ( !(state & (1 << DSW_LSR_TEMT)) );

	writel((unsigned int)ch, (dw_serialbase + DSW_THR));
}

void serial_put_sync(int ch)
{
	unsigned int  state;

	do {
		state = readl(dw_serialbase + DSW_LSR);
	} while ( !(state & (1 << DSW_LSR_TEMT)) );

	writel((unsigned int)ch, (dw_serialbase + DSW_THR));

	do {
		state = readl(dw_serialbase + DSW_LSR);
	} while ( !(state & (1 << DSW_LSR_TEMT)) );

	return ;
}

extern void spl_clk_set_gate_enable(GX_CLOCK_MODULE module, unsigned int enable);
void serial_init(int use_rom_baud, int baudrate)
{
#ifdef CONFIG_BOOT_BY_UART
	if (boot_info.boot_device == BOOT_DEVICE_UART) {
		dw_serialbase = boot_info.info.uart.uart_boot_port == 1 ? GX_REG_BASE_UART1 : GX_REG_BASE_UART0;

		spl_clk_set_gate_enable(CLOCK_MODULE_UART0_UART1, 1);
		spl_clk_set_gate_enable(CLOCK_MODULE_UART0, 1);
		spl_clk_set_gate_enable(CLOCK_MODULE_UART1, 1);

		serial_init_port(baudrate, use_rom_baud);

		return;
	}
#endif
	serial_init_port(CONFIG_SERIAL_BAUD_RATE, 0);
}

int serial_try_get(unsigned char *c)
{
	unsigned int state;

	state = readl(dw_serialbase + DSW_LSR);
	if (state & (1 << DSW_LSR_DR)) {
		*c = readl(dw_serialbase + DSW_RBR);
		return 0;
	}

	return -1;
}

int serial_get_char(void)
{
	unsigned int state;
	unsigned int ch;

	do {
		state = readl(dw_serialbase + DSW_LSR);
	}while (!(state & (1 << DSW_LSR_DR)));
	ch = readl(dw_serialbase + DSW_RBR);
	return (ch & 0xff);
}

#define SERIAL_RECV_SYNC()   do { \
	serial_put('r');              \
	serial_put('e');              \
	serial_put('a');              \
	serial_put('d');              \
	serial_put('y');              \
} while (0)

int serial_read_buff(unsigned char *buff, unsigned int len)
{
#ifdef CONFIG_DW_UART_DMA
	extern void spl_dw_dma_xfer_poll(void *dst, void *src, int len, int hw_src_per);
	int handle = (dw_serialbase == GX_REG_BASE_UART0) ? DMA_HS_PRE_UART0_RX : DMA_HS_PRE_UART1_RX;
	serial_dma_init();

	spl_dw_dma_config(buff, (void*)(dw_serialbase + DSW_RBR), len, handle);
	SERIAL_RECV_SYNC();
	spl_dw_dma_start();
#else
	int i = 0;

	SERIAL_RECV_SYNC();

	while (len--) {
		buff[i] = serial_get_char();
		i++;
	}
#endif
	return 0;
}

#ifdef CONFIG_BOOT_BY_UART

int serial_read_snpu(unsigned char *buff, unsigned int len)
{
#ifdef CONFIG_DW_UART_DMA
	extern void spl_dw_dma_xfer_poll(void *dst, void *src, int len, int hw_src_per);
	int handle = (dw_serialbase == GX_REG_BASE_UART0) ? DMA_HS_PRE_UART0_RX : DMA_HS_PRE_UART1_RX;
	serial_dma_init();

	spl_dw_dma_config(buff, (void*)(dw_serialbase + DSW_RBR), len, handle);
	spl_dw_dma_start();
#else
	int i = 0;

	while (len--) {
		buff[i] = serial_get_char();
		i++;
	}
#endif
	return 0;
}

void serial_hello(void)
{
	char *ok = "OK";
	unsigned char recv_char;
	int char_index = 0;

	spl_delay_init();
    serial_init(0, 2000000);

	while (1) {
		serial_put('G');
		serial_put('E');
		serial_put('T');

		while (!serial_try_get(&recv_char)) {
			if (recv_char == ok[char_index]) {
				char_index++;
				if (ok[char_index] == '\0')
					break;
			} else
				char_index = 0;
		}

		if (ok[char_index] == '\0')
			break;

		spl_mdelay(1);
	}
}
#endif
