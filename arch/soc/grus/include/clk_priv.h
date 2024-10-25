#ifndef __CLK_PRIV_H__
#define __CLK_PRIV_H__

#include <base_addr.h>
#include <driver/gx_clock.h>

typedef struct {
	GX_CLOCK_MODULE mod;
	int8_t gate_high_offs;
	int8_t gate_all_offs;
	int8_t clk_offs;

	GX_CLOCK_DIV *div;
	GX_CLOCK_DTO *dto;
} GX_CLOCK_MODULE_PARAM;

typedef struct {
	GX_CLOCK_MODULE_PARAM *param;

	unsigned int base_addr;
	unsigned int clk_sel_addr;
	unsigned int gate_val_addr;
	unsigned int gate_set_addr;
	unsigned int gate_clr_addr;
} GX_CLOCK_MODULE_INFO;

static GX_CLOCK_DIV gx_clock_div_table[] = {
	{.addr = 0x80, .offs =  8, .mask =  0x3f}, // CLOCK_MODULE_RTC
	{.addr = 0x80, .offs = 16, .mask =  0x3f}, // CLOCK_MODULE_PMU
	{.addr = 0x80, .offs = 24, .mask =  0x3f}, // CLOCK_MODULE_AUDIO_IN_SYS
	{.addr = 0x84, .offs = 16, .mask =  0x3f}, // CLOCK_MODULE_FFT
	{.addr = 0x84, .offs = 24, .mask =  0x3f}, // CLOCK_MODULE_GPIO
	{.addr = 0x84, .offs =  0, .mask =  0x3f}, // CLOCK_MODULE_HW_I2C
	{.addr = 0x80, .offs =  0, .mask =  0x3f}, // CLOCK_MODULE_SRAM
	{.addr = 0x88, .offs =  0, .mask = 0x3ff}, // CLOCK_MODULE_AUDIO_IN_ADC
	{.addr = 0x84, .offs =  8, .mask =  0x3f}, // CLOCK_MODULE_AUDIO_IN_PDM
	{.addr = 0x88, .offs = 12, .mask =  0x3f}, // CLOCK_MODULE_OSC_REF

	{.addr = 0x84, .offs = 16, .mask =  0x3f}, // CLOCK_MODULE_SCPU
	{.addr = 0x80, .offs =  8, .mask =  0x3f}, // CLOCK_MODULE_I2C0_I2C1
	{.addr = 0x84, .offs =  8, .mask =  0x3f}, // CLOCK_MODULE_NPU
	{.addr = 0x80, .offs = 16, .mask =  0x3f}, // CLOCK_MODULE_FLASH_SPI
	{.addr = 0x80, .offs = 24, .mask =  0x3f}, // CLOCK_MODULE_GENERAL_SPI
	{.addr = 0x84, .offs =  0, .mask =  0x3f}, // CLOCK_MODULE_AUDIO_LODAC
	{.addr = 0x80, .offs =  0, .mask =  0x3f}, // CLOCK_MODULE_TIMER_WDT
};

static GX_CLOCK_DTO gx_clock_dto_table[] = {
	{.addr = 0x24}, // CLOCK_MODULE_AUDIO_IN_SYS
	{.addr = 0x24}, // CLOCK_MODULE_UART0_UART1
	{.addr = 0x28}, // CLOCK_MODULE_AUDIO_PLAY
};

