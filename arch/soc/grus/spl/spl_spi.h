#ifndef __SPL_SPI_H__
#define __SPL_SPI_H__
#include <common.h>
#include <base_addr.h>

/* SPI register */
#define SPIM_ENHENCE_BASE        GX_REG_BASE_SPI1
#define SPIM_CTRLR0              (0x00 + SPIM_ENHENCE_BASE)
#define SPIM_CTRLR1              (0x04 + SPIM_ENHENCE_BASE)
#define SPIM_ENR                 (0x08 + SPIM_ENHENCE_BASE)
#define SPIM_SER                 (0x10 + SPIM_ENHENCE_BASE)
#define SPIM_BAUDR               (0x14 + SPIM_ENHENCE_BASE)
#define SPIM_TXFTLR              (0x18 + SPIM_ENHENCE_BASE)
#define SPIM_RXFTLR              (0x1c + SPIM_ENHENCE_BASE)
#define SPIM_TXFLR               (0x20 + SPIM_ENHENCE_BASE)
#define SPIM_RXFLR               (0x24 + SPIM_ENHENCE_BASE)
#define SPIM_SR                  (0x28 + SPIM_ENHENCE_BASE)
#define SPIM_IMR                 (0x2c + SPIM_ENHENCE_BASE)
#define SPIM_ISR                 (0x30 + SPIM_ENHENCE_BASE)
#define SPIM_RISR                (0x34 + SPIM_ENHENCE_BASE)
#define DW_SPI_TXOICR            (0x38 + SPIM_ENHENCE_BASE)
#define DW_SPI_RXOICR            (0x3c + SPIM_ENHENCE_BASE)
#define DW_SPI_RXUICR            (0x40 + SPIM_ENHENCE_BASE)
#define SPIM_ICR                 (0x48 + SPIM_ENHENCE_BASE)
#define SPIM_DMACR               (0x4c + SPIM_ENHENCE_BASE)
#define SPIM_DMATDLR             (0x50 + SPIM_ENHENCE_BASE)
#define SPIM_DMARDLR             (0x54 + SPIM_ENHENCE_BASE)
/* SPI先发/收低地址数据 */
#define SPIM_TXDR_LE             (0x60 + SPIM_ENHENCE_BASE)
/* SPI先发/收高地址数据 */
#define SPIM_TXDR_BE             (0x64 + SPIM_ENHENCE_BASE)
/* SPI先发/收低地址数据 */
#define SPIM_RXDR_LE             (0x60 + SPIM_ENHENCE_BASE)
/* SPI先发/收高地址数据 */
#define SPIM_RXDR_BE             (0x64 + SPIM_ENHENCE_BASE)
#define SPIM_SAMPLE_DLY          (0xF0 + SPIM_ENHENCE_BASE)
#define SPIM_SPI_CTRL0           (0xF4 + SPIM_ENHENCE_BASE)
#define XIP_MODE_BITS            (0xfc  + SPIM_ENHENCE_BASE)
#define XIP_INCR_INST            (0x100 + SPIM_ENHENCE_BASE)
#define XIP_WRAP_INST            (0x104 + SPIM_ENHENCE_BASE)
#define XIP_CTRL                 (0x108 + SPIM_ENHENCE_BASE)
#define XIP_SER                  (0x10c + SPIM_ENHENCE_BASE)
#define XIP_CNT_TIME_OUT         (0x114 + SPIM_ENHENCE_BASE)

/* --------Bit fields in CTRLR0--------begin */
#define SPI_DFS_OFFSET           0
#define SPI_DFS_8BIT             0x07
#define SPI_DFS_16BIT            0x0F
#define SPI_DFS_32BIT            0x1F

#define SPI_FRF2_STANDARD        0x0
#define SPI_FRF2_DUAL            0x1
#define SPI_FRF2_QUAD            0x2
#define SPI_FRF2_OCTAL           0x3

