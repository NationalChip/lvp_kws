#ifndef _SOC_H_
#define _SOC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------  Interrupt Number Definition  ------------------------ */
#define NR_IRQS              (32)
#define NR_FIQS              (0)

/**
 *  IRQS
 */
#define IRQ_NUM_SYSTICK      (0)
#define IRQ_NUM_GPIO         (1)
#define IRQ_NUM_AUDIO_IN     (2)
#define IRQ_NUM_PMU          (3)
#define IRQ_NUM_PMU_RTC      (4)
#define IRQ_NUM_PMU_OSC      (5)
#define IRQ_NUM_DW_UART1     (6)
#define IRQ_NUM_DW_UART2     (7)
#define IRQ_NUM_DW_I2C0      (8)
#define IRQ_NUM_DW_I2C1      (9)
#define IRQ_NUM_DMA          (10)
#define IRQ_NUM_WDT          (11)
#define IRQ_NUM_NPU          (12)
#define IRQ_NUM_AUDIO_PLAY   (13)
#define IRQ_NUM_TIMER        (14)
#define IRQ_NUM_DW_SPI       (15)
#define IRQ_NUM_GENERAL_SPI  (16)
#define IRQ_NUM_HW_I2C       (17)


/* ================================================================================ */
/* ================      Processor and Core Peripheral Section     ================ */
/* ================================================================================ */


/* ================================================================================ */
/* ================       Device Specific Peripheral Section       ================ */
/* ================================================================================ */

/* DMA config */
#define DW_DMAC_MAX_CH  2

#define DMA_HS_PRE_I2C0_RX  0
#define DMA_HS_PRE_I2C0_TX  1
#define DMA_HS_PRE_I2C1_RX  2
#define DMA_HS_PRE_I2C1_TX  3

#define DMA_HS_PRE_UART0_RX  6
#define DMA_HS_PRE_UART0_TX  7
#define DMA_HS_PRE_UART1_RX  4
#define DMA_HS_PRE_UART1_TX  5

#define DMA_HS_PRE_SPI0_RX  8
#define DMA_HS_PRE_SPI0_TX  9

#define DMA_HS_PRE_FLASH_SPI_RX  10
#define DMA_HS_PRE_FLASH_SPI_TX  11

/* cache */
#define DCACHE_LINE_SIZE  16


/* mmu channel num */
#define MMU_CHANNEL_MAX 6

/* ================================================================================ */
/* ================              Peripheral memory map             ================ */
/* ================================================================================ */


/* ================================================================================ */
/* ================             Peripheral declaration             ================ */
/* ================================================================================ */


#ifdef __cplusplus
}
#endif

#endif  /* SOC_H */
