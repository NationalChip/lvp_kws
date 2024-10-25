/*
 * Copyright (C) 2019 nationalchip, Inc.
 *
 */
#ifndef __STAGE1_DW_ENHENCE_HEADER_H_
#define __STAGE1_DW_ENHENCE_HEADER_H_
#include <common.h>
#include <base_addr.h>

#define FLASH_SPI_CS_REG         GX_REG_BASE_XIP_CS
/* SPI register */
#define SPIM_ENHENCE_BASE        GX_REG_BASE_XIP
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
#define SPIM_SPEED_LEVEL         40000000

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

#define SPI_SLVOE_OFFSET         12
#define SPI_SRL_OFFSET           13
#define SPI_CFS_OFFSET           16
#define SPI_FRF2_OFFSET          22

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

//Bits in XIP_CTRL
#define XIP_FRF             0
#define XIP_TRANS_TYPE      2
#define XIP_ADDR_L          4
#define XIP_INST_L          9
#define XIP_WAIT_CYCLES     13
#define XIP_INST_EN         22
#define XIP_PREFETCH_EN     29
#define XIP_MBL             26
#define XIP_CONT_XFER_EN    23
#define XIP_MD_BITS_EN      12
#define MBL_2               0x0
#define MBL_4               0x1
#define MBL_8               0x2
#define MBL_16              0x3
#define XIP_TRANS_TYPE_TT0  0x0
#define XIP_TRANS_TYPE_TT1  0x1
#define XIP_TRANS_TYPE_TT2  0x2
#define XIP_STANDARD        0x00
#define XIP_DUAL            0x01
#define XIP_QUAD            0x02
#define XIP_OCTAL           0x03
#define XIP_INST_L0         0x0
#define XIP_INST_L4         0x1
#define XIP_INST_L8         0x2
#define XIP_INST_L16        0x3

// GX series serial flash
#define GX_CMD_WREN           0x06      // write enable
#define GX_CMD_WRDI           0x04      // write disable
#define GX_CMD_RDSR1          0x05      // read status1 register
#define GX_CMD_RDSR2          0x35      // read status2/configure register
#define GX_CMD_RDSR3          0x15      // read status3/configure register
#define GX_CMD_WRSR1          0x01      // write status/configure register
#define GX_CMD_WRSR2          0X31      // write status/configure register
#define GX_CMD_WRSR3          0X11      // write status/configure register
#define GX_CMD_READ           0x03      // read data bytes
#define GX_CMD_FAST_READ      0x0b      // read data bytes at higher speed
#define GX_CMD_DREAD          0x3B      // read data dual output
#define GX_CMD_QREAD          0x6B      // read data quad output
#define GX_CMD_QREAD2         0xEB      // read data quad output
#define GX_CMD_PP             0x02      // page program
#define GX_CMD_QPP            0x32      // quad page program
#define GX_CMD_SE             0x20      // sector erase
#define GX_CMD_BE             0xd8      // block erase
#define GX_CMD_CE             0xc7      // chip erase
#define GX_CMD_DP             0xb9      // deep power down
#define GX_CMD_RES            0xab      // release from deep power down
#define GX_CMD_READID         0x9F      // read ID
#define GX_CMD_READID_BAK     0x90      // read ID for WinBond
#define GX_CMD_WREAR          0xC5      // Write Extended Address Register
#define GX_CMD_DPD            0xB9      // deep power down
#define GX_CMD_RPD            0xAB      // release power down

// dummy clock
#define FAST_READ_DUMMY_CLOCK 8         //fast read dummy clock size

/* Used for Macronix and Winbond flashes. */
#define GX_CMD_EN4B     0xb7    /* Enter 4-byte mode */
#define GX_CMD_EX4B     0xe9    /* Exit 4-byte mode */
/* Used for Spansion flashes only. */
#define GX_CMD_BRWR     0x17    /* Bank register write */
/* Usec for ISSI flashes only. */
#define GX_CMD_RDFR     0x48    /* Read Function Register */
#define GX_CMD_WRFR     0x42    /* Write Function Register */


