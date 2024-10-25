#ifndef __GX_CLOCK_V1_H__
#define __GX_CLOCK_V1_H__

/* 31 - 30 bit reserve */
#define AUDIO_IN_CLK_EN         (0x3f << 20)
#define AHB_USB_CLK_EN          (0x1 << 19)
/* 18 bit reserve */
#define USB_PHY_CLK_EN          (0x1 << 17)
#define USB_SRAM_CLK_EN         (0x1 << 16)
#define AUDIO_IN_SRAM_CLK_EN    (0x1 << 15)
#define DSP_SRAM_CLK_EN         (0x1 << 14)
#define SNPU_SRAM_CLK_EN        (0x1 << 13)
#define AUDIO_PLAY_SRAM_CLK_EN  (0x1 << 12)
#define SCPU_SRAM_CLK_EN        (0x1 << 11)
#define CONTROLLER_SRAM_CLK_EN  (0x1 << 10)
#define SRL_AUDIO_PLAY_CLK_EN   (0x1 << 9)
/* 8 bit reserve */
#define LODAC_CLK_EN            (0x1 << 7)
#define AUDIO_PLAY_CLK_EN       (0x1 << 6)
#define SECURE_CLK_EN           (0x1 << 5)
#define DSP_CLK_EN              (0x1 << 4)
#define SNPU_CLK_EN             (0x1 << 3)
#define APB2_UART_CLK_EN        (0x1 << 2)
#define APB2_0_CLK_EN           (0x1 << 1)

#define AUDIO_PLAY_GATE         (SRL_AUDIO_PLAY_CLK_EN | AUDIO_PLAY_SRAM_CLK_EN)
#define DSP_GATE                (DSP_CLK_EN | DSP_SRAM_CLK_EN)
#define USB_SLAVE_GATE          (USB_SRAM_CLK_EN | USB_PHY_CLK_EN)
#define SNPU_GATE               (SNPU_CLK_EN | SNPU_SRAM_CLK_EN)
#define LODAC_GATE              (LODAC_CLK_EN)
#define SECURE_GATE             (SECURE_CLK_EN)
#define AUDIO_IN_GATE           (AUDIO_IN_CLK_EN | AUDIO_IN_SRAM_CLK_EN)

#define MOUDLE_GATE  (DSP_GATE | USB_SLAVE_GATE | SNPU_GATE | LODAC_GATE | SECURE_GATE | AUDIO_PLAY_GATE)

#define SCPU_UART_GATE          (1 << 5)
#define SCPU_APB2_GATE          (1 << 4)
#define SCPU_CORE_SDC_GATE      (1 << 3)
#define SCPU_SDC_GATE           (1 << 3)
#define SCPU_AHB_GATE           (1 << 2)
#define SCPU_CORE_GATE          (1 << 1)

#define CSKY_IDLE_CLK_GATE (SCPU_UART_GATE | SCPU_APB2_GATE | SCPU_CORE_SDC_GATE | SCPU_SDC_GATE | SCPU_AHB_GATE | SCPU_CORE_GATE)

void clk_moudle_enable(unsigned int config);
void clk_moudle_disable(unsigned int config);
void clk_idle_config(unsigned int config);
void clk_suspend(void);
void clk_resume(void);
void clk_init(void);

#endif
