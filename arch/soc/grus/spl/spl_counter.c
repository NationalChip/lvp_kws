#include <autoconf.h>
#include <base_addr.h>
#include <board_config.h>
#include <common.h>
#include <spl/spl.h>
#include <base_addr.h>
#include <misc_regs.h>
#include <soc.h>
#include <driver/gx_clock.h>
#include <div64.h>

/* counter */
#define GXSCPU_VA_COUNTER_2_STATUS  (GX_REG_BASE_COUNTER + 0x00)
#define GXSCPU_VA_COUNTER_2_VALUE   (GX_REG_BASE_COUNTER + 0x04)
#define GXSCPU_VA_COUNTER_2_ACCSNAP (GX_REG_BASE_COUNTER + 0x08)
#define GXSCPU_VA_COUNTER_2_CONTROL (GX_REG_BASE_COUNTER + 0x10)
#define GXSCPU_VA_COUNTER_2_CONFIG  (GX_REG_BASE_COUNTER + 0x20)
#define GXSCPU_VA_COUNTER_2_PRE     (GX_REG_BASE_COUNTER + 0x24)
#define GXSCPU_VA_COUNTER_2_INI     (GX_REG_BASE_COUNTER + 0x28)
#define GXSCPU_VA_COUNTER_2_ACC     (GX_REG_BASE_COUNTER + 0x30)

#define GXSCPU_VA_COUNTER_PSYSCLK   spl_clk_get_frequence(CLOCK_MODULE_TIMER)
#define GXSCPU_VA_COUNTER_2_RATE    (1000000)
#define GXSCPU_VA_COUNTER_CLK(val)  ((GXSCPU_VA_COUNTER_PSYSCLK/(val))-1)

#define CTR_INIT_VALUE  (0 - 1000)
#define MAX_REG_NUM     10

extern int spl_clk_get_frequence(GX_CLOCK_MODULE module);
static unsigned long long gx_get_time_us(void)
{
	unsigned int timebase_l = readl(GXSCPU_VA_COUNTER_2_VALUE);
	unsigned long long timebase_h = readl(GXSCPU_VA_COUNTER_2_ACCSNAP);
	return timebase_l|(timebase_h << 32);
}

unsigned int spl_get_time_ms(void)
{
	unsigned long long time = gx_get_time_us();
	do_div(time, 1000);

	return (unsigned int)time;
}

void spl_udelay(unsigned int usec)
{
	unsigned long long tmp;
	tmp = gx_get_time_us() + usec;/* get current timestamp */
	while (gx_get_time_us() < tmp+1);
}

void spl_mdelay(unsigned int msec)
{
	while (msec--)
		spl_udelay(1000);
}

extern void spl_clk_set_gate_enable(GX_CLOCK_MODULE module, unsigned int enable);
void spl_delay_init(void)
{
	/* csi_vic_enable_irq(CORET_IRQn); */
	spl_clk_set_gate_enable(CLOCK_MODULE_TIMER, 1);
	writel(0x1, GXSCPU_VA_COUNTER_2_CONTROL);
	writel(0x0, GXSCPU_VA_COUNTER_2_CONTROL);
	writel(0x1, GXSCPU_VA_COUNTER_2_CONFIG);
	writel(GXSCPU_VA_COUNTER_CLK(GXSCPU_VA_COUNTER_2_RATE), GXSCPU_VA_COUNTER_2_PRE); //1us 1/pre_clk
	writel(0, GXSCPU_VA_COUNTER_2_INI);
	writel(0x2, GXSCPU_VA_COUNTER_2_CONTROL); //begin count
}
