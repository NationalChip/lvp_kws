#ifndef RF_CODE_H
#define RF_CODE_H

#include "lvp_app_common.h"

void getUniqueId(void);
int rf_send_code(unsigned int code);
void rf_send_code_times(unsigned int code, int timers);

#endif