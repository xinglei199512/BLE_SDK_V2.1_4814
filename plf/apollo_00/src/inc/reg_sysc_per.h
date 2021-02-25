#ifndef _SYSC_PER_MAIN_H__
#define _SYSC_PER_MAIN_H__
#include "bx_config.h"

#if HW_BX_VERSION == 00
	#include "reg_sysc_per_apollo_00.h"
#elif HW_BX_VERSION == 01
	#include "reg_sysc_per_apollo_01.h"
#else
	#error ERROR: ERROR to define HW_BX_VERSION
#endif

#endif
