#ifndef __INTR_TRANSMIT__
#define __INTR_TRANSMIT__

#define INTR_TRANSMIT_DW_UART1		(0)
#define INTR_TRANSMIT_DW_UART2		(1)
#define INTR_TRANSMIT_DW_I2C0		(2)
#define INTR_TRANSMIT_DW_I2C1		(3)
#define INTR_TRANSMIT_DW_SPI		(4)
#define INTR_TRANSMIT_COUNT1		(5)
#define INTR_TRANSMIT_EPORT1		(6)
#define INTR_TRANSMIT_DMA		(7)
#define INTR_TRANSMIT_SNPU		(8)
#define INTR_TRANSMIT_AUDIO		(9)
#define INTR_TRANSMIT_MTC		(10)
#define INTR_TRANSMIT_DSP		(11)
#define INTR_TRANSMIT_AUDIO_PLAY	(12)
#define INTR_TRANSMIT_USB_SLAVE_WU	(13)
#define INTR_TRANSMIT_USB_SLAVE		(14)
#define INTR_TRANSMIT_USB_DMA		(15)

#define REG_OFFSET_CK_A7_INT		(0x0274)
#define REG_OFFSET_A7_CK_INT		(0x0268)

#define INTR_TRANSMIT_REG_BASE		(GX_REG_BASE_RESET + REG_OFFSET_CK_A7_INT)
#define INTR_TRANSMIT_INT		(0X00)
#define INTR_TRANSMIT_CLEAN		(0X04)
#define INTR_TRANSMIT_ENABLE		(0X08)

struct intr_transmit;
typedef void intr_transmit_user_isr_t(struct intr_transmit*);

struct intr_transmit {
	unsigned short irq;
	unsigned short intr_transmit_chan;
	intr_transmit_user_isr_t *intr_transmit_user_isr;
};

extern int intr_transmit_request(intr_transmit_user_isr_t *intr_user_isr, int irq);
extern void intr_transmit_unrequest(int irq);

/** interrupt cpu to mcu */
#define CPU_INT_BASE		(GX_REG_BASE_RESET + REG_OFFSET_A7_CK_INT)
#define CPU_USER_ISR_NUM	(sizeof(int) * 8)
typedef void (*cpu_user_isr_t)(void *priv);

struct cpu_int_regs {
	unsigned int a7_ck_int;
	unsigned int a7_ck_clear;
	unsigned int a7_ck_enable;
	unsigned int ck_a7_int;
	unsigned int ck_a7_clear;
	unsigned int ck_a7_enable;
};

enum cpu_intr_chan {
	cpu_audio	= 0,
	cpu_vsp_flag	= 4,
	cpu_halt	= 30,
};

extern int cpu_intr_request(enum cpu_intr_chan chan, cpu_user_isr_t isr, void *priv);
extern void cpu_intr_unrequest(enum cpu_intr_chan chan);

#endif