static GX_CLOCK_MODULE_PARAM gx_clock_param_table[] = {
	/*  Mod                    HGate AGate Clk             Div                     Dto         */
	{CLOCK_MODULE_RTC         ,  0 ,  1 ,  1 , gx_clock_div_table +  0 , NULL                  },
	{CLOCK_MODULE_PMU         ,  2 ,  3 ,  3 , gx_clock_div_table +  1 , NULL                  },
	{CLOCK_MODULE_AUDIO_IN_SYS,  4 ,  5 ,  6 , gx_clock_div_table +  2 , gx_clock_dto_table + 0},
	{CLOCK_MODULE_FFT         ,  7 ,  8 ,  9 , gx_clock_div_table +  3 , NULL                  },
	{CLOCK_MODULE_GPIO        ,  9 , 10 , 10 , gx_clock_div_table +  4 , NULL                  },
	{CLOCK_MODULE_HW_I2C      , 11 , 12 , 11 , gx_clock_div_table +  5 , NULL                  },
	{CLOCK_MODULE_SRAM        , 13 , 14 , 12 , gx_clock_div_table +  6 , NULL                  },
	{CLOCK_MODULE_AUDIO_IN_ADC,  4 ,  6 , 19 , gx_clock_div_table +  7 , NULL                  },
	{CLOCK_MODULE_AUDIO_IN_PDM,  4 , 19 , 20 , gx_clock_div_table +  8 , NULL                  },
	{CLOCK_MODULE_OSC_REF     , 20 , 21 , 24 , gx_clock_div_table +  9 , NULL                  },

	{CLOCK_MODULE_SCPU        ,  4 , 12 ,  0 , gx_clock_div_table + 10 , NULL                  },
	{CLOCK_MODULE_UART0_UART1 ,  5 ,  5 ,  1 , NULL                    , gx_clock_dto_table + 1},
	{CLOCK_MODULE_UART0       ,  5 , 24 , -1 , NULL                    , NULL                  },
	{CLOCK_MODULE_UART1       ,  5 , 25 , -1 , NULL                    , NULL                  },
	{CLOCK_MODULE_AUDIO_PLAY  ,  6 , 26 ,  2 , NULL                    , gx_clock_dto_table + 2},
	{CLOCK_MODULE_I2C0_I2C1   ,  7 ,  7 ,  3 , gx_clock_div_table + 11 , NULL                  },
	{CLOCK_MODULE_I2C0        ,  7 , 28 , -1 , NULL                    , NULL                  },
	{CLOCK_MODULE_I2C1        ,  7 , 29 , -1 , NULL                    , NULL                  },
	{CLOCK_MODULE_NPU         ,  8 ,  8 ,  4 , gx_clock_div_table + 12 , NULL                  },
	{CLOCK_MODULE_FLASH_SPI   ,  9 ,  9 ,  5 , gx_clock_div_table + 13 , NULL                  },
	{CLOCK_MODULE_GENERAL_SPI , 10 , 10 ,  6 , gx_clock_div_table + 14 , NULL                  },
	{CLOCK_MODULE_AUDIO_LODAC , 11 , 11 ,  7 , gx_clock_div_table + 15 , NULL                  },
	{CLOCK_MODULE_TIMER_WDT   , 27 , 27 , 11 , gx_clock_div_table + 16 , NULL                  },
	{CLOCK_MODULE_TIMER       , 27 , 22 , -1 , NULL                    , NULL                  },
	{CLOCK_MODULE_WDT         , 27 , 23 , -1 , NULL                    , NULL                  },
	{CLOCK_MODULE_DMA         , -1 , 14 , -1 , NULL                    , NULL                  },
};

static void __reg_set_val(unsigned int addr, unsigned int offs, int val, unsigned int mask)
{
	unsigned int value = 0;
	value = *(volatile unsigned int*)addr;

	value &= ~(mask << offs);
	value |=  (val  << offs);

	*(volatile unsigned int*)addr = value;
}

static unsigned int __reg_get_val(unsigned int addr, unsigned int offs, unsigned int mask)
{
	unsigned int value = 0;
	value = *(volatile unsigned int*)addr;

	return (value >> offs) & mask;
}