#define SPI_FRF_OFFSET           6    /* Frame Format */
#define SPI_FRF_SPI              0x0  /* motorola spi */
#define SPI_FRF_SSP              0x1  /* Texas Instruments SSP*/
#define SPI_FRF_MICROWIRE        0x2  /*  National Semiconductors Microwire */
#define SPI_FRF_RESV             0x3

#define SPI_MODE_OFFSET          8     /* SCPH & SCOL */
#define SPI_SCPH_OFFSET          8     /* Serial Clock Phase */
#define SPI_SCPH_TOGMID          0     /* Serial clock toggles in middle of first data bit */
#define SPI_SCPH_TOGSTA          1     /* Serial clock toggles at start of first data bit */
#define SPI_SCOL_OFFSET          9     /* Serial Clock Polarity */

#define SPI_TMOD_OFFSET          10
#define SPI_TMOD_MASK            (0x3 << SPI_TMOD_OFFSET)
#define SPI_TMOD_TR              0x0   /* xmit & recv */
#define SPI_TMOD_TO              0x1   /* xmit only */
#define SPI_TMOD_RO              0x2   /* recv only */
#define SPI_TMOD_EPROMREAD       0x3   /* eeprom read mode */

#define SPI_MST_SLAVE            0
#define SPI_MST_MASTER           1

#define SPI_SLVOE_OFFSET         12
#define SPI_SRL_OFFSET           13
#define SPI_CFS_OFFSET           16
#define SPI_FRF2_OFFSET          22
#define SPI_MST_OFFSET           31

#define SPI_SIZE_08_BIT          7

#define SPI_CTRLR0_CLK_STRETCH_OFFSET   30
#define SPI_CTRLR0_WAIT_CYCLES_OFFSET   11
#define SPI_CTRLR0_INST_L_OFFSET        8
#define SPI_CTRLR0_ADDR_L_OFFSET        2
#define SPI_CTRLR0_TRANS_TYPE_OFFSET    0

#define SPI_ADDR_NULL                   0x0
#define SPI_ADDR_24BITS                 0x6
#define SPI_ADDR_32BITS                 0x8
#define SPI_INST_8BITS                  0x2
#define SPI_CTRLR0_TRANS_TYPE_OFFSET    0
#define SPI_CTRLR0_TRANS_TYPE_OFFSET    0
/* --------Bit fields in CTRLR0--------end */
/* Bit fields in SR, 7 bits */
#define SR_MASK                 0x7f            /* cover 7 bits */
#define SR_BUSY                 (1 << 0)
#define SR_TF_NOT_FULL          (1 << 1)
#define SR_TF_EMPT              (1 << 2)
#define SR_RF_NOT_EMPT          (1 << 3)
#define SR_RF_FULL              (1 << 4)
#define SR_TX_ERR               (1 << 5)
#define SR_DCOL                 (1 << 6)
/* Bit fields in ISR, IMR, RISR, 7 bits */
#define SPI_INT_TXEI            (1 << 0)
#define SPI_INT_TXOI            (1 << 1)
#define SPI_INT_RXUI            (1 << 2)
#define SPI_INT_RXOI            (1 << 3)
#define SPI_INT_RXFI            (1 << 4)
#define SPI_INT_MSTI            (1 << 5)
#define SPI_INT_XRXOI           (1 << 6)

/* Bit fields in DMACR */
#define SPI_DMACR_TX_ENABLE     (1 << 1)
#define SPI_DMACR_RX_ENABLE     (1 << 0)

/* Bit fields in ICR */
#define SPI_CLEAR_INT_ALL       (1<< 0)

#define STANDARD_SLAVE_RO_8BITS_MODE     ((SPI_MST_SLAVE << SPI_MST_OFFSET) | (SPI_SIZE_08_BIT << SPI_CFS_OFFSET) | (SPI_TMOD_RO << SPI_TMOD_OFFSET) | SPI_DFS_8BIT)
#endif
