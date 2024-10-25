#ifndef __PM_H__
#define __PM_H__

#define CPU_HALT_RESET      (0xa030a260)
#define CPU_HALT_PM         (0xa030a150)
#define CPU_HALT_ISOLATE    (0xa030a264)

enum reset_mode {
	RESET_REBOOT,
	RESET_POWEROFF,
	RESET_SUSPEND,
	RESET_RESUME,
};

void ddr_reset(void);
void cpu_reset(void);
void cpu_suspend(void);
void cpu_resume(void);
void cpu_reboot(void);

struct bq25890_config {
    u32 ichg;   /* charge current       */
    u32 vreg;   /* regulation voltage       */
    u32 iterm;  /* termination current      */
    u32 iprechg;    /* precharge current        */
    u32 sysvmin;    /* minimum system voltage limit */
    u32 boostv; /* boost regulation voltage */
    u32 boosti; /* boost current limit      */
    u32 boostf; /* boost frequency      */
    u32 ilim_en;    /* enable ILIM pin      */
    u32 treg;   /* thermal regulation threshold */
    u32 enable;
};

struct bq25890_info{
	unsigned int chrg_status;
	unsigned int online;
	unsigned int enable;
	unsigned int bat_fault;
	unsigned int temp;
	unsigned int Vbus;
	unsigned int chrg_I;
	unsigned int chrg_Imax;
	unsigned int chrg_V;
	unsigned int chrg_Vmax;
	unsigned int chrg_Iterm;
};

struct cw2015_info{
	unsigned int voltage;
	unsigned int capacity;
	unsigned int time;
};

extern void gx_pm_init(void);

int bq25890_init(int i2c_bus, struct bq25890_config *config);
int bq25890_get_property(struct bq25890_info *info);

int cw2015_init(int bus_id);
int cw2015_get_property(struct cw2015_info *info);

int rt5037_buck_set(unsigned char buck_num, unsigned int voltage);
int rt5037_ldo_set(unsigned char ldo_num, unsigned int voltage);
int rt5037_init(int bus_id);

#endif