static int __module_get_info(GX_CLOCK_MODULE module, GX_CLOCK_MODULE_INFO *info)
{
	if (module >= CLOCK_MODULE_MAX)
		return -1;

	if (!info)
		return -1;

	info->param = NULL;
	if (gx_clock_param_table[module].mod == module) {
		info->param = &gx_clock_param_table[module];
	} else  {
		for (int i = 0; i < sizeof(gx_clock_param_table)/sizeof(GX_CLOCK_MODULE_PARAM); i++) {
			if (gx_clock_param_table[i].mod == module) {
				info->param = &gx_clock_param_table[i];
				break;
			}
		}
	}

	if (!info->param)
		return -1;

	if (module < CLOCK_MODULE_SCPU) {
		info->base_addr     = GX_REG_BASE_PMU_CONFIG;
		info->clk_sel_addr  = GX_REG_BASE_PMU_CONFIG + GX_CLOCK_PMU_SOURCE_SEL;
		info->gate_val_addr = GX_REG_BASE_PMU_CONFIG + GX_CLOCK_PMU_INHIBIT_NORM;
		info->gate_set_addr = GX_REG_BASE_PMU_CONFIG + GX_CLOCK_PMU_INHIBIT_SET;
		info->gate_clr_addr = GX_REG_BASE_PMU_CONFIG + GX_CLOCK_PMU_INHIBIT_CLR;
	} else {
		info->base_addr     = GX_REG_BASE_MCU_CONFIG;
		info->clk_sel_addr  = GX_REG_BASE_MCU_CONFIG + GX_CLOCK_MCU_SOURCE_SEL;
		info->gate_val_addr = GX_REG_BASE_MCU_CONFIG + GX_CLOCK_MCU_INHIBIT_NORM;
		info->gate_set_addr = GX_REG_BASE_MCU_CONFIG + GX_CLOCK_MCU_INHIBIT_SET;
		info->gate_clr_addr = GX_REG_BASE_MCU_CONFIG + GX_CLOCK_MCU_INHIBIT_CLR;
	}

	return 0;
}

static int __module_get_div(GX_CLOCK_MODULE_INFO info)
{
	if (!info.param->div)
		return 0;

	unsigned int offs = info.param->div->offs;
	unsigned int mask = info.param->div->mask;

	unsigned int div = (*(volatile unsigned int*)(info.base_addr + info.param->div->addr)) >> offs & mask;

	return ((div == 0) ? 0 : div + 1);
}

static int __module_get_dto(GX_CLOCK_MODULE_INFO info, int *enable)
{
	if (!info.param->dto)
		return 0;

	unsigned int dto_addr = info.base_addr + info.param->dto->addr;
	unsigned int dto = *(volatile unsigned int*)dto_addr;

	if (enable != NULL)
		*enable = !((dto >> 27) & 0x1);

	return dto & 0x01ffffff;
}

static inline void _clk_set_pll(GX_CLOCK_PLL *pll)
{
	if (pll == NULL)
		return;

	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x3c) &= ~(0x01 << 1); //disable
	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x3c) &= ~(0x01 << 0); //rst

	if (!pll->pll_enable)
		return;

	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x1c) &= ~(0x3f << 0);
	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x1c) |=  (pll->pll_div_in << 0);

	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x20) &= ~(0xff << 0);
	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x24) &= ~(0x1f << 0);
	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x20) |=  (((pll->pll_div_fb >> 0) & 0xff) << 0);
	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x24) |=  (((pll->pll_div_fb >> 8) & 0x1f) << 0);

	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x28) &= ~(0x07 << 0);
	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x28) |=  (pll->pll_div_out << 0);

	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x2c) &= ~(0x7f << 0);
	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x2c) |=  (pll->pll_icpsel << 0);

	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x30) &= ~(0x07 << 0);
	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x30) |= ((pll->pll_vco_trim & 0x07) << 0);

	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x30) &= ~(0x03 << 4);
	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x30) |=  (pll->pll_vco_subband << 4);

	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x3c) &= ~(0x01 << 2);
	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x3c) |=  ((pll->pll_bwsel_lpf & 0x01) << 2);

	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x3c) |=  (0x01 << 1); //enable
	*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x3c) |=  (0x01 << 0); //nrst
}

