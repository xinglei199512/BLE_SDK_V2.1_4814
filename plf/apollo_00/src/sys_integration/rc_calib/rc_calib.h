#ifndef RC_CALIB_H_
#define RC_CALIB_H_
#include <stdint.h>
#include <stdbool.h>
#include "apollo_00.h"

void rc_calib_start(void);
void rc_calib_end(bool wait);
uint32_t get_rc32k_calib_val(void);



#endif
