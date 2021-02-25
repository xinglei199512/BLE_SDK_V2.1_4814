#ifndef SWINT_H_
#define SWINT_H_
#include "apollo_00.h"

#define SWINT_SYS_INT_CLR() NVIC_ClearPendingIRQ(SFT_IRQn)
#define SWINT_SYS_INT_EN() NVIC_EnableIRQ(SFT_IRQn)

void swint_req(void (*cb)());

#endif