static inline void _clk_set_div(GX_CLOCK_MODULE module, uint16_t div)
{
	GX_CLOCK_MODULE_INFO info;
	if (__module_get_info(module, &info) != 0)
		return;

	GX_CLOCK_DIV *div_table = info.param->div;
	if (!div_table || div_table->addr == 0)
		return;

	if (div == __module_get_div(info))
		return;

	unsigned int offs = div_table->offs;
	unsigned int mask = div_table->mask;

	int i;
	for (i = 0;; i++) {
		if (mask >> i == 0)
			break;
	}
	unsigned int load = offs + i;
	unsigned int rst  = offs + i + 1;

	int8_t clk_offs  = info.param->clk_offs;
	int8_t gate_offs = info.param->gate_high_offs;
	unsigned int clk_addr  = info.clk_sel_addr;
	unsigned int gate = gate_offs < 0 ? 0 : (*(volatile unsigned int*)info.gate_val_addr >> gate_offs) & 0x1;
	if (gate)
		*(volatile unsigned int*)info.gate_clr_addr = (1 << gate_offs);

	unsigned int clk_mask = 0x1;
	unsigned int clk = __reg_get_val(clk_addr, clk_offs, clk_mask);
	if (clk &&
		(module == CLOCK_MODULE_SRAM || module >= CLOCK_MODULE_SCPU))
		__reg_set_val(clk_addr, clk_offs, 0, clk_mask);

	unsigned int div_addr = info.base_addr + div_table->addr;
	*(volatile unsigned int *)div_addr &= ~(1    <<  rst);
	*(volatile unsigned int *)div_addr |=  (1    <<  rst);
	*(volatile unsigned int *)div_addr &= ~(mask << offs);
	*(volatile unsigned int *)div_addr |=  ((div == 0 ? 0 : div-1) << offs);
	*(volatile unsigned int *)div_addr &= ~(1    << load);
	*(volatile unsigned int *)div_addr |=  (1    << load);

	if (clk &&
		(module == CLOCK_MODULE_SRAM || module >= CLOCK_MODULE_SCPU))
		__reg_set_val(clk_addr, clk_offs, 1, clk_mask);

	if (gate)
		*(volatile unsigned int*)info.gate_set_addr = (1 << gate_offs);
}

static inline int _clk_get_div(GX_CLOCK_MODULE module)
{
	GX_CLOCK_MODULE_INFO info;
	if (__module_get_info(module, &info) != 0)
		return -1;

	if (!info.param || !info.param->div)
		return -1;

	return __module_get_div(info);
}

static inline void _clk_set_dto(GX_CLOCK_MODULE module, uint32_t dto, int enable)
{
	GX_CLOCK_MODULE_INFO info;
	if (__module_get_info(module, &info) != 0)
		return;

	GX_CLOCK_DTO *dto_table = info.param->dto;
	if (!dto_table || dto_table->addr == 0)
		return;

	int org_enable = 0;
	if ((dto == __module_get_dto(info, &org_enable)) && (org_enable == enable))
		return;

	unsigned int bypass   = !enable;
	unsigned int clk_addr = info.clk_sel_addr;
	unsigned int dto_addr = info.base_addr + dto_table->addr;

	int8_t clk_offs  = info.param->clk_offs;
	int8_t gate_offs = info.param->gate_high_offs;

	unsigned int gate = gate_offs < 0 ? 0 : (*(volatile unsigned int*)info.gate_val_addr >> gate_offs) & 0x1;
	if (gate)
		*(volatile unsigned int*)info.gate_clr_addr = (1 << gate_offs);

	unsigned int clk_mask = 0x1;
	unsigned int clk = __reg_get_val(clk_addr, clk_offs, clk_mask);
	if (clk)
		__reg_set_val(clk_addr, clk_offs, 0, clk_mask);

	*(volatile unsigned int *)dto_addr = (1 << 26);
	*(volatile unsigned int *)dto_addr = 0;
	*(volatile unsigned int *)dto_addr = (1 << 26);
	*(volatile unsigned int *)dto_addr = (bypass << 27) | (1 << 26) | dto;
	*(volatile unsigned int *)dto_addr = (bypass << 27) | (1 << 26) | (1 << 25) | dto;
	*(volatile unsigned int *)dto_addr = (bypass << 27) | (1 << 26) | (1 << 25) | dto;
	*(volatile unsigned int *)dto_addr = (bypass << 27) | (1 << 26) | dto;

	if (clk)
		__reg_set_val(clk_addr, clk_offs, 1, clk_mask);

	if (gate)
		*(volatile unsigned int*)info.gate_set_addr = (1 << gate_offs);
}

