#ifndef _PADMUX_H_
#define _PADMUX_H_

struct pin_config {
	unsigned char pin_id;
	unsigned char function;
};

int padmux_set(int pad_id, int function);
int padmux_get(int pad_id);
int padmux_check(int pad_id, int function);
int padmux_init(const struct pin_config *pin_table, int size);

#endif