// GX25Lxx series status regsiter
#define GX_STAT_WIP           0x01      // write in progress bit
#define GX_STAT_WEL           0x02      // write enable latch
#define GX_STAT_BP0           0x04      // block protect bit 0
#define GX_STAT_BP1           0x08      // block protect bit 1
#define GX_STAT_BP2           0x10      // block protect bit 2
#define GX_STAT_SRWD          0x80      // status register write protect

// page size of pageprogram command
#define GX_PAGESIZE            256      // GX25L : 256 bytes

/*
 * spi read/write/erase  mode Macro
 */
#define FAST_READ             0X10
#define DUAL_READ             0X20
#define QUAD_READ             0X40

#define FAST_PP               0X100
#define DUAL_PP               0X200

#define FAST_ERASE            0X1000
#define DUAL_ERASE            0X2000

#define SFLASH_READY_TIMEOUT       200000         //0.2s

#define STANDARD_TO_8BITS_MODE     ((SPI_FRF2_STANDARD << SPI_FRF2_OFFSET) | (SPI_TMOD_TO << SPI_TMOD_OFFSET) | SPI_DFS_8BIT)
#define STANDARD_RO_8BITS_MODE     ((SPI_FRF2_STANDARD << SPI_FRF2_OFFSET) | (SPI_TMOD_RO << SPI_TMOD_OFFSET) | SPI_DFS_8BIT)
#define STANDARD_RO_32BITS_MODE    ((SPI_FRF2_STANDARD << SPI_FRF2_OFFSET) | (SPI_TMOD_RO << SPI_TMOD_OFFSET) | SPI_DFS_32BIT)

#define EEPROM_8BITS_MODE          ((SPI_FRF2_STANDARD << SPI_FRF2_OFFSET) | (SPI_TMOD_EPROMREAD << SPI_TMOD_OFFSET) | SPI_DFS_8BIT)
#define DUAL_RO_32BIT_MODE         ((SPI_FRF2_DUAL << SPI_FRF2_OFFSET) | (SPI_TMOD_RO << SPI_TMOD_OFFSET) | SPI_DFS_32BIT)
#define DUAL_RO_8BIT_MODE          ((SPI_FRF2_DUAL << SPI_FRF2_OFFSET) | (SPI_TMOD_RO << SPI_TMOD_OFFSET) | SPI_DFS_8BIT)

#define QUAD_TO_32BITS_MODE        ((SPI_FRF2_QUAD << SPI_FRF2_OFFSET) | (SPI_TMOD_TO << SPI_TMOD_OFFSET) | SPI_DFS_32BIT)
#define QUAD_TO_8BITS_MODE         ((SPI_FRF2_QUAD << SPI_FRF2_OFFSET) | (SPI_TMOD_TO << SPI_TMOD_OFFSET) | SPI_DFS_8BIT)
#define QUAD_RO_32BITS_MODE        ((SPI_FRF2_QUAD << SPI_FRF2_OFFSET) | (SPI_TMOD_RO << SPI_TMOD_OFFSET) | SPI_DFS_32BIT)
#define QUAD_RO_8BITS_MODE         ((SPI_FRF2_QUAD << SPI_FRF2_OFFSET) | (SPI_TMOD_RO << SPI_TMOD_OFFSET) | SPI_DFS_8BIT)

#define STRETCH_INST8_ADDR24       (1<<SPI_CTRLR0_CLK_STRETCH_OFFSET | 2<<SPI_CTRLR0_INST_L_OFFSET | 6<<SPI_CTRLR0_ADDR_L_OFFSET)
#define STRETCH_WAIT8_INST8_ADDR24 (1<<SPI_CTRLR0_CLK_STRETCH_OFFSET | 8 << SPI_CTRLR0_WAIT_CYCLES_OFFSET | \
					2<< SPI_CTRLR0_INST_L_OFFSET | 6<<SPI_CTRLR0_ADDR_L_OFFSET)
#define STRETCH_WAIT4_INST8_ADDR24 (1<<SPI_CTRLR0_CLK_STRETCH_OFFSET | 6 << SPI_CTRLR0_WAIT_CYCLES_OFFSET | \
					2<< SPI_CTRLR0_INST_L_OFFSET | 6<<SPI_CTRLR0_ADDR_L_OFFSET | 0x01)

#endif /* __STAGE1_DW_ENHENCE_HEADER_H_ */