static inline void _clk_adjust_dto(GX_CLOCK_MODULE module, uint32_t dto)
{
	GX_CLOCK_MODULE_INFO info;
	if (__module_get_info(module, &info) != 0)
		return;

	GX_CLOCK_DTO *dto_table = info.param->dto;
	if (!dto_table || dto_table->addr == 0)
		return;

	int org_enable;
	if ((dto == __module_get_dto(info, &org_enable)) || (org_enable == 0))
		return;

	unsigned int dto_addr = info.base_addr + dto_table->addr;

	int value_cfg = (1 << 26) | dto;
	int value_upd = (1 << 26) | (1 << 25) | dto;
	*(volatile unsigned int *)dto_addr = value_cfg;
	*(volatile unsigned int *)dto_addr = value_upd;
	*(volatile unsigned int *)dto_addr = value_cfg;
}

static inline int _clk_get_dto(GX_CLOCK_MODULE module)
{
	GX_CLOCK_MODULE_INFO info;
	if (__module_get_info(module, &info) != 0)
		return -1;

	if (!info.param || !info.param->dto)
		return -1;

	return __module_get_dto(info, NULL);
}

static inline int _clk_set_module(GX_CLOCK_MODULE module, GX_CLOCK_MODULE_SOURCE source)
{
	if (module >= CLOCK_MODULE_MAX)
		return -1;

	unsigned int clk_mask = 0x1;
	if (source == MODULE_SOURCE_32K) {
		switch (module) {
		case CLOCK_MODULE_RTC:
		case CLOCK_MODULE_PMU:
		case CLOCK_MODULE_OSC_REF:
		case CLOCK_MODULE_SRAM:
		case CLOCK_MODULE_AUDIO_IN_SYS:
			break;
		default:
			return -1;
		}
	} else if (source > MODULE_SOURCE_32K) {
		if (module == CLOCK_MODULE_AUDIO_IN_ADC) {
			source = source / 4;// special conversion
		} else if (module == CLOCK_MODULE_AUDIO_IN_PDM) {
			source = source - 5;// special conversion
			clk_mask = 0x3;
		}
		else
			return -1;
	}

	GX_CLOCK_MODULE_INFO info;
	if (__module_get_info(module, &info) != 0)
		return -1;

	int8_t clk_offs       = info.param->clk_offs;
	int8_t gate_high_offs = info.param->gate_high_offs;
	int8_t gate_all_offs  = info.param->gate_all_offs;

	if (clk_offs == -1)
		return -1;

	unsigned int clk = source;
	unsigned int clk_addr = info.clk_sel_addr;
	if ((clk == 2) && (module != CLOCK_MODULE_AUDIO_IN_PDM)) {
		clk      = clk  - 1;
		clk_offs = clk_offs + 1;
	}

	if (clk == __reg_get_val(clk_addr, clk_offs, clk_mask))
		return 0;

	unsigned int gate_all_enable = 0;
	if (module == CLOCK_MODULE_UART0_UART1 ||
			module == CLOCK_MODULE_I2C0_I2C1 ||
			module == CLOCK_MODULE_TIMER_WDT) {

		int i = module;
		unsigned int gate = *(volatile unsigned int*)info.gate_val_addr;
		gate_all_enable |= !((gate >> gx_clock_param_table[++i].gate_all_offs) & 0x1);
		gate_all_enable |= !((gate >> gx_clock_param_table[++i].gate_all_offs) & 0x1);

	} else {
		gate_all_enable = !((*(volatile unsigned int*)info.gate_val_addr >> gate_all_offs) & 0x1);
	}

	if (!gate_all_enable) {

		__reg_set_val(clk_addr, clk_offs, clk, clk_mask);

		return 0;
	}

	if (module == CLOCK_MODULE_AUDIO_IN_PDM || module == CLOCK_MODULE_AUDIO_IN_ADC) {
		__reg_set_val(clk_addr, clk_offs, clk, clk_mask);
		*(volatile unsigned int*)info.gate_set_addr = (1 << gate_high_offs);

		unsigned int gate = *(volatile unsigned int*)info.gate_val_addr;
		unsigned int clk_sel = *(volatile unsigned int*)clk_addr;

		unsigned int clk_sys = (clk_sel >> gx_clock_param_table[CLOCK_MODULE_AUDIO_IN_SYS].clk_offs) & 0x1;
		unsigned int gate_high_sys_enable = !((gate >> gx_clock_param_table[CLOCK_MODULE_AUDIO_IN_SYS].gate_all_offs) & 0x1);

		if (clk_sys == 1 && gate_high_sys_enable == 0) {

			unsigned int clk_pdm = (clk_sel >> gx_clock_param_table[CLOCK_MODULE_AUDIO_IN_PDM].clk_offs) & 0x3;
			unsigned int clk_adc = (clk_sel >> gx_clock_param_table[CLOCK_MODULE_AUDIO_IN_ADC].clk_offs) & 0x1;
			unsigned int gate_all_pdm_enable = !((gate >> gx_clock_param_table[CLOCK_MODULE_AUDIO_IN_PDM].gate_all_offs) & 0x1);
			unsigned int gate_all_adc_enable = !((gate >> gx_clock_param_table[CLOCK_MODULE_AUDIO_IN_ADC].gate_all_offs) & 0x1);
			if ((clk_pdm == 0 && gate_all_pdm_enable == 1) ||
					(clk_adc == 0 && gate_all_adc_enable == 1))
				*(volatile unsigned int*)info.gate_clr_addr = (1 << gate_high_offs);
		}

		return 0;
	}

	if (clk == 1) {
		*(volatile unsigned int*)info.gate_clr_addr = (1 << gate_high_offs);

		__reg_set_val(clk_addr, clk_offs, clk, clk_mask);

	} else {
		__reg_set_val(clk_addr, clk_offs, clk, clk_mask);

		*(volatile unsigned int*)info.gate_set_addr = (1 << gate_high_offs);
	}

	return 0;
}

static inline GX_CLOCK_MODULE_SOURCE _clk_get_module(GX_CLOCK_MODULE module)
{
	GX_CLOCK_MODULE_INFO info;
	if (__module_get_info(module, &info) != 0)
		return -1;

	int offs = info.param->clk_offs;
	if (offs == -1)
		return MODULE_SOURCE_ERROR;

	int offs_w = 0;
	switch (module) {
	case CLOCK_MODULE_RTC:
	case CLOCK_MODULE_PMU:
	case CLOCK_MODULE_OSC_REF:
	case CLOCK_MODULE_SRAM:
		offs_w = offs + 1;
		break;
	default:
		break;
	}

	unsigned int clk_sel  = 0;
	unsigned int value = *(volatile unsigned int*)info.clk_sel_addr;
	if (offs_w) {
		clk_sel = (value >> offs_w) & 0x01;
		clk_sel = (clk_sel == 1) ? 2 : 0;
	}

	if (clk_sel == 0)
		clk_sel = (value >> offs) & 0x01;

	if (module == CLOCK_MODULE_AUDIO_IN_ADC)
		clk_sel = (clk_sel == 0) ? MODULE_SOURCE_ADC_SYS : MODULE_SOURCE_ADC_32K;
	else if (module == CLOCK_MODULE_AUDIO_IN_PDM)
		clk_sel = (clk_sel == 0) ? MODULE_SOURCE_PDM_SYS : MODULE_SOURCE_PDM_OSC_1M;

	return clk_sel;
}

static inline void _clk_set_source(GX_CLOCK_SOURCE_TABLE *source_table)
{
	unsigned int addr = GX_REG_BASE_PMU_CONFIG + GX_CLOCK_PMU_SOURCE_SEL;

	unsigned int offs = source_table->source;
	unsigned int clk  = source_table->clk;

	__reg_set_val(addr, offs, clk, 0x1);
}

static inline unsigned int _clk_get_source(GX_CLOCK_SOURCE source)
{
	unsigned int offs = source;
	unsigned int addr = GX_REG_BASE_PMU_CONFIG + GX_CLOCK_PMU_SOURCE_SEL;

	unsigned int clk = (*(volatile unsigned int*)addr >> offs) & 0x01;

	return clk;
}

static inline int _clk_set_all_gate(GX_CLOCK_MODULE module, unsigned int enable)
{
	GX_CLOCK_MODULE_INFO info;
	if (__module_get_info(module, &info) != 0)
		return -1;

	unsigned int gate = (1 << info.param->gate_all_offs);
	switch (module) {
	case CLOCK_MODULE_SRAM:
		gate |= 1 << 15;
		break;
	case CLOCK_MODULE_UART0:
		gate |= 1 << 16;
		break;
	case CLOCK_MODULE_UART1:
		gate |= 1 << 17;
		break;
	case CLOCK_MODULE_AUDIO_PLAY:
		gate |= 1 << 3;
		break;
	case CLOCK_MODULE_I2C0:
		gate |= 1 << 18;
		break;
	case CLOCK_MODULE_I2C1:
		gate |= 1 << 19;
		break;
	case CLOCK_MODULE_FLASH_SPI:
		gate |= 1 << 13;
		break;
	case CLOCK_MODULE_GENERAL_SPI:
		gate |= 1 << 15;
		break;
	case CLOCK_MODULE_TIMER:
		gate |= 1 << 20;
		break;
	case CLOCK_MODULE_WDT:
		gate |= 1 << 21;
		break;
	default:
		break;
	}

	if ((module == CLOCK_MODULE_HW_I2C) || (module == CLOCK_MODULE_PMU))
		return -1;

	if (!gate)
		return -1;

	if (enable)
		*(volatile unsigned int*)info.gate_clr_addr = gate;
	else
		*(volatile unsigned int*)info.gate_set_addr = gate;

	return 0;
}

static inline int _clk_get_all_gate(GX_CLOCK_MODULE module)
{
	GX_CLOCK_MODULE_INFO info;
	if (__module_get_info(module, &info) != 0)
		return -1;

	int8_t offs = info.param->gate_all_offs;
	if (!offs)
		return -1;

	switch (module) {
	case CLOCK_MODULE_UART0_UART1:
	case CLOCK_MODULE_I2C0_I2C1:
	case CLOCK_MODULE_TIMER_WDT:
		{
			unsigned int enable = 0;
			unsigned int value = *(volatile unsigned int*)info.gate_val_addr;

			int i = module;
			enable |= !((value >> gx_clock_param_table[++i].gate_all_offs) & 0x1);
			enable |= !((value >> gx_clock_param_table[++i].gate_all_offs) & 0x1);

			return enable;
		}
	default:
		break;
	}

	unsigned int enable = !((*(volatile int*)info.gate_val_addr >> offs) & 0x1);

	return enable;
}

static inline void _clk_set_high_gate(GX_CLOCK_MODULE module, unsigned int enable)
{
	switch (module) {
	case CLOCK_MODULE_AUDIO_IN_ADC:
	case CLOCK_MODULE_AUDIO_IN_PDM:
		module = CLOCK_MODULE_AUDIO_IN_SYS;
		break;
	case CLOCK_MODULE_UART0:
	case CLOCK_MODULE_UART1:
		module = CLOCK_MODULE_UART0_UART1;
		break;
	case CLOCK_MODULE_I2C0:
	case CLOCK_MODULE_I2C1:
		module = CLOCK_MODULE_I2C0_I2C1;
		break;
	case CLOCK_MODULE_TIMER:
	case CLOCK_MODULE_WDT:
		module = CLOCK_MODULE_TIMER_WDT;
		break;
	default:
		break;
	}

	GX_CLOCK_MODULE_INFO info;
	if (__module_get_info(module, &info) != 0)
		return;

	unsigned int clk_offs = info.param->clk_offs;
	if (clk_offs == -1)
		return;

	unsigned int clk_offs_w = 0;
	switch (module) {
	case CLOCK_MODULE_RTC:
	case CLOCK_MODULE_PMU:
	case CLOCK_MODULE_OSC_REF:
	case CLOCK_MODULE_SRAM:
		clk_offs_w = clk_offs + 1;
		break;
	default:
		break;
	}

	unsigned int clk_sel = 0;
	unsigned int source_sel = *(volatile unsigned int*)info.clk_sel_addr;
	if (clk_offs_w != 0) {
		clk_sel = (source_sel >> clk_offs_w) & 0x01;
		clk_sel = (clk_sel == 1) ? 2 : 0;
	}

	if (clk_sel == 0)
		clk_sel = (source_sel >> clk_offs) & 0x01;

	if (clk_sel == 1 && enable)
		*(volatile unsigned int*)info.gate_clr_addr = 1 << info.param->gate_high_offs;
	else
		*(volatile unsigned int*)info.gate_set_addr = 1 << info.param->gate_high_offs;

}

static inline unsigned int _clk_get_module_frequence(GX_CLOCK_MODULE module)
{
	switch (module) {
	case CLOCK_MODULE_I2C0:
	case CLOCK_MODULE_I2C1:
		module = CLOCK_MODULE_I2C0_I2C1;
		break;
	case CLOCK_MODULE_UART0:
	case CLOCK_MODULE_UART1:
		module = CLOCK_MODULE_UART0_UART1;
		break;
	case CLOCK_MODULE_TIMER:
	case CLOCK_MODULE_WDT:
		module = CLOCK_MODULE_TIMER_WDT;
		break;
	case CLOCK_MODULE_DMA:
		module = CLOCK_MODULE_SCPU;
		break;
	case CLOCK_MODULE_AUDIO_IN_ADC:
	case CLOCK_MODULE_AUDIO_IN_PDM:
		return 0;
	default:
		break;
	}

	GX_CLOCK_MODULE_INFO info;
	if (__module_get_info(module, &info) != 0)
		return 0;

	int offs = info.param->clk_offs;
	if (offs == -1)
		return 0;

	int offs_w = 0;
	switch (module) {
	case CLOCK_MODULE_RTC:
	case CLOCK_MODULE_PMU:
	case CLOCK_MODULE_OSC_REF:
	case CLOCK_MODULE_SRAM:
		offs_w = offs + 1;
		break;
	default:
		break;
	}
	unsigned int source = *(volatile unsigned int*)(GX_REG_BASE_PMU_CONFIG + GX_CLOCK_PMU_SOURCE_SEL);

	unsigned int fre = 0;
	unsigned int clk_mask = 0x1;
	if (offs_w)
		fre = (__reg_get_val(info.clk_sel_addr, offs_w, clk_mask) == 1) ? 32000 : 0;

	if (fre == 0) {
		if (__reg_get_val(info.clk_sel_addr, offs, clk_mask) == 0)
			fre = (((source >> CLOCK_SOURCE_1M_12M)  & 0x01) == 0) ? 12288000 :  1024000;
		else if (__reg_get_val(info.clk_sel_addr, offs, clk_mask) == 1)
			fre = (((source >> CLOCK_SOURCE_24M_PLL) & 0x01) == 0) ? 24576000 : 49152000;
	}

	if ((fre == 12288000) || (fre == 1024000)) {
		if (module >= CLOCK_MODULE_SCPU ||
				module == CLOCK_MODULE_SRAM ||
				module == CLOCK_MODULE_OSC_REF ||
				module == CLOCK_MODULE_AUDIO_IN_SYS)
			return fre;

	} else if ((fre == 24576000) || (fre == 49152000)) {
		if (fre == 49152000) {
			unsigned int fvco;
			unsigned int fin;
			unsigned int div_in      = ((*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x1c)) & 0x3f) + 1;
			unsigned int div_fb      = ((*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x20)) | ((*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x24) & 0x1f) << 8)) + 1;
			unsigned int div_out     = ((*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x28) & 0x7) + 1) * 2;
			unsigned int vco_subband = (*(volatile unsigned int *)(GX_REG_BASE_HW_I2C + 0x30) >> 4) & 0x3;
			switch (vco_subband) {
			case 0:
				fvco = 61440000;
				break;
			case 1:
				fvco = 73728000;
				break;
			case 2:
				fvco = 86016000;
				break;
			case 3:
				fvco = 98304000;
				break;
			default:
				return -1;
			}

			fin = fvco * div_in / div_fb;
			if (fin >= 16000 && fin < 48000)
				fin = 32000;
			else if (fin >= 512000 && fin < 1536000)
				fin = 1024000;
			else if (fin >= 153600 && fin < 3072000)
				fin = 2048000;
			else
				return -1;

			fre = fin / div_in * div_fb / div_out;
		}

		int dto_enable = 0;
		unsigned int dto = __module_get_dto(info, &dto_enable);
		fre = (dto_enable == 0) ? fre : (unsigned int)(dto*(uint64_t)fre/0x2000000);
	}

	unsigned int div = __module_get_div(info);
	fre = (div == 0) ? fre : fre/div;

	return fre;
}
#endif
